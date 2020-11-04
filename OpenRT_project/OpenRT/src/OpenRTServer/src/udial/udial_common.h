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
** 文   件   名: udial_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: 拨号处理共用头文件
*********************************************************************************************************/
#ifndef __UDIAL_COMMON_H
#define __UDIAL_COMMON_H

#include <SylixOS.h>
#include "libudial_config.h"
/*********************************************************************************************************
  EXPORT 拨号操作函数声明
*********************************************************************************************************/
INT         udialInit (VOID);
INT         natWanCfg (CPCHAR  cpcWanName);
INT         udialRecovery (VOID);
INT         udialStatusInit (VOID);
INT         udialTableCreate (VOID);
VOID        udialPppNumUpdate (VOID);
INT         netIfMaxIndexGet (VOID);
VOID        udialHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT         netif_get_max_index (VOID);
/*********************************************************************************************************
  EXPORT 拨号数据库函数声明
*********************************************************************************************************/
INT32  udialGetSpecInfo (__PUDIAL_HANDLE  pUdialInfo, CPCHAR  cpcField, PCHAR  pcArg, INT  *piArg);

INT    accountInfoGetFromIniFile (CPCHAR  cpcNetif, PCHAR  pcAccount, PCHAR  pcPasswd);

#endif                                                                  /*  __UDIAL_COMMON_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
