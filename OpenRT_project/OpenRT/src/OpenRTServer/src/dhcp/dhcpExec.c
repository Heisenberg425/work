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
** ��   ��   ��: dhcpExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: DHCP ����
*********************************************************************************************************/
#include "SylixOS.h"
#include "lwip/netif.h"
#include "stdlib.h"
#include "net/if.h"
#include "dhcpExec.h"
#include "interface/interface_common.h"
#include "net/lwip/lwip_iphook.h"
#include "common.h"
/*********************************************************************************************************
  DHCP �����
*********************************************************************************************************/
#define DHCP_CLIENT_ENABLE_CMD    "-dhcp"                               /*  ʹ�� DHCP �ͻ�������        */
#define DHCP_CLIENT_DISABLE_CMD   "-nodhcp"                             /*  ���� DHCP �ͻ�������        */

/*********************************************************************************************************
** ��������: netIfDhcpClient
** ��������: ʹ�� / �������� DHCP �ͻ���
** �䡡��  : pDhcpClintHandle   �ͻ��˲����ṹ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT netIfDhcpClient (__PDHCP_CLIENT_SET  pDhcpClintHandle)
{
    INT              iRet        = PX_ERROR;
    __IFPARAM_INFO   ifParamInfo;
    CHAR             cCmd[32]    = {0};
    struct netif    *pNetif;
    struct netdev   *pNetDev;

    if (LW_NULL == pDhcpClintHandle) {
        return  (PX_ERROR);
    }

    if (DHCP_CLIENT_ENABLE  != pDhcpClintHandle->cHandle &&
        DHCP_CLIENT_DISABLE != pDhcpClintHandle->cHandle) {
        return  (PX_ERROR);
    }

    snprintf(cCmd,
             sizeof(cCmd),
             "ifdown %s",
             pDhcpClintHandle->cNetifName);

    system(cCmd);

    snprintf(cCmd,
             sizeof(cCmd),
             "ifup %s %s",
             pDhcpClintHandle->cNetifName,
             DHCP_CLIENT_ENABLE == pDhcpClintHandle->cHandle ?
             DHCP_CLIENT_ENABLE_CMD : DHCP_CLIENT_DISABLE_CMD);

    iRet = system(cCmd);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    /*
     *  ����Ϣ���浽ini��
     */
    bzero(&ifParamInfo, sizeof(ifParamInfo));
    ifParamInfo.uiUpdateFlag = IF_INI_DHCP_EN;
    ifParamInfo.cHandle      = INI_UPDATE_SEC;
    ifParamInfo.cDhcp        = pDhcpClintHandle->cHandle;

    /*
     *  ��ȡ dev name
     */
    if_list_lock(LW_FALSE);                                             /*  �����ٽ���                  */
    pNetif = netif_find(pDhcpClintHandle->cNetifName);
    if (LW_NULL == pNetif) {
        if_list_unlock();
        return  (PX_ERROR);
    }
    if_list_unlock();
    pNetDev = net_ip_hook_netif_get_netdev(pNetif);
    strncpy(ifParamInfo.cDevName, pNetDev->dev_name, sizeof(ifParamInfo.cDevName));

    iRet = ifParamIniFileHandle(&ifParamInfo);

    return  (iRet);
}
