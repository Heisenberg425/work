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
** 文   件   名: updatefirmware_common.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 3 日
**
** 描        述: update firmware头文件
*********************************************************************************************************/
#ifndef __UPDATEFIRMWARE_COMMON_H
#define __UPDATEFIRMWARE_COMMON_H

#include "libupdatefirmware_config.h"

/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT    updateFirmwareRecovery (VOID);
INT    updateFirmwareDoConfig (VOID);
VOID   updateFirmwareHandleEntry (INT  iCommand, PVOID  pvArg);

#endif                                                                  /*  __UPDATEFIRMWARE_COMMON_H   */
