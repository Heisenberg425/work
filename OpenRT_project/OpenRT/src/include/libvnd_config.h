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
** 文   件   名: libvpn_config.h
**
** 创   建   人: Li.xiaocheng (李孝成)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: OpenRT 配置工具和前端服务定义文件
*********************************************************************************************************/
#ifndef __LIBVND_CONFIG_H
#define __LIBVND_CONFIG_H

#include "lwip/ip4_addr.h"
#include "libserver_type.h"

/*********************************************************************************************************
  VPN 服务ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_ADD_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 1)        /*  添加 VND                    */
#define OPENRT_ADD_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 2)        /*  添加 VND   应答             */

#define OPENRT_DEL_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 3)        /*  删除 VND                    */
#define OPENRT_DEL_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 4)        /*  删除 VND 应答               */

#define OPENRT_GET_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 5)        /*  获取 VND 信息               */
#define OPENRT_GET_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 6)        /*  获取 VND 信息 应答          */

#define OPENRT_GET_IFINDEX_CMD     (OPEN_RT_VND_CONFIG_BASE | 7)        /*  获取 NETIF INDEX 信息       */
#define OPENRT_GET_IFINDEX_ACK_CMD (OPEN_RT_VND_CONFIG_BASE | 8)        /*  获取 NETIF INDEX 信息应答   */

#define OPENRT_GET_VNDID_CMD       (OPEN_RT_VND_CONFIG_BASE | 9)        /*  获取 VND ID 信息            */
#define OPENRT_GET_VNDID_ACK_CMD   (OPEN_RT_VND_CONFIG_BASE | 10)       /*  获取 VND ID 信息应答        */
/*********************************************************************************************************
  vnd 信息的结构
*********************************************************************************************************/
struct vnd_handle {
    INT                iVndId;                                          /*  虚拟网卡 ID                 */
    ip4_addr_t         uiIpAddr;                                        /*  IP 地址                     */
    ip4_addr_t         uiNetMask;                                       /*  子网掩码                    */
    ip4_addr_t         uiGateWay;                                       /*  网关地址                    */
    UCHAR              ucHwAddr[6];                                     /*  MAC 地址                    */
    INT                iMtu;                                            /*  MTU 值                      */
    INT                iNetIfIndex;                                     /*  索引值，如 vn7              */
};
typedef struct vnd_handle   __VND_HANDLE;
typedef struct vnd_handle  *__PVND_HANDLE;

#endif                                                                  /* __LIBVND_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
