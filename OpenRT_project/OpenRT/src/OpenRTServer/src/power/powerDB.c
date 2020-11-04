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
** ��   ��   ��: powerDB.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: power event ���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "power_common.h"
#include "common.h"
#include "database/database_common.h"
/*********************************************************************************************************
** ��������: powerEventGetFromDB
** ��������: �����ݿ��ȡ��Դ�¼�
** ��  ��  : NONE
** ��  ��  : piRecordNum     ���ݼ�¼��Ŀ
** ��  ��  : ��Դ�¼���Ϣ����ָ��
*********************************************************************************************************/
PVOID  powerEventGetFromDB (INT  *piRecordNum)
{
    INT              iRet;
    sqlite3         *pDb             = NULL;
    CHAR             cSelectSql[200] = {0};
    CHAR           **cDbResult       = NULL;
    CHAR            *pcErrMsg        = NULL;
    INT              iRow            = 0;
    INT              iColumn         = 0;
    __PPOWER_HANDLE  pPowerHeader    = NULL;
    __PPOWER_HANDLE  pTmpNode        = NULL;
    INT              i;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", POWER_EVENT_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || iRow == 0) {
        goto Error_Handle;
    }

    pPowerHeader = (__PPOWER_HANDLE)malloc(sizeof(__POWER_HANDLE) * iRow);
    if (NULL == pPowerHeader){
        goto Error_Handle;
    }

    bzero(pPowerHeader, sizeof(__POWER_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {                                       /*  ���л�ȡ(���˵�һ��)        */
        pTmpNode = (__PPOWER_HANDLE)(pPowerHeader + (i - 1));

        pTmpNode->uiEventId = atoi(cDbResult[i * iColumn]);
        pTmpNode->iStatus   = atoi(cDbResult[i * iColumn + 4]);

        strncpy(pTmpNode->cEvent, cDbResult[i * iColumn + 1], sizeof(pTmpNode->cEvent));
        strncpy(pTmpNode->cDate,  cDbResult[i * iColumn + 2], sizeof(pTmpNode->cDate));
        strncpy(pTmpNode->cTime,  cDbResult[i * iColumn + 3], sizeof(pTmpNode->cTime));
        strncpy(pTmpNode->cDesc,  cDbResult[i * iColumn + 5], sizeof(pTmpNode->cDesc));
    }

    *piRecordNum = iRow;
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pPowerHeader);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
    *piRecordNum = 0;

    return  (NULL);
}
/*********************************************************************************************************
** ��������: powerEventInsertIntoDB
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iNatIpFragInfo    ��Ƭ����Ϣ
*********************************************************************************************************/
INT  powerEventInsertIntoDB (__PPOWER_HANDLE  pPowerInfo)
{
    sqlite3      *pDb;
    CHAR          cSqlCmd[300]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pPowerInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "INSERT INTO %s (EVENT_ID ,EVENT, DATE, TIME, STATUS, DESC) " \
            "VALUES(%d, '%s', '%s', '%s', %d, '%s');",
            POWER_EVENT_TABLE,
            pPowerInfo->uiEventId,
            pPowerInfo->cEvent,
            pPowerInfo->cDate,
            pPowerInfo->cTime,
            pPowerInfo->iStatus,
            pPowerInfo->cDesc);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerEventDeleteFromDB
** ��������: ɾ��һ����Դ�¼�
** ��  ��  : pPowerInfo    ��Դ�¼�ָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventDeleteFromDB (__PPOWER_HANDLE  pPowerInfo)
{
    sqlite3      *pDb;
    CHAR          cSqlCmd[200]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pPowerInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "delete from %s where EVENT_ID = %d;", POWER_EVENT_TABLE, pPowerInfo->uiEventId);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerEventStatSetDB
** ��������: ���õ�Դ�¼���״̬
** ��  ��  : pPowerInfo    ��Դ�¼�ָ��
**           iStatus       ��Դ�¼�״̬
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventStatSetDB (__PPOWER_HANDLE  pPowerInfo, INT  iStatus)
{
    sqlite3      *pDb;
    CHAR          cSqlCmd[200]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pPowerInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "update %s set STATUS = %d where EVENT_ID = %d;",
            POWER_EVENT_TABLE, iStatus, pPowerInfo->uiEventId);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerEventUpdateToDB
** ��������: ���µ�Դ�¼����е�һ����Ŀ
** ��  ��  : pPowerInfo    ��Դ�¼�ָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventUpdateToDB (__PPOWER_HANDLE  pPowerInfo)
{
    sqlite3      *pDb;
    CHAR          cSqlCmd[300]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pPowerInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "UPDATE %s SET EVENT = '%s', DATE = '%s', TIME = '%s', "
            "STATUS = %d, DESC = '%s' where EVENT_ID = %d;",
            POWER_EVENT_TABLE,
            pPowerInfo->cEvent,
            pPowerInfo->cDate,
            pPowerInfo->cTime,
            pPowerInfo->iStatus,
            pPowerInfo->cDesc,
            pPowerInfo->uiEventId);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerEventTableCreate
** ��������: ��ʼ�� power event ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventTableCreate (VOID)
{
    sqlite3   *pDb = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, POWER_EVENT_TABLE, POWER_EVENT_TABLE_HEAD);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
