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
** 文   件   名: flowctlhandle.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 29 日
**
** 描        述:  流控配置处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "flowctl_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: flowctlInfoFree
** 功能描述: 释放流控数组
** 输　入  : pFlowctlArrayTableHead           流控信息数组
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  flowctlInfoFree (PVOID pFlowctlArrayTableHead)
{
    if (pFlowctlArrayTableHead) {
        free(pFlowctlArrayTableHead);
    }

    return;
}

/*********************************************************************************************************
** 函数名称: flowctlShow
** 功能描述: 查询流控信息
** 输　入  : pFlowctlHandle    流控入参信息
** 输　出  : piRecordTotal     流控记录查询结果总数
** 返　回  : 流控查询结果数组指针
*********************************************************************************************************/
PVOID  flowctlShow (__PFLOWCTL_HANDLE  pFlowctlHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_FLOWCTL_CMD, pFlowctlHandle, sizeof(__FLOWCTL_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: flowctlConfig
** 功能描述: 配置流控
** 输　入  : pFlowctlHandle      流控入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  flowctlConfig (__PFLOWCTL_HANDLE pFlowctlHandle)
{
   return  requestConfigMsg (OPENRT_CFG_FLOWCTL_CMD, pFlowctlHandle, sizeof(__FLOWCTL_HANDLE));
}
