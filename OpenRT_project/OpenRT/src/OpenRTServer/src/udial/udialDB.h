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
** 文   件   名: udialDB.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录数据库公用头文件
*********************************************************************************************************/
#ifndef __UDIAL_DB_H
#define __UDIAL_DB_H
#include "libudial_config.h"

/*********************************************************************************************************
  udial 数据库函数声明
*********************************************************************************************************/
INT     udialTableCreate (VOID);
INT     udialUpdateToDb (__PUDIAL_HANDLE  pUdialInfo);
INT32   udialGetSpecInfo (__PUDIAL_HANDLE  pUdialInfo, CPCHAR  cpcField, PCHAR  pcArg, INT  *piArg);

#endif                                                                  /* __UDIAL_DB_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
