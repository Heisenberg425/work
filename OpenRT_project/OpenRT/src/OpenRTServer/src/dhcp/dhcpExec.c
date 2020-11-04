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
** 文   件   名: dhcpExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: DHCP 操作
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
  DHCP 命令宏
*********************************************************************************************************/
#define DHCP_CLIENT_ENABLE_CMD    "-dhcp"                               /*  使能 DHCP 客户端命令        */
#define DHCP_CLIENT_DISABLE_CMD   "-nodhcp"                             /*  禁能 DHCP 客户端命令        */

/*********************************************************************************************************
** 函数名称: netIfDhcpClient
** 功能描述: 使能 / 禁能网口 DHCP 客户端
** 输　入  : pDhcpClintHandle   客户端操作结构
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  将信息保存到ini中
     */
    bzero(&ifParamInfo, sizeof(ifParamInfo));
    ifParamInfo.uiUpdateFlag = IF_INI_DHCP_EN;
    ifParamInfo.cHandle      = INI_UPDATE_SEC;
    ifParamInfo.cDhcp        = pDhcpClintHandle->cHandle;

    /*
     *  获取 dev name
     */
    if_list_lock(LW_FALSE);                                             /*  进入临界区                  */
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
