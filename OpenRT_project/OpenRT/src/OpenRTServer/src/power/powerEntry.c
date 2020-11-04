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
** 文   件   名: powerEntry.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统电源状态配置入口
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
#include "power_common.h"
/*********************************************************************************************************
** 函数名称: powerOffDeal
** 功能描述: 系统电源关闭处理函数
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerOffDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerDoOff(pPowerInfo);

    /*
     *  由于关机，所以程序运行不到该处
     */
    replyConfigMsg(OPENRT_POWER_OFF_ACK_CMD, iRet);
}
/*********************************************************************************************************
** 函数名称: powerRestartDeal
** 功能描述: 系统重启处理函数
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerRestartDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerDoRestart(pPowerInfo);

    /*
     *  由于重启，所以程序运行不到该处
     */
    replyConfigMsg(OPENRT_POWER_RESTART_ACK_CMD, iRet);
}
/*********************************************************************************************************
** 函数名称: powerEventConfigDeal
** 功能描述: 电源事件配置处理函数
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerEventConfigDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerEventDoConfig(pPowerInfo);

    replyConfigMsg(OPENRT_POWER_EVT_CFG_ACK_CMD, iRet);
}
/*********************************************************************************************************
** 函数名称: powerEventShowDeal
** 功能描述: 电源事件显示处理函数
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerEventShowDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT              iRecordNum;
    __PPOWER_HANDLE  pPowerArrayHeader = LW_NULL;

    pPowerArrayHeader = powerEventGetFromDB(&iRecordNum);

    replyQueryMsg(OPENRT_POWER_EVT_SHOW_ACK_CMD, sizeof(__POWER_HANDLE), iRecordNum , pPowerArrayHeader);

    free(pPowerArrayHeader);
}
/*********************************************************************************************************
** 函数名称: powerHandleEntry
** 功能描述: 系统电源配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_POWER_OFF_CMD:
        powerOffDeal(pArgBuf);
        break;

    case OPENRT_POWER_RESTART_CMD:
        powerRestartDeal(pArgBuf);
        break;

    case OPENRT_POWER_EVT_CFG_CMD:
        powerEventConfigDeal(pArgBuf);
        break;

    case OPENRT_POWER_EVT_SHOW_CMD:
        powerEventShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
