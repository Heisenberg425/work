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
** 文   件   名: libdhcpserver_config.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 18 日
**
** 描        述: DHCP Server 信息头文件
*********************************************************************************************************/
#ifndef __LIBDHCPSERVER_CONFIG_H
#define __LIBDHCPSERVER_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  DHCP Server 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 1)  /*  配置 DHCP Server            */
#define OPENRT_CFG_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 2)  /*  配置 DHCP Server 应答       */

#define OPENRT_GET_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 3)  /*  获取 DHCP Server 信息       */
#define OPENRT_GET_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 4)  /*  获取 DHCP Server 信息应答   */

#define OPENRT_DEL_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 5)  /*  删除 DHCP Server 信息       */
#define OPENRT_DEL_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 6)  /*  删除 DHCP Server 信息应答   */
/*********************************************************************************************************
 用于 DHCP Server 信息的结构
*********************************************************************************************************/
struct dhcpserver_handle {
    INT                 iEnable;                                        /*  启动状态：1 启动 0 禁止     */
    in_addr_t           uiIpStart;                                      /*  地址池开始地址              */
    in_addr_t           uiIpEnd;                                        /*  地址池结束地址              */
    CHAR                cInterface[NETIF_NAMESIZE];                     /*  服务使用的网卡名称          */
    in_addr_t           uiIpDns1;                                       /*  主 DNS                      */
    in_addr_t           uiIpDns2;                                       /*  备用 DSN                    */
    in_addr_t           uiIpMask;                                       /*  掩码                        */
    in_addr_t           uiIpGateway;                                    /*  网关                        */
    UINT                uiLease;                                        /*  租期 (s)                    */
};
typedef struct dhcpserver_handle   __DHCPSERVER_HANDLE;
typedef struct dhcpserver_handle  *__PDHCPSERVER_HANDLE;

#endif                                                                  /* __LIBDHCPSERVER_CONFIG_H     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
