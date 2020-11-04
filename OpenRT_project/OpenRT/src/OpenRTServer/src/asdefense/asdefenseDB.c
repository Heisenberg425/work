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
** 文   件   名: devnameDB.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 9 月 12 日
**
** 描        述: OpenRT  ARP 防御数据库状态记录
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  ARP 防御状态宏
*********************************************************************************************************/
#define ASDEFENSE_OPEN           "OPEN"                                 /*  打开                        */
#define ASDEFENSE_CLOSE          "CLOSE"                                /*  关闭                        */
/*********************************************************************************************************
  端口映射配置信息
*********************************************************************************************************/
#define AS_DEFENSE_TABLE          "asdefense"                           /*  ARP 防御 表单名             */
#define AS_DEFENSE_TABLE_HEAD     "(enable      VARCHAR(32))"           /*  ARP 防御 表单格式           */
/*********************************************************************************************************
** 函数名称: asDefenseGetFromDB
** 功能描述: 从数据库获取ARP防御状态
** 输  入  : NONE
** 输  出  : LW_TRUE           ARP 防御处于开启状态
**           LW_FALSE          ARP 防御处于关闭状态
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT32  asDefenseGetFromDB (BOOL  *pbStatus)
{
    INT                iRet;
    sqlite3           *pDb;
    CHAR             **cDbResult;
    CHAR              *pcErrMsg        = NULL;
    CHAR               cSelectSql[200] = {0};
    INT                iRow;
    INT                iColumn;

    *pbStatus = LW_FALSE;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", AS_DEFENSE_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        iRet  = PX_ERROR;

        goto __back;
    }

    if (1 == iRow) {
        if (!strcmp(ASDEFENSE_OPEN ,cDbResult[1])) {
            *pbStatus = LW_TRUE;
        }
    }

    iRet = ERROR_NONE;

__back:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: __asDefenseInfoInsert
** 功能描述: 插入数据到数据库
** 输  入  : pDb           数据库句柄
**           cpcDevName    设备名指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __asDefenseInfoInsert (sqlite3  *pDb, CPCHAR  cpcDevName)
{
    INT    iRet;
    CHAR   cSqlCmd[200] = {0};
    PCHAR  pcErrMsg     = NULL;

    if (NULL == cpcDevName) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "insert into %s (enable) values ('%s');",
             AS_DEFENSE_TABLE,
             cpcDevName);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __asDefenseUpdateItem
** 功能描述: 更新数据库
** 输  入  : pDb         数据库句柄
**           pcStatus    状态
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __asDefenseUpdateItem (sqlite3  *pDb, PCHAR  pcStatus)
{
    INT    iRet;
    CHAR   cSqlCmd[200] = {0};
    PCHAR  pcErrMsg     = NULL;

    if (NULL == pcStatus) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "update %s set enable = '%s';",
             AS_DEFENSE_TABLE,
             pcStatus);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: asDefenseUpdateToDb
** 功能描述: 更新数据库
** 输  入  : bStatus           防御状态
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  asDefenseUpdateToDb (BOOL  bStatus)
{
    sqlite3  *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    __asDefenseUpdateItem(pDb, bStatus == LW_TRUE ? ASDEFENSE_OPEN : ASDEFENSE_CLOSE);

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: asDefenseTableCreate
** 功能描述: 初始化 ARP 防御数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT asDefenseTableCreate (VOID)
{
    sqlite3  *pDb    = NULL;
    BOOL      bStatus;
    INT       iRet;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, AS_DEFENSE_TABLE, AS_DEFENSE_TABLE_HEAD));
    iRet = asDefenseGetFromDB(&bStatus);
    if (PX_ERROR == iRet) {
        __asDefenseInfoInsert(pDb, ASDEFENSE_CLOSE);

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
