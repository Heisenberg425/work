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
** 文   件   名: timesetEntry.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统时间配置入口
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
#include "timeset_common.h"
/*********************************************************************************************************
** 函数名称: timeShowDeal
** 功能描述: 配置系统时间处理函数
** 输　入  : pTmInfo  时间参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  timeShowDeal (__PTIME_HANDLE  pTmInfo)
{
    __TIME_HANDLE  timeInfo = {{0}};

    systemTimeGet(&timeInfo);

    replyQueryMsg(OPENRT_GET_TIME_ACK_CMD, sizeof(__TIME_HANDLE), 1 , &timeInfo);
}
/*********************************************************************************************************
** 函数名称: timeConfigDeal
** 功能描述: 配置系统时间处理函数
** 输　入  : pTmInfo  时间参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  timeConfigDeal (__PTIME_HANDLE  pTmInfo)
{
    replyConfigMsg(OPENRT_SET_TIME_ACK_CMD, timeDoConfig(pTmInfo));
}
/*********************************************************************************************************
** 函数名称: timeHandleEntry
** 功能描述: 时间配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  timeHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_SET_TIME_CMD:
        timeConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_TIME_CMD:
        timeShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
