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
** 文   件   名: powerHandle.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统电源管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "power_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: powerOff
** 功能描述: 配置系统电源关机
** 输　入  : pPowerHandle      系统电源入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  powerOff (__PPOWER_HANDLE  pPowerHandle)
{
    return  requestConfigMsg (OPENRT_POWER_OFF_CMD, pPowerHandle, sizeof(__POWER_HANDLE));
}
/*********************************************************************************************************
** 函数名称: powerRestart
** 功能描述: 配置系统电源重启
** 输　入  : pPowerHandle      系统电源入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  powerRestart (__PPOWER_HANDLE  pPowerHandle)
{
    return  requestConfigMsg (OPENRT_POWER_RESTART_CMD, pPowerHandle, sizeof(__POWER_HANDLE));
}
/*********************************************************************************************************
** 函数名称: powerConfig
** 功能描述: 查询电源事件
** 输　入  : pPowerHandle      系统电源入参信息
** 输　出  : piRecordTotal     power event 查询结果总数
** 返　回  : 查询结果数组指针
*********************************************************************************************************/
PVOID  powerEventShow (__PPOWER_HANDLE  pPowerHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_POWER_EVT_SHOW_CMD,
                            pPowerHandle,
                            sizeof(__POWER_HANDLE),
                            piRecordTotal);
}
/*********************************************************************************************************
** 函数名称: powerConfig
** 功能描述: 配置系统电源状态
** 输　入  : pPowerHandle      系统电源入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  powerEventConfig (__PPOWER_HANDLE  pPowerHandle)
{
    return  requestConfigMsg (OPENRT_POWER_EVT_CFG_CMD, pPowerHandle, sizeof(__POWER_HANDLE));
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
