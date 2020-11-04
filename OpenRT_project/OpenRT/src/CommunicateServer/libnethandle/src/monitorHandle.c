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
** 文   件   名: monitorHandle.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 16 日
**
** 描        述:  流控配置处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "monitor_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: monitorInfoFree
** 功能描述: 释放监控数组
** 输　入  : pMonitorArrayTableHead           流控信息数组
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  monitorInfoFree (PVOID pMonitorArrayTableHead)
{
    if (pMonitorArrayTableHead) {
        free(pMonitorArrayTableHead);
    }

    return;
}

/*********************************************************************************************************
** 函数名称: monitorShow
** 功能描述: 查询监控信息
** 输　入  : pMonitorHandle    监控入参信息
** 输　出  : piRecordTotal     监控记录查询结果总数
** 返　回  : 监控查询结果数组指针
*********************************************************************************************************/
PVOID  monitorShow (PTM_STATS  pMonitorHandle, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_GET_MONITOR_CMD, pMonitorHandle, sizeof(TM_STATS), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: cpuMemShow
** 功能描述: 查询CPU/MEMORY/在线用户数信息
** 输　入  : pMonitorHandle    监控入参信息
** 输　出  : piRecordTotal     监控记录查询结果总数
** 返　回  : 监控查询结果数组指针
*********************************************************************************************************/
PVOID  cpuMemShow (PTM_STATS  pMonitorHandle, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_GET_CPU_MEM_CMD, pMonitorHandle, sizeof(TM_STATS), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: monitorConfig
** 功能描述: 配置监控
** 输　入  : pMonitorHandle      监控入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  monitorConfig (PTM_STATS  pMonitorHandle)
{
    return  requestConfigMsg(OPENRT_CFG_MONITOR_CMD, pMonitorHandle, sizeof(TM_STATS));
}

/*********************************************************************************************************
** 函数名称: libInternetStatusMonitor
** 功能描述: 获取互联网连接状态
** 输　入  : pMonitorHandle      监控入参信息
** 输　出  : pInternetStatus     互联网连接结果
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT32  libInternetStatusMonitor (__PINTERNET_STATUS  pInternetStatus)
{
    __PINTERNET_STATUS  pTmp = LW_NULL;
    INT32               iNum = 0;
    if (LW_NULL == pInternetStatus) {
        return  (PX_ERROR);
    }

    pTmp = requestQueryMsg(OPENRT_GET_INTERNET_CMD, NULL, 0, &iNum);
    if (LW_NULL == pTmp || 0 == iNum) {
        return  (PX_ERROR);
    }

    memcpy(pInternetStatus, pTmp, sizeof(__INTERNET_STATUS));

    if (NULL != pTmp) {
        free(pTmp);
    }

    return  (ERROR_NONE);
}
