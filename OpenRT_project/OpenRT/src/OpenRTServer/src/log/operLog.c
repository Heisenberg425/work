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
** 文   件   名: operLog.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 26 日
**
** 描        述: 操作日志实现
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "log_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  日志数据库配置信息
*********************************************************************************************************/
#define OPER_LOG_TABLE_HEAD       "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"    \
                                   "TIMES        TEXT,"                               \
                                   "TIMEI        BIGINT,"                             \
                                   "MODULE_NAME  VARCHAR(32),"                        \
                                   "CLIENT_IP    VARCHAR(64),"                        \
                                   "OPERATOR     VARCHAR(32),"                        \
                                   "RESULT       INT,"                                \
                                   "LOG          TEXT)"                 /*  操作日志 格式               */

/*********************************************************************************************************
** 函数名称: operLog
** 功能描述: 记录操作日志，保存到数据库中，提供管理员进行日志管理
** 输　入  : pOperLog     操作日志信息
** 输　出  : NONE
** 返　回  : INT          操作结果
*********************************************************************************************************/
INT  operLog (__POPER_LOG  pOperLog)
{
    CHAR               cTbInsertCmd[512]  = {0};
    time_t             iTime;
    CHAR               cTImeString[30]    = {0};

    sqlite3            *pDb               = LW_NULL;

    if (LW_NULL == pOperLog) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb)  {
        return  (PX_ERROR);
    }

    iTime = logTimeGen(cTImeString);
    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (TIMES, TIMEI, MODULE_NAME, CLIENT_IP, OPERATOR, RESULT, LOG) "
             "VALUES ('%s', %lld, '%s', '%s', '%s' , %d, '%s');",
             OPER_LOG_TABLE,
             cTImeString,
             iTime,
             pOperLog->cModuleName,
             pOperLog->cClientIP,
             pOperLog->cOperator,
             pOperLog->iResult,
             pOperLog->cLog);

    dataBaseCmdExec(pDb, cTbInsertCmd);

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "DELETE FROM %s WHERE %s NOT IN (SELECT %s FROM %s ORDER BY %s DESC LIMIT %d);",
             OPER_LOG_TABLE, "ID", "ID", OPER_LOG_TABLE, "ID", MAX_RECORD_NUM);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}

/*********************************************************************************************************
** 函数名称: operLogGetFromDB
** 功能描述: 获取操作日志,内存由调用者释放
** 输  入  : NONE
** 输  出  : piRecordNum        用户总条数
** 返  回  : pLoginHead         用户链表头
*********************************************************************************************************/
PVOID  operLogGetFromDB (__POPER_LOG  pOperLog, INT*  piRecordNum)
{
#define CMD_BUFF_LEN  256
    __POPER_LOG     pOperLogHead             = LW_NULL;
    __POPER_LOG     pTmpNode                 = LW_NULL;
    CHAR            cSelectSql[CMD_BUFF_LEN] = {0};
    sqlite3        *pDb;
    CHAR          **cDbResult;
    CHAR           *pcErrMsg                 = LW_NULL;
    INT             iRet;
    INT             iRow                     = 0;
    INT             iColumn                  = 0;
    INT             i;
    INT             k                        = 0;
    INT             iRestLength              = 0;
    INT             iRecordCount             = 0;
    INT             iStrlen;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb)  {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iStrlen = snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE 1=1 ", OPER_LOG_TABLE);

    if ((pOperLog->logStartTm > 0) &&
        (pOperLog->logEndTm   > 0) &&
        (pOperLog->logEndTm   >= pOperLog->logStartTm)) {
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen += snprintf(cSelectSql + iStrlen, iRestLength,  "and  TIMEI >= %lld AND TIMEI <= %lld ",
                            pOperLog->logStartTm, pOperLog->logEndTm);
    }

    if (strlen(pOperLog->cModuleName)>0){
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen += snprintf(cSelectSql + iStrlen, iRestLength,  "and  MODULE_NAME like '%%%s%%' ",
                            pOperLog->cModuleName);
    }

    if (strlen(pOperLog->cLog)>0){
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen += snprintf(cSelectSql + iStrlen, iRestLength,  "and  LOG like '%%%s%%' ",
                            pOperLog->cLog);
    }

    iRestLength = CMD_BUFF_LEN - iStrlen;
    iStrlen += snprintf(cSelectSql + iStrlen, iRestLength, " order by ID desc;");

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || iRow == 0) {
        goto  Error_Handle;
    }

    for (i = 1; i <= iRow; i++) {
        if (i < pOperLog->llStartIndex) {
            continue;
        }

        if (i > pOperLog->llEndIndex) {
            break;
        }

        iRecordCount++;
    }

    pOperLogHead = (__POPER_LOG)malloc(sizeof(__OPER_LOG) * iRecordCount);
    if (LW_NULL == pOperLogHead) {
        goto  Error_Handle;
    }

    bzero(pOperLogHead, sizeof(__OPER_LOG) * iRecordCount);

    for (i = 1; (i <= iRow) && (k < iRecordCount); i++) {
        if (i < pOperLog->llStartIndex) {
            continue;
        }

        if (i > pOperLog->llEndIndex) {
            break;
        }

        pTmpNode = (__POPER_LOG)(pOperLogHead + k);
        pTmpNode->iTotal = iRow;
        k = k + 1;

        pTmpNode->llTime  = atoi(cDbResult[i * iColumn + 2]);
        pTmpNode->iResult = atoi(cDbResult[i * iColumn + 6]);

        strncpy(pTmpNode->cModuleName, cDbResult[i * iColumn + 3], sizeof(pTmpNode->cModuleName));
        strncpy(pTmpNode->cClientIP,   cDbResult[i * iColumn + 4], sizeof(pTmpNode->cClientIP));
        strncpy(pTmpNode->cOperator,   cDbResult[i * iColumn + 5], sizeof(pTmpNode->cOperator));
        strncpy(pTmpNode->cLog,        cDbResult[i * iColumn + 7], sizeof(pTmpNode->cLog));
    }

    *piRecordNum = iRecordCount;
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pOperLogHead);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
    *piRecordNum = 0;

    return  (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: operLogDel
** 功能描述: 清除操作日志
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  operLogDel (VOID)
{
    CHAR               cTbCmd[64] = {0};
    sqlite3            *pDb       = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb)  {
        return  (PX_ERROR);
    }

    snprintf(cTbCmd, sizeof(cTbCmd), "DELETE FROM %s;", OPER_LOG_TABLE);

    return  dataBaseCmdExec(pDb, cTbCmd);
}

/*********************************************************************************************************
** 函数名称: operLogTableCreate
** 功能描述: 初始化 操作日志数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CDE
*********************************************************************************************************/
INT  operLogTableCreate(VOID)
{
    sqlite3      *pDb         = LW_NULL;;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, OPER_LOG_TABLE, OPER_LOG_TABLE_HEAD);
}
