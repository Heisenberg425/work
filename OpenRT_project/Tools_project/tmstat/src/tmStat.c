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
 ** 文   件   名: tmStat.c
 **
 ** 创   建   人: Chen.Hongbang (陈洪邦)
 **
 ** 文件创建日期: 2018 年 7 月 11 日
 **
 ** 描        述: OpenRT 流量监控管理统计操作
 *********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <time.h>
#include <stdio.h>
#include "lwip/ip.h"
#include "lwip/prot/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include <ifaddrs.h>
#include "netdev.h"
#include "net/if.h"
#include "net/lwip/lwip_if.h"
#include "common.h"
#include "tmStat.h"
#include "tmStatItem.h"
#include "tmHash.h"
#include "socket.h"
#include "net/if_iphook.h"
#include "lwip/etharp.h"
/*********************************************************************************************************
  静态局部变量
*********************************************************************************************************/
static char _G_cWanName[LW_CFG_NET_NAT_MAX_AP_IF][NETIF_NAMESIZE];      /*  保存 WAN 网口名数组         */
static char _G_cLanName[LW_CFG_NET_NAT_MAX_LOCAL_IF][NETIF_NAMESIZE];   /*  保存 LAN 网口名数组         */
/*********************************************************************************************************
 ** 函数名称: tmNotifyWanChange
 ** 功能描述: 通知 WAN 变更
 ** 输　入  : pvWan    WAN 信息
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tmNotifyWanChange (PVOID  pvWan)
{
    memcpy(_G_cWanName, pvWan, sizeof(_G_cWanName));
}

/*********************************************************************************************************
 ** 函数名称: tmNotifyLanChange
 ** 功能描述: 通知 LAN 变更
 ** 输　入  : pvLan    LAN 信息
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tmNotifyLanChange (PVOID  pvLan)
{
    memcpy(_G_cLanName, pvLan, sizeof(_G_cLanName));
}
/*********************************************************************************************************
 ** 函数名称: __ipAddrGetByIfName
 ** 功能描述: 根据网口名获取 IP 地址
 ** 输　入  : cpcIfName      网口信息
 **           uiAddrLen      地址长度
 ** 输　出  : pcIpAddr       保存 IP 信息缓冲区
 ** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __ipAddrGetByIfName (CPCHAR  cpcIfName, PCHAR  pcIpAddr, UINT32  uiAddrLen)
{
    INT                  iRet;
    struct ifreq         ifr;
    struct sockaddr_in  *pSockIn;
    INT                  iSockFd;

    if (NULL == cpcIfName ||
        '\0' == cpcIfName[0] ||
        NULL == pcIpAddr) {
        return  (PX_ERROR);
    }

    iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSockFd < 0) {
        return  (PX_ERROR);
    }

    strncpy(ifr.ifr_name, cpcIfName, sizeof(ifr.ifr_name));

    iRet = ioctl(iSockFd, SIOCGIFADDR, &ifr);
    if (iRet < 0) {
        printf("Get SIOCGIFADDR Failed.\n");

        close(iSockFd);

        return (PX_ERROR);
    }

    close(iSockFd);

    pSockIn = (struct sockaddr_in *) &(ifr.ifr_addr);
    if (NULL == inet_ntoa(pSockIn->sin_addr)) {
        return (PX_ERROR);
    }

    strncpy(pcIpAddr, inet_ntoa(pSockIn->sin_addr), uiAddrLen);

    return (ERROR_NONE);
}
/*********************************************************************************************************
 ** 函数名称: tmWalkStats
 ** 功能描述: 统计控制哈希表遍历函数
 ** 输　入  : INT (*callback)     遍历时执行的回调函数
 **           pvArg0              回调函数第一个参数
 **           pvArg1              回调函数第二个参数
 **           pvArg2              回调函数第三个参数
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmWalkStats (INT    (*callback)(),
                   PVOID    pvArg0,
                   PVOID    pvArg1,
                   PVOID    pvArg2)
{
    PTM_STATS_INNER  ptmStat;
    PLW_LIST_LINE    plineTemp   = NULL;
    PLW_LIST_LINE    plineDel    = NULL;
    PLW_LIST_LINE    plineHeader = NULL;
    INT              iAction     = 0;
    INT              i;

    for (i = 0; i < TM_STAT_CB_SIZE; i++) {
        plineHeader = (PLW_LIST_LINE)__G_tmLineStatCbtable[i];
        plineTemp = plineHeader;
        while (plineTemp) {
            ptmStat = &((PTM_STAT_ENTRY)plineTemp)->tm_stats;
            iAction = callback(ptmStat, pvArg0, pvArg1, pvArg2);

            if (STAT_ITEM_PARAM_INVALID == iAction) {                   /*  参数异常                    */
                break;
            }

            if (STAT_ITEM_SHOULD_DELETE == iAction) {                   /*  删除统计项                  */

                /*
                 *  s1 从链表中删除符合条件的节点
                 *  s2 如果待删除节点是头节点，则将 header->next 赋值给 __G_tmStatCbtable[i]
                 */
                plineDel  = plineTemp;
                plineTemp = _list_line_get_next(plineTemp);
                if (plineDel->LINE_plistPrev == LW_NULL) {
                    __G_tmLineStatCbtable[i] = (PTM_STAT_ENTRY) plineTemp;
                }

                _List_Line_Del(plineDel, &plineHeader);

                sys_free(plineDel);

                plineDel = NULL;
            } else {
                plineTemp = _list_line_get_next(plineTemp);
            }
        }

        if (STAT_ITEM_PARAM_INVALID == iAction) {
            break;
        }
    }

    for (i = 0; i < TM_STAT_CB_SIZE; i++) {
        plineHeader = (PLW_LIST_LINE) __G_tmStatCbtable[i];
        plineTemp   = plineHeader;
        while (plineTemp) {
            ptmStat = &((PTM_STAT_ENTRY)plineTemp)->tm_stats;
            iAction = callback(ptmStat, pvArg0, pvArg1, pvArg2);

            if (iAction == STAT_ITEM_PARAM_INVALID) {                   /*  参数异常                    */
                break;
            }

            if (iAction == STAT_ITEM_SHOULD_DELETE) {                   /*  删除统计项                  */

                /*
                 *  s1 从链表中删除符合条件的节点
                 *  s2 如果待删除节点是头节点，则将 header->next 赋值给 __G_tmStatCbtable[i]
                 */
                plineDel  = plineTemp;
                plineTemp = _list_line_get_next(plineTemp);

                if (plineDel->LINE_plistPrev == LW_NULL) {
                    __G_tmStatCbtable[i] = (PTM_STAT_ENTRY)plineTemp;
                }

                _List_Line_Del(plineDel, &plineHeader);

                sys_free(plineDel);

                plineDel = NULL;
            } else {
                plineTemp = _list_line_get_next(plineTemp);
            }
        }

        if (STAT_ITEM_PARAM_INVALID == iAction) {
            break;
        }
    }
}

/*********************************************************************************************************
 ** 函数名称: __tmCalcRate
 ** 功能描述: 周期性（2s）的计算统计发送、接受速率
 ** 输　入  : ptmStat     monitor 参数信息
 **           pvArg0      暂未用到
 **           pvArg1      暂未用到
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
static INT  __tmCalcRate (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    struct timespec  tsMono;                                            /*  单调递增时间                */

    ptmStat->iSendRate    = (ptmStat->iSendCurrLen) / (TM_CALC_PERIOD);
    ptmStat->iRecvRate    = (ptmStat->iRecvCurrLen) / (TM_CALC_PERIOD);
    ptmStat->iSendCurrLen = 0;
    ptmStat->iRecvCurrLen = 0;

    if (TMT_TERM == ptmStat->cType) {
        lib_clock_gettime(CLOCK_MONOTONIC, &tsMono);

        ptmStat->iDuration = (UINT32) ((tsMono.tv_sec - ptmStat->iJoinTimestamp) / 60);
    }

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: tmStatCalcRate
 ** 功能描述: 计数所有统计项的速率
 ** 输　入  : NONE
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmStatCalcRate (VOID)
{
    tmWalkStats(__tmCalcRate, LW_NULL, LW_NULL, LW_NULL);
}

/*********************************************************************************************************
 ** 函数名称: __tmStatCountSetZero
 ** 功能描述: 清空统计数据
 ** 输　入  : ptmStat    统计项
 **           pvArg0     暂未用到
 **           pvArg1     暂未用到
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
static INT  __tmStatCountSetZero (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    ptmStat->iSendRate         = 0;
    ptmStat->iRecvRate         = 0;
    ptmStat->iSendCurrLen      = 0;
    ptmStat->iRecvCurrLen      = 0;
    ptmStat->iDuration         = 0;
    ptmStat->iSendTotalLen     = 0;
    ptmStat->iRecvTotalLen     = 0;
    ptmStat->iSendDrops        = 0;
    ptmStat->iRecvDrops        = 0;
    ptmStat->iSendTotalPackets = 0;
    ptmStat->iRecvTotalPackets = 0;
    ptmStat->iJoinTimestamp    = 0;

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: tmStatCountReset
 ** 功能描述: 统计计数清零
 ** 输　入  : NONE
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmStatCountReset (VOID)
{
    tmWalkStats(__tmStatCountSetZero, LW_NULL, LW_NULL, LW_NULL);
}

/*********************************************************************************************************
 ** 函数名称: __tmPrintStatInfo
 ** 功能描述: 统计控制信息打印输出
 ** 输　入  : ptmStat   待输出的统计信息
 **           pvArg0    类型
 **           pvArg1    IP 信息
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
static INT  __tmPrintStatInfo (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    CHAR                pStrUserIP[IP4ADDR_STRLEN_MAX];
    CHAR                pIpAddr[IP4ADDR_STRLEN_MAX];
    INT                 iType    = *(INT*)pvArg0;
    ip4_addr_t         *pip4Addr = (ip4_addr_t*)pvArg1;
    struct sockaddr_in  tmpAddr;

    if (iType == 0 || ptmStat->cType == iType) {
        if ((LW_NULL == pip4Addr) ||
           ((LW_NULL != pip4Addr) &&
            (ptmStat->stIpAddr.addr == pip4Addr->addr))) {
            tmpAddr.sin_len    = sizeof(struct sockaddr_in);
            tmpAddr.sin_family = AF_INET;

            inet_addr_from_ip4addr(&((struct sockaddr_in * )&tmpAddr)->sin_addr, &(ptmStat->stIpAddr));
            inet_ntoa_r(((struct sockaddr_in *)&tmpAddr)->sin_addr, pStrUserIP, IP4ADDR_STRLEN_MAX);

            if (TMT_TERM != ptmStat->cType) {
                __ipAddrGetByIfName(ptmStat->pIfname, pIpAddr, sizeof(pIpAddr));
            }

            printf("%s \t %6s   %02x:%02x:%02x:%02x:%02x:%02x\t  %u\t "
                    " %u\t %u\t "
                    " %10u\t %10u\t  %15llu   \t "
                    " %15llu\t %15llu\t  %15llu   \t "
                    " %10u\t %10u\n\n",
                    pIpAddr,
                    ptmStat->pIfname,
                    ptmStat->pMac[0],
                    ptmStat->pMac[1],
                    ptmStat->pMac[2],
                    ptmStat->pMac[3],
                    ptmStat->pMac[4],
                    ptmStat->pMac[5],
                    ptmStat->cType,
                    ptmStat->cProtocol,
                    ntohs(ptmStat->usPort),
                    ptmStat->iRecvCurrLen,
                    ptmStat->iSendCurrLen,
                    ptmStat->iRecvTotalLen,
                    ptmStat->iSendTotalLen,
                    ptmStat->iRecvTotalPackets,
                    ptmStat->iSendTotalPackets,
                    ptmStat->iRecvRate,
                    ptmStat->iSendRate);
        }
    }

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: tmShowStats
 ** 功能描述: 打印所有统计信息
 ** 输　入  : iType     过滤条件类型：0 表示所有, 其他参照 TMT_*, 如 TMT_TERM 等
 **           pip4Addr  过滤条件 ip 地址
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmShowStats (INT  iType, ip4_addr_t  *pip4Addr)
{
    printf("Show traffic statistic:\n");

    printf("IP\t  ifname\t      MAC\t        TYPE\t  "
            "protocol\t port\t "
            "recv_curr_len\t send_curr_len\t recv_total_len(Bytes)\t"
            "send_total_len(Bytes)\trecv_total_packets\t send_total_packets\t"
            " recv_rate\t send_rate \n\n");

    tmWalkStats(__tmPrintStatInfo, &iType, pip4Addr, LW_NULL);
}

/*********************************************************************************************************
 ** 函数名称: __tmAssign
 ** 功能描述: 查询流控处理函数
 ** 输　入  : ptmStat        monitor 参数信息
 ** 输　出  : pTmStatHead    流量相关信息
 ** 返　回  : NONE
*********************************************************************************************************/
static VOID  __tmAssign (PTM_STATS  pTmStatHead, PTM_STATS_INNER  ptmStat)
{
    struct sockaddr_in  tmpAddr;

    pTmStatHead->cType             = ptmStat->cType;
    pTmStatHead->cEnable           = ptmStat->cEnable;
    pTmStatHead->cProtocol         = ptmStat->cProtocol;
    pTmStatHead->iDuration         = ptmStat->iDuration;
    pTmStatHead->iSendRate         = ptmStat->iSendRate;
    pTmStatHead->iRecvRate         = ptmStat->iRecvRate;
    pTmStatHead->iSendTotalLen     = ptmStat->iSendTotalLen;
    pTmStatHead->iRecvTotalLen     = ptmStat->iRecvTotalLen;
    pTmStatHead->iSendTotalPackets = ptmStat->iSendTotalPackets;
    pTmStatHead->iRecvTotalPackets = ptmStat->iRecvTotalPackets;

    strncpy(pTmStatHead->pIfname, ptmStat->pIfname, sizeof(pTmStatHead->pIfname));
    strncpy(pTmStatHead->pAlias, ptmStat->pAlias, sizeof(pTmStatHead->pAlias));

    bzero(&tmpAddr, sizeof(struct sockaddr_in));

    tmpAddr.sin_len    = sizeof(struct sockaddr_in);
    tmpAddr.sin_family = AF_INET;

    inet_addr_from_ip4addr(&((struct sockaddr_in * )&tmpAddr)->sin_addr, &(ptmStat->stIpAddr));
    inet_ntoa_r(((struct sockaddr_in *) &tmpAddr)->sin_addr, pTmStatHead->pIpAddr, IP4ADDR_STRLEN_MAX);

    sprintf(pTmStatHead->pMac, "%02x:%02x:%02x:%02x:%02x:%02x",
            ptmStat->pMac[0],
            ptmStat->pMac[1],
            ptmStat->pMac[2],
            ptmStat->pMac[3],
            ptmStat->pMac[4],
            ptmStat->pMac[5]);
}

/*********************************************************************************************************
 ** 函数名称: __tmCopy
 ** 功能描述: 将统计信息拷贝到web前端的结构体中
 ** 输　入  : ptmStat   统计信息
 **           pvArg0    返回的链表表头
 **           pvArg1    查询条件
 **           pvArg2    过滤条件
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
static INT  __tmCopy (PTM_STATS_INNER   ptmStat,
                      PVOID             pvArg0,
                      PVOID             pvArg1,
                      PVOID             pvArg2)
{
    PTM_STATS           pTmStatHead = *(PTM_STATS*)pvArg0;
    PTM_STATS           pCondition  = (PTM_STATS)pvArg1;
    INT                 iFilter     = *(INT*)pvArg2;
    struct sockaddr_in  sockAddr;
    ip4_addr_t          ip4Addr;
    CHAR                pMac[32];

    if (NULL == pCondition) {
        return  (1);
    }

    if (ptmStat->cType == pCondition->cType) {
        if (TMF_IP == iFilter) {
            if (!inet_aton(pCondition->pIpAddr, &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
                return  (1);
            }

            sockAddr.sin_len    = sizeof(struct sockaddr_in);
            sockAddr.sin_family = AF_INET;
            inet_addr_to_ip4addr(&ip4Addr, &((struct sockaddr_in * )&sockAddr)->sin_addr);

            if (ip4Addr.addr == ptmStat->stIpAddr.addr) {
                __tmAssign(pTmStatHead, ptmStat);

                *(PTM_STATS*)pvArg0 = *(PTM_STATS*)pvArg0 + 1;
            }

        } else if (TMF_MAC == iFilter) {
            sprintf(pMac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    ptmStat->pMac[0],
                    ptmStat->pMac[1],
                    ptmStat->pMac[2],
                    ptmStat->pMac[3],
                    ptmStat->pMac[4],
                    ptmStat->pMac[5]);
            if (0 == strncmp(pMac, pCondition->pMac, sizeof(pMac))) {
                __tmAssign(pTmStatHead, ptmStat);

                *(PTM_STATS*)pvArg0 = *(PTM_STATS*)pvArg0 + 1;
            }
        } else {
            __tmAssign(pTmStatHead, ptmStat);

            *(PTM_STATS*)pvArg0 = *(PTM_STATS*)pvArg0 + 1;
        }
    }

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: __tmSum
 ** 功能描述: 累计统计值
 ** 输　入  : pTmStatHead    返回的统计块地址
 **           ptmStat        待累加的统计块
 **           cProtocolType  协议分类 取值参见 app_type 枚举值
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
static VOID  __tmSum (PTM_STATS  pTmStatHead, PTM_STATS_INNER  ptmStat, CHAR  cProtocolType)
{
    pTmStatHead->cType              = ptmStat->cType;
    pTmStatHead->cProtocol          = cProtocolType;
    pTmStatHead->iDuration          = ptmStat->iDuration;
    pTmStatHead->iSendRate         += ptmStat->iSendRate;
    pTmStatHead->iRecvRate         += ptmStat->iRecvRate;
    pTmStatHead->iSendTotalLen     += ptmStat->iSendTotalLen;
    pTmStatHead->iRecvTotalLen     += ptmStat->iRecvTotalLen;
    pTmStatHead->iSendTotalPackets += ptmStat->iSendTotalPackets;
    pTmStatHead->iRecvTotalPackets += ptmStat->iRecvTotalPackets;
}

/*********************************************************************************************************
 ** 函数名称: __tmSumType
 ** 功能描述: 查询流控处理函数
 ** 输　入  : ptmStat     待累加的统计块
 **           pvArg0      返回的链表表头
 **           pvArg1      查询条件
 ** 输　出  : NONE
 ** 返　回  : 是否继续遍历， 0 继续，1 终止
*********************************************************************************************************/
static INT  __tmSumType (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    PTM_STATS  pTmStatHead = *(PTM_STATS*)pvArg0;
    PTM_STATS  pTempStat   = (PTM_STATS)pvArg1;
    INT        i;

    if (!pTempStat) {
        return  (1);
    }

    /*
     *  WAN 口总流量统计
     */
    if (TMT_WAN_TOTAL == pTempStat->cType) {
        if (TMT_LINE == ptmStat->cType) {

            for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {
                if ((_G_cLanName[i] != NULL) &&
                   (0 != strcmp(_G_cLanName[i], "null")) &&
                   (0 == strcmp(_G_cLanName[i], ptmStat->pIfname))) {
                    __tmSum(pTmStatHead, ptmStat, 0);
                }
            }
        }
    } else if (TMT_LINE_APP == pTempStat->cType) {
        if (ptmStat->cType == pTempStat->cType) {
            switch (ntohs(ptmStat->usPort)) {
            case 80:
            case 443:
            case 8080:
                __tmSum(pTmStatHead, ptmStat, APP_TYPE_HTTP);

                return  (0);
            case 20:
            case 21:
            case 22:
            case 23:
            case 69:
            case 3076:
            case 5200:
            case 6200:
                __tmSum((pTmStatHead + APP_TYPE_TRANSFER - 1), ptmStat, APP_TYPE_TRANSFER);

                return  (0);
            case 25:
            case 110:
            case 143:
            case 465:
            case 995:
            case 993:
                __tmSum((pTmStatHead + APP_TYPE_MAIL - 1), ptmStat, APP_TYPE_MAIL);

                return  (0);
            case 8000:
                __tmSum((pTmStatHead + APP_TYPE_CHAT - 1), ptmStat, APP_TYPE_CHAT);

                return  (0);
            default:
                __tmSum((pTmStatHead + APP_TYPE_OTHERS - 1), ptmStat, APP_TYPE_OTHERS);

                return  (0);
            }
        }
    } else {
        return  (1);
    }

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: tmStatItemsSumCopy
 ** 功能描述: 按照几个协议分类进行流量统计
 ** 输　入  : ppTmStat    待返回的统计数组地址
 **           pCondition  查询条件
 ** 输　出  : NONE
 ** 返　回  : 成功返回流量信息指针，失败返回 NULL
*********************************************************************************************************/
LW_SYMBOL_EXPORT PTM_STATS  tmStatItemsSumCopy (PTM_STATS  *ppTmStat, PTM_STATS  pCondition)
{
    PTM_STATS  pTmStatHead = *ppTmStat;

    tmWalkStats(__tmSumType, ppTmStat, pCondition, LW_NULL);

    if (TMT_LINE_APP == pCondition->cType) {

        /*
         * 当没有任何协议归到 others 时,需要给 cProtocol 赋值
         */
        (pTmStatHead + APP_TYPE_OTHERS - 1)->cProtocol = APP_TYPE_OTHERS;
    }

    *ppTmStat = pTmStatHead;

    return  (pTmStatHead);
}

/*********************************************************************************************************
 ** 函数名称: tmStatItemsCopy
 ** 功能描述: 查询流控处理函数
 ** 输　入  : ppTmStat    待返回的统计数组地址
 **           pCondition  查询条件
 ** 输　出  : piFilter    过滤条件
 ** 返　回  : 成功返回流量信息指针，失败返回 NULL
*********************************************************************************************************/
LW_SYMBOL_EXPORT PTM_STATS  tmStatItemsCopy (PTM_STATS  *ppTmStat, PTM_STATS  pCondition, INT  *piFilter)
{
    PTM_STATS  pTmStatHead = *ppTmStat;

    tmWalkStats(__tmCopy, ppTmStat, pCondition, piFilter);

    *ppTmStat = pTmStatHead;

    return  (pTmStatHead);
}

/*********************************************************************************************************
 ** 函数名称: __tmCount
 ** 功能描述: 查询流控处理函数
 ** 输　入  : ptmStat     monitor 参数信息
 **           arg0        参数 0
 **           arg1        参数 1
 **           arg2        参数 2
 ** 输　出  : NONE
 ** 返　回  : 返回结果
*********************************************************************************************************/
static INT  __tmCount (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1, PVOID  pvArg2)
{
    PTM_STATS           pCondition = (PTM_STATS)pvArg0;
    INT                 iFilter    = *(INT*)pvArg2;
    struct sockaddr_in  sockAddr;
    ip4_addr_t          ip4Addr;
    CHAR                pMac[32];

    if (NULL == pCondition) {
        return  (1);
    }

    if (ptmStat->cType == pCondition->cType) {
        if (TMF_IP == iFilter) {
            if (!inet_aton(pCondition->pIpAddr, &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
                return  (1);
            }

            sockAddr.sin_len    = sizeof(struct sockaddr_in);
            sockAddr.sin_family = AF_INET;
            inet_addr_to_ip4addr(&ip4Addr, &((struct sockaddr_in * )&sockAddr)->sin_addr);

            if (ip4Addr.addr == ptmStat->stIpAddr.addr) {
                *((INT*)pvArg1) = *((INT*)pvArg1) + 1;
            }
        } else if (TMF_MAC == iFilter) {
            sprintf(pMac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    ptmStat->pMac[0],
                    ptmStat->pMac[1],
                    ptmStat->pMac[2],
                    ptmStat->pMac[3],
                    ptmStat->pMac[4],
                    ptmStat->pMac[5]);
            if (0 == strncmp(pMac, pCondition->pMac, sizeof(pMac))) {
                *((INT*)pvArg1) = *((INT*)pvArg1) + 1;
            }
        } else {
            *((INT*)pvArg1) = *((INT*)pvArg1) + 1;
        }
    }

    return  (0);
}

/*********************************************************************************************************
 ** 函数名称: tmStatItemCount
 ** 功能描述: 计数符合条件的统计项
 ** 输　入  : pCondition    统计条件
 **           piFilter      过滤条件
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT  tmStatItemCount (PTM_STATS  pCondition, INT  *piFilter)
{
    INT  iCount = 0;

    tmWalkStats(__tmCount, pCondition, &iCount, piFilter);

    return  (iCount);
}

/*********************************************************************************************************
 ** 函数名称: tmSendStatsInc
 ** 功能描述: 发送流量累计
 ** 输　入  : pTmStatEntry  统计控制块
 **           usIpLen       报文长度
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
INT  tmSendStatsInc (PTM_STAT_ENTRY  pTmStatEntry, u16_t  usIpLen)
{
    if (NULL == pTmStatEntry) {
        return  (PX_ERROR);
    }

    pTmStatEntry->tm_stats.iSendCurrLen  += usIpLen;
    pTmStatEntry->tm_stats.iSendTotalLen += usIpLen;
    pTmStatEntry->tm_stats.iSendTotalPackets++;

    return  (ERROR_NONE);
}

/*********************************************************************************************************
 ** 函数名称: tmRecvStatsInc
 ** 功能描述: 接收流量累计
 ** 输　入  : pTmStatEntry  统计控制块
 **           usIpLen       报文长度
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
INT  tmRecvStatsInc (PTM_STAT_ENTRY  pTmStatEntry, u16_t  usIpLen)
{
    if (NULL == pTmStatEntry) {
        return  (PX_ERROR);
    }

    pTmStatEntry->tm_stats.iRecvCurrLen  += usIpLen;
    pTmStatEntry->tm_stats.iRecvTotalLen += usIpLen;
    pTmStatEntry->tm_stats.iRecvTotalPackets++;

    return  (ERROR_NONE);
}

/*********************************************************************************************************
 ** 函数名称: tmSendAppStatsInc
 ** 功能描述: app发送流量累计
 ** 输　入  : pTmStatTmp    统计控制块
 **           pBuf          报文的 pbuf
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmSendAppStatsInc (PTM_STAT_ENTRY  pTmStatTmp, struct pbuf  *pBuf)
{
    struct tcp_hdr  *ptcpHdr;
    struct udp_hdr  *pudpHdr;
    struct ip_hdr   *pipHdr;
    INT              ipLen;
    INT              iphLen;

    pipHdr  = (struct ip_hdr *)pBuf->payload;
    iphLen = IPH_HL_BYTES(pipHdr);
    ipLen  = lwip_ntohs(IPH_LEN(pipHdr));

    if (IPH_PROTO(pipHdr) == IP_PROTO_TCP && pTmStatTmp->tm_stats.cProtocol == IP_PROTO_TCP) {
        ptcpHdr = (struct tcp_hdr *)((u8_t *)pBuf->payload + iphLen);
        if (pTmStatTmp->tm_stats.usPort == ptcpHdr->src) {
            tmSendStatsInc(pTmStatTmp, ipLen);
        }
    } else if (IPH_PROTO(pipHdr) == IP_PROTO_UDP && pTmStatTmp->tm_stats.cProtocol == IP_PROTO_UDP) {
        pudpHdr = (struct udp_hdr *)((u8_t *)pBuf->payload + iphLen);
        if (pTmStatTmp->tm_stats.usPort == pudpHdr->src) {
            tmSendStatsInc(pTmStatTmp, ipLen);
        }
    }
}

/*********************************************************************************************************
 ** 函数名称: tmRecvAppStatsInc
 ** 功能描述: app接收流量累计
 ** 输　入  : pTmStatTmp    统计控制块
 **           pBuf          报文的 pbuf
 ** 输　出  : NONE
 ** 返　回  : NONE
*********************************************************************************************************/
VOID  tmRecvAppStatsInc (PTM_STAT_ENTRY  pTmStatTmp, struct pbuf  *pBuf)
{
    struct tcp_hdr  *ptcpHdr;
    struct udp_hdr  *pudpHdr;
    struct ip_hdr   *pipHdr;
    INT              ipLen;
    INT              iphLen;

    pipHdr  = (struct ip_hdr *)pBuf->payload;
    iphLen = IPH_HL_BYTES(pipHdr);
    ipLen  = lwip_ntohs(IPH_LEN(pipHdr));

    if (IPH_PROTO(pipHdr) == IP_PROTO_TCP && pTmStatTmp->tm_stats.cProtocol == IP_PROTO_TCP) {
        ptcpHdr = (struct tcp_hdr *) ((u8_t *)pBuf->payload + iphLen);
        if (pTmStatTmp->tm_stats.usPort == ptcpHdr->dest) {
            tmRecvStatsInc(pTmStatTmp, ipLen);
        }
    } else if (IPH_PROTO(pipHdr) == IP_PROTO_UDP && pTmStatTmp->tm_stats.cProtocol == IP_PROTO_UDP) {
        pudpHdr = (struct udp_hdr *) ((u8_t *)pBuf->payload + iphLen);
        if (pTmStatTmp->tm_stats.usPort == pudpHdr->dest) {
            tmRecvStatsInc(pTmStatTmp, ipLen);
        }
    }
}
/*********************************************************************************************************
 ** 函数名称: tmStatsStart
 ** 功能描述: 对满足条件的统计控制块累加报文总长度、累计速率统计当前长度
 ** 输　入  : iIpType      IP 协议版本
 **           iHookType    hook 类型
 **           pBuf         报文的 pbuf 指针
 **           pinp         入接口 netif
 **           poutp        出接口 netif
 ** 输　出  : NONE
 ** 返　回  : 操作结果
*********************************************************************************************************/
INT  tmStatsStart (INT            iIpType,
                   INT            iHookType,
                   struct pbuf   *pBuf,
                   struct netif  *pinp,
                   struct netif  *poutp)
{
    PTM_STAT_ENTRY  pTmStatEntry;
    PTM_STAT_ENTRY  pTmStatTmp;
    PLW_LIST_LINE   pline;
    struct ip_hdr  *pipHdr;
    u16_t           ipLen;
    CHAR            cIfName[IF_NAMESIZE];

    if (IP_HOOK_V4 != iIpType) {
        return  (ERROR_NONE);
    }

    if ((IP_HT_PRE_ROUTING != iHookType) && (IP_HT_POST_ROUTING != iHookType)) {
        return  (ERROR_NONE);
    }

    pipHdr = (struct ip_hdr *)pBuf->payload;
    if (NULL == pipHdr) {
        printf("enter tm_stats_start iphdr is null");

        return  (ERROR_NONE);
    }

    ipLen = lwip_ntohs(IPH_LEN(pipHdr));
    if (IP_HT_PRE_ROUTING == iHookType) {

        if (ip4_addr_isany(&pipHdr->src) ||
            ip4_addr_isbroadcast(&pipHdr->src, pinp) ||
            ip4_addr_ismulticast(&pipHdr->src)) {
            return  (ERROR_NONE);
        } else {
            pTmStatEntry = tmStatCbFindByMac(pinp->hwaddr);
            netif_get_name(pinp, cIfName);

            /*
             *  匹配 IP 和接口名称后，再计数
             */
            for (pline = (PLW_LIST_LINE) pTmStatEntry;
                 pline != NULL;
                 pline = _list_line_get_next(pline)) {
                pTmStatTmp = (PTM_STAT_ENTRY)pline;

                if (0 != strcmp(pTmStatTmp->tm_stats.pIfname, cIfName)) {
                    continue;
                }

                if (TMT_LINE == pTmStatTmp->tm_stats.cType) {
                    tmSendStatsInc(pTmStatTmp, ipLen);
                } else if (TMT_LINE_APP == pTmStatTmp->tm_stats.cType) {
                    tmSendAppStatsInc(pTmStatTmp, pBuf);
                }
            }

            pTmStatEntry = tmStatCbFindByIp((ip4_addr_t*)&(pipHdr->src));
            for (pline = (PLW_LIST_LINE)pTmStatEntry;
                 pline != NULL;
                 pline = _list_line_get_next(pline)) {
                pTmStatTmp = (PTM_STAT_ENTRY)pline;

                if (TMT_TERM == pTmStatTmp->tm_stats.cType) {
                    tmSendStatsInc(pTmStatTmp, ipLen);
                } else if (TMT_APP == pTmStatTmp->tm_stats.cType) {
                    tmSendAppStatsInc(pTmStatTmp, pBuf);
                }
            }
        }
    }

    if (IP_HT_POST_ROUTING == iHookType) {

        if (ip4_addr_isany(&pipHdr->dest) ||
            ip4_addr_isbroadcast(&pipHdr->dest, poutp) ||
            ip4_addr_ismulticast(&pipHdr->dest)) {
            return  (ERROR_NONE);
        } else {
            pTmStatEntry = tmStatCbFindByMac(poutp->hwaddr);
            netif_get_name(poutp, cIfName);

            /*
             * 匹配 IP 和接口名称后，再计数
             */
            for (pline = (PLW_LIST_LINE) pTmStatEntry;
                 pline != NULL;
                 pline = _list_line_get_next(pline)) {
                pTmStatTmp = (PTM_STAT_ENTRY)pline;

                if (0 != strcmp(pTmStatTmp->tm_stats.pIfname, cIfName)) {
                    continue;
                }

                if (TMT_LINE == pTmStatTmp->tm_stats.cType) {
                    tmRecvStatsInc(pTmStatTmp, ipLen);
                } else if (TMT_LINE_APP == pTmStatTmp->tm_stats.cType) {
                    tmRecvAppStatsInc(pTmStatTmp, pBuf);
                }
            }

            pTmStatEntry = tmStatCbFindByIp((ip4_addr_t*)&(pipHdr->dest));
            for (pline = (PLW_LIST_LINE) pTmStatEntry;
                 pline != NULL;
                 pline = _list_line_get_next(pline)) {
                pTmStatTmp = (PTM_STAT_ENTRY) pline;
                if (TMT_TERM == pTmStatTmp->tm_stats.cType) {
                    tmRecvStatsInc(pTmStatTmp, ipLen);
                } else if (TMT_APP == pTmStatTmp->tm_stats.cType) {
                    tmRecvAppStatsInc(pTmStatTmp, pBuf);
                }
            }
        }
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
 ** 函数名称: __tmStatTimerProc
 ** 功能描述: 定时器回调处理函数。
 **           1、遍历 arp 表，如果 ip 不在统计表中，则添加为终端监控表项
 **           2、根据计算频率，计算并更新速率，2秒(终端)，(5s)线路更新一次
 ** 输　入  : NONE
 ** 输　出  : NONE
*********************************************************************************************************/
static VOID  __tmStatTimerProc (VOID)
{
    tmStatTermUserAutoInstall();

    tmStatCalcRate();
}

/*********************************************************************************************************
 ** 函数名称: __tmStatCountResetTimerProc
 ** 功能描述: 定时器回调处理函数。
 **           1、遍历 arp 表，如果 ip 不在统计表中，则添加为终端监控表项
 **           2、根据计算频率，计算并更新速率，2秒(终端)，(5s)线路更新一次
 ** 输　入  : NONE
 ** 输　出  : NONE
*********************************************************************************************************/
static VOID  __tmStatCountResetTimerProc (VOID)
{
    API_TimerCancel(_G_ulTmStatTimer);
    API_TimerCancel(_G_ulTmStatCountResetTimer);
    net_ip_hook_delete(tmStatsStart);

    sleep(1);

    tmStatItemAllTermRemove();
    tmStatCountReset();

    tmStartStats();
}

/*********************************************************************************************************
 ** 函数名称: tmStartStats
 ** 功能描述: 开始统计计数和速率计算
 **           1、挂统计函数钩子到 ip 协议栈
 **           2、启动速率计数定时器
 ** 输　入  : NONE
 ** 输　出  : NONE
 ** 返　回  : iRet   操作结果
*********************************************************************************************************/
INT  tmStartStats (VOID)
{
    INT  iRet;

    iRet = net_ip_hook_add("tm_stats_start", tmStatsStart);

    API_TimerStart(_G_ulTmStatTimer,
                   TM_STAT_PERIOD,
                   LW_OPTION_AUTO_RESTART,
                   (PTIMER_CALLBACK_ROUTINE)__tmStatTimerProc,
                   LW_NULL);

    API_TimerStart(_G_ulTmStatCountResetTimer,
                   TM_STAT_COUNT_RESET_PERIOD,
                   LW_OPTION_AUTO_RESTART,
                   (PTIMER_CALLBACK_ROUTINE)__tmStatCountResetTimerProc,
                   LW_NULL);

    return  (iRet);
}
