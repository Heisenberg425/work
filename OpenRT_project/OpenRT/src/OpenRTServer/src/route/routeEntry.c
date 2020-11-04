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
** 文   件   名: routeEntry.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: OpenRT 路由配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "libroute_config.h"
#include "route_common.h"
/*********************************************************************************************************
** 函数名称: routeConfigDeal
** 功能描述: 路由配置处理函数
** 输　入  : pRouteHandle           前端传入的路由信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  routeConfigDeal (__PROUTE_HANDLE  pRouteHandle)
{
    INT iRet = 0;

    iRet = routeDoConfig(pRouteHandle);

    replyConfigMsg(OPENRT_CFG_ROUTE_ACK_CMD, iRet);
}
/*********************************************************************************************************
** 函数名称: routeShowDeal
** 功能描述: 路由查询处理函数
** 输　入  : pRouteHandle           前端传入的路由信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  routeShowDeal (__PROUTE_HANDLE  pRouteHandle)
{
    __PROUTE_HANDLE pRouteArrayHeader = LW_NULL;
    INT             iRecordNum        = 0;

    pRouteArrayHeader = routeDoShow(pRouteHandle, &iRecordNum);

    replyQueryMsg(OPENRT_GET_ROUTE_ACK_CMD, sizeof(__ROUTE_HANDLE), iRecordNum , pRouteArrayHeader);

    free(pRouteArrayHeader);
}
/*********************************************************************************************************
** 函数名称: routeHandleEntry
** 功能描述: 路由配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  routeHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_ROUTE_CMD:
        routeConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_ROUTE_CMD:
        routeShowDeal(pArgBuf);
        break;
    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
