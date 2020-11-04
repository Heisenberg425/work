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
** ��   ��   ��: devnameDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 9 �� 12 ��
**
** ��        ��: OpenRT  ARP �������ݿ�״̬��¼
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  ARP ����״̬��
*********************************************************************************************************/
#define ASDEFENSE_OPEN           "OPEN"                                 /*  ��                        */
#define ASDEFENSE_CLOSE          "CLOSE"                                /*  �ر�                        */
/*********************************************************************************************************
  �˿�ӳ��������Ϣ
*********************************************************************************************************/
#define AS_DEFENSE_TABLE          "asdefense"                           /*  ARP ���� ����             */
#define AS_DEFENSE_TABLE_HEAD     "(enable      VARCHAR(32))"           /*  ARP ���� ����ʽ           */
/*********************************************************************************************************
** ��������: asDefenseGetFromDB
** ��������: �����ݿ��ȡARP����״̬
** ��  ��  : NONE
** ��  ��  : LW_TRUE           ARP �������ڿ���״̬
**           LW_FALSE          ARP �������ڹر�״̬
** ��  ��  : ERROR_CODE
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
** ��������: __asDefenseInfoInsert
** ��������: �������ݵ����ݿ�
** ��  ��  : pDb           ���ݿ���
**           cpcDevName    �豸��ָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: __asDefenseUpdateItem
** ��������: �������ݿ�
** ��  ��  : pDb         ���ݿ���
**           pcStatus    ״̬
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: asDefenseUpdateToDb
** ��������: �������ݿ�
** ��  ��  : bStatus           ����״̬
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: asDefenseTableCreate
** ��������: ��ʼ�� ARP �������ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
