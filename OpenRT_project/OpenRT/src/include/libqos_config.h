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
** 文   件   名: libqos_config.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 15 日
**
** 描        述: QoS 信息头文件
*********************************************************************************************************/

#ifndef __LIBQOS_CONFIG_H
#define __LIBQOS_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  QoS 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 1)    /*  配置 QoS                    */
#define OPENRT_CFG_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 2)    /*  配置 QoS 应答               */

#define OPENRT_DEL_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 3)    /*  删除 QoS                    */
#define OPENRT_DEL_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 4)    /*  删除 QoS 应答               */

#define OPENRT_GET_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 5)    /*  获取 QoS 信息               */
#define OPENRT_GET_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 6)    /*  获取 QoS 信息应答           */
/*********************************************************************************************************
  用于QoS信息的结构
*********************************************************************************************************/
struct qos_handle {
    in_addr_t  uiIpStart;                                               /*  起始 ip                     */
    in_addr_t  uiIpEnd;                                                 /*  终止 ip                     */
    UINT16     uiPortStart;                                             /*  起始 port                   */
    UINT16     uiPortEnd;                                               /*  终止 port                   */
    UINT8      uiPriority;                                              /*  优先级                      */
    UINT8      uiDontDrop;                                              /*  不丢包                      */
    CHAR       cNetif[NETIF_NAMESIZE];                                  /*  网卡接口                    */
    CHAR       cWebName[NETIF_NAMESIZE];                                /*  网卡接口                    */
    CHAR       cRule[10];                                               /*  规则                        */
    CHAR       cCmpMethod[10];                                          /*  比较方法                    */
};
typedef struct qos_handle   __QOS_HANDLE;
typedef struct qos_handle  *__PQOS_HANDLE;

#endif                                                                  /*  __LIBQOS_CONFIG_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
