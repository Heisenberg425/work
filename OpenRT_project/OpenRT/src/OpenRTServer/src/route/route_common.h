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
** 描        述: route信息头文件
*********************************************************************************************************/

#ifndef __ROUTE_COMMON_H
#define __ROUTE_COMMON_H

#include "libroute_config.h"
/*********************************************************************************************************
  静态路由相关宏定义
*********************************************************************************************************/
#define HOST_ROUTE            "-host"                                   /*主机路由                      */
#define NET_ROUTE             "-net"                                    /*网络路由                      */
#define GATEWAY_ROUTE         "-gateway"                                /*网关路由                      */
#define DEFAULT_ROUTE         "default"                                 /*默认网关路由                  */
#define STATIC_ROUTE          "static"                                  /*静态路由                      */
#define IPv4                  "IPv4"                                    /*IPv4地址族路由                */
#define IPv6                  "IPv6"                                    /*IPv6地址族路由                */
#define IPv4_ROUTE_TABLE_HEAD "IPv4 Route Table"                        /*IPv4地址族表头                */
#define IPv6_ROUTE_TABLE_HEAD "IPv6 Route Table"                        /*IPv6地址族表头                */
#define AODV_ROUTE_TABLE_HEAD "AODV Route Table"                        /*AODV表头                      */
#define ROUTE_SHOW            "query"                                   /*查询路由表                    */
#define ROUTE_ADD             "add"                                     /*增加静态路由                  */
#define ROUTE_DEL             "del"                                     /*删除静态路由                  */
#define ROUTE_CHG             "chg"                                     /*修改静态路由                  */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT    routeDoConfig(__PROUTE_HANDLE  pRouteInfo);
PVOID  routeDoShow(__PROUTE_HANDLE  pRouteHandle, INT  *piRecordNum);
PVOID  routeGetFromDB(INT  *piRecordNum);
INT    staticRouteCleanFromDbByIfName(CPCHAR  cpcIfName);
INT    routeUpdateToDb(CPCHAR  cpcOperType, __PROUTE_HANDLE  pRouteInfo);
VOID   routeHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    routeTableCreate(VOID);
INT    routeRecovery(VOID);

#endif                                                                  /*  __ROUTE_COMMON_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
