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
 ** ��   ��   ��: tmStat.c
 **
 ** ��   ��   ��: Chen.Hongbang (�º��)
 **
 ** �ļ���������: 2018 �� 7 �� 11 ��
 **
 ** ��        ��: OpenRT ������ع���ͳ�Ʋ���
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
  ��̬�ֲ�����
*********************************************************************************************************/
static char _G_cWanName[LW_CFG_NET_NAT_MAX_AP_IF][NETIF_NAMESIZE];      /*  ���� WAN ����������         */
static char _G_cLanName[LW_CFG_NET_NAT_MAX_LOCAL_IF][NETIF_NAMESIZE];   /*  ���� LAN ����������         */
/*********************************************************************************************************
 ** ��������: tmNotifyWanChange
 ** ��������: ֪ͨ WAN ���
 ** �䡡��  : pvWan    WAN ��Ϣ
 ** �䡡��  : NONE
 ** ������  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tmNotifyWanChange (PVOID  pvWan)
{
    memcpy(_G_cWanName, pvWan, sizeof(_G_cWanName));
}

/*********************************************************************************************************
 ** ��������: tmNotifyLanChange
 ** ��������: ֪ͨ LAN ���
 ** �䡡��  : pvLan    LAN ��Ϣ
 ** �䡡��  : NONE
 ** ������  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tmNotifyLanChange (PVOID  pvLan)
{
    memcpy(_G_cLanName, pvLan, sizeof(_G_cLanName));
}
/*********************************************************************************************************
 ** ��������: __ipAddrGetByIfName
 ** ��������: ������������ȡ IP ��ַ
 ** �䡡��  : cpcIfName      ������Ϣ
 **           uiAddrLen      ��ַ����
 ** �䡡��  : pcIpAddr       ���� IP ��Ϣ������
 ** ������  : ERROR_CODE
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
 ** ��������: tmWalkStats
 ** ��������: ͳ�ƿ��ƹ�ϣ���������
 ** �䡡��  : INT (*callback)     ����ʱִ�еĻص�����
 **           pvArg0              �ص�������һ������
 **           pvArg1              �ص������ڶ�������
 **           pvArg2              �ص���������������
 ** �䡡��  : NONE
 ** ������  : NONE
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

            if (STAT_ITEM_PARAM_INVALID == iAction) {                   /*  �����쳣                    */
                break;
            }

            if (STAT_ITEM_SHOULD_DELETE == iAction) {                   /*  ɾ��ͳ����                  */

                /*
                 *  s1 ��������ɾ�����������Ľڵ�
                 *  s2 �����ɾ���ڵ���ͷ�ڵ㣬�� header->next ��ֵ�� __G_tmStatCbtable[i]
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

            if (iAction == STAT_ITEM_PARAM_INVALID) {                   /*  �����쳣                    */
                break;
            }

            if (iAction == STAT_ITEM_SHOULD_DELETE) {                   /*  ɾ��ͳ����                  */

                /*
                 *  s1 ��������ɾ�����������Ľڵ�
                 *  s2 �����ɾ���ڵ���ͷ�ڵ㣬�� header->next ��ֵ�� __G_tmStatCbtable[i]
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
 ** ��������: __tmCalcRate
 ** ��������: �����ԣ�2s���ļ���ͳ�Ʒ��͡���������
 ** �䡡��  : ptmStat     monitor ������Ϣ
 **           pvArg0      ��δ�õ�
 **           pvArg1      ��δ�õ�
 ** �䡡��  : NONE
 ** ������  : �������
*********************************************************************************************************/
static INT  __tmCalcRate (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    struct timespec  tsMono;                                            /*  ��������ʱ��                */

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
 ** ��������: tmStatCalcRate
 ** ��������: ��������ͳ���������
 ** �䡡��  : NONE
 ** �䡡��  : NONE
 ** ������  : NONE
*********************************************************************************************************/
VOID  tmStatCalcRate (VOID)
{
    tmWalkStats(__tmCalcRate, LW_NULL, LW_NULL, LW_NULL);
}

/*********************************************************************************************************
 ** ��������: __tmStatCountSetZero
 ** ��������: ���ͳ������
 ** �䡡��  : ptmStat    ͳ����
 **           pvArg0     ��δ�õ�
 **           pvArg1     ��δ�õ�
 ** �䡡��  : NONE
 ** ������  : �������
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
 ** ��������: tmStatCountReset
 ** ��������: ͳ�Ƽ�������
 ** �䡡��  : NONE
 ** �䡡��  : NONE
 ** ������  : NONE
*********************************************************************************************************/
VOID  tmStatCountReset (VOID)
{
    tmWalkStats(__tmStatCountSetZero, LW_NULL, LW_NULL, LW_NULL);
}

/*********************************************************************************************************
 ** ��������: __tmPrintStatInfo
 ** ��������: ͳ�ƿ�����Ϣ��ӡ���
 ** �䡡��  : ptmStat   �������ͳ����Ϣ
 **           pvArg0    ����
 **           pvArg1    IP ��Ϣ
 ** �䡡��  : NONE
 ** ������  : �������
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
 ** ��������: tmShowStats
 ** ��������: ��ӡ����ͳ����Ϣ
 ** �䡡��  : iType     �����������ͣ�0 ��ʾ����, �������� TMT_*, �� TMT_TERM ��
 **           pip4Addr  �������� ip ��ַ
 ** �䡡��  : NONE
 ** ������  : NONE
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
 ** ��������: __tmAssign
 ** ��������: ��ѯ���ش�����
 ** �䡡��  : ptmStat        monitor ������Ϣ
 ** �䡡��  : pTmStatHead    ���������Ϣ
 ** ������  : NONE
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
 ** ��������: __tmCopy
 ** ��������: ��ͳ����Ϣ������webǰ�˵Ľṹ����
 ** �䡡��  : ptmStat   ͳ����Ϣ
 **           pvArg0    ���ص������ͷ
 **           pvArg1    ��ѯ����
 **           pvArg2    ��������
 ** �䡡��  : NONE
 ** ������  : �������
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
 ** ��������: __tmSum
 ** ��������: �ۼ�ͳ��ֵ
 ** �䡡��  : pTmStatHead    ���ص�ͳ�ƿ��ַ
 **           ptmStat        ���ۼӵ�ͳ�ƿ�
 **           cProtocolType  Э����� ȡֵ�μ� app_type ö��ֵ
 ** �䡡��  : NONE
 ** ������  : NONE
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
 ** ��������: __tmSumType
 ** ��������: ��ѯ���ش�����
 ** �䡡��  : ptmStat     ���ۼӵ�ͳ�ƿ�
 **           pvArg0      ���ص������ͷ
 **           pvArg1      ��ѯ����
 ** �䡡��  : NONE
 ** ������  : �Ƿ���������� 0 ������1 ��ֹ
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
     *  WAN ��������ͳ��
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
 ** ��������: tmStatItemsSumCopy
 ** ��������: ���ռ���Э������������ͳ��
 ** �䡡��  : ppTmStat    �����ص�ͳ�������ַ
 **           pCondition  ��ѯ����
 ** �䡡��  : NONE
 ** ������  : �ɹ�����������Ϣָ�룬ʧ�ܷ��� NULL
*********************************************************************************************************/
LW_SYMBOL_EXPORT PTM_STATS  tmStatItemsSumCopy (PTM_STATS  *ppTmStat, PTM_STATS  pCondition)
{
    PTM_STATS  pTmStatHead = *ppTmStat;

    tmWalkStats(__tmSumType, ppTmStat, pCondition, LW_NULL);

    if (TMT_LINE_APP == pCondition->cType) {

        /*
         * ��û���κ�Э��鵽 others ʱ,��Ҫ�� cProtocol ��ֵ
         */
        (pTmStatHead + APP_TYPE_OTHERS - 1)->cProtocol = APP_TYPE_OTHERS;
    }

    *ppTmStat = pTmStatHead;

    return  (pTmStatHead);
}

/*********************************************************************************************************
 ** ��������: tmStatItemsCopy
 ** ��������: ��ѯ���ش�����
 ** �䡡��  : ppTmStat    �����ص�ͳ�������ַ
 **           pCondition  ��ѯ����
 ** �䡡��  : piFilter    ��������
 ** ������  : �ɹ�����������Ϣָ�룬ʧ�ܷ��� NULL
*********************************************************************************************************/
LW_SYMBOL_EXPORT PTM_STATS  tmStatItemsCopy (PTM_STATS  *ppTmStat, PTM_STATS  pCondition, INT  *piFilter)
{
    PTM_STATS  pTmStatHead = *ppTmStat;

    tmWalkStats(__tmCopy, ppTmStat, pCondition, piFilter);

    *ppTmStat = pTmStatHead;

    return  (pTmStatHead);
}

/*********************************************************************************************************
 ** ��������: __tmCount
 ** ��������: ��ѯ���ش�����
 ** �䡡��  : ptmStat     monitor ������Ϣ
 **           arg0        ���� 0
 **           arg1        ���� 1
 **           arg2        ���� 2
 ** �䡡��  : NONE
 ** ������  : ���ؽ��
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
 ** ��������: tmStatItemCount
 ** ��������: ��������������ͳ����
 ** �䡡��  : pCondition    ͳ������
 **           piFilter      ��������
 ** �䡡��  : NONE
 ** ������  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT  tmStatItemCount (PTM_STATS  pCondition, INT  *piFilter)
{
    INT  iCount = 0;

    tmWalkStats(__tmCount, pCondition, &iCount, piFilter);

    return  (iCount);
}

/*********************************************************************************************************
 ** ��������: tmSendStatsInc
 ** ��������: ���������ۼ�
 ** �䡡��  : pTmStatEntry  ͳ�ƿ��ƿ�
 **           usIpLen       ���ĳ���
 ** �䡡��  : NONE
 ** ������  : �������
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
 ** ��������: tmRecvStatsInc
 ** ��������: ���������ۼ�
 ** �䡡��  : pTmStatEntry  ͳ�ƿ��ƿ�
 **           usIpLen       ���ĳ���
 ** �䡡��  : NONE
 ** ������  : �������
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
 ** ��������: tmSendAppStatsInc
 ** ��������: app���������ۼ�
 ** �䡡��  : pTmStatTmp    ͳ�ƿ��ƿ�
 **           pBuf          ���ĵ� pbuf
 ** �䡡��  : NONE
 ** ������  : NONE
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
 ** ��������: tmRecvAppStatsInc
 ** ��������: app���������ۼ�
 ** �䡡��  : pTmStatTmp    ͳ�ƿ��ƿ�
 **           pBuf          ���ĵ� pbuf
 ** �䡡��  : NONE
 ** ������  : NONE
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
 ** ��������: tmStatsStart
 ** ��������: ������������ͳ�ƿ��ƿ��ۼӱ����ܳ��ȡ��ۼ�����ͳ�Ƶ�ǰ����
 ** �䡡��  : iIpType      IP Э��汾
 **           iHookType    hook ����
 **           pBuf         ���ĵ� pbuf ָ��
 **           pinp         ��ӿ� netif
 **           poutp        ���ӿ� netif
 ** �䡡��  : NONE
 ** ������  : �������
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
             *  ƥ�� IP �ͽӿ����ƺ��ټ���
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
             * ƥ�� IP �ͽӿ����ƺ��ټ���
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
 ** ��������: __tmStatTimerProc
 ** ��������: ��ʱ���ص���������
 **           1������ arp ����� ip ����ͳ�Ʊ��У������Ϊ�ն˼�ر���
 **           2�����ݼ���Ƶ�ʣ����㲢�������ʣ�2��(�ն�)��(5s)��·����һ��
 ** �䡡��  : NONE
 ** �䡡��  : NONE
*********************************************************************************************************/
static VOID  __tmStatTimerProc (VOID)
{
    tmStatTermUserAutoInstall();

    tmStatCalcRate();
}

/*********************************************************************************************************
 ** ��������: __tmStatCountResetTimerProc
 ** ��������: ��ʱ���ص���������
 **           1������ arp ����� ip ����ͳ�Ʊ��У������Ϊ�ն˼�ر���
 **           2�����ݼ���Ƶ�ʣ����㲢�������ʣ�2��(�ն�)��(5s)��·����һ��
 ** �䡡��  : NONE
 ** �䡡��  : NONE
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
 ** ��������: tmStartStats
 ** ��������: ��ʼͳ�Ƽ��������ʼ���
 **           1����ͳ�ƺ������ӵ� ip Э��ջ
 **           2���������ʼ�����ʱ��
 ** �䡡��  : NONE
 ** �䡡��  : NONE
 ** ������  : iRet   �������
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
