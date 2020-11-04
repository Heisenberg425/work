/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: operLog.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 26 ��
**
** ��        ��: ������־ʵ��
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
  ��־���ݿ�������Ϣ
*********************************************************************************************************/
#define OPER_LOG_TABLE_HEAD       "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"    \
                                   "TIMES        TEXT,"                               \
                                   "TIMEI        BIGINT,"                             \
                                   "MODULE_NAME  VARCHAR(32),"                        \
                                   "CLIENT_IP    VARCHAR(64),"                        \
                                   "OPERATOR     VARCHAR(32),"                        \
                                   "RESULT       INT,"                                \
                                   "LOG          TEXT)"                 /*  ������־ ��ʽ               */

/*********************************************************************************************************
** ��������: operLog
** ��������: ��¼������־�����浽���ݿ��У��ṩ����Ա������־����
** �䡡��  : pOperLog     ������־��Ϣ
** �䡡��  : NONE
** ������  : INT          �������
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
** ��������: operLogGetFromDB
** ��������: ��ȡ������־,�ڴ��ɵ������ͷ�
** ��  ��  : NONE
** ��  ��  : piRecordNum        �û�������
** ��  ��  : pLoginHead         �û�����ͷ
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
** ��������: operLogDel
** ��������: ���������־
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: operLogTableCreate
** ��������: ��ʼ�� ������־���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CDE
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
