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
** 文   件   名: flowctlEntry.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 29 日
**
** 描        述: OpenRT 流控配置处理入口
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
#include "flowctl_common.h"
/*********************************************************************************************************
** 函数名称: flowctlConfigDeal
** 功能描述: 配置流控处理函数
** 输　入  : pFlowctlHandle     flowctl参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  flowctlConfigDeal (__PFLOWCTL_HANDLE  pFlowctlHandle)
{
    replyConfigMsg(OPENRT_CFG_FLOWCTL_ACK_CMD, flowctlDoConfig(pFlowctlHandle));
}

/*********************************************************************************************************
** 函数名称: flowctlShowDeal
** 功能描述: 查询流控处理函数
** 输　入  : pFlowctlHandle     flowctl参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  flowctlShowDeal (__PFLOWCTL_HANDLE  pFlowctlHandle)
{
    __PFLOWCTL_HANDLE  pFlowctlHeader = LW_NULL;
    INT                iRecordNum     = 0;

    pFlowctlHeader = flowctlGetFromDB(pFlowctlHandle, &iRecordNum);

    replyQueryMsg(OPENRT_GET_FLOWCTL_ACK_CMD, sizeof(__FLOWCTL_HANDLE), iRecordNum , pFlowctlHeader);

    if (NULL != pFlowctlHeader) {
        free(pFlowctlHeader);
    }
}
/*********************************************************************************************************
** 函数名称: flowctlHandleEntry
** 功能描述: 流控配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID flowctlHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_FLOWCTL_CMD:
        flowctlConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_FLOWCTL_CMD:
        flowctlShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}

