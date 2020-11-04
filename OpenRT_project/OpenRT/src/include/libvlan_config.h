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
** 文   件   名: vlan_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: vlan 信息头文件
*********************************************************************************************************/

#ifndef __LIBVLAN_CONFIG_H
#define __LIBVLAN_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  vlan 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_VLAN_CMD         (OPEN_RT_VLAN_CONFIG_BASE | 1)      /*  配置 VLAN                   */
#define OPENRT_CFG_VLAN_ACK_CMD     (OPEN_RT_VLAN_CONFIG_BASE | 2)      /*  配置 VLAN 应答              */

#define OPENRT_GET_VLAN_CMD         (OPEN_RT_VLAN_CONFIG_BASE | 3)      /*  获取 VLAN 信息              */
#define OPENRT_GET_VLAN_ACK_CMD     (OPEN_RT_VLAN_CONFIG_BASE | 4)      /*  获取 VLAN 信息应答          */
/*********************************************************************************************************
 用于 vlan 信息的结构
*********************************************************************************************************/
struct vlan_handle {
    INT  iVlanID;                                                       /*  vlan 标识                   */
    CHAR cIface[NETIF_NAMESIZE];                                        /*  接口名称                    */
    INT  iTag;                                                          /*  vlan tag                    */
    INT  iPriority;                                                     /*  vlan 优先级                 */
    CHAR cOperType[8];                                                  /*  操作类型，增删改            */
};
typedef struct vlan_handle  __VLAN_HANDLE;
typedef struct vlan_handle  *__PVLAN_HANDLE;

#endif                                                                  /*  __LIBVLAN_CONFIG_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
