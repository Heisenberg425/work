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
** 文   件   名: libinterface_config.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: 网口设置相关头文件
*********************************************************************************************************/
#ifndef __LIBINTERFACE_CONFIG_H
#define __LIBINTERFACE_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  网口服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_NETINFO_GET_CMD      (OPEN_RT_INTERFACE_CONFIG_BASE | 1) /*  获取所有网口信息            */
#define OPENRT_NETINFO_GET_ACK_CMD  (OPEN_RT_INTERFACE_CONFIG_BASE | 2) /*  获取所有网口应答            */

#define OPENRT_NETINFO_SET_CMD      (OPEN_RT_INTERFACE_CONFIG_BASE | 3) /*  设置指定网口参数            */
#define OPENRT_NETINFO_SET_ACK_CMD  (OPEN_RT_INTERFACE_CONFIG_BASE | 4) /*  设置指定网口参数应答        */

/*********************************************************************************************************
  cNatFunction 字段
*********************************************************************************************************/
#define NETIF_ERROR            (-1)                                     /*  网口信息无效                */
#define NETIF_NORMAL           (0)                                      /*  正常网口                    */
#define NETIF_WAN              (1)                                      /*  用作 WAN 口                 */
#define NETIF_LAN              (2)                                      /*  用作 LAN 口                 */
/*********************************************************************************************************
  cNetType 字段
*********************************************************************************************************/
#define NETIF_ADSL             (3)                                      /*  拨号接入外网网口            */
#define NETIF_DHCP             (4)                                      /*  DHCP 接入外网网口           */
#define NETIF_STATIC           (5)                                      /*  静态 IP 接入外网网口        */
/*********************************************************************************************************
  网口设置返回值
*********************************************************************************************************/
#define RET_COLLISION         (-2)                                      /*  网口 IP 冲突                */
/*********************************************************************************************************
  IP buff长度
*********************************************************************************************************/
#define IP_BUF_LEN             (16)                                     /*  IP buff 长度                */
/*********************************************************************************************************
  单个网卡信息结构
*********************************************************************************************************/
struct netif_info {
    CHAR                       cNetifName[NETIF_NAMESIZE];              /*  接口名称                    */
    CHAR                       cDevName[NETIF_NAMESIZE];                /*  设备名称                    */
    CHAR                       cWebName[NETIF_NAMESIZE];                /*  Web 网口名称                */
    UINT8                      ucMac[NETIF_MAX_HWADDR_LEN];             /*  MAC 地址                    */
    INT32                      iWebIndex;                               /*  Web 网口偏移                */
    CHAR                       cNatFunction;                            /*  网口功能                    */
    CHAR                       cNetType;                                /*  网口类型                    */
    INT32                      iLinkFlag;                               /*  连接状态                    */
    UINT32                     uiIp;                                    /*  IP地址                      */
    UINT32                     uiNetMask;                               /*  掩码地址                    */
    UINT32                     uiGateWay;                               /*  网关地址                    */
    UINT32                     uiSpeed;                                 /*  网速                        */
};
typedef struct netif_info   __NETIF_INFO;
typedef struct netif_info*  __PNETIF_INFO;
/*********************************************************************************************************
  设置网口结构
*********************************************************************************************************/
struct netif_set {
    CHAR                       cNetifName[NETIF_NAMESIZE];              /*  接口名称                    */
    UINT8                      ucMac[NETIF_MAX_HWADDR_LEN];             /*  MAC 地址                    */
    INT                        iLinkFlag;                               /*  连接状态                    */
    CHAR                       cIpAddr[IP_BUF_LEN];                     /*  IP 地址                     */
    CHAR                       cNetMask[IP_BUF_LEN];                    /*  子网掩码                    */
    CHAR                       cGateWay[IP_BUF_LEN];                    /*  网关地址                    */
    UINT32                     uiSpeed;                                 /*  网速                        */
};
typedef struct netif_set   __NETIF_SET;
typedef struct netif_set*  __PNETIF_SET;

#endif                                                                  /* __LIBINTERFACE_CONFIG_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
