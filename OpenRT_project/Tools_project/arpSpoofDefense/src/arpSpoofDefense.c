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
** 文   件   名: arpSpoofDefense.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: ARP 欺骗防御操作
**               更新0.2版本：增加配置文件操作(2019年1月2日)
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include <time.h>
#include "net/if_ether.h"
#include "lwip/prot/ip4.h"
#include "netdev.h"
#include "stdlib.h"
#include "netif/etharp.h"
#include "arpSpoofDefense.h"
#include "asd_cfg.h"
/*********************************************************************************************************
  版本
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_VERSION    "0.2"                              /* ARP 防御版本                 */
/*********************************************************************************************************
  ARP 标志位
*********************************************************************************************************/
#define ARP_NORMAL_FLAG            (0x0)                                /*  Arp信息正常                 */
#define ARP_NORMAL_STR             ("normal")                           /*  Arp正常字符串               */
#define ARP_CONFLICT_FLAG          (0x01)                               /*  发生MAC地址冲突             */
#define ARP_CONFLICT_STR           ("conflict")                         /*  Arp冲突字符串               */
#define ARP_ATTACK_FLAG            (0x02)                               /*  发生MAC欺骗                 */
#define ARP_ATTACK_STR             ("attack")                           /*  Arp攻击字符串               */
/*********************************************************************************************************
  HASH 配置宏
*********************************************************************************************************/
#define HASH_SIZE                  (512)                                /*  HASH 桶深                   */
/*********************************************************************************************************
  HASH 操作宏
*********************************************************************************************************/
#if (HASH_SIZE & (HASH_SIZE - 1)) == 0
#define HASH_ENTRY(h)  ((h) & (HASH_SIZE - 1))
#else
#define HASH_ENTRY(h)  ((h) % HASH_SIZE)
#endif
/*********************************************************************************************************
  ARP 防御信息结构体
*********************************************************************************************************/
struct as_devinfo {
    LW_LIST_LINE                    lineManag;                          /*  管理链表                    */

    ip4_addr_t                      uiIpAddr;                           /*  IP地址                      */
    UCHAR                           ucTrueMacAddr[ETHARP_HWADDR_LEN];   /*  正确MAC地址                 */
    UINT8                           ucTrueMacCounter;                   /*  正确MAC收到的ARP应答次数    */
    UCHAR                           ucConMacAddr[ETHARP_HWADDR_LEN];    /*  冲突MAC地址                 */
    UINT8                           ucConMacCounter;                    /*  冲突MAC收到的ARP应答次数    */
    UINT8                           ucFlag;                             /*  当前ARP信息状态标志         */
    PVOID                           pvNetif;                            /*  用于保存 netif 结点         */
    UINT32                          uiConflitCount;                     /*  MAC冲突计数值               */
    UINT64                          ullTimeOut;                         /*  时间戳                      */
    UINT32                          uiTimeCount;                        /*  定时器计数值(alive时间)     */
};
typedef struct as_devinfo      __AS_DEVINFO;
typedef struct as_devinfo     *__PAS_DEVINFO;
/*********************************************************************************************************
  操作全局变量定义
*********************************************************************************************************/
UINT8                _G_ucArpSpoofDefenseEn;                            /*  防御使能                    */
LW_OBJECT_HANDLE     _G_ulAsDefenseTimer;                               /*  定时器句柄                  */
__ASD_CONFIG         _G_asdConfig;                                      /*  配置文件参数                */
/*********************************************************************************************************
  ARP 欺骗防御 HASH 表
*********************************************************************************************************/
PVOID  _G_pvAsDefenseHashtable[HASH_SIZE];
/*********************************************************************************************************
  操作锁
*********************************************************************************************************/
static LW_OBJECT_HANDLE        _G_ulAsDefenseLock;
#define __ASDEFENSE_LOCK()     API_SemaphoreBPend(_G_ulAsDefenseLock, LW_OPTION_WAIT_INFINITE)
#define __ASDEFENSE_UNLOCK()   API_SemaphoreBPost(_G_ulAsDefenseLock)
/*********************************************************************************************************
  日志全局变量定义
*********************************************************************************************************/
INT32                _G_iLogFd;                                         /*  LOG 文件句柄                */
CHAR                 _G_cLogTimeBuff[30] = {0};                         /*  LOG 显示的时间              */
INT32                _G_iFileLen;                                       /*  LOG 文件长度                */
/*********************************************************************************************************
  LOG 配置
*********************************************************************************************************/
#define FILE_MODE     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)           /*  文件操作模式                */
/*********************************************************************************************************
  配置文件
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_CFG "/etc/arpSpoofDefense.ini"                 /*  配置文件名称                */
/*********************************************************************************************************
  LOG 输出
*********************************************************************************************************/
#define __ASD_LOG(format, ...)                                                                 \
    do {                                                                                       \
        CHAR  cBuff[256] = {0};                                                                \
        __logTimeGen(_G_cLogTimeBuff);                                                         \
        snprintf(cBuff, sizeof(cBuff), "[%s] "format, _G_cLogTimeBuff, ##__VA_ARGS__);         \
        printk("%s", cBuff);                                                                   \
        __logInput(cBuff, strlen(cBuff));                                                      \
    } while (0)
/*********************************************************************************************************
** 函数名称: __logInit
** 功能描述: 日志初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT32  __logInit (VOID)
{
    _G_iLogFd = open(_G_asdConfig.pcLogFile, O_RDWR | O_CREAT, FILE_MODE);
    if (_G_iLogFd < 0) {
        printk("Create file failed. ret = %d\n", _G_iLogFd);
        _G_iLogFd = PX_ERROR;

        return  (PX_ERROR);
    }

    /*
     *  偏移到文件末尾
     */
    _G_iFileLen = lseek(_G_iLogFd, 0, SEEK_END);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __logTimeGen
** 功能描述: 产生一条 log 日志的时间
** 输　入  : NONE
** 输　出  : pcTimeBuf          时间字符串
** 返　回  : iTime              时间整形值
*********************************************************************************************************/
static time_t  __logTimeGen (PCHAR  pcTimeBuf)
{
    struct tm   logtm;
    time_t      iTime;

    bzero(&logtm, sizeof(logtm));
    iTime = time(NULL);

    lib_ctime_r(&iTime, pcTimeBuf);

    pcTimeBuf[strlen(pcTimeBuf) - 1] = 0;                               /*  需要删除最后的换行符        */

    return  (iTime);
}

/*********************************************************************************************************
** 函数名称: __logInput
** 功能描述: 填写日志
** 输　入  : pcLogInfo          日志内容
**           uiLogLen           日志长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT32  __logInput (PCHAR  pcLogInfo, UINT32  uiLogLen)
{
    CHAR   cCmdString[1024] = {0};
    if  (LW_NULL == pcLogInfo) {
        return  (PX_ERROR);
    }

    /*
     *  判断当前文件的长度
     */
    if (_G_asdConfig.iLogFilelen * 1024 * 1024  < _G_iFileLen + uiLogLen) {
        if(ERROR_NONE == access(_G_asdConfig.pcLogFileBk, F_OK)) {      /*  存在备份文件                */
            snprintf(cCmdString, sizeof(cCmdString), "rm %s\n", _G_asdConfig.pcLogFileBk);
            system(cCmdString);
        }

        close(_G_iLogFd);                                               /*  先关闭当前文件              */
        _G_iLogFd = PX_ERROR;

        bzero(cCmdString, sizeof(cCmdString));
        snprintf(cCmdString,
                 sizeof(cCmdString),
                 "mv %s %s\n",
                 _G_asdConfig.pcLogFile,
                 _G_asdConfig.pcLogFileBk);                             /*  将原本的文件改成备份        */
        __logInit();                                                    /*  重新创建一个新的Log文件     */
    }

    /*
     *  写日志
     */
    if (_G_iLogFd > 0) {
        write(_G_iLogFd, pcLogInfo, uiLogLen);
        fdatasync(_G_iLogFd);
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __asDefenseNodeAdd
** 功能描述: 添加设备结点
** 输　入  : uiIpAddr           IP 地址
** 输　出  : NONE
** 返　回  : PX_ERROR           添加失败
**           ERROR_NONE         添加成功
*********************************************************************************************************/
static INT32  __asDefenseNodeAdd (UINT32  uiIpAddr, PVOID  pvNode)
{
    UINT32            uiHashEntry = 0;

    if (LW_NULL == pvNode) {
        return  (PX_ERROR);
    }

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    _List_Line_Add_Ahead((PLW_LIST_LINE)pvNode,
                         (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[uiHashEntry]);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __asDefenseNodeDelete
** 功能描述: 设备结点删除
** 输　入  : uiIpAddr           IP 地址
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseNodeDelete (UINT32  uiIpAddr)
{
    UINT32            uiHashEntry = 0;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    pLineTemp   = _G_pvAsDefenseHashtable[uiHashEntry];
    for (; LW_NULL != pLineTemp ; pLineTemp = _list_line_get_next(pLineTemp)) {
        if (((__PAS_DEVINFO)pLineTemp)->uiIpAddr.addr == uiIpAddr) {    /*  找到对应的结点              */
            _List_Line_Del(pLineTemp, (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[uiHashEntry]);
            free(pLineTemp);
            break;
        }
    }
}

/*********************************************************************************************************
** 函数名称: __asDefenseNodeFind
** 功能描述: 设备结点查找
** 输　入  : uiIpAddr           IP 地址
** 输　出  : NONE
** 返　回  : pLineTemp          找到的结点
*********************************************************************************************************/
static PVOID  __asDefenseNodeFind (UINT32  uiIpAddr)
{
    UINT32            uiHashEntry = 0;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    pLineTemp   = _G_pvAsDefenseHashtable[uiHashEntry];
    for (; LW_NULL != pLineTemp; pLineTemp = _list_line_get_next(pLineTemp)) {
        if (((__PAS_DEVINFO)pLineTemp)->uiIpAddr.addr == uiIpAddr) {
            break;
        }
    }

    return  (PVOID)(pLineTemp);
}

/*********************************************************************************************************
** 函数名称: __asDefenseDeleteAll
** 功能描述: 删除所有结点
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseDeleteAll (VOID)
{
    INT32             i;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;
    PLW_LIST_LINE     pLineNext   = LW_NULL;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i]; LW_NULL  != pLineTemp; ) {
            pLineNext    = _list_line_get_next(pLineTemp);
            _List_Line_Del(pLineTemp, (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[i]);
            free(pLineTemp);
            pLineTemp = pLineNext;
        }
    }

    bzero(_G_pvAsDefenseHashtable, sizeof(_G_pvAsDefenseHashtable));
    __ASDEFENSE_UNLOCK();
}


/*********************************************************************************************************
** 函数名称: __asDefenseCheckTimer
** 功能描述: 定时器监测函数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseCheckTimer (VOID)
{
    INT32             i;
    UINT32            uiTimerCount;
    ip4_addr_t        tIpaddr;
    __PAS_DEVINFO     pNodeInfo   = LW_NULL;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;
    PLW_LIST_LINE     pLineNext   = LW_NULL;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i]; LW_NULL  != pLineTemp; ) {
            pLineNext    = _list_line_get_next(pLineTemp);
            pNodeInfo    = (__PAS_DEVINFO)pLineTemp;
            uiTimerCount = ++pNodeInfo->uiTimeCount;
            if ((_G_asdConfig.iNodeDelTime / _G_asdConfig.iNodeCheckHz) <= uiTimerCount) {
                /*
                 *  需要将此结点删除
                 */
                __asDefenseNodeDelete(pNodeInfo->uiIpAddr.addr);        /*  删除空闲结点                */
            } else if (ARP_ATTACK_FLAG == pNodeInfo->ucFlag &&
                       API_TimeGet64() - pNodeInfo->ullTimeOut > _G_asdConfig.iForbidTime * LW_TICK_HZ) {
                /*
                 *  如果被攻击IP的拉黑时间结束，则将标志设置为 conflict
                 */
                pNodeInfo->ucConMacCounter = 0;
                pNodeInfo->ucTrueMacCounter = 0;
                pNodeInfo->ullTimeOut      = 0;
                pNodeInfo->ucFlag          = ARP_CONFLICT_FLAG;

                tIpaddr.addr = htonl(pNodeInfo->uiIpAddr.addr);
                etharp_request(pNodeInfo->pvNetif, &tIpaddr);           /*  主动探测网络环境            */
            }

            pLineTemp = pLineNext;
        }
    }
    __ASDEFENSE_UNLOCK();
}

/*********************************************************************************************************
** 函数名称: __asDefenseShow
** 功能描述: 信息显示
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static INT32  __asDefenseShow (INT32  iArgC, PCHAR  ppcArgV[])
{
    INT32             i;
    struct in_addr    uiIpAddr;
    __PAS_DEVINFO     pasDevInfo  = LW_NULL;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    fprintf(stdout, "Arp Spoof Defense Info >>\n");
    fprintf(stdout, "    IP                TRUE MAC         CONFLICT MAC         STATUS       ALIAVE\n");
    fprintf(stdout, "--------------- ------------------- ------------------- ------------ ------------\n");

    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL != pLineTemp;
             pLineTemp  = _list_line_get_next(pLineTemp)) {
            pasDevInfo      = (__PAS_DEVINFO)pLineTemp;
            uiIpAddr.s_addr = htonl(pasDevInfo->uiIpAddr.addr);
            fprintf(stdout,
                    "%-15s  %02x:%02x:%02x:%02x:%02x:%02x   %02x:%02x:%02x:%02x:%02x:%02x%11s%11d\n",
                    inet_ntoa(uiIpAddr),
                    pasDevInfo->ucTrueMacAddr[0],
                    pasDevInfo->ucTrueMacAddr[1],
                    pasDevInfo->ucTrueMacAddr[2],
                    pasDevInfo->ucTrueMacAddr[3],
                    pasDevInfo->ucTrueMacAddr[4],
                    pasDevInfo->ucTrueMacAddr[5],
                    pasDevInfo->ucConMacAddr[0],
                    pasDevInfo->ucConMacAddr[1],
                    pasDevInfo->ucConMacAddr[2],
                    pasDevInfo->ucConMacAddr[3],
                    pasDevInfo->ucConMacAddr[4],
                    pasDevInfo->ucConMacAddr[5],
                    (ARP_NORMAL_FLAG == pasDevInfo->ucFlag) ? ARP_NORMAL_STR :
                    ((ARP_CONFLICT_FLAG == pasDevInfo->ucFlag) ? ARP_CONFLICT_STR : ARP_ATTACK_STR),
                    pasDevInfo->uiTimeCount);
        }
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __asDefensePacketCheck
** 功能描述: ARP 欺骗检测
** 输　入  : pcPacket           报文
**           pNetdev            网卡结构
** 输　出  : NONE
** 返　回  : PX_ERROR           发生错误
**           LW_TRUE            报文允许通过
**           LW_FALSE           报文不允许通过
*********************************************************************************************************/
static INT32  __asDefensePacketCheck (UINT8  *pcPacket, struct netdev  *pNetdev)
{
    ip4_addr_t             tIpaddr;
    struct netif          *pNetif;
    UINT8                  ucArpPass    = LW_FALSE;
    UINT32                 uiSrcIpAddr, uiDesIpAddr;
    INT                    iTmpVal      = 0;

    __PAS_DEVINFO          pArpInfo = LW_NULL;
    struct etharp_hdr     *pArphdr  = LW_NULL;

    if (LW_NULL == pcPacket || LW_NULL == pNetdev) {
        return  (LW_TRUE);
    }

    pArphdr = (struct etharp_hdr *)(pcPacket + sizeof(struct eth_hdr));
    pNetif  = (struct netif *)&(pNetdev->sys[0]);

    /*
     *  前后16位组成32位IP地址
     */
    uiSrcIpAddr  = (ntohs(pArphdr->sipaddr.addrw[0]) << 16) | ntohs(pArphdr->sipaddr.addrw[1]);
    uiDesIpAddr  = (ntohs(pArphdr->dipaddr.addrw[0]) << 16) | ntohs(pArphdr->dipaddr.addrw[1]);

    /*
     *  对给自己的ARP报文做处理
     */
    if ((ARP_REPLY   == ntohs(pArphdr->opcode))                   ||
        (uiDesIpAddr == (ntohl(pNetif->ip_addr.u_addr.ip4.addr))  &&
        (ARP_REQUEST == ntohs(pArphdr->opcode)))) {
        __ASDEFENSE_LOCK();
        pArpInfo = __asDefenseNodeFind(uiSrcIpAddr);
        if (LW_NULL == pArpInfo) {                                      /*  找不到对应的设备信息        */
            /*
             *  创建一个新的结点，并加进管理链表
             */
            pArpInfo = (__PAS_DEVINFO)malloc(sizeof(__AS_DEVINFO));
            if (LW_NULL == pArpInfo) {                                  /*  申请内存失败                */
                __ASDEFENSE_UNLOCK();

                return  (LW_TRUE);                                      /*  报文允许通过,协议栈处理     */
            }

            /*
             *  第一个报文设置为冲突状态,从而可以进行判断当前此设备真正的网络环境
             */
            bzero(pArpInfo, sizeof(__AS_DEVINFO));
            pArpInfo->uiIpAddr.addr   = uiSrcIpAddr;                    /*  保存ip地址                  */
            pArpInfo->ucFlag          = ARP_CONFLICT_FLAG;
            memcpy(pArpInfo->ucConMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN);
            memcpy(pArpInfo->ucTrueMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN);

            __asDefenseNodeAdd(uiSrcIpAddr, pArpInfo);

        } else {                                                        /*  存在当前设备信息            */
            pArpInfo->uiTimeCount = 0;
            if (ARP_ATTACK_FLAG == pArpInfo->ucFlag) {
                goto  __back;
            }

            /*
             *  报文MAC 等于 True_mac
             */
            if (!memcmp(pArpInfo->ucTrueMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN)) {
                pArpInfo->ucTrueMacCounter++;
                if (ARP_NORMAL_FLAG == pArpInfo->ucFlag) {
                    ucArpPass = 1;                                      /*  报文允许通过                */
                }
            } else {
                if (!memcmp(pArpInfo->ucConMacAddr,pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN)) {
                    /*
                     *   报文MAC 等于 new_mac
                     */
                    pArpInfo->ucConMacCounter++;
                } else {                                                /*  与当前新的MAC也不相符       */
                    memcpy(pArpInfo->ucConMacAddr,
                           pArphdr->shwaddr.addr,
                           ETHARP_HWADDR_LEN);                          /*  更新New MAC数组             */
                    pArpInfo->ucConMacCounter = 1;
                }
            }
        }

        if (ARP_NORMAL_FLAG == pArpInfo->ucFlag) {
            if (pArpInfo->ucConMacCounter) {
                pArpInfo->ucFlag          = ARP_CONFLICT_FLAG;          /*  发生冲突                    */
                pArpInfo->ucConMacCounter = 0;
                pArpInfo->ucTrueMacCounter = 0;
                if (pArpInfo->ullTimeOut &&
                    ((API_TimeGet64() - pArpInfo->ullTimeOut) <=
                      _G_asdConfig.iConToSpoTime * LW_TICK_HZ)) {
                    /*
                     *  两次冲突的时间很靠近，则认为是被攻击
                     *  将计数设置为欺骗的计数值，则下一个报文就会被设置为欺骗状态
                     */
                    pArpInfo->ucConMacCounter = _G_asdConfig.iArpSpoofVal;
                    pArpInfo->ucTrueMacCounter = _G_asdConfig.iArpSpoofVal;
                } else {
                    pArpInfo->ullTimeOut = API_TimeGet64();
                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                    __ASD_LOG("warning: a device(%s) get a different MAC. "
                              "the different mac is %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                              inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                              pArpInfo->ucConMacAddr[0],
                              pArpInfo->ucConMacAddr[1],
                              pArpInfo->ucConMacAddr[2],
                              pArpInfo->ucConMacAddr[3],
                              pArpInfo->ucConMacAddr[4],
                              pArpInfo->ucConMacAddr[5]);
                }
            } else {
                pArpInfo->ucTrueMacCounter = 1;                          /*  否则让 old 计数一直保持为1  */
            }
        } else {                                                        /*  如果是冲突结点              */
            if (!pArpInfo->ucTrueMacCounter) {
                /*
                 *  认为 MAC 发生了变化
                 */
                if (_G_asdConfig.iMacChangeVal <= pArpInfo->ucConMacCounter) {
                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                    __ASD_LOG("the device(%s) mac has changed from %02x:%02x:%02x:%02x:%02x:%02x to "
                              "%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                              inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                              pArpInfo->ucTrueMacAddr[0],
                              pArpInfo->ucTrueMacAddr[1],
                              pArpInfo->ucTrueMacAddr[2],
                              pArpInfo->ucTrueMacAddr[3],
                              pArpInfo->ucTrueMacAddr[4],
                              pArpInfo->ucTrueMacAddr[5],
                              pArpInfo->ucConMacAddr[0],
                              pArpInfo->ucConMacAddr[1],
                              pArpInfo->ucConMacAddr[2],
                              pArpInfo->ucConMacAddr[3],
                              pArpInfo->ucConMacAddr[4],
                              pArpInfo->ucConMacAddr[5]);
                    memcpy(pArpInfo->ucTrueMacAddr,
                           pArpInfo->ucConMacAddr,
                           sizeof(pArpInfo->ucTrueMacAddr));
                    bzero(pArpInfo->ucConMacAddr, sizeof(pArpInfo->ucConMacAddr));
                    pArpInfo->ucFlag          = ARP_NORMAL_FLAG;        /*  恢复正常                    */
                    pArpInfo->ucConMacCounter = 0;
                    pArpInfo->ucTrueMacCounter = 0;

                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                }
            } else {
                if (!pArpInfo->ucConMacCounter) {
                    if (_G_asdConfig.iMacChangeVal <= pArpInfo->ucTrueMacCounter) {
                        bzero(pArpInfo->ucConMacAddr, sizeof(pArpInfo->ucConMacAddr));
                        pArpInfo->ucFlag           = ARP_NORMAL_FLAG;
                        pArpInfo->ucConMacCounter  = 0;
                        pArpInfo->ucTrueMacCounter = 0;
                    }
                } else {
                    iTmpVal = abs(pArpInfo->ucTrueMacCounter - pArpInfo->ucConMacCounter);
                    if ((_G_asdConfig.iArpSpoofVal <= pArpInfo->ucConMacCounter  &&
                         _G_asdConfig.iArpSpoofVal <= pArpInfo->ucTrueMacCounter) ||
                         _G_asdConfig.iArpSpoofVal <= iTmpVal) {        /*  认为遭受到攻击              */
                        pArpInfo->ucFlag          = ARP_ATTACK_FLAG;
                        pArpInfo->pvNetif         = (PVOID)pNetif;
                        pArpInfo->ucConMacCounter = 0;
                        pArpInfo->ucTrueMacCounter = 0;
                        pArpInfo->ullTimeOut      = API_TimeGet64();
                        /*
                         *  为了防御当前的系统 ARP 表里的条目是错误的信息，
                         *  这里删除 ARP 表的全部内容，此后，存在欺骗风险的
                         *  ARP 信息不会存在系统 ARP 里
                         */
                        system("arp -d");
                        tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                        __ASD_LOG("wraning: device(%s) may be spoofed, "
                                  "the first mac is %02x:%02x:%02x:%02x:%02x:%02x, "
                                  "another mac is %02x:%02x:%02x:%02x:%02x:%02x. "
                                  "The device is blacked out.\r\n",
                                  inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                                  pArpInfo->ucTrueMacAddr[0],
                                  pArpInfo->ucTrueMacAddr[1],
                                  pArpInfo->ucTrueMacAddr[2],
                                  pArpInfo->ucTrueMacAddr[3],
                                  pArpInfo->ucTrueMacAddr[4],
                                  pArpInfo->ucTrueMacAddr[5],
                                  pArpInfo->ucConMacAddr[0],
                                  pArpInfo->ucConMacAddr[1],
                                  pArpInfo->ucConMacAddr[2],
                                  pArpInfo->ucConMacAddr[3],
                                  pArpInfo->ucConMacAddr[4],
                                  pArpInfo->ucConMacAddr[5]);
                    }
                }
            }
        }

        if (ARP_CONFLICT_FLAG == pArpInfo->ucFlag) {
            tIpaddr.addr = htonl(uiSrcIpAddr);
            etharp_request(pNetif, &tIpaddr);                           /*  再次发送ARP请求             */
        }

__back:
        __ASDEFENSE_UNLOCK();

        /*
         *  如果不是正常状态或者是收到的第一个ARP报文，都不允许通过
         */
        return  (ucArpPass ? LW_TRUE : LW_FALSE);
    }

    return  (LW_TRUE);
}

/*********************************************************************************************************
** 函数名称: __asDefenseEntry
** 功能描述: arp 防御入口
** 输　入  : inp                网卡结构
**           pPbuf              报文数据
** 输　出  : 报文是否被eat
** 返　回  : PX_ERROR           报文不允许通过
**           ERROR_NONE         报文可以通过
*********************************************************************************************************/
static INT32 __asDefenseEntry (struct netdev  *inp, struct pbuf *pPbuf)
{
    INT                 iOffset      = 0;                               /*  数据的起始偏移量            */
    UINT8              *pucDrvPacket = LW_NULL;                         /*  驱动报文                    */
    struct eth_hdr     *pEthhdr      = LW_NULL;                         /*  eth 头                      */

    if (ARP_SPOOF_DEFENSE_DISABLE == _G_ucArpSpoofDefenseEn) {
        goto __allow_input;                                             /*  没有开启任何操作，返回正确  */
    }

    if (LW_NULL == pPbuf || pPbuf->tot_len < sizeof(struct eth_hdr)) {
        goto __allow_input;                                             /*  协议栈处理                  */
    }

    pucDrvPacket = (UINT8 *)(pPbuf->payload);                           /*  获取报文内容                */

    /*
     *  ethernet 过滤处理
     */
    if (inp->net_type & NETDEV_TYPE_ETHERNET) {                         /*  以太网接口                  */
        pEthhdr = (struct eth_hdr *)(pucDrvPacket + iOffset);
        iOffset = sizeof(struct eth_hdr);
    }

    if (iOffset == sizeof(struct eth_hdr)) {                            /*  前面有 eth hdr 字段         */
        /*
         *  todo: 对VLAN报文需要做一次判断，详细判断参考npf
         */
        if (pEthhdr->type == PP_HTONS(ETHTYPE_ARP)) {
            if (LW_FALSE == __asDefensePacketCheck(pucDrvPacket, inp)) {
                goto __drop_input;
            }
        } else {
            goto    __allow_input;                                      /*  其他报文, 放行              */
        }
    } else {
        goto    __allow_input;                                          /*  没有以太网字段, 放行        */
    }

__allow_input:
    return  (0);                                                        /*  允许输入                    */

__drop_input:
    netdev_linkinfo_drop_inc(inp);
    pbuf_free(pPbuf);                                                   /*  释放数据包                  */
    return  (1);                                                        /*  eaten                       */
}

/*********************************************************************************************************
** 函数名称: __asDefenseInstall
** 功能描述: 安装 ARP 欺骗防御
** 输　入  : inp                网卡结构
**           ucDrvPacket        网卡收到的报文
**           usPacketLen        报文长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT32 __asDefenseInstall (struct netdev *pNetdev,
                                 PVOID arg0, PVOID arg1, PVOID arg2,
                                 PVOID arg3, PVOID arg4, PVOID arg5)
{
    netdev_firewall(pNetdev, __asDefenseEntry);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __asDefenseUnInstall
** 功能描述: 卸载装 ARP 欺骗防御
** 输　入  : inp                网卡结构
**           ucDrvPacket        网卡收到的报文
**           usPacketLen        报文长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT32 __asDefenseUnInstall (struct netdev *pNetdev,
                                   PVOID arg0, PVOID arg1, PVOID arg2,
                                   PVOID arg3, PVOID arg4, PVOID arg5)
{
    /*
     *  todo:这里考虑是否要恢复成之前设置的接口
     */
    netdev_firewall(pNetdev, LW_NULL);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __fireWallInstall
** 功能描述: 安装防火墙接口
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
static VOID __fireWallInstall (VOID)
{
    netdev_foreache(__asDefenseInstall, 0, 0, 0, 0, 0, 0);              /*  遍历网卡添加防火墙代码      */
}

/*********************************************************************************************************
** 函数名称: __fireWallUninstall
** 功能描述: 卸载防火墙接口
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
static VOID __fireWallUninstall (VOID)
{
    netdev_foreache(__asDefenseUnInstall, 0, 0, 0, 0, 0, 0);            /*  遍历网卡卸载防火墙代码      */
}

/*********************************************************************************************************
** 函数名称: __asDefenseEn
** 功能描述: 开启 / 关闭 ARP 防御
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static INT32  __asDefenseEn (INT32  iArgC, PCHAR  ppcArgV[])
{
    if (2 != iArgC) {
        printk("\nVersion: %s\n", ARP_SPOOF_DEFENSE_VERSION);
        printk("Usage: asden 1 (enable arp spoof defense)\n");
        printk("       asden 0 (disable arp spoof defense)\n");
        printk("       asdshow (show arp spoof defense info)\n");

        return  (ERROR_NONE);
    }

    if (atoi(ppcArgV[1])) {
        _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_ENABLE;
        __fireWallInstall();                                            /*  安装网络防火墙              */

        __ASD_LOG("arp spoof defense start.\r\n");
    } else {
        _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;
        __fireWallUninstall();
        __asDefenseDeleteAll();

        __ASD_LOG("arp spoof defense stop.\r\n");
    }

    system("arp -d");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __iniCfgInit
** 功能描述: 配置文件初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
static INT32  __iniCfgInit (VOID)
{
    PVOID    pCfg = LW_NULL;
    PVOID    pLog = LW_NULL;

    bzero(&_G_asdConfig, sizeof(_G_asdConfig));
    pCfg = asdCfgLoad(ARP_SPOOF_DEFENSE_CFG, CFG_SECTOR);
    if (!pCfg) {
        printk("[arpSpoofDefense] Load INI file Failed.\n");

        return  (PX_ERROR);
    }

    pLog = asdCfgLoad(ARP_SPOOF_DEFENSE_CFG, LOG_SECTOR);
    if (!pCfg) {
        printk("[arpSpoofDefense] Load INI file Failed.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iMacChangeVal = asdCfgGetInt(pCfg, MAC_CHANGE_VAL, -1);
    if (_G_asdConfig.iMacChangeVal < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found mac_change_val setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iArpSpoofVal = asdCfgGetInt(pCfg, ARP_SPOOF_VAL, -1);
    if (_G_asdConfig.iArpSpoofVal < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found arp_spoof_val setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iConToSpoTime = asdCfgGetInt(pCfg, CON_TO_SPO_TIME, -1);
    if (_G_asdConfig.iConToSpoTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found conflict_to_spoof_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iForbidTime = asdCfgGetInt(pCfg, SPOOF_FORBID_TIME, -1);
    if (_G_asdConfig.iForbidTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found spoof_forbid_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iNodeDelTime = asdCfgGetInt(pCfg, NODE_DEL_TIME, -1);
    if (_G_asdConfig.iNodeDelTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found node_del_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iNodeCheckHz = asdCfgGetInt(pCfg, NODE_CHECK_HZ, -1);
    if (_G_asdConfig.iNodeCheckHz < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found node_check_hz setting.\n");

        return  (PX_ERROR);
    }

    asdCfgUnload(pCfg);
    _G_asdConfig.iLogFilelen = asdCfgGetInt(pLog, LOG_FILE_LEN_INI, -1);
    if (_G_asdConfig.iLogFilelen < 0) {
        asdCfgUnload(pLog);
        printk("[arpSpoofDefense] Can't found logfile_len setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.pcLogFile = (PCHAR)malloc(LOG_FILE_NAME_LEN);
    if (!_G_asdConfig.pcLogFile) {
        return  (PX_ERROR);
    }
    _G_asdConfig.pcLogFile = asdCfgGetString(pLog, LOG_FILE_INI, NULL);
    if (!_G_asdConfig.pcLogFile) {
        asdCfgUnload(pLog);
        free(_G_asdConfig.pcLogFile);
        printk("[arpSpoofDefense] Can't found logfile setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.pcLogFileBk = (PCHAR)malloc(LOG_FILE_NAME_LEN);
    if (!_G_asdConfig.pcLogFileBk) {
        free(_G_asdConfig.pcLogFile);

        return  (PX_ERROR);
    }
    _G_asdConfig.pcLogFileBk = asdCfgGetString(pLog, LOG_FILE_BK_INI, NULL);
    if (!_G_asdConfig.pcLogFileBk) {
        asdCfgUnload(pLog);
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);
        printk("[arpSpoofDefense] Can't found logfile setting.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: arpSpoofDefenseInit
** 功能描述: ARP 欺骗防御初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT32  arpSpoofDefenseInit (VOID)
{
    if (__iniCfgInit() != ERROR_NONE) {
        return  (PX_ERROR);
    }

    if (__logInit() != ERROR_NONE) {
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);

        return  (PX_ERROR);
    }

    /*
     *  资源初始化
     */
    bzero(_G_pvAsDefenseHashtable, sizeof(_G_pvAsDefenseHashtable));
    _G_ulAsDefenseLock = API_SemaphoreBCreate("sem_arpdefenselcok",
                                              LW_TRUE,
                                              LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL,
                                              LW_NULL);                 /*  创建锁                      */
    if (LW_OBJECT_HANDLE_INVALID == _G_ulAsDefenseLock) {
        fprintf(stdout, "asdefense: create lock error.\r\n");
        close(_G_iLogFd);
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);

        return  (PX_ERROR);
    }

    _G_ulAsDefenseTimer = API_TimerCreate("asdefense_timer",
                                       LW_OPTION_ITIMER, LW_NULL);
    API_TimerStart(_G_ulAsDefenseTimer,
                   (LW_TICK_HZ * _G_asdConfig.iNodeCheckHz),
                   LW_OPTION_AUTO_RESTART,
                   (PTIMER_CALLBACK_ROUTINE)__asDefenseCheckTimer,
                   LW_NULL);                                            /*  启动检测定时器              */

    _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;                 /*  默认关闭 ARP 防御           */

    /*
     *  注册 shell 命令
     */
    API_TShellKeywordAdd("asdshow", __asDefenseShow);
    API_TShellHelpAdd("asdshow", "show arp spoof defense info.\n");

    API_TShellKeywordAdd("asden", __asDefenseEn);
    API_TShellHelpAdd("asden", "show arp spoof defense info.\n");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: module_init
** 功能描述: ARP 欺骗防御模块加载
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROE_CODE
*********************************************************************************************************/
INT32  module_init (void)
{
    INT32   iRet;
    iRet = arpSpoofDefenseInit();

    if (ERROR_NONE == iRet) {
        printk("arp spoof defense module init success.\r\n");
    } else {
        printk("arp spoof defense module init failed.\r\n");
    }

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: module_exit
** 功能描述: ARP 欺骗防御模块卸载
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
void module_exit (void)
{
    _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;
    __fireWallUninstall();
    __asDefenseDeleteAll();

    API_SemaphoreBDelete(&_G_ulAsDefenseLock);
    API_TimerDelete(&_G_ulAsDefenseTimer);

    __ASD_LOG("arp spoof defense module exit.\r\n");
    if (_G_iLogFd > 0) {
        close(_G_iLogFd);
    }
}

/*********************************************************************************************************
** 函数名称: asDefenseStatus
** 功能描述: ARP 欺骗防御开启与关闭状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : _G_ucArpSpoofDefenseEn
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32 asDefenseStatus (VOID)
{
    return  (_G_ucArpSpoofDefenseEn);
}

/*********************************************************************************************************
** 函数名称: asDefenseGetSpecInfoNum
** 功能描述: 获取 ARP 欺骗防御指定信息个数
** 输　入  : ucInfoType         信息类型 (全部信息 / 有问题信息)
** 输　出  : NONE
** 返　回  : uiInfoNum          信息个数
*********************************************************************************************************/
LW_SYMBOL_EXPORT UINT32 asDefenseGetSpecInfoNum (UINT8  ucInfoType)
{
    UINT32            uiInfoNum  = 0;
    PLW_LIST_LINE     pLineTemp  = LW_NULL;
    INT32             i;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL  != pLineTemp;
             pLineTemp = _list_line_get_next(pLineTemp)) {
            if (ASDEFENSE_GET_ALL == ucInfoType) {
                uiInfoNum++;
            } else if (ASDEFENSE_GET_SPEC == ucInfoType &&
                       (ARP_NORMAL_FLAG != ((__PAS_DEVINFO)pLineTemp)->ucFlag)) {
                uiInfoNum++;
            }
        }
    }
    __ASDEFENSE_UNLOCK();
    return  (uiInfoNum);
}

/*********************************************************************************************************
** 函数名称: asDefenseGetSpecInfo
** 功能描述: 获取 ARP 欺骗防御指定信息
** 输　入  : iStartIndex        开始条目数
**           iEndIndex          结束条目数
**           ucInfoType         类型
**           pvLogBuff          buff 地址
**           iBuffLen           buff 长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32 asDefenseGetSpecInfo (INT32  iStartIndex,
                                             INT32  iEndIndex,
                                             UINT8  ucInfoType,
                                             PCHAR  pcBuff,
                                             INT32  iBuffLen)
{
    INT32                 i;
    UINT32                uiInfoNum     = 0;
    PCHAR                 pcBuffTmp     = pcBuff;
    __PAS_DEVINFO         pAsMangerInfo = LW_NULL;
    __PASDEFENSE_INFO     pAsNodeInfo   = LW_NULL;
    PLW_LIST_LINE         pLineTemp     = LW_NULL;

    if (LW_NULL == pcBuff) {
        return  (PX_ERROR);
    }

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL  != pLineTemp;
             pLineTemp = _list_line_get_next(pLineTemp)) {
            /*
             *  先找到对应类型的结点
             */
            if (ASDEFENSE_GET_ALL == ucInfoType) {
                uiInfoNum++;
            } else if (ASDEFENSE_GET_SPEC == ucInfoType &&
                       (ARP_NORMAL_FLAG != ((__PAS_DEVINFO)pLineTemp)->ucFlag)) {
                uiInfoNum++;
            } else {
                continue;
            }

            if (uiInfoNum >= iStartIndex && uiInfoNum <= iEndIndex) {   /*  需要进行结点信息的获取      */
                pAsMangerInfo = (__PAS_DEVINFO)pLineTemp;
                pAsNodeInfo   = (__PASDEFENSE_INFO)pcBuff;
                if (pcBuff - pcBuffTmp > iBuffLen) {                    /*  buff 溢出                   */
                    __ASDEFENSE_UNLOCK();

                    return  (PX_ERROR);
                }

                /*
                 *  获取信息
                 */
                pAsNodeInfo->uiIpAddr.addr = pAsMangerInfo->uiIpAddr.addr;
                pAsNodeInfo->ucFlag        = pAsMangerInfo->ucFlag;
                memcpy(pAsNodeInfo->ucTrueMacAddr,
                       pAsMangerInfo->ucTrueMacAddr,
                       sizeof(pAsNodeInfo->ucTrueMacAddr));
                memcpy(pAsNodeInfo->ucConMacAddr,
                       pAsMangerInfo->ucConMacAddr,
                       sizeof(pAsNodeInfo->ucConMacAddr));

                pcBuff += sizeof(__ASDEFENSE_INFO);
            }
        }
    }

    __ASDEFENSE_UNLOCK();

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
