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
** ��   ��   ��: tmStatItem.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 11 ��
**
** ��        ��: OpenRT ������ع���ͳ�������
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "socket.h"
#include "tmHash.h"
#include "tmStat.h"
#include "tmCmd.h"
#include "tmStatItem.h"
#include "net/if.h"
#include "network/lwip/netifapi.h"
#include "lwip/netif.h"
#include "net/if_type.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static PTM_STAT_ENTRY __tmStatCbCreateAndAdd (ip4_addr_t  *pIp4Addr, UINT8  *pucMac, UCHAR  ucType);
/*********************************************************************************************************
** ��������: __tmStatTermUserInstall
** ��������: arp ����ʱ�����ͳ�����ͳ�Ʊ��У������ͳ����
** �䡡��  : pNetif     ����ӿ�
**           pip4Addr   IP
**           ethaddr    ��̫����ַ
**           iStatic    ��̬
**           pvArg0     ��δʹ��
**           pvArg1     ��δʹ��
**           pvArg2     ��δʹ��
**           pvArg3     ��δʹ��
** �䡡��  : 0: �������� 1: �˳�����
*********************************************************************************************************/
static INT  __tmStatTermUserInstall (struct netif    *pNetif,
                                     ip4_addr_t      *pip4Addr,
                                     struct eth_addr *pEthAddr,
                                     INT              iStatic,
                                     PVOID            pvArg0,
                                     PVOID            pvArg1,
                                     PVOID            pvArg2,
                                     PVOID            pvArg3)
{
    PTM_STAT_ENTRY  pTmStatEntry = tmStatCbFindByIp(pip4Addr);
    PLW_LIST_LINE   pline;
    char            ifname[IF_NAMESIZE];

    netif_get_name(pNetif, ifname);

    if (pTmStatEntry == LW_NULL) {
        tmStatTermUserAdd(pip4Addr, pEthAddr->addr, ifname);
    } else {
        for (pline = (PLW_LIST_LINE)pTmStatEntry;
             pline != NULL;
             pline = _list_line_get_next(pline)) {
            if (((PTM_STAT_ENTRY)pline)->tm_stats.cType == TMT_TERM) {
                return  (0);
            }
        }

        tmStatTermUserAdd(pip4Addr, pEthAddr->addr, ifname);
    }

    return  (0);
}

/*********************************************************************************************************
** ��������: tmStatTermUserAutoInstall
** ��������: ����arp���Զ���װ�ն��û�ͳ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatTermUserAutoInstall (VOID)
{
    netifapi_arp_traversal(LW_NULL,
                           __tmStatTermUserInstall,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __tmCalcOnlineUsers
** ��������: ���������û�������
** �䡡��  : pNetif      ����ӿ�
**           pip4Addr     ip
**           pEthAddr    ��̫����ַ
**           iStatic    ��̬
**           pvArg1     ��δ�õ�
**           pvArg2     ��δ�õ�
**           pvArg3     ��δ�õ�
** �䡡��  : piOnlineUsers      �����û���
** ������  : �������
*********************************************************************************************************/
static INT  __tmCalcOnlineUsers (struct netif    *pNetif,
                                 ip4_addr_t      *pip4Addr,
                                 struct eth_addr *pEthAddr,
                                 INT              iStatic,
                                 INT*             piOnlineUsers,
                                 PVOID            pvArg1,
                                 PVOID            pvArg2,
                                 PVOID            pvArg3)
{
    if (piOnlineUsers) {
        *piOnlineUsers = *piOnlineUsers + 1;
    }

    return  (0);
}

/*********************************************************************************************************
** ��������: tmGetOnlineUsers
** ��������: ���� arp �����������û���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : �����û���
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT tmGetOnlineUsers (VOID)
{
    UINT32  uiOnlineUsers = 0 ;

    netifapi_arp_traversal(LW_NULL,
                           __tmCalcOnlineUsers,
                           &uiOnlineUsers,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL,
                           LW_NULL);

    return  (uiOnlineUsers);
}

/*********************************************************************************************************
** ��������: __tmStatItemDelete
** ��������: ���� ip ��ַ��ӿ�����ɾ��ͳ����
** �䡡��  : ptmStat   ͳ����Ϣ
**           pvArg0    ���ص������ͷ
**           pvArg1    ��ѯ����
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
static INT  __tmStatItemDelete (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    CHAR        *ifname = pvArg0;
    ip4_addr_t  *ip     = pvArg1;

    if (!ptmStat) {
        return  (STAT_ITEM_PARAM_INVALID);
    }

    if (TMT_LINE == ptmStat->cType || TMT_LINE_APP == ptmStat->cType) {
        if (LW_NULL == ifname) {
            return  (STAT_ITEM_PARAM_INVALID);
        }

        if (0 == strcmp(ifname, ptmStat->pIfname)) {
            _DebugFormat(__PRINTMESSAGE_LEVEL, "ifname=%s, ip=%x\r\n", ifname, ip->addr);

            return  (STAT_ITEM_SHOULD_DELETE);
        }
    } else {
        if (LW_NULL == ip) {
            return  (STAT_ITEM_PARAM_INVALID);
        }

        if (ptmStat->stIpAddr.addr == ip->addr) {
            return  (STAT_ITEM_SHOULD_DELETE);
        }
    }

    return  (0);
}
/*********************************************************************************************************
** ��������: tmStatItemDel
** ��������: ���� ip ��ַ��ӿ�����ɾ��ͳ����
** �䡡��  : pcIfName    ��������
**           pip4Addr    ���� ip ��ַ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatItemDel (PCHAR  pcIfName, ip4_addr_t  *pip4Addr)
{
    tmWalkStats(__tmStatItemDelete, pcIfName, pip4Addr, LW_NULL);

    return  (0);
}
/*********************************************************************************************************
** ��������: tmStatLinesInstall
** ��������: ��·��װ����
** �䡡��  : pcIfName         ��������
**           pip4Addr         ���� ip ��ַ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatLinesInstall (PCHAR  pcIfName, ip4_addr_t  *pip4Addr)
{
    UINT8 mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    tmStatLineAdd(pip4Addr, mac, pcIfName);

    tmStatLineAllAppsInstall(pip4Addr, mac, pcIfName);

    return  (0);
}
/*********************************************************************************************************
** ��������: tmStatAllLinesIpUpdate
** ��������: ����ifname������·�����ͳ�����ip��ַ
** �䡡��  : pcIfName        ��������
**           pip4Addr        ����ip��ַ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatAllLinesIpUpdate (PCHAR  pcIfName, ip4_addr_t  *pip4Addr)
{
    API_TimerCancel(_G_ulTmStatTimer);

    tmEnable(LW_FALSE);

    sleep(2);

    tmStatItemDel(pcIfName, LW_NULL);

    tmStatLinesInstall(pcIfName, pip4Addr);

    sleep(1);

    tmStartStats();

    return  (0);
}

/*********************************************************************************************************
** ��������: tmStatAllLinesInstall
** ��������: ��װ�豸�����е���·���
** �䡡��  : pcIfName    ��������
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatAllLinesInstall (PCHAR  pcIfName)
{
    ip4_addr_t       ip4Addr;
    UINT8            ucMac[6];
    CHAR             cIfName[16];
    INT              iCount = 0;
    INT32            iSocketFd;
    struct  ifconf   iconf;
    INT32            iRet, i;
    INT32            iBuffSize;

    struct ifreq    *pIfreq     = LW_NULL;
    PCHAR            pcIconfBuf = LW_NULL;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        return  (PX_ERROR);
    }

    /*
     *  ��ȡ��Ҫ���� buf ���
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        goto  __error;
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        goto  __error;
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;

    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        goto  __error;
    }

    /*
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];

        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            goto  __error;
        }

        if (0 == strncmp(pIfreq->ifr_name, "lo", 2)) {
            continue;
        }

        if (IFT_ETHER == pIfreq->ifr_type ||
            IFT_PPP   == pIfreq->ifr_type ||
            IFT_SLIP  == pIfreq->ifr_type) {
            strncpy(cIfName, pIfreq->ifr_name, sizeof(cIfName));

            /*
             * ָ�� ifname ��� line ���͵�ͳ����
             */
            if (pcIfName && (0 != strcmp(cIfName, pcIfName))) {
                continue;
            }

            ip4Addr.addr = ((struct sockaddr_in *)&pIfreq->ifr_addr)->sin_addr.s_addr;

            if (0 != strncmp(pIfreq->ifr_name, "pp", 2)) {
                iRet = ioctl(iSocketFd, SIOCGIFHWADDR, pIfreq);             /*  ��ȡ���е�����������        */
                if (iRet < 0) {
                    printf("add line stat item failed, ifname= %s,"
                           " ifr_type=0x%x, ret= %d, ip=0x%x\n",
                           cIfName,
                           pIfreq->ifr_type,
                           iRet,
                           ip4Addr.addr);

                    continue;
                }
            }

            memcpy(&ucMac, pIfreq->ifr_hwaddr.sa_data, IFHWADDRLEN);

            tmStatLineAdd(&ip4Addr, ucMac, cIfName);

            tmStatLineAllAppsInstall(&ip4Addr, ucMac, cIfName);
        }
    }

    free(pcIconfBuf);
    close(iSocketFd);

    return  ((1 == iCount) ? ERROR_NONE : PX_ERROR);

__error:
    if (NULL != pcIconfBuf) {
        free(pcIconfBuf);
    }

    close(iSocketFd);

    return  (PX_ERROR);
}

/*********************************************************************************************************
** ��������: tmStatTermUserAdd
** ��������: �����ն��û�ͳ����
** �䡡��  : pip4Addr     IP  ��ַ
**           pucMac       MAC ��ַ
**           pcIfName     ��������
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatTermUserAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName)
{
    struct timespec  tsMono;                                            /*  ��������ʱ��                */
    PTM_STAT_ENTRY   ptmStatEntry = __tmStatCbCreateAndAdd(pip4Addr, pucMac, TMT_TERM);

    if (NULL == ptmStatEntry) {
        return  (PX_ERROR);
    }

    lib_clock_gettime(CLOCK_MONOTONIC, &tsMono);

    ptmStatEntry->tm_stats.cType          = TMT_TERM;
    ptmStatEntry->tm_stats.iJoinTimestamp = tsMono.tv_sec;

    strncpy(ptmStatEntry->tm_stats.pIfname, pcIfName, sizeof(ptmStatEntry->tm_stats.pIfname));

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: tmStatLineAppAdd
** ��������: ������·Э��Ӧ��ͳ����
** �䡡��  : pip4Addr     IP  ��ַ
**           pucMac       MAC ��ַ
**           pcIfName     ����ӿ�����
**           cProtocol    Э��
**           usPort       �˿ں�
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatLineAppAdd (ip4_addr_t  *pip4Addr,
                       UINT8       *pucMac,
                       PCHAR        pcIfName,
                       UINT8        cProtocol,
                       UINT16       usPort)
{
    PTM_STAT_ENTRY ptmStatEntry = LW_NULL;

    if (NULL == pcIfName) {
        return  (PX_ERROR);
    }

    ptmStatEntry = __tmStatCbCreateAndAdd(pip4Addr, pucMac, TMT_LINE_APP);
    if (NULL == ptmStatEntry) {
        return  (PX_ERROR);
    }

    ptmStatEntry->tm_stats.cType     = TMT_LINE_APP;
    ptmStatEntry->tm_stats.cProtocol = cProtocol;
    ptmStatEntry->tm_stats.usPort    = usPort;

    strncpy(ptmStatEntry->tm_stats.pIfname, pcIfName, sizeof(ptmStatEntry->tm_stats.pIfname));

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: tmStatLineAllAppsInstall
** ��������: ����������·Э��Ӧ��ͳ����
** �䡡��  : pip4Addr     IP  ��ַ
**           pucMac       MAC ��ַ
**           pcIfName     ����ӿ�����
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmStatLineAllAppsInstall (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName)
{
    INT  i;
    INT  tcp_ports[] = {20, 21, 22, 23, 25, 80, 443, 8080, 110, 143, 993, 995, 465};
    INT  udp_ports[] = {69, 443, 3076, 5200, 6200, 8000};

    for (i=0; i< sizeof(tcp_ports)/sizeof(INT); i++) {
        tmStatLineAppAdd(pip4Addr, pucMac, pcIfName, IP_PROTO_TCP, htons(tcp_ports[i]));
    }

    for (i=0; i< sizeof(udp_ports)/sizeof(INT); i++) {
        tmStatLineAppAdd(pip4Addr, pucMac, pcIfName, IP_PROTO_UDP, htons(udp_ports[i]));
    }
}

/*********************************************************************************************************
** ��������: tmStatLineAdd
** ��������: ������·���ͳ����
** �䡡��  : pip4Addr     IP  ��ַ
**           pucMac       MAC ��ַ
**           pcIfName     ����ӿ�����
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatLineAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName)
{
    PTM_STAT_ENTRY ptmStatEntry = LW_NULL;

    if (LW_NULL == pcIfName) {
        return  (PX_ERROR);
    }

    ptmStatEntry = __tmStatCbCreateAndAdd(pip4Addr, pucMac, TMT_LINE);
    if (LW_NULL == ptmStatEntry) {
        return  (PX_ERROR);
    }

    ptmStatEntry->tm_stats.cType = TMT_LINE;
    strncpy(ptmStatEntry->tm_stats.pIfname, pcIfName, sizeof(ptmStatEntry->tm_stats.pIfname));

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: tmStatAppAdd
** ��������: ����Ӧ��Э��ͳ����
** �䡡��  : pip4Addr      IP  ��ַ
**           pucMac        MAC ��ַ
**           iProtocol     Э��
**           usPort        �˿ں�
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatAppAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, INT  iProtocol, UINT16  usPort)
{
    PTM_STAT_ENTRY ptmStatEntry = __tmStatCbCreateAndAdd(pip4Addr, pucMac, TMT_APP);

    if (LW_NULL == ptmStatEntry) {
        return  (PX_ERROR);
    }

    printf("protocol=%d, port=%d\n", iProtocol, usPort);

    ptmStatEntry->tm_stats.cType     = TMT_APP;
    ptmStatEntry->tm_stats.cProtocol = (UCHAR)iProtocol;
    ptmStatEntry->tm_stats.usPort    = htons(usPort);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __tmStatCbCreateAndAdd
** ��������: ���������һ��ͳ�ƿ��ƿ�
** �䡡��  : pIp4Addr     IP ��ַ
**           pucMac       MAC ��ַ
** �䡡��  : NONE
** ������  : �ɹ�������ӵ���ڵ�ַ��ʧ�ܷ���NULL
*********************************************************************************************************/
static PTM_STAT_ENTRY  __tmStatCbCreateAndAdd (ip4_addr_t  *pIp4Addr, UINT8  *pucMac, UCHAR  ucType)
{
    PTM_STAT_ENTRY  ptmStatNode = LW_NULL;
    INT             i;

    if (LW_NULL == pucMac) {
        return  (LW_NULL);
    }

    ptmStatNode = sys_malloc(sizeof(TM_STAT_ENTRY));
    if (LW_NULL == ptmStatNode) {
        return  (LW_NULL);
    }

    bzero(ptmStatNode, sizeof(TM_STAT_ENTRY));
    ptmStatNode->tm_stats.stIpAddr.addr = pIp4Addr->addr;

    for (i = 0; i < 6; i++) {
        ptmStatNode->tm_stats.pMac[i] = pucMac[i];
    }

    if (TMT_TERM == ucType || TMT_APP == ucType) {
        tmStatCbAdd(pIp4Addr, ptmStatNode);
    } else {
        tmStatCbAddByMac(pucMac, ptmStatNode);
    }

    return  (ptmStatNode);
}
/*********************************************************************************************************
** ��������: __tmStatItemRemoveAll
** ��������: ͳ�����Ƴ�
** �䡡��  : ptmStat    ����������ƿ�
**           pvArg0     ����
**           pvArg1     ��δʹ��
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
static INT  __tmStatItemRemoveAll (PTM_STATS_INNER  ptmStat, PVOID  pvArg0, PVOID  pvArg1)
{
    INT  iType = *(INT*)pvArg0;

    if (ptmStat->cType == iType) {
       return  (STAT_ITEM_SHOULD_DELETE);
    }

    return  (0);
}
/*********************************************************************************************************
** ��������: tmStatItemAllTermRemove
** ��������: ͳ�����Ƴ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatItemAllTermRemove (VOID)
{
    INT  iType = TMT_TERM;

    tmWalkStats(__tmStatItemRemoveAll, &iType, LW_NULL, LW_NULL);

    return  (0);
}

/*********************************************************************************************************
** ��������: tmStatItemDelete
** ��������: ������ɾ��ͳ����
** �䡡��  : iArgC     �����в�������
**           ppcArgV   �����в�������
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatItemDelete (INT  iArgC, PCHAR  *ppcArgV)
{
    CHAR                cIfname[16] = {0};
    INT                 iType       = 0;
    struct sockaddr_in  sockAddr;
    ip4_addr_t          ip4Addr;

    if (iArgC >= 3) {
        if (!lib_strcmp(ppcArgV[2], "term")) {
            iType = TMT_TERM;
        } else if (!lib_strcmp(ppcArgV[2], "line")) {
            iType = TMT_LINE;
        } else if (!lib_strcmp(ppcArgV[2], "app")) {
            iType = TMT_APP;
        } else if (!lib_strcmp(ppcArgV[2], "lineapp")) {
            iType = TMT_LINE_APP;
        } else {
            goto  __arg_error;
        }
    }

    if (TMT_LINE == iType || TMT_LINE_APP == iType) {
        if(iArgC != 4) {
            goto  __arg_error;
        }

        lib_strlcpy(cIfname, ppcArgV[3], sizeof(cIfname));

        if (strlen(cIfname) == 0) {
            goto  __arg_error;
        }

    } else {
        if (iArgC != 4) {
            goto  __arg_error;
        }

        if (!inet_aton(ppcArgV[3], &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
            goto  __arg_error;
        }

        sockAddr.sin_len    = sizeof(struct sockaddr_in);
        sockAddr.sin_family = AF_INET;
        inet_addr_to_ip4addr(&ip4Addr, &((struct sockaddr_in *)&sockAddr)->sin_addr);
    }

    API_TimerCancel(_G_ulTmStatTimer);
    tmEnable(LW_FALSE);

    sleep(1);

    tmStatItemDel(cIfname, &ip4Addr);
    tmStartStats();

    return  (0);

__arg_error:
    fprintf(stderr, "arguments error!\n");

    return  (-ERROR_TSHELL_EPARAM);
}

/*********************************************************************************************************
** ��������: tmStatItemAdd
** ��������: ����������ͳ����
** �䡡��  : iArgC     �����в�������
**           ppcArgV   �����в�������
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmStatItemAdd (INT  iArgC, PCHAR  *ppcArgV)
{
    INT                 iType          = 0;
    ip4_addr_t          ip4Addr;
    INT                 iPort          = 0;
    INT                 iProtocol      = 0;
    UINT8               ucMac[6]       = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    CHAR                cIfname[16]    = {0};
    struct sockaddr_in  sockAddr;

    if (iArgC < 4) {
        goto   __arg_error;
    }

    if (!lib_strcmp(ppcArgV[2], "term")) {
        iType = TMT_TERM;
    } else if (!lib_strcmp(ppcArgV[2], "line") && iArgC == 4) {
        iType = TMT_LINE;

        lib_strlcpy(cIfname, ppcArgV[3], sizeof(cIfname));
        tmStatAllLinesInstall(cIfname);

        return  (ERROR_NONE);
    } else if (!lib_strcmp(ppcArgV[2], "app") && iArgC == 6) {
        iType = TMT_APP;

        if (!lib_strcmp(ppcArgV[4], "tcp")) {
            iProtocol = IPPROTO_TCP;
        } else if (!lib_strcmp(ppcArgV[4], "udp")) {
            iProtocol = IPPROTO_UDP;
        } else {
            goto    __arg_error;
        }

        iPort = atoi(ppcArgV[5]);
        if (-1 == iPort) {
            goto    __arg_error;
        }
    } else {
        goto    __arg_error;
    }

    if (!inet_aton(ppcArgV[3], &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
        goto    __arg_error;
    }

    sockAddr.sin_len    = sizeof(struct sockaddr_in);
    sockAddr.sin_family = AF_INET;
    inet_addr_to_ip4addr(&ip4Addr, &((struct sockaddr_in *)&sockAddr)->sin_addr);

    if(TMT_TERM == iType) {
        if (5 == iArgC) {
            if (strstr(ppcArgV[4],":")) {
                sscanf(ppcArgV[4], "%2x:%2x:%2x:%2x:%2x:%2x",
                                   (UINT*)&ucMac[0], (UINT*)&ucMac[1], (UINT*)&ucMac[2],
                                   (UINT*)&ucMac[3], (UINT*)&ucMac[4], (UINT*)&ucMac[5]);
            } else {
                fprintf(stderr, "please input mac address, format: 48:4d:7e:9f:bc:83\n");

                goto    __arg_error;
            }
        } else if (iArgC > 5)  {
            goto    __arg_error;
        }

        tmStatTermUserAdd(&ip4Addr, ucMac, cIfname);
    }  else if (TMT_APP == iType) {
        tmStatAppAdd(&ip4Addr, ucMac, iProtocol, (UINT16)iPort);
    }

    return  (ERROR_NONE);

__arg_error:
    fprintf(stderr, "arguments error!\n");

    return  (-ERROR_TSHELL_EPARAM);
}
