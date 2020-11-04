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
** 文   件   名: database_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: 数据库操作共用头文件
*********************************************************************************************************/
#ifndef __DATABASE_COMMON_H
#define __DATABASE_COMMON_H

/*********************************************************************************************************
  数据库函数声明
*********************************************************************************************************/
INT  dataBaseCmdExec (sqlite3  *pDb, CPCHAR  cpcSqCmd);
INT  dataBaseItemCheck (sqlite3  *pDb, CPCHAR  cpcSqCmd);
INT  dataBaseTbInfoDelete (sqlite3  *pDb, CPCHAR  cpcTbName);
INT  dataBaseTbCreate (sqlite3  *pDb, CPCHAR  cpcTableName, CPCHAR  cpcTbFormatDef);
INT  dataBaseGetTbRowCol (sqlite3  *pDb, CPCHAR  cpcTbName, INT  *piRow, INT  *piColumn);

#endif                                                                  /*  __DATABASE_COMMON_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
