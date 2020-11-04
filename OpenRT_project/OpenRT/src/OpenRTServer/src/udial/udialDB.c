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
** ��   ��   ��: udailDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 15 ��
**
** ��        ��: �������ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "network/lwip/netif.h"
#include "udialDB.h"
#include "database/database_common.h"
/*********************************************************************************************************
  �������ݿ�������Ϣ
*********************************************************************************************************/
#define UDIAL_TABLE                "udial"                              /*  ���ű���                  */
#define UDIAL_TABLE_HEAD           "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "USERNAME    TEXT,"                               \
                                   "PASSWORD    TEXT,"                               \
                                   "NETIF       TEXT,"                               \
                                   "PPPNAME     TEXT,"                               \
                                   "PID         INT,"                                \
                                   "STATUS      TEXT)"                  /*  ���ű���ʽ                */
/*********************************************************************************************************
** ��������: __udialUpdateItem
** ��������: ���� udial �����е�һ����Ŀ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __udialUpdateItem (__PUDIAL_HANDLE  pUdialInfo)
{
    CHAR          cSqlCmd[300]  = {0};
    INT           iRet;

    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == pUdialInfo)) {
        return  (PX_ERROR);
    }

    bzero(cSqlCmd, sizeof(cSqlCmd));

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "UPDATE %s SET USERNAME = '%s', PASSWORD = '%s', "
             "PPPNAME = '%s', PID = %d, STATUS = %d "
             "WHERE NETIF = '%s';" ,
             UDIAL_TABLE,
             pUdialInfo->cUsrname,
             pUdialInfo->cPassword,
             pUdialInfo->cPppname,
             pUdialInfo->uiPid,
             pUdialInfo->iUdialStatus,
             pUdialInfo->cNetifname);
    iRet = dataBaseCmdExec(pDb, cSqlCmd);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: __udialInsertItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __udialInsertItem ( __PUDIAL_HANDLE  pUdialInfo)
{
    CHAR               cTbInsertCmd[200]  = {0};
    INT                iRet;

    sqlite3            *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == pUdialInfo)) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (USERNAME, PASSWORD, NETIF, PPPNAME, PID, STATUS) "
             "VALUES ('%s', '%s', '%s', '%s', %d, %d);",
             UDIAL_TABLE,
             pUdialInfo->cUsrname,
             pUdialInfo->cPassword,
             pUdialInfo->cNetifname,
             pUdialInfo->cPppname,
             pUdialInfo->uiPid,
             pUdialInfo->iUdialStatus);

    iRet = dataBaseCmdExec(pDb, cTbInsertCmd);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: udialUpdateToDb
** ��������: �����ݿ������һ����Ŀ(����Ѿ����ڣ��������Ŀ����)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  udialUpdateToDb (__PUDIAL_HANDLE  pUdialInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    INT           iRet;

    INT           iFlag = 0;
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pUdialInfo) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where NETIF = '%s';",
             UDIAL_TABLE,
             pUdialInfo->cNetifname);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_prepare_v2 error.\n");

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
        iFlag = 1;
    }
    sqlite3_finalize(pstmt);

    iRet = iFlag ? __udialUpdateItem(pUdialInfo) : __udialInsertItem(pUdialInfo);

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: udialGetSpecInfo
** ��������: ��ȡָ��������Ϣ
** ��  ��  : cpcField          �ֶ�
**           pcArg             �ַ���ֵ
**           piArg             ����ֵ
** ��  ��  : pUdialInfo        ������Ϣ
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT32  udialGetSpecInfo (__PUDIAL_HANDLE  pUdialInfo, CPCHAR  cpcField, PCHAR  pcArg, INT  *piArg)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    INT           iRet;

    sqlite3      *pDb           = LW_NULL;
    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== cpcField || LW_NULL == pUdialInfo || LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    if (LW_NULL== pcArg && LW_NULL == piArg) {
        return  (PX_ERROR);
    }

    if (LW_NULL != pcArg) {
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "select * from %s where %s = '%s';",
                UDIAL_TABLE, cpcField, pcArg);
    } else {
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "select * from %s where %s = %d;",
                UDIAL_TABLE, cpcField, *piArg);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
        strncpy(pUdialInfo->cUsrname,
                (const CHAR *)sqlite3_column_text(pstmt, 1),
                sizeof(pUdialInfo->cUsrname));
        strncpy(pUdialInfo->cPassword,
                (const CHAR *)sqlite3_column_text(pstmt, 2),
                sizeof(pUdialInfo->cPassword));
        strncpy(pUdialInfo->cNetifname,
                (const CHAR *)sqlite3_column_text(pstmt, 3),
                sizeof(pUdialInfo->cNetifname));
        strncpy(pUdialInfo->cPppname,
                (const CHAR *)sqlite3_column_text(pstmt, 4),
                sizeof(pUdialInfo->cPppname));

        pUdialInfo->uiPid        = sqlite3_column_int(pstmt, 5);
        pUdialInfo->iUdialStatus = sqlite3_column_int(pstmt, 6);

        iRet = ERROR_NONE;
    } else {
        iRet = PX_ERROR;
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: udialTableCreate
** ��������: ��ʼ�� udial ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT udialTableCreate (VOID)
{
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, UDIAL_TABLE, UDIAL_TABLE_HEAD);
}

