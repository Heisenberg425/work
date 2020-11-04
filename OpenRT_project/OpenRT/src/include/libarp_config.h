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
** 文   件   名: libarp_config.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 11 日
**
** 描        述: ARP 信息头文件
*********************************************************************************************************/

#ifndef __LIBARP_CONFIG_H
#define __LIBARP_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"

/*********************************************************************************************************
  ARP 服务ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_ARP_CMD             (OPEN_RT_ARP_CONFIG_BASE | 1)    /*  配置 ARP                    */
#define OPENRT_CFG_ARP_ACK_CMD         (OPEN_RT_ARP_CONFIG_BASE | 2)    /*  配置 ARP 应答               */

#define OPENRT_GET_ARP_CMD             (OPEN_RT_ARP_CONFIG_BASE | 3)    /*  获取 ARP 信息               */
#define OPENRT_GET_ARP_ACK_CMD         (OPEN_RT_ARP_CONFIG_BASE | 4)    /*  获取 ARP 信息应答           */

#define OPENRT_GET_ARP_DYNAMIC_CMD     (OPEN_RT_ARP_CONFIG_BASE | 5)    /*  获取动态 ARP 信息           */
#define OPENRT_GET_ARP_DYNAMIC_ACK_CMD (OPEN_RT_ARP_CONFIG_BASE | 6)    /*  获取动态 ARP 信息应答       */
/*********************************************************************************************************
 用于 ARP 信息的结构
*********************************************************************************************************/
struct arp_handle {
    in_addr_t              uiIpAddr;                                    /*  IP 地址                     */
    UCHAR                  ucHwAddr[6];                                 /*  MAC 地址                    */
    CHAR                   cOperType[64];                               /*  操作类型                    */
    CHAR                   cStatus[20];                                 /*  当前状态                    */
};
typedef struct arp_handle   __ARP_HANDLE;
typedef struct arp_handle  *__PARP_HANDLE;

#endif                                                                  /*  __LIBARP_CONFIG_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
