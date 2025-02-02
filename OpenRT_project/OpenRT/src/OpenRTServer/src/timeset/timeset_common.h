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
** 文   件   名: timeset_common.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统时间信息头文件
*********************************************************************************************************/
#ifndef __TMSET_COMMON_H
#define __TMSET_COMMON_H

#include "libtmset_config.h"
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT  systemTimeGet(__PTIME_HANDLE  pTmInfo);
INT  timeDoConfig(__PTIME_HANDLE  pTmInfo);
VOID  timeHandleEntry(INT  iCommand, PVOID pArgBuf);

#endif                                                                  /* __TMSET_COMMON_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
