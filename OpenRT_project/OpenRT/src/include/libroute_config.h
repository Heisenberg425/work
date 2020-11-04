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
** 文   件   名: route_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 22 日
**
** 描        述: route 信息头文件
*********************************************************************************************************/

#ifndef __LIBROUTE_CONFIG_H
#define __LIBROUTE_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  route 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_ROUTE_CMD         (OPEN_RT_ROUTE_CONFIG_BASE | 1)    /*  配置静态路由                */
#define OPENRT_CFG_ROUTE_ACK_CMD     (OPEN_RT_ROUTE_CONFIG_BASE | 2)    /*  配置静态路由应答            */

#define OPENRT_GET_ROUTE_CMD         (OPEN_RT_ROUTE_CONFIG_BASE | 3)    /*  获取静态路由信息            */
#define OPENRT_GET_ROUTE_ACK_CMD     (OPEN_RT_ROUTE_CONFIG_BASE | 4)    /*  获取静态路由信息应答        */
/*********************************************************************************************************
  用于路由管理信息的结构
*********************************************************************************************************/
struct route_handle {
    CHAR  cDestination[64];                                             /*  目的地址                    */
    CHAR  cGateway[64];                                                 /*  路由网关                    */
    CHAR  cGenmask[20];                                                 /*  掩码                        */
    CHAR  cFlags[12];                                                   /*  路由类型                    */
    INT   iPrefix;                                                      /*  ipv6 路由前缀               */
    INT   iMetric;                                                      /*  路由度量值                  */
    INT   iRef;                                                         /*  路由 ref                    */
    INT   iUse;                                                         /*  路由 use                    */
    CHAR  cIface[NETIF_NAMESIZE];                                       /*  路由出接口                  */
    CHAR  cIpFamily[8];                                                 /*  路由地址族，ipv4 和 ipv6    */
    CHAR  cOperType[8];                                                 /*  路由操作类型，增删改        */
    CHAR  cRouteType[12];                                               /*  路由种类，主机、网络、静态等*/
};
typedef struct route_handle  __ROUTE_HANDLE;
typedef struct route_handle  *__PROUTE_HANDLE;

#endif                                                                  /*  __LIBROUTE_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
