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
** 文   件   名: routehandle.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 22 日
**
** 描        述:  路由配置处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "route_info.h"
#include "msgcomm.h"
#include "libroute_config.h"

/*********************************************************************************************************
** 函数名称: routeInfoQuery
** 功能描述: 获取路由信息
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : pRouteInfo        路由信息链表
*********************************************************************************************************/
PVOID  routeInfoQuery (__PROUTE_HANDLE  pRouteHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ROUTE_CMD, pRouteHandle, sizeof(__ROUTE_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: routeConfig
** 功能描述: 路由配置
** 输　入  : PROUTE_HANDLE pRouteHandle
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  routeConfig (__PROUTE_HANDLE pRouteHandle)
{
    return  requestConfigMsg (OPENRT_CFG_ROUTE_CMD, pRouteHandle, sizeof(__ROUTE_HANDLE));
}

/*********************************************************************************************************
** 函数名称: routeArrayFree
** 功能描述: 释放路由数组内存
** 输　入  : PROUTE_HANDLE pRouteHandle
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
VOID routeArrayFree(PVOID  pRouteHandle)
{
    if (pRouteHandle) {
        free(pRouteHandle);
    }
    return;
}
