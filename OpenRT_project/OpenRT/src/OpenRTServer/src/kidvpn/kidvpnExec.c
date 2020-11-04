/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: kidVpnExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: VPN 操作
** 2018.08.16  根据新版本 KidVPN 修改代码。
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stddef.h>
#include <pthread.h>
#include <spawn.h>
#include <time.h>
#include <wait.h>
#include <sys/select.h>
#include "common.h"
#include "kidvpnDB.h"
#include "vnd/vnd_common.h"
#include "kidvpn_common.h"
#include "fileparser/fileParser.h"
#include "situationMode/situation_common.h"
#include "log/log_common.h"
#include <memory.h>
#include <sys/time.h>
#include "ipdb.h"
/********************************************************************************************************
  获取 KidVPN IP地址库信息结构
*********************************************************************************************************/
struct vpn_ipip{
    PUCHAR  pucData;                                                    /*  数据缓冲区                  */
    PUCHAR  pucIndex;                                                   /*  索引指针                    */
    UINT   *puiFlag;                                                    /*  标志                        */
    UINT    uiOffset;                                                   /*  偏移                        */
};
typedef struct vpn_ipip   __VPN_IPIP;
typedef struct vpn_ipip  *__PVPN_IPIP;
/********************************************************************************************************
  获取 KidVPN IP 位置信息结构
*********************************************************************************************************/
struct pos_info {
    CHAR  cCountry[32];                                                 /*  国家                        */
    CHAR  cProvice[32];                                                 /*  省份                        */
    CHAR  cCity[32];                                                    /*  城市                        */
    CHAR  cRegion[16];                                                  /*  地区                        */
    CHAR  cOperat[16];                                                  /*  运营商                      */
};
typedef struct pos_info   __VPN_POSITION;
typedef struct pos_info  *__PVPN_POSITION;
/*********************************************************************************************************
  通过 IP 地址获取地理位置的配置宏
*********************************************************************************************************/
#define B2IL(b)                (((b)[0] & 0xFF) |            \
                               (((b)[1] << 8) & 0xFF00) |    \
                               (((b)[2] << 16) & 0xFF0000) | \
                               (((b)[3] << 24) & 0xFF000000))           /*  内存数据高低位不变          */

#define B2IU(b)                (((b)[3] & 0xFF) |            \
                               (((b)[2] << 8) & 0xFF00) |    \
                               (((b)[1] << 16) & 0xFF0000) | \
                               (((b)[0] << 24) & 0xFF000000))           /*  内存数据高低位转换          */

#define COUNTRY_INDEX          (0)                                      /*  国家                        */
#define PROVINCE_INDEX         (1)                                      /*  省份                        */
#define CITY_INDEX             (2)                                      /*  城市                        */
#define REGION_INDEX           (3)                                      /*  地区                        */
#define OPERAT_INDEX           (4)                                      /*  运营商                      */
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static LW_HANDLE               _G_hKidVPNMutex;                         /*  操作该数据结构互斥信号量    */
static __VPN_IPIP              _G_ipPos;                                /*  IP 地址库文件相关信息       */
/*********************************************************************************************************
  链表头全局变量
*********************************************************************************************************/
static LW_LIST_LINE_HEADER     _G_plineKidVPNHeader = LW_NULL;          /*  KidVPN 链表头               */
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define VPN_ENABLE             "1"                                      /*  vpn 客户端连接              */
#define VPN_DISABLE            "0"                                      /*  vpn 客户端连接断开          */
#define ETH_DATA_LEN           (1500)                                   /*  最大 MTU                    */
#define KV_VND_MIN_MTU         (1280)                                   /*  kidvpn 最小MTU              */
#define KV_VND_MAX_MTU         (ETH_DATA_LEN - 28)                      /*  kidvpn 最大MTU              */
#define KV_VND_DEF_MTU         (KV_VND_MAX_MTU - 8)                     /*  kidvpn 默认MTU              */

#define BUF_SIZE               (1024)                                   /*  缓冲区大小                  */
#define PORT                   (80)                                     /*  端口号                      */
#define CURL_STRING            "www.3322.org/dyndns/getip"              /*  CURL 语句                   */
/*********************************************************************************************************
** 函数名称: __fdNodeGetByVndId
** 功能描述: 根据 VND ID 查找文件句柄节点
** 输  入  : iVndId      VND ID
** 输  出  : NONE
** 返  回  : 文件句柄节点
*********************************************************************************************************/
static __PFD_NODE  __fdNodeGetByVndId (INT  iVndId)
{
    PLW_LIST_LINE  plineTemp = LW_NULL;
    __PFD_NODE     pfdNode   = LW_NULL;

    for (plineTemp  = _G_plineKidVPNHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  遍历文件句柄链表            */

        pfdNode = _LIST_ENTRY(plineTemp, __FD_NODE, fdListLine);
        if (iVndId == pfdNode->tNodeData.iVndId) {
            break;                                                      /*  找到                        */
        }
    }

    if (plineTemp) {
        return  (pfdNode);
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** 函数名称: __fdNodeAdd
** 功能描述: 插入一个新的节点（后插法）
** 输  入  : pFdInfo        文件句柄信息指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __fdNodeAdd (__PFD_HANDLE  pFdInfo)
{
    __PFD_NODE  pNode = NULL;

    if (NULL == pFdInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "invalid arguments.\n");

        return  (PX_ERROR);
    }

    pNode = (__PFD_NODE)malloc(sizeof(__FD_NODE));
    if (NULL == pNode) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc Memory Failed.\n");

        return  (PX_ERROR);
    }

    bzero(pNode, sizeof(__FD_NODE));

    memcpy(&pNode->tNodeData, pFdInfo, sizeof(__FD_HANDLE));

    /*
     *  插入头节点后
     */
    _List_Line_Add_Tail(&pNode->fdListLine, &_G_plineKidVPNHeader);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __fdNodeDel
** 功能描述: 根据VND ID 删除节点
** 输  入  : iVndId          VND ID
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __fdNodeDel (INT  iVndId)
{
    __PFD_NODE  pPoint = NULL;

    pPoint = __fdNodeGetByVndId(iVndId);
    if (NULL != pPoint) {
        _List_Line_Del(&pPoint->fdListLine, &_G_plineKidVPNHeader);

        free(pPoint);
    }
}

/*********************************************************************************************************
** 函数名称: getPublicIp
** 功能描述: 获取本机公网 ip
** 输　入  : pcIpBuf     存放 ip 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  getPublicIp (PCHAR  pcIpBuf)
{
    struct sockaddr_in  sockIn;
    struct hostent     *pHostEnt;
    PCHAR               pTmp;
    PCHAR               pCur;
    INT                 isockFd = 0;
    INT                 iLen    = 0;

    CHAR                cBuf[BUF_SIZE] = {0};
    CHAR                cUrl[100]      = {0};
    CHAR                cHost[100]     = {0};
    CHAR                cGet[100]      = {0};
    CHAR                cHeader[240]   = {0};
    PCHAR               pHost          = NULL;
    INT                 iPos;

    /*
     *  从 url 中获取主机名和相关的地址信息
     */
    strcpy(cUrl, CURL_STRING);

    for (pHost = cUrl; *pHost != '/' && *pHost != '\0'; ++pHost) {
        ;
    }

    if ((INT)(pHost - cUrl) == strlen(cUrl)) {
        strcpy(cGet, "/");
    } else {
        strcpy(cGet, pHost);
    }

    *pHost = '\0';
    strcpy(cHost, cUrl);

    /*
     *  设置 socket 参数
     */
    if (0 == (pHostEnt = lwip_gethostbyname(cHost))) {
        perror("error get host.\n");

        return  (PX_ERROR);
    }

    bzero(&sockIn, sizeof(sockIn));
    sockIn.sin_family = AF_INET;
    sockIn.sin_addr.s_addr = htonl(INADDR_ANY);
    sockIn.sin_addr.s_addr = ((struct in_addr *)(pHostEnt->h_addr))->s_addr;
    sockIn.sin_port = htons(PORT);

    if ((isockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error opening socket.\n");

        return  (PX_ERROR);
    }

    /*
     *  合成请求信息，用于发送到 web 服务器
     */
    strcat(cHeader, "GET");
    strcat(cHeader, " ");
    strcat(cHeader, cGet);
    strcat(cHeader, " ");
    strcat(cHeader, "HTTP/1.1\r\n");
    strcat(cHeader, "HOST:");
    strcat(cHeader, cHost);
    strcat(cHeader, "\r\n");
    strcat(cHeader, "ACCEPT:*/*");
    strcat(cHeader, "\r\nConnection: close\r\n\r\n\r\n");

    /*
     *  连接 web 服务器，发送头数据，并接收 web 返回的数据
     */
    if (connect(isockFd, (void *)&sockIn, sizeof(sockIn)) == -1) {
        perror("error connect to socket.\n");

        return  (PX_ERROR);
    }

    if (send(isockFd, cHeader, strlen(cHeader), 0) == -1) {
        perror("error in send.\n");

        return  (PX_ERROR);
    }

    iLen = recv(isockFd, cBuf, BUF_SIZE, 0);
    if (iLen < 0) {
        perror("receive data error.\n");

        return  (PX_ERROR);
    }

    if ('\0' == cBuf[0]) {
        return  (PX_ERROR);
    }

    /*
     *  提取公网 ip 信息
     */
    pTmp = lib_strstr(cBuf, "Connection");
    if (NULL == pTmp) {
        return  (PX_ERROR);
    }

    pTmp = lib_index(pTmp, '.');
    if (NULL == pTmp) {
        return  (PX_ERROR);
    }

    pTmp = pTmp - 3;
    iPos = 3;
    while (iPos) {
        if ((*pTmp <= '9') && (*pTmp >= '0')) {
            break;
        }

        pTmp++;
        iPos--;
    }

    if (0 == iPos) {
        return  (PX_ERROR);
    }

    pCur = lib_index(pTmp, '\n');
    if (NULL == pCur) {
        return  (PX_ERROR);
    }

    lib_memcpy(pcIpBuf, pTmp, pCur - pTmp);

    close(isockFd);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: getPositionByIpAddr
** 功能描述: 根据 ip 地址获取地理位置
** 输　入  : cpcIp           IP 信息
** 输　出  : pcResult        保存地理位置信息
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  getPositionByIpAddr (CPCHAR  cpcIp, PCHAR  pcResult)
{
    ipdb_reader *reader = LW_NULL;
    CHAR         cCountryName[20] = {0};
    CHAR         cRegionName[20]  = {0};
    CHAR         cCityName[20]    = {0};

    int err = ipdb_reader_new("/etc/OpenRT/ipipfree.ipdb", &reader);
    if (!err) {
        char body[512] = {0};
        err = ipdb_reader_find(reader, cpcIp, "CN", body);
        if (!err) {
            /*
             *  注意:这里 ip 数据库使用的是免费版本，只可以查三个数据，因此格式目前就是：%s    %s    %s
             *       后期如果更新数据库，那么这里也要对面的替换
             */
            sscanf(body,
                   "%s    %s    %s",
                   cCountryName, cRegionName, cCityName);

#if 0
            sprintf(pcResult,
                    "[%s, %s, %s]",
                    cCountryName, cRegionName, cCityName);
#endif

            /*
             *  局域网 cCityName 为空
             */
            if (cCityName[0] == 0) {
                strncpy(cCityName, cRegionName, sizeof(cCityName));
            }
            /*
             *  这里前端只需要城市
             */
            strncpy(pcResult, cCityName, sizeof(cCityName));
        }

        ipdb_reader_free(&reader);
    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __kidvpnFdInfoSetToFdSet
** 功能描述: 从文件描述符链表中获取文件句柄信息并设置到 fd_set
** 输　入  : pFdSet    fd_set 文件集
** 输　出  : NONE
** 返　回  : 文件句柄
*********************************************************************************************************/
static INT  __kidvpnFdInfoSetToFdSet(fd_set  *pFdSet)
{
    INT            iFd;
    INT            iFdMax    = -1;
    PLW_LIST_LINE  plineTemp = LW_NULL;
    __PFD_NODE     pFdNode   = LW_NULL;

    /*
     *  获取管道文件句柄并设置到 fd_set 集合中
     */
    for (plineTemp  = _G_plineKidVPNHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  遍历适配器表                */

        pFdNode = _LIST_ENTRY(plineTemp, __FD_NODE, fdListLine);

        iFd = pFdNode->tNodeData.iPipeFd[0];

        if (iFd >= 0) {
            FD_SET(iFd, pFdSet);
        }

        iFdMax = (iFdMax > iFd) ? iFdMax:iFd;
    }

    return  (iFdMax);
}
/*********************************************************************************************************
** 函数名称: __kidvpnClientIpMacSave
** 功能描述: kidvpn 子进程信息写入数据库
** 输　入  : cpcBuf       kidvpn 子进程信息
**           iLen         信息长度
**           iVndId       vnd id
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnClientIpMacSave(CPCHAR  cpcBuf, INT  iLen, INT  iVndId)
{
    CHAR           cClientIp[32]  = {0};
    CHAR           cClientMac[32] = {0};
    PCHAR          pcBuf;
    PCHAR          pcTmp;
    PCHAR          pcCur;
    __KIDVPN_LINK  kidVpnLinkInfo;

    bzero(&kidVpnLinkInfo, sizeof(kidVpnLinkInfo));

    pcBuf = lib_strstr(cpcBuf, "Client add");
    if (NULL == pcBuf) {
        return  (PX_ERROR);
    }

    pcBuf += strlen("Client add");

    /*
     *  获取客户端 ip
     */
    pcTmp  = lib_index(pcBuf, ':');
    pcCur  = lib_index(pcBuf, '[');
    pcTmp += 2;
    pcCur -= 1;

    lib_memcpy(cClientIp, pcTmp, pcCur - pcTmp);

    /*
     *  获取客户端 MAC
     */
    pcTmp  = lib_index(pcBuf, '[');
    pcCur  = lib_index(pcBuf, ']');
    pcTmp += 1;

    lib_memcpy(cClientMac, pcTmp, pcCur - pcTmp);

    kidVpnLinkInfo.iVndId = iVndId;
    strncpy(kidVpnLinkInfo.cIp,  cClientIp,  sizeof(kidVpnLinkInfo.cIp));
    strncpy(kidVpnLinkInfo.cMac, cClientMac, sizeof(kidVpnLinkInfo.cMac));

    kidvpnLinkAddItemToDb(&kidVpnLinkInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidvpnClientIpMacDiscard
** 功能描述: kidvpn 子进程连接信息从数据库删除
** 输　入  : cpcBuf       kidvpn 子进程信息
**           iLen         信息长度
**           iVndId       vnd id
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnClientIpMacDiscard(CPCHAR  cpcBuf, INT  iLen, INT  iVndId)
{
    CHAR           cClientIp[32]  = {0};
    CHAR           cClientMac[32] = {0};
    PCHAR          pcBuf;
    PCHAR          pcTmp;
    PCHAR          pcCur;
    __KIDVPN_LINK  kidVpnLinkInfo;

    bzero(&kidVpnLinkInfo, sizeof(kidVpnLinkInfo));

    pcBuf = lib_strstr(cpcBuf, "Client lost");
    if (NULL == pcBuf) {
        return  (PX_ERROR);
    }

    pcBuf += strlen("Client lost");

    /*
     *  获取客户端 ip
     */
    pcTmp  = lib_index(pcBuf, ':');
    pcCur  = lib_index(pcBuf, '[');
    pcTmp += 2;
    pcCur -= 1;

    lib_memcpy(cClientIp, pcTmp, pcCur - pcTmp);

    /*
     *  获取客户端 MAC
     */
    pcTmp  = lib_index(pcBuf, '[');
    pcCur  = lib_index(pcBuf, ']');
    pcTmp += 1;

    lib_memcpy(cClientMac, pcTmp, pcCur - pcTmp);

    kidVpnLinkInfo.iVndId = iVndId;
    strncpy(kidVpnLinkInfo.cIp,  cClientIp,  sizeof(kidVpnLinkInfo.cIp));
    strncpy(kidVpnLinkInfo.cMac, cClientMac, sizeof(kidVpnLinkInfo.cMac));

    kidvpnLinkDelDBItem(&kidVpnLinkInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidvpnMsgDoHandle
** 功能描述: kidvpn 子进程信息处理
** 输　入  : cpcBuf       kidvpn 子进程信息
**           iLen         信息长度
**           iVndId       vnd id
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnMsgDoHandle(CPCHAR  cpcBuf, INT  iLen, INT  iVndId)
{
    if (lib_strstr(cpcBuf, "Server connected")) {
        kidvpnUpdateVpnStatToDb(iVndId, VPN_ENABLE);
    } else if (lib_strstr(cpcBuf, "Try connect server")) {
        kidvpnUpdateVpnStatToDb(iVndId, VPN_DISABLE);
    } else if (lib_strstr(cpcBuf, "Client add")) {
        __kidvpnClientIpMacSave(cpcBuf, iLen, iVndId);
    } else if (lib_strstr(cpcBuf, "Client lost")) {
        __kidvpnClientIpMacDiscard(cpcBuf, iLen, iVndId);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidvpnChildMsgParseHandle
** 功能描述: kidvpn 子进程信息解析及处理
** 输　入  : cpcBuf       kidvpn 子进程信息
**           iLen         信息长度
**           iVndId       vnd id
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnChildMsgParseHandle(CPCHAR  cpcBuf, INT  iLen, INT  iVndId)
{
    CHAR          cTmpBuf[128] = {0};
    PCHAR         pcCur        = NULL;
    PCHAR         pcTmp        = NULL;

    pcCur = (PCHAR)cpcBuf;

    if (!strstr(cpcBuf, "[KidVPN]")) {
        return  (ERROR_NONE);
    }

    pcTmp = lib_index(cpcBuf, '\n');
    if (NULL == pcTmp) {
        __kidvpnMsgDoHandle(cpcBuf, iLen, iVndId);                      /*  处理 kidvpn 信息            */
        vpnLogInsert(iVndId, (PCHAR)cpcBuf);                            /*  记录日志                    */
    }

    while (1) {
        lib_memcpy(cTmpBuf, pcCur, pcTmp-pcCur);

        __kidvpnMsgDoHandle(cTmpBuf, strlen(cTmpBuf), iVndId);          /*  处理 kidvpn 信息            */
        vpnLogInsert(iVndId, cTmpBuf);                                  /*  记录日志                    */

        if (pcTmp == &cpcBuf[iLen-1]) {
            break;
        }

        pcCur = pcTmp + 1;
        pcTmp = lib_index(pcCur, '\n');

        bzero(cTmpBuf, sizeof(cTmpBuf));
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidvpnMessageGetHandle
** 功能描述: kidvpn 信息获取线程
** 输　入  : pvArg          参数指针
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static PVOID  __kidvpnMessageGetHandle (PVOID  pvArg)
{
    INT                iRet;
    INT                iFd;
    INT                iFdMax;
    INT                iSize;
    CHAR               cBuf[1024] = {0};
    struct timeval     timeout;
    fd_set             fdset;
    PLW_LIST_LINE      plineTemp = LW_NULL;
    __PFD_NODE         pFdNode   = LW_NULL;

    timeout.tv_sec  = 2;
    timeout.tv_usec = 0;

    for (;;) {
        FD_ZERO(&fdset);

        Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);

        iFdMax = __kidvpnFdInfoSetToFdSet(&fdset);

        /*
         *  多路复用，等待可读文件集
         */
        iRet = select(iFdMax + 1, &fdset, NULL, NULL, &timeout);
        if (iRet > 0) {
            for (plineTemp  = _G_plineKidVPNHeader;
                 plineTemp != LW_NULL;
                 plineTemp  = plineTemp->LINE_plistNext) {                      /*  遍历适配器表                */

                pFdNode = _LIST_ENTRY(plineTemp, __FD_NODE, fdListLine);

                iFd = pFdNode->tNodeData.iPipeFd[0];
                if (iFd > 0) {
                    if (FD_ISSET(iFd, &fdset)) {
                        bzero(cBuf, sizeof(cBuf));

                        iSize = read(iFd, cBuf, sizeof(cBuf));
                        if (iSize > 0) {

                            /*
                             * 获取到数据，进行解析处理
                             */
                            __kidvpnChildMsgParseHandle(cBuf, iSize, pFdNode->tNodeData.iVndId);
                        }
                    }
                }
            }
        }

        Lw_SemaphoreM_Post(_G_hKidVPNMutex);
    }

    return   (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: __kidvpnMsgThreadInit
** 功能描述: kidvpn 信息处理线程初始化
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnMsgThreadInit (VOID)
{
    LW_HANDLE             tid;
    LW_CLASS_THREADATTR   threadAttr;

    /*
     *  创建 kidvpn 日志处理子线程
     */
    Lw_ThreadAttr_Build(&threadAttr,
                        4 * LW_CFG_THREAD_DEFAULT_STK_SIZE,
                        LW_PRIO_NORMAL,
                        LW_OPTION_THREAD_STK_CHK,
                        NULL);

    tid = Lw_Thread_Create("t_kidvpnLog",
                           __kidvpnMessageGetHandle,
                           &threadAttr,
                           LW_NULL);
    if (LW_OBJECT_HANDLE_INVALID == tid) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Thread Create Failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: kidVpnAesKeyDel
** 功能描述: 删除本地 AES KEY 文件
** 输　入  : iVndID             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : iRet               是否成功
*********************************************************************************************************/
INT  kidVpnAesKeyDel (INT  iVndID)
{
    CHAR    cDelCmd[32]    = {0};
    CHAR    cFileName[128] = {0};

    sprintf(cFileName, "/etc/kidvpn/%d.key", iVndID);

    if (ERROR_NONE == access(cFileName, F_OK)) {                        /*  如果文件存在                */
        sprintf(cDelCmd, "rm %s", cFileName);

        system(cDelCmd);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidVpnCfgFileCreate
** 功能描述: 创建本地配置文件
** 输　入  : pKidVPNHandle      VPN 操作信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidVpnCfgFileCreate (__PKIDVPN_HANDLE  pKidVPNHandle)
{
    INT                   iRet;
    CHAR                  cSector[32]         = {0};
    CHAR                  cModeBuf[32]        = {0};
    CHAR                  cKeyFile[64]        = {0};
    CHAR                  cVndIDBuf[32]       = {0};
    CHAR                  cTapName[32]        = {0};
    CHAR                  cMtuBuf[32]         = {0};
    CHAR                  cLocalIp[32]        = {0};
    CHAR                  cServer[32]         = {0};
    CHAR                  cIpBuf[32]          = {0};
    CHAR                  cPortBuf[32]        = {0};
    CHAR                  cIniFile[32]       = {0};

    CHAR                 *pcIniInfo           = NULL;
    UINT64                ulOffset            = 0;
    INT                   iFd;

    if (LW_NULL == pKidVPNHandle) {
        return  (PX_ERROR);
    }

    pcIniInfo = (PCHAR)malloc(1024);
    if (NULL == pcIniInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");

        return  (PX_ERROR);
    }

    bzero(pcIniInfo, 1024);

    sprintf(cIniFile, "/etc/kidvpn/%d.ini", pKidVPNHandle->iVndID);

    iFd = open(cIniFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (iFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Create file failed.\n");

        free(pcIniInfo);

        return  (PX_ERROR);
    }

    ip4addr_ntoa_r(&(pKidVPNHandle->uiIpAddr), cIpBuf, sizeof(cIpBuf));

    /*
     *  设置配置文件缓冲区
     */
    if (TYPE_ADD_VPN_S == pKidVPNHandle->cHandleType) {
        sprintf(cSector, "[server_%d]\n", pKidVPNHandle->iVndID);
        memcpy(pcIniInfo + ulOffset, cSector, strlen(cSector));
        ulOffset += strlen(cSector);

        sprintf(cModeBuf, "mode=server\n");
        memcpy(pcIniInfo + ulOffset, cModeBuf, strlen(cModeBuf));
        ulOffset += strlen(cModeBuf);

        sprintf(cLocalIp, "local_ip=%s\n", cIpBuf);
        memcpy(pcIniInfo + ulOffset, cLocalIp, strlen(cLocalIp));
        ulOffset += strlen(cLocalIp);
    } else if (TYPE_ADD_VPN_C == pKidVPNHandle->cHandleType) {
        sprintf(cSector, "[client_%d]\n", pKidVPNHandle->iVndID);
        memcpy(pcIniInfo + ulOffset, cSector, strlen(cSector));
        ulOffset += strlen(cSector);

        sprintf(cModeBuf, "mode=client\n");
        memcpy(pcIniInfo + ulOffset, cModeBuf, strlen(cModeBuf));
        ulOffset += strlen(cModeBuf);

        sprintf(cServer, "server=%s\n", cIpBuf);
        memcpy(pcIniInfo + ulOffset, cServer, strlen(cServer));
        ulOffset += strlen(cServer);
    }

    sprintf(cKeyFile, "key_file=/etc/kidvpn/%d.key\n", pKidVPNHandle->iVndID);
    memcpy(pcIniInfo + ulOffset, cKeyFile, strlen(cKeyFile));
    ulOffset += strlen(cKeyFile);

    sprintf(cVndIDBuf, "vnd_id=%d\n", pKidVPNHandle->iVndID);
    memcpy(pcIniInfo + ulOffset, cVndIDBuf, strlen(cVndIDBuf));
    ulOffset += strlen(cVndIDBuf);

    sprintf(cTapName, "tap_name=tap0\n");
    memcpy(pcIniInfo + ulOffset, cTapName, strlen(cTapName));
    ulOffset += strlen(cTapName);

    if ((pKidVPNHandle->iMtu < KV_VND_MIN_MTU) || (pKidVPNHandle->iMtu > KV_VND_MAX_MTU)) {
        pKidVPNHandle->iMtu = KV_VND_DEF_MTU;

        sprintf(cMtuBuf, "mtu=%d\n", pKidVPNHandle->iMtu);
        memcpy(pcIniInfo + ulOffset, cMtuBuf, strlen(cMtuBuf));
        ulOffset += strlen(cMtuBuf);
    } else {
        sprintf(cMtuBuf, "mtu=%d\n", pKidVPNHandle->iMtu);
        memcpy(pcIniInfo + ulOffset, cMtuBuf, strlen(cMtuBuf));
        ulOffset += strlen(cMtuBuf);
    }

    sprintf(cPortBuf, "port=%d\n", pKidVPNHandle->iPort);
    memcpy(pcIniInfo + ulOffset, cPortBuf, strlen(cPortBuf));
    ulOffset += strlen(cPortBuf);

    iRet = write(iFd, pcIniInfo, strlen(pcIniInfo));
    if (iRet != strlen(pcIniInfo)) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Write file failed.\n");
    }

    close(iFd);

    free(pcIniInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidVpnCfgFileDel
** 功能描述: 删除本地配置文件
** 输　入  : iVndID             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : iRet               是否成功
*********************************************************************************************************/
static INT  __kidVpnCfgFileDel (INT  iVndID)
{
    CHAR    cDelCmd[32]    = {0};
    CHAR    cFileName[128] = {0};

    sprintf(cFileName, "/etc/kidvpn/%d.ini", iVndID);

    if (ERROR_NONE == access(cFileName, F_OK)) {                        /*  如果文件存在                */
        sprintf(cDelCmd, "rm %s", cFileName);

        system(cDelCmd);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __kidVpnInfoDel
** 功能描述: 删除 VPN 相关信息操作
** 输  入  : iVndID            虚拟网卡地址
** 输  出  : NONE
** 返  回  : iRet
*********************************************************************************************************/
static INT  __kidVpnInfoDel (INT  iVndID)
{
    INT    iRet;
    /*
     *  更新数据库删除此条目
     */
    iRet = kidvpnDelDBItem(iVndID);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delet kidvpn item error\n");

        return  (PX_ERROR);
    }

    /*
     *  删除 key 文件
     */
    kidVpnAesKeyDel(iVndID);


    /*
     *  删除 ini 配置文件
     */
    __kidVpnCfgFileDel(iVndID);

    /*
     *  删除 vnd
     */
    iRet = vndDel(iVndID);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delet vnd item error\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __kidVpnProcessExitHandle
** 功能描述: VPN 进程退出操作
** 输  入  : iPid    进程号
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __kidVpnProcessExitHandle (INT  iPid)
{
    INT                iRet  = PX_ERROR;
    __KIDVPN_DB_INFO   kidVpnInfo;

    /*
     *  通过pid，查数据库判断是否是 VPN 进程，如果是，则进行 VPN 的信息删除操作
     */
    bzero(&kidVpnInfo, sizeof(kidVpnInfo));
    iRet = kidvpnGetSpecInfo(&kidVpnInfo, "PID", LW_NULL, &iPid);
    if (ERROR_NONE == iRet) {
        __kidVpnInfoDel(kidVpnInfo.kidvpnHandle.iVndID);

        return;
    }
}
/*********************************************************************************************************
** 函数名称: kidVpnAesKeySave
** 功能描述: 保存 AES KEY
** 输　入  : cpcKey             AES KEY
**           iVndId             虚拟网卡 ID
**           iLen               KEY 长度
** 输　出  : NONE
** 返　回  : iRet               是否成功
*********************************************************************************************************/
INT  kidVpnAesKeySave (CPCHAR  cpcKey, INT  iVndId, INT  iLen)
{
    CHAR    cKeyFileName[20] = {0};
    INT     iFileId          = -1;
    INT     iRet             = PX_ERROR;

    if (LW_NULL == cpcKey) {
        return  (PX_ERROR);
    }

    if (access("/etc/kidvpn", F_OK)) {
        system("mkdir /etc/kidvpn");
    }
	
    sprintf(cKeyFileName, "/etc/kidvpn/%d.key", iVndId);

    iFileId = open(cKeyFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (iFileId < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Create file failed.\n");
        return  (PX_ERROR);
    }

    iRet = write(iFileId, cpcKey, iLen);

    close(iFileId);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: kidVpnAesKeyGen
** 功能描述: 生成 AES KEY
** 输　入  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY 长度
** 输　出  : NONE
** 返　回  : iRet               是否成功
*********************************************************************************************************/
INT  kidVpnAesKeyGen (PCHAR  pcKey, INT  iBit, INT  iLen)
{
    INT    iRandFd;
    INT    i;
    UCHAR  ucKeyCode[32]  = {0};
    CHAR   cCodeBuf[3]    = {0};

    if (LW_NULL == pcKey) {
        return  (PX_ERROR);
    }

    /*
     *  目前支持这三种 bit
     */
    if (128 != iBit && 192 != iBit && 256 != iBit) {
        return  (PX_ERROR);
    }

    if (128 == iBit &&  iLen < 32) {
        return  (PX_ERROR);
    }

    if (192 == iBit &&  iLen < 48) {
        return  (PX_ERROR);
    }

    if (256 == iBit &&  iLen < 64) {
        return  (PX_ERROR);
    }

    iRandFd = open("/dev/random", O_RDONLY);
    if (0 > iRandFd) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,
                        "Can not open /dev/random file, error: %s\n",
                        strerror(errno));

        return  (PX_ERROR);
    }

    switch (iBit) {
    case 128:
        read(iRandFd, ucKeyCode, 16);
        close(iRandFd);
        break;

    case 192:
        read(iRandFd, ucKeyCode, 24);
        close(iRandFd);
        break;

    case 256:
        read(iRandFd, ucKeyCode, 32);
        close(iRandFd);
        break;

    default:
        close(iRandFd);
        return  (PX_ERROR);
    }

    /*
     *  转化为字符串
     */
    for (i = 0; i < (iBit >> 3); i++) {
        bzero(cCodeBuf, sizeof(cCodeBuf));
        sprintf(cCodeBuf, "%02x", ucKeyCode[i]);
        strcat(pcKey, cCodeBuf);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: kidVpnAesKeyGet
** 功能描述: 获取本地 AES KEY
** 输　入  : cpcKey             AES KEY
**           iVndID             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : iRet               是否成功
*********************************************************************************************************/
INT  kidVpnAesKeyGet (PCHAR  pcKey, INT  iVndID, INT  iLen)
{
    CHAR    cFileName[20] = {0};
    INT     iKeyFd;
    INT     iRet;

    if (LW_NULL == pcKey) {
        return  (PX_ERROR);
    }

    sprintf(cFileName, "/etc/kidvpn/%d.key", iVndID);
    iKeyFd = open(cFileName, O_RDONLY);
    if (0 > iKeyFd) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,
                        "Can not open %s file, error: %s\n",
                        cFileName, strerror(errno));

        return  (PX_ERROR);
    }

    iRet = read(iKeyFd, pcKey, iLen);
    close(iKeyFd);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: __kidVpnChildProcSet
** 功能描述: kidvpn 子进程设置并启动
** 输　入  : ppcCmdArg      命令行参数
**           piFd           无名管道文件句柄指针
**           iVndId         VND ID
** 输　出  : pPid           进程 id 指针
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidVpnChildProcSet (pid_t  *pPid, CHAR  **ppcCmdArg, INT  *piFd, INT  iVndId)
{
    struct sched_param          schedParam;
    posix_spawnattr_t           spawnAttr;
    posix_spawn_file_actions_t  fileActions;

    __FD_HANDLE                 fdInfo;

    fdInfo.iPipeFd[0] = piFd[0];
    fdInfo.iPipeFd[1] = piFd[1];
    fdInfo.iVndId     = iVndId;

    /*
     * 初始化进程属性对象
     */
    if (posix_spawnattr_init(&spawnAttr) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * 初始化文件操作对象
     */
    if (posix_spawn_file_actions_init(&fileActions) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawn_file_actions_t failed\n");

        return (PX_ERROR);
    }

    /*
     * 关闭标准输入、标准输出、错误输出和管道的输入
     */
    if (posix_spawn_file_actions_addclose(&fileActions, STDIN_FILENO)  != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, STDOUT_FILENO) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, STDERR_FILENO) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, piFd[0]) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "close std file failed\n");

        return (PX_ERROR);
    }

    /*
     * 将写端复制到子进程的标准输出、标准出错
     */
    posix_spawn_file_actions_adddup2(&fileActions, piFd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&fileActions, piFd[1], STDERR_FILENO);

    /*
     * 新建进程优先级为 NORMAL
     */
    schedParam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnAttr, &schedParam);
    posix_spawnattr_setflags(&spawnAttr, POSIX_SPAWN_SETSCHEDPARAM);

    Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);
    __fdNodeAdd(&fdInfo);
    Lw_SemaphoreM_Post(_G_hKidVPNMutex);

    /*
     *  启动进程
     */
    if (posix_spawnp(pPid, "kidvpn", &fileActions, &spawnAttr, ppcCmdArg, NULL) != 0) {
        posix_spawnattr_destroy(&spawnAttr);
        posix_spawn_file_actions_destroy(&fileActions);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "posix_spawnp error\n");

        Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);
        __fdNodeDel(iVndId);
        Lw_SemaphoreM_Post(_G_hKidVPNMutex);

        return  (PX_ERROR);
    }

    posix_spawnattr_destroy(&spawnAttr);
    posix_spawn_file_actions_destroy(&fileActions);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: kidVpnStart
** 功能描述: 开启 VPN
** 输　入  : pKidVPNHandle      VPN 操作信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnStart (__PKIDVPN_HANDLE  pKidVPNHandle)
{
    INT                   iRet;
    CHAR                 *cCmdArg[5];
    CHAR                  cSecArg[32]  = {0};
    CHAR                  cIniFile[32] = {0};
    CHAR                  cPassWd[32]  = {0};
    INT                   iStatLoc     = PX_ERROR;

    __KIDVPN_DB_INFO      kidvpnDBInfo;
    pid_t                 pid = 0;
    INT                   iFd[2];

    if (LW_NULL == pKidVPNHandle) {
        return  (PX_ERROR);
    }

    iRet = pipe(iFd);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pipe error.\n");

        return  (PX_ERROR);
    }

    /*
     *  创建配置文件
     */
    iRet = __kidVpnCfgFileCreate(pKidVPNHandle);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kidvpn Cfg file create failed.\n");

        goto error0;
    }

    sprintf(cIniFile, "/etc/kidvpn/%d.ini", pKidVPNHandle->iVndID);

    if (TYPE_ADD_VPN_S == pKidVPNHandle->cHandleType) {
        sprintf(cSecArg, "server_%d", pKidVPNHandle->iVndID);
    } else {
        sprintf(cSecArg, "client_%d", pKidVPNHandle->iVndID);
    }

    strncpy(cPassWd, pKidVPNHandle->cPassWd, sizeof(cPassWd));

    /*
     *  kidVpn 执行命令:#kidvpn File.ini sector passwd
     */
    cCmdArg[0] = "kidvpn";
    cCmdArg[1] = cIniFile;
    cCmdArg[2] = cSecArg;
    cCmdArg[3] = cPassWd;
    cCmdArg[4] = NULL;

    /*
     *  设置 kidvpn 子进程并启动
     */
    iRet = __kidVpnChildProcSet(&pid, cCmdArg, iFd, pKidVPNHandle->iVndID);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kidvpn process spawn failed.\n");

        goto error1;
    }

    /*
     *  更新数据库信息
     */
    bzero(&kidvpnDBInfo, sizeof(kidvpnDBInfo));
    memcpy(&(kidvpnDBInfo.kidvpnHandle), pKidVPNHandle, sizeof(kidvpnDBInfo.kidvpnHandle));
    kidvpnDBInfo.iPid    = pid;
    kidvpnDBInfo.cStatus = 0;

    iRet = kidvpnAddItemToDb(&kidvpnDBInfo);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kidvpn info add to database failed.\n");

        goto error2;
    }

    return  (ERROR_NONE);

error2:
    kill(kidvpnDBInfo.iPid, 9);
    waitpid(kidvpnDBInfo.iPid, &iStatLoc, 0);

    Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);
    __fdNodeDel(pKidVPNHandle->iVndID);
    Lw_SemaphoreM_Post(_G_hKidVPNMutex);

error1:
    __kidVpnInfoDel(pKidVPNHandle->iVndID);

error0:
    close(iFd[0]);
    close(iFd[1]);

    return  (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: kidVpnDel
** 功能描述: 删除 VPN 操作
** 输  入  : iVndID            虚拟网卡地址
** 输  出  : NONE
** 返  回  : iStatLoc          删除状态
*********************************************************************************************************/
INT  kidVpnDel (INT  iVndID)
{
    CHAR                  cKillCmd[20]    = {0};
    INT                   iStatLoc        = PX_ERROR;
    __KIDVPN_DB_INFO      kidvpnDbInfo;
    INT                   iRet;

    bzero(&kidvpnDbInfo, sizeof(kidvpnDbInfo));

    /*
     *  从数据库中获取 vndid
     */
    iRet = kidvpnGetSpecInfo(&kidvpnDbInfo, "VNDID", LW_NULL, &iVndID);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);
    __fdNodeDel(iVndID);
    Lw_SemaphoreM_Post(_G_hKidVPNMutex);

    if (TYPE_ADD_VPN_S == kidvpnDbInfo.kidvpnHandle.cHandleType) {
        kidvpnLinkDelDBItemById(iVndID);
    }

    /*
     *  TODO:清除情景模式数据库中的数据
     */
    if (TYPE_ADD_VPN_S == kidvpnDbInfo.kidvpnHandle.cHandleType) {
        kidVpnServInfoDeleteFromDbByVndid(iVndID);
    } else if (TYPE_ADD_VPN_C == kidvpnDbInfo.kidvpnHandle.cHandleType) {
        kidVpnCliInfoDeleteFromDbByVndid(iVndID);
    }


    snprintf(cKillCmd, sizeof(cKillCmd), "kill %d", kidvpnDbInfo.iPid);
    iRet = system(cKillCmd);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kill kidvpn error\n");

        return  (PX_ERROR);
    }

    waitpid(kidvpnDbInfo.iPid, &iStatLoc, 0);
    if (iRet < 0) {
        return  (PX_ERROR);
    }

    return  (__kidVpnInfoDel(iVndID));
}

/*********************************************************************************************************
** 函数名称: kidVpnInit
** 功能描述: VPN 初始化
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
INT  kidVpnInit (VOID)
{
    INT  iRet;

    _G_hKidVPNMutex = Lw_SemaphoreM_Create("kidvpn_fd_lock",            /*  创建文件句柄链表操作互斥锁  */
                                           LW_PRIO_HIGH,
                                           LW_OPTION_WAIT_FIFO |
                                           LW_OPTION_OBJECT_LOCAL |
                                           LW_OPTION_INHERIT_PRIORITY |
                                           LW_OPTION_ERRORCHECK,
                                           LW_NULL);

    if (LW_OBJECT_HANDLE_INVALID == _G_hKidVPNMutex) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "mutex initialize failed.\n");

        goto  error;
    }

    /*
     *  kidvpn 信息处理线程初始化
     */
    iRet = __kidvpnMsgThreadInit();
    if (ERROR_NONE != iRet) {
        goto  error;
    }

    /*
     *  注册一个处理函数，用于 VPN 进程退出时处理
     */
    return  (subRecliamHandleAdd(__kidVpnProcessExitHandle));

error:
    if (LW_OBJECT_HANDLE_INVALID != _G_hKidVPNMutex) {
        Lw_SemaphoreM_Delete(&_G_hKidVPNMutex);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
