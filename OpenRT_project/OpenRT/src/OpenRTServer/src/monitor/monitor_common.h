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
** 文   件   名: monitor_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 16 日
**
** 描        述: monitor信息头文件
*********************************************************************************************************/

#ifndef __MONITOR_COMMON_H
#define __MONITOR_COMMON_H

#include "libmonitor_config.h"

/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT          monitorDoConfig (PTM_STATS  pMonitorInfo);
PTM_STATS    monitorDoShow(PTM_STATS     pMonitorHandle, INT   *piRecordNum);
PTM_CPU_MEM  cpuMemDoShow(PTM_STATS      pMonitorHandle, INT   *piRecordNum);
VOID         monitorHandleEntry (INT     iCommand,       PVOID  pArgBuf);
INT32        internetStatusGet (__PINTERNET_STATUS  pInternetStatus);

#endif                                                                  /* __MONITOR_COMMON_H           */
