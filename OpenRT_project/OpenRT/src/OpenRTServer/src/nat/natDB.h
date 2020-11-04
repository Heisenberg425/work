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
** 文   件   名: natDB.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录数据库公用头文件
*********************************************************************************************************/

#ifndef __NAT_DB_H
#define __NAT_DB_H

#include "nat_common.h"
/*********************************************************************************************************
  NAT 数据库函数声明
*********************************************************************************************************/
INT    natTableCreate (VOID);
INT    natLanUpdateToDB (VOID);
INT    natWanUpdateToDB (VOID);
INT    natMapUpdateToDB (VOID);
INT    natAliasUpdateToDB (VOID);
INT    natIpFragUpdateToDB (VOID);
INT    natLanGetFromDB (__PLAN_INFO   pLanInfo);
INT    natWanGetFromDB (__PWAN_INFO   pWanInfo);
PVOID  natMapGetDBInfo (VOID);
INT    natMapGetDBRow (VOID);
PVOID  natAliasGetDBInfo (VOID);
INT    natAliasGetDBRow (VOID);
INT    natIpFragGetFromDB (VOID);

#endif                                                                  /* __NAT_DB_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
