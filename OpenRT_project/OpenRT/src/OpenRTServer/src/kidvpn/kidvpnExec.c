/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: kidVpnExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: VPN ����
** 2018.08.16  �����°汾 KidVPN �޸Ĵ��롣
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
  ��ȡ KidVPN IP��ַ����Ϣ�ṹ
*********************************************************************************************************/
struct vpn_ipip{
    PUCHAR  pucData;                                                    /*  ���ݻ�����                  */
    PUCHAR  pucIndex;                                                   /*  ����ָ��                    */
    UINT   *puiFlag;                                                    /*  ��־                        */
    UINT    uiOffset;                                                   /*  ƫ��                        */
};
typedef struct vpn_ipip   __VPN_IPIP;
typedef struct vpn_ipip  *__PVPN_IPIP;
/********************************************************************************************************
  ��ȡ KidVPN IP λ����Ϣ�ṹ
*********************************************************************************************************/
struct pos_info {
    CHAR  cCountry[32];                                                 /*  ����                        */
    CHAR  cProvice[32];                                                 /*  ʡ��                        */
    CHAR  cCity[32];                                                    /*  ����                        */
    CHAR  cRegion[16];                                                  /*  ����                        */
    CHAR  cOperat[16];                                                  /*  ��Ӫ��                      */
};
typedef struct pos_info   __VPN_POSITION;
typedef struct pos_info  *__PVPN_POSITION;
/*********************************************************************************************************
  ͨ�� IP ��ַ��ȡ����λ�õ����ú�
*********************************************************************************************************/
#define B2IL(b)                (((b)[0] & 0xFF) |            \
                               (((b)[1] << 8) & 0xFF00) |    \
                               (((b)[2] << 16) & 0xFF0000) | \
                               (((b)[3] << 24) & 0xFF000000))           /*  �ڴ����ݸߵ�λ����          */

#define B2IU(b)                (((b)[3] & 0xFF) |            \
                               (((b)[2] << 8) & 0xFF00) |    \
                               (((b)[1] << 16) & 0xFF0000) | \
                               (((b)[0] << 24) & 0xFF000000))           /*  �ڴ����ݸߵ�λת��          */

#define COUNTRY_INDEX          (0)                                      /*  ����                        */
#define PROVINCE_INDEX         (1)                                      /*  ʡ��                        */
#define CITY_INDEX             (2)                                      /*  ����                        */
#define REGION_INDEX           (3)                                      /*  ����                        */
#define OPERAT_INDEX           (4)                                      /*  ��Ӫ��                      */
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static LW_HANDLE               _G_hKidVPNMutex;                         /*  ���������ݽṹ�����ź���    */
static __VPN_IPIP              _G_ipPos;                                /*  IP ��ַ���ļ������Ϣ       */
/*********************************************************************************************************
  ����ͷȫ�ֱ���
*********************************************************************************************************/
static LW_LIST_LINE_HEADER     _G_plineKidVPNHeader = LW_NULL;          /*  KidVPN ����ͷ               */
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define VPN_ENABLE             "1"                                      /*  vpn �ͻ�������              */
#define VPN_DISABLE            "0"                                      /*  vpn �ͻ������ӶϿ�          */
#define ETH_DATA_LEN           (1500)                                   /*  ��� MTU                    */
#define KV_VND_MIN_MTU         (1280)                                   /*  kidvpn ��СMTU              */
#define KV_VND_MAX_MTU         (ETH_DATA_LEN - 28)                      /*  kidvpn ���MTU              */
#define KV_VND_DEF_MTU         (KV_VND_MAX_MTU - 8)                     /*  kidvpn Ĭ��MTU              */

#define BUF_SIZE               (1024)                                   /*  ��������С                  */
#define PORT                   (80)                                     /*  �˿ں�                      */
#define CURL_STRING            "www.3322.org/dyndns/getip"              /*  CURL ���                   */
/*********************************************************************************************************
** ��������: __fdNodeGetByVndId
** ��������: ���� VND ID �����ļ�����ڵ�
** ��  ��  : iVndId      VND ID
** ��  ��  : NONE
** ��  ��  : �ļ�����ڵ�
*********************************************************************************************************/
static __PFD_NODE  __fdNodeGetByVndId (INT  iVndId)
{
    PLW_LIST_LINE  plineTemp = LW_NULL;
    __PFD_NODE     pfdNode   = LW_NULL;

    for (plineTemp  = _G_plineKidVPNHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  �����ļ��������            */

        pfdNode = _LIST_ENTRY(plineTemp, __FD_NODE, fdListLine);
        if (iVndId == pfdNode->tNodeData.iVndId) {
            break;                                                      /*  �ҵ�                        */
        }
    }

    if (plineTemp) {
        return  (pfdNode);
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: __fdNodeAdd
** ��������: ����һ���µĽڵ㣨��巨��
** ��  ��  : pFdInfo        �ļ������Ϣָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
     *  ����ͷ�ڵ��
     */
    _List_Line_Add_Tail(&pNode->fdListLine, &_G_plineKidVPNHeader);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __fdNodeDel
** ��������: ����VND ID ɾ���ڵ�
** ��  ��  : iVndId          VND ID
** ��  ��  : NONE
** ��  ��  : NONE
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
** ��������: getPublicIp
** ��������: ��ȡ�������� ip
** �䡡��  : pcIpBuf     ��� ip ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  �� url �л�ȡ����������صĵ�ַ��Ϣ
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
     *  ���� socket ����
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
     *  �ϳ�������Ϣ�����ڷ��͵� web ������
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
     *  ���� web ������������ͷ���ݣ������� web ���ص�����
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
     *  ��ȡ���� ip ��Ϣ
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
** ��������: getPositionByIpAddr
** ��������: ���� ip ��ַ��ȡ����λ��
** �䡡��  : cpcIp           IP ��Ϣ
** �䡡��  : pcResult        �������λ����Ϣ
** ������  : ERROR_CODE
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
             *  ע��:���� ip ���ݿ�ʹ�õ�����Ѱ汾��ֻ���Բ��������ݣ���˸�ʽĿǰ���ǣ�%s    %s    %s
             *       ��������������ݿ⣬��ô����ҲҪ������滻
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
             *  ������ cCityName Ϊ��
             */
            if (cCityName[0] == 0) {
                strncpy(cCityName, cRegionName, sizeof(cCityName));
            }
            /*
             *  ����ǰ��ֻ��Ҫ����
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
** ��������: __kidvpnFdInfoSetToFdSet
** ��������: ���ļ������������л�ȡ�ļ������Ϣ�����õ� fd_set
** �䡡��  : pFdSet    fd_set �ļ���
** �䡡��  : NONE
** ������  : �ļ����
*********************************************************************************************************/
static INT  __kidvpnFdInfoSetToFdSet(fd_set  *pFdSet)
{
    INT            iFd;
    INT            iFdMax    = -1;
    PLW_LIST_LINE  plineTemp = LW_NULL;
    __PFD_NODE     pFdNode   = LW_NULL;

    /*
     *  ��ȡ�ܵ��ļ���������õ� fd_set ������
     */
    for (plineTemp  = _G_plineKidVPNHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  ������������                */

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
** ��������: __kidvpnClientIpMacSave
** ��������: kidvpn �ӽ�����Ϣд�����ݿ�
** �䡡��  : cpcBuf       kidvpn �ӽ�����Ϣ
**           iLen         ��Ϣ����
**           iVndId       vnd id
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  ��ȡ�ͻ��� ip
     */
    pcTmp  = lib_index(pcBuf, ':');
    pcCur  = lib_index(pcBuf, '[');
    pcTmp += 2;
    pcCur -= 1;

    lib_memcpy(cClientIp, pcTmp, pcCur - pcTmp);

    /*
     *  ��ȡ�ͻ��� MAC
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
** ��������: __kidvpnClientIpMacDiscard
** ��������: kidvpn �ӽ���������Ϣ�����ݿ�ɾ��
** �䡡��  : cpcBuf       kidvpn �ӽ�����Ϣ
**           iLen         ��Ϣ����
**           iVndId       vnd id
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  ��ȡ�ͻ��� ip
     */
    pcTmp  = lib_index(pcBuf, ':');
    pcCur  = lib_index(pcBuf, '[');
    pcTmp += 2;
    pcCur -= 1;

    lib_memcpy(cClientIp, pcTmp, pcCur - pcTmp);

    /*
     *  ��ȡ�ͻ��� MAC
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
** ��������: __kidvpnMsgDoHandle
** ��������: kidvpn �ӽ�����Ϣ����
** �䡡��  : cpcBuf       kidvpn �ӽ�����Ϣ
**           iLen         ��Ϣ����
**           iVndId       vnd id
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: __kidvpnChildMsgParseHandle
** ��������: kidvpn �ӽ�����Ϣ����������
** �䡡��  : cpcBuf       kidvpn �ӽ�����Ϣ
**           iLen         ��Ϣ����
**           iVndId       vnd id
** �䡡��  : NONE
** ������  : ERROR_CODE
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
        __kidvpnMsgDoHandle(cpcBuf, iLen, iVndId);                      /*  ���� kidvpn ��Ϣ            */
        vpnLogInsert(iVndId, (PCHAR)cpcBuf);                            /*  ��¼��־                    */
    }

    while (1) {
        lib_memcpy(cTmpBuf, pcCur, pcTmp-pcCur);

        __kidvpnMsgDoHandle(cTmpBuf, strlen(cTmpBuf), iVndId);          /*  ���� kidvpn ��Ϣ            */
        vpnLogInsert(iVndId, cTmpBuf);                                  /*  ��¼��־                    */

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
** ��������: __kidvpnMessageGetHandle
** ��������: kidvpn ��Ϣ��ȡ�߳�
** �䡡��  : pvArg          ����ָ��
** �䡡��  : NONE
** ������  : NONE
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
         *  ��·���ã��ȴ��ɶ��ļ���
         */
        iRet = select(iFdMax + 1, &fdset, NULL, NULL, &timeout);
        if (iRet > 0) {
            for (plineTemp  = _G_plineKidVPNHeader;
                 plineTemp != LW_NULL;
                 plineTemp  = plineTemp->LINE_plistNext) {                      /*  ������������                */

                pFdNode = _LIST_ENTRY(plineTemp, __FD_NODE, fdListLine);

                iFd = pFdNode->tNodeData.iPipeFd[0];
                if (iFd > 0) {
                    if (FD_ISSET(iFd, &fdset)) {
                        bzero(cBuf, sizeof(cBuf));

                        iSize = read(iFd, cBuf, sizeof(cBuf));
                        if (iSize > 0) {

                            /*
                             * ��ȡ�����ݣ����н�������
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
** ��������: __kidvpnMsgThreadInit
** ��������: kidvpn ��Ϣ�����̳߳�ʼ��
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnMsgThreadInit (VOID)
{
    LW_HANDLE             tid;
    LW_CLASS_THREADATTR   threadAttr;

    /*
     *  ���� kidvpn ��־�������߳�
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
** ��������: kidVpnAesKeyDel
** ��������: ɾ������ AES KEY �ļ�
** �䡡��  : iVndID             �������� ID
** �䡡��  : NONE
** ������  : iRet               �Ƿ�ɹ�
*********************************************************************************************************/
INT  kidVpnAesKeyDel (INT  iVndID)
{
    CHAR    cDelCmd[32]    = {0};
    CHAR    cFileName[128] = {0};

    sprintf(cFileName, "/etc/kidvpn/%d.key", iVndID);

    if (ERROR_NONE == access(cFileName, F_OK)) {                        /*  ����ļ�����                */
        sprintf(cDelCmd, "rm %s", cFileName);

        system(cDelCmd);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kidVpnCfgFileCreate
** ��������: �������������ļ�
** �䡡��  : pKidVPNHandle      VPN ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  ���������ļ�������
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
** ��������: __kidVpnCfgFileDel
** ��������: ɾ�����������ļ�
** �䡡��  : iVndID             �������� ID
** �䡡��  : NONE
** ������  : iRet               �Ƿ�ɹ�
*********************************************************************************************************/
static INT  __kidVpnCfgFileDel (INT  iVndID)
{
    CHAR    cDelCmd[32]    = {0};
    CHAR    cFileName[128] = {0};

    sprintf(cFileName, "/etc/kidvpn/%d.ini", iVndID);

    if (ERROR_NONE == access(cFileName, F_OK)) {                        /*  ����ļ�����                */
        sprintf(cDelCmd, "rm %s", cFileName);

        system(cDelCmd);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kidVpnInfoDel
** ��������: ɾ�� VPN �����Ϣ����
** ��  ��  : iVndID            ����������ַ
** ��  ��  : NONE
** ��  ��  : iRet
*********************************************************************************************************/
static INT  __kidVpnInfoDel (INT  iVndID)
{
    INT    iRet;
    /*
     *  �������ݿ�ɾ������Ŀ
     */
    iRet = kidvpnDelDBItem(iVndID);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delet kidvpn item error\n");

        return  (PX_ERROR);
    }

    /*
     *  ɾ�� key �ļ�
     */
    kidVpnAesKeyDel(iVndID);


    /*
     *  ɾ�� ini �����ļ�
     */
    __kidVpnCfgFileDel(iVndID);

    /*
     *  ɾ�� vnd
     */
    iRet = vndDel(iVndID);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delet vnd item error\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __kidVpnProcessExitHandle
** ��������: VPN �����˳�����
** ��  ��  : iPid    ���̺�
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __kidVpnProcessExitHandle (INT  iPid)
{
    INT                iRet  = PX_ERROR;
    __KIDVPN_DB_INFO   kidVpnInfo;

    /*
     *  ͨ��pid�������ݿ��ж��Ƿ��� VPN ���̣�����ǣ������ VPN ����Ϣɾ������
     */
    bzero(&kidVpnInfo, sizeof(kidVpnInfo));
    iRet = kidvpnGetSpecInfo(&kidVpnInfo, "PID", LW_NULL, &iPid);
    if (ERROR_NONE == iRet) {
        __kidVpnInfoDel(kidVpnInfo.kidvpnHandle.iVndID);

        return;
    }
}
/*********************************************************************************************************
** ��������: kidVpnAesKeySave
** ��������: ���� AES KEY
** �䡡��  : cpcKey             AES KEY
**           iVndId             �������� ID
**           iLen               KEY ����
** �䡡��  : NONE
** ������  : iRet               �Ƿ�ɹ�
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
** ��������: kidVpnAesKeyGen
** ��������: ���� AES KEY
** �䡡��  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY ����
** �䡡��  : NONE
** ������  : iRet               �Ƿ�ɹ�
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
     *  Ŀǰ֧�������� bit
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
     *  ת��Ϊ�ַ���
     */
    for (i = 0; i < (iBit >> 3); i++) {
        bzero(cCodeBuf, sizeof(cCodeBuf));
        sprintf(cCodeBuf, "%02x", ucKeyCode[i]);
        strcat(pcKey, cCodeBuf);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: kidVpnAesKeyGet
** ��������: ��ȡ���� AES KEY
** �䡡��  : cpcKey             AES KEY
**           iVndID             �������� ID
** �䡡��  : NONE
** ������  : iRet               �Ƿ�ɹ�
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
** ��������: __kidVpnChildProcSet
** ��������: kidvpn �ӽ������ò�����
** �䡡��  : ppcCmdArg      �����в���
**           piFd           �����ܵ��ļ����ָ��
**           iVndId         VND ID
** �䡡��  : pPid           ���� id ָ��
** ������  : ERROR_CODE
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
     * ��ʼ���������Զ���
     */
    if (posix_spawnattr_init(&spawnAttr) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * ��ʼ���ļ���������
     */
    if (posix_spawn_file_actions_init(&fileActions) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawn_file_actions_t failed\n");

        return (PX_ERROR);
    }

    /*
     * �رձ�׼���롢��׼�������������͹ܵ�������
     */
    if (posix_spawn_file_actions_addclose(&fileActions, STDIN_FILENO)  != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, STDOUT_FILENO) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, STDERR_FILENO) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, piFd[0]) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "close std file failed\n");

        return (PX_ERROR);
    }

    /*
     * ��д�˸��Ƶ��ӽ��̵ı�׼�������׼����
     */
    posix_spawn_file_actions_adddup2(&fileActions, piFd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&fileActions, piFd[1], STDERR_FILENO);

    /*
     * �½��������ȼ�Ϊ NORMAL
     */
    schedParam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnAttr, &schedParam);
    posix_spawnattr_setflags(&spawnAttr, POSIX_SPAWN_SETSCHEDPARAM);

    Lw_SemaphoreM_Wait(_G_hKidVPNMutex, LW_OPTION_WAIT_INFINITE);
    __fdNodeAdd(&fdInfo);
    Lw_SemaphoreM_Post(_G_hKidVPNMutex);

    /*
     *  ��������
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
** ��������: kidVpnStart
** ��������: ���� VPN
** �䡡��  : pKidVPNHandle      VPN ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  ���������ļ�
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
     *  kidVpn ִ������:#kidvpn File.ini sector passwd
     */
    cCmdArg[0] = "kidvpn";
    cCmdArg[1] = cIniFile;
    cCmdArg[2] = cSecArg;
    cCmdArg[3] = cPassWd;
    cCmdArg[4] = NULL;

    /*
     *  ���� kidvpn �ӽ��̲�����
     */
    iRet = __kidVpnChildProcSet(&pid, cCmdArg, iFd, pKidVPNHandle->iVndID);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kidvpn process spawn failed.\n");

        goto error1;
    }

    /*
     *  �������ݿ���Ϣ
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
** ��������: kidVpnDel
** ��������: ɾ�� VPN ����
** ��  ��  : iVndID            ����������ַ
** ��  ��  : NONE
** ��  ��  : iStatLoc          ɾ��״̬
*********************************************************************************************************/
INT  kidVpnDel (INT  iVndID)
{
    CHAR                  cKillCmd[20]    = {0};
    INT                   iStatLoc        = PX_ERROR;
    __KIDVPN_DB_INFO      kidvpnDbInfo;
    INT                   iRet;

    bzero(&kidvpnDbInfo, sizeof(kidvpnDbInfo));

    /*
     *  �����ݿ��л�ȡ vndid
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
     *  TODO:����龰ģʽ���ݿ��е�����
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
** ��������: kidVpnInit
** ��������: VPN ��ʼ��
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
INT  kidVpnInit (VOID)
{
    INT  iRet;

    _G_hKidVPNMutex = Lw_SemaphoreM_Create("kidvpn_fd_lock",            /*  �����ļ�����������������  */
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
     *  kidvpn ��Ϣ�����̳߳�ʼ��
     */
    iRet = __kidvpnMsgThreadInit();
    if (ERROR_NONE != iRet) {
        goto  error;
    }

    /*
     *  ע��һ�������������� VPN �����˳�ʱ����
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
