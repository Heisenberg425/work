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
** 文   件   名: flowctl_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 29 日
**
** 描        述: flowctl 信息头文件
*********************************************************************************************************/

#ifndef __LIBFLOWCTL_CONFIG_H
#define __LIBFLOWCTL_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  route 服务ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_FLOWCTL_CMD     (OPEN_RT_FLOWCTL_CONFIG_BASE | 1)    /*  配置流控                    */
#define OPENRT_CFG_FLOWCTL_ACK_CMD (OPEN_RT_FLOWCTL_CONFIG_BASE | 2)    /*  配置流控应答                */
#define OPENRT_GET_FLOWCTL_CMD     (OPEN_RT_FLOWCTL_CONFIG_BASE | 3)    /*  获取流控信息                */
#define OPENRT_GET_FLOWCTL_ACK_CMD (OPEN_RT_FLOWCTL_CONFIG_BASE | 4)    /*  获取流控信息应答            */

#define FCT_IP                     (1)                                  /*  对协议进行流控              */
#define FCT_IF                     (2)                                  /*  对网口进行流控              */
/*********************************************************************************************************
 用于 flowctl 信息的结构
*********************************************************************************************************/
struct flowctl_handle {
    INT     iType;                                                      /*  1: 对协议进行流控
                                                                            2: 对网口进行流控           */
    CHAR    cEnable[8];                                                 /*  1: 使能 0: 去使能           */
    CHAR    cProtocol[8];                                               /*  生效协议类型 tcp/udp/all    */
    CHAR    cStartIp[64];                                               /*  流控起始 IP 地址            */
    CHAR    cEndIp[64];                                                 /*  流控结束 IP 地址            */
    INT     iStartPort;                                                 /*  流控起始端口                */
    INT     iEndPort;                                                   /*  流控结束端口                */
    CHAR    cIfname[NETIF_NAMESIZE];                                    /*  网口                        */
    ULONG   ulUpRate;                                                   /*  上行流量（KB/s）            */
    ULONG   ulDownRate;                                                 /*  下行流量（KB/s）            */
    ULONG   ulBufSize;                                                  /*  缓冲区大小(16K ~ 128M Bytes)*/
    CHAR    cIpFamily[8];                                               /*  ip 地址族，IPv4 或 IPv6     */
    CHAR    cOperType[8];                                               /*  操作类型，增删改            */
};
typedef struct flowctl_handle   __FLOWCTL_HANDLE;
typedef struct flowctl_handle  *__PFLOWCTL_HANDLE;

#endif                                                                  /* __LIBFLOWCTL_CONFIG_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
