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
** ��   ��   ��: intAdaptionExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ����˳������Ӧִ�нӿ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"
#include "netdev.h"
#include "common.h"
#include "intada_common.h"
#include "intAdaptionExec.h"
#include "net/lwip/lwip_iphook.h"
/*********************************************************************************************************
** ��������: __onlyNetifLinkUpGet
** ��������: ��ȡ����������Ψһ���ӳɹ�������
** �䡡��  : NONE
** �䡡��  : pcNetifName        ��������
** ������  : error code
*********************************************************************************************************/
static INT __onlyNetifLinkUpGet (CHAR   *pcNetifName)
{
    INT                    iCount      = 0;
    INT32                  iSocketFd;
    struct  ifconf         iconf;
    INT32                  iRet, i;
    INT32                  iBuffSize;

    struct ifreq          *pIfreq     = LW_NULL;
    PCHAR                  pcIconfBuf = LW_NULL;


    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");
        return  (PX_ERROR);
    }

    /*
     *  ��ȡ��Ҫ���� buf ���
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;
    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");
        close(iSocketFd);

        goto  __error;
    }

    /*
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");

            goto  __error;
        }

        /*
         * ֻ��������̫������ͳ��
         */
        if (IFT_ETHER == pIfreq->ifr_type && (!strstr(pIfreq->ifr_name, "vn"))) {
            iRet = ioctl(iSocketFd, SIOCGIFFLAGS, pIfreq);              /*  ��ȡ������־                */
            if (iRet < 0) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFFLAGS error.\n");

                goto  __error;
            }
            if (IFF_RUNNING == (pIfreq->ifr_flags & IFF_RUNNING)) {     /*  ��ǰ��������״̬            */
                iCount++;
                if (iCount >= 2) {                                      /*  ������������Ψһ            */
                    goto  __error;
                } else {
                    strncpy(pcNetifName, pIfreq->ifr_name, sizeof(pIfreq->ifr_name));
                }
            }
        }
    }

    close(iSocketFd);
    free(pcIconfBuf);
    iRet = (1 == iCount) ? ERROR_NONE : PX_ERROR;
    return  (iRet);

__error:
    close(iSocketFd);
    free(pcIconfBuf);
    return  (PX_ERROR);
}

/*********************************************************************************************************
** ��������: intAdaptionCheck
** ��������: �Ƿ����������˳������Ӧ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           û���
**           ERROR_NONE         ���
*********************************************************************************************************/
INT intAdaptionCheck (VOID)
{
    return  intAdaptionDBFindFinish();
}

/*********************************************************************************************************
** ��������: intAdaptionIsExist
** ��������: �ж�ǰ̨��ĳ�������Ƿ��Ѿ����������ڶ�λ
** �䡡��  : iWebIndex          ǰ̨��������
** �䡡��  : NONE
** ������  : PX_ERROR           û���
**           ERROR_NONE         ���
*********************************************************************************************************/
INT  intAdaptionIsExist (INT  iWebIndex)
{
    __INTADAPTION_HANDLE_INFO  netInfo;

    bzero(&netInfo, sizeof(netInfo));

    netInfo.iWebIndex = iWebIndex;

    return  intAdaptionDBGetInfo(&netInfo, 0);
}

/*********************************************************************************************************
** ��������: intAdaptionGetNetif
** ��������: ͨ��ǰ���������ƻ�ȡ��Ӧ�� netif ����
** �䡡��  : cpcWebNetifName    ǰ��������
** �䡡��  : pcNetifName        Netif����
** ������  : PX_ERROR           û���
**           ERROR_NONE         ���
*********************************************************************************************************/
INT  intAdaptionGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cWebName, cpcWebNetifName, sizeof(netInfo.cWebName));

    iRet = intAdaptionDBGetInfo(&netInfo, 2);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcNetifName, netInfo.cNetIfName, strlen(netInfo.cNetIfName));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: intAdaptionGetWebNetif
** ��������: ͨ���������ƻ�ȡ��Ӧ��WEB ��������
** �䡡��  : cpcNetifName          Netif����
** �䡡��  : pcWebNetifName        ǰ��������
** ������  : PX_ERROR              û���
**           ERROR_NONE            ���
*********************************************************************************************************/
INT  intAdaptionGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcWebNetifName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cNetIfName, cpcNetifName, sizeof(netInfo.cNetIfName));

    iRet = intAdaptionDBGetInfo(&netInfo, 1);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcWebNetifName, netInfo.cWebName, strlen(netInfo.cWebName));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: intAdaptionGetWebName
** ��������: ͨ�� netif ���ƻ��ǰ��������Ϣ
** �䡡��  : pcNetifName        Netif����
** �䡡��  : pcWebName          ǰ����������
**           piWebIndex         ǰ����������
** ������  : PX_ERROR           û���
**           ERROR_NONE         ���
*********************************************************************************************************/
INT intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcNetifName || LW_NULL == pcWebName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cNetIfName, pcNetifName, sizeof(netInfo.cNetIfName));
    iRet = intAdaptionDBGetInfo(&netInfo, 1);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcWebName, netInfo.cWebName, strlen(netInfo.cWebName));
    *piWebIndex = netInfo.iWebIndex;

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: intAdaptionClear
** ��������: ������ڶ�λ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID intAdaptionClear (VOID)
{
    intAdaptionDBClear();
}

/*********************************************************************************************************
** ��������: intAdaptionHandle
** ��������: ��������Ӧ����
** �䡡��  : pWebInfo           WEB ��Ϣ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT intAdaptionHandle (__PINT_ADA_SET  pcIntAdaptionInfo)
{
    CHAR   cNetifName[NETIF_NAMESIZE] = {0};
    INT    iRet;

    __INTADAPTION_HANDLE_INFO   intadaptionHandle;
    __INTADAPTION_HANDLE_INFO   NetInfo;

    if (LW_NULL == pcIntAdaptionInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement is null\n");

       return  (PX_ERROR);
    }

    if (INT_ADA_FLAG_FIRST == pcIntAdaptionInfo->cFlag) {               /*  ��һ��������Ҫ������ݿ�    */
        intAdaptionDBClear();
    }

    iRet = __onlyNetifLinkUpGet(cNetifName);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_LOG_LEVEL, "there is no only link netif\n");

        return  (PX_ERROR);
    }

    bzero(&NetInfo, sizeof(NetInfo));

    strncpy(NetInfo.cNetIfName, cNetifName, sizeof(NetInfo.cNetIfName));

    iRet = intAdaptionDBGetInfo(&NetInfo, 1);                           /*  ͨ�� netif �����Ҷ�Ӧ���豸 */
    if (ERROR_NONE == iRet) {                                           /*  �Ѿ�������Ϣ                */
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "netif info has saved\n");

        return  (PX_ERROR);
    }

    strncpy(intadaptionHandle.cNetIfName, cNetifName, sizeof(intadaptionHandle.cNetIfName));
    strncpy(intadaptionHandle.cWebName,   pcIntAdaptionInfo->cWebName, sizeof(intadaptionHandle.cWebName));
    strncpy(intadaptionHandle.cStatus,    NET_ADAPTING, sizeof(intadaptionHandle.cStatus));
    intadaptionHandle.iWebIndex = pcIntAdaptionInfo->iWebIndex;

    if (INT_ADA_FLAG_LAST == pcIntAdaptionInfo->cFlag) {                /*  ���һ��������Ҫ��������״̬*/
        intAdaptionDBSetFinish();

        bzero(intadaptionHandle.cStatus, sizeof(intadaptionHandle.cStatus));

        strncpy(intadaptionHandle.cStatus, NET_FINISH, sizeof(intadaptionHandle.cStatus));
    }

    return (intAdaptionDBUpdate(&intadaptionHandle));
}
