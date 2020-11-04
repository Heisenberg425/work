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
** 文   件   名: dataBaseHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述:  数据库通用操作
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "database_common.h"
/*********************************************************************************************************
** 函数名称: dataBaseGetTbRowCol
** 功能描述: 从数据库表格中获取信息行数、列数
** 输  入  : pDb               数据库链接句柄
**           cpcTbName         表格名称
** 输  出  : piRow             行数(包含字段)
**           piColumn          列数
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dataBaseGetTbRowCol (sqlite3  *pDb, CPCHAR  cpcTbName, INT  *piRow, INT  *piColumn)
{
    CHAR            cSelectSql[200]  = {0};
    INT             iRet;
    CHAR          **cDbResult;
    CHAR           *pcErrMsg = LW_NULL;

    if (LW_NULL == pDb || LW_NULL == cpcTbName || LW_NULL == piRow || LW_NULL == piColumn) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", cpcTbName);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, piRow, piColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base  failed.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}


/*********************************************************************************************************
** 函数名称: dataBaseCmdExec
** 功能描述: 执行数据库语句
** 输  入  : pDb               数据库链接句柄
**           cpcSqCmd          sqlite 语句
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dataBaseCmdExec (sqlite3  *pDb, CPCHAR  cpcSqCmd)
{
    INT           iRet;
    PCHAR         pcErrMsg    = LW_NULL;

    if (LW_NULL == pDb || LW_NULL == cpcSqCmd) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    /*
     * 执行 Sql 语句
     */
    iRet = sqlite3_exec(pDb, cpcSqCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed. error: %s\n", pcErrMsg);

        if (NULL != pcErrMsg) {
            free(pcErrMsg);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dataBaseTbInfoDelete
** 功能描述: 删除表中所有信息
** 输  入  : pDb               数据库链接句柄
**           cpcTbName         表名
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dataBaseTbInfoDelete (sqlite3  *pDb, CPCHAR  cpcTbName)
{
    CHAR          cDeleteSql[200]  = {0};

    if (LW_NULL == pDb || LW_NULL == cpcTbName) {
        return  (PX_ERROR);
    }

    snprintf(cDeleteSql, sizeof(cDeleteSql), "DELETE FROM %s;", cpcTbName);

    return  dataBaseCmdExec(pDb, cDeleteSql);
}

/*********************************************************************************************************
** 函数名称: dataBaseTbCreate
** 功能描述: 创建数据库表格
** 输　入  : pDb                数据库链接句柄
**           cpcTableName       数据库表格名称
**           cpcTbFormatDef     数据库表格格式
** 输　出  : NONE
** 返　回  : 数据库执行结果
*********************************************************************************************************/
INT  dataBaseTbCreate (sqlite3  *pDb, CPCHAR  cpcTableName, CPCHAR  cpcTbFormatDef)
{
    CHAR      cTbCreateCmd[1024] = {0};

    if (LW_NULL == pDb || LW_NULL == cpcTbFormatDef || LW_NULL == cpcTableName) {
        return  (PX_ERROR);
    }

    snprintf(cTbCreateCmd,
            sizeof(cTbCreateCmd),
            "CREATE TABLE IF NOT EXISTS %s %s;",
            cpcTableName,
            cpcTbFormatDef);

    return  dataBaseCmdExec(pDb, cTbCreateCmd);
}

/*********************************************************************************************************
** 函数名称: dataBaseItemCheck
** 功能描述: 确认数据库中是否存在一行对应的条目
** 输  入  : pDb               数据库链接句柄
**           cpcSqCmd        sqlite 语句
** 输  出  : NONE
** 返  回  : PX_ERROR        没找到或者操作失败
**           ERROR_NONE      存在对应条目
*********************************************************************************************************/
INT  dataBaseItemCheck (sqlite3  *pDb, CPCHAR  cpcSqCmd)
{
    INT             iRet;
    sqlite3_stmt   *pstmt       = LW_NULL;
    const CHAR     *pcTail      = LW_NULL;

    if (LW_NULL == pDb || LW_NULL == cpcSqCmd) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "argv error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = sqlite3_prepare_v2(pDb, cpcSqCmd, strlen(cpcSqCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  只要存在一个就说明找到了    */
        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    }

    /*
     *  只要有一个不存在就说明没有对应内容
     */
    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (PX_ERROR);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
