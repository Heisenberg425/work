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
** ��   ��   ��: quaggaDB.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 9 �� 4 ��
**
** ��        ��: quagga ���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "quagga_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
** ��������: __quaggaInfoInsertDBItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pQuaggaInfo       quagga ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __quaggaInfoInsertDBItem (sqlite3  *pDb, __PQUAGGA_HANDLE  pQuaggaInfo)
{
    CHAR               cTbInsertCmd[200]  = {0};

    if ((NULL == pDb) || (NULL == pQuaggaInfo)) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));
    snprintf(cTbInsertCmd,
            sizeof(cTbInsertCmd),
            "INSERT INTO %s (NAME, STATUS, PID, CLIPID) "
            "VALUES ('%s', %d, %d, %d);",
            QUAGGA_TABLE,
            pQuaggaInfo->cName,
            pQuaggaInfo->iStatus,
            pQuaggaInfo->pid,
            INVALID_PID);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: quaggaInfoInsertDBItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           cpcName           quagga ·��Э����
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaInitInfoInsertToDB (sqlite3  *pDb, CPCHAR  cpcName)
{
    INT   iFlag;
    CHAR  cSqlCmd[200] = {0};

    if ((NULL == pDb) || (NULL == cpcName)) {
        return  (PX_ERROR);
    }

    bzero(cSqlCmd, sizeof(cSqlCmd));

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "SELECT * FROM %s WHERE NAME='%s';", QUAGGA_TABLE, cpcName);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (ERROR_NONE != iFlag) {
        bzero(cSqlCmd, sizeof(cSqlCmd));

        snprintf(cSqlCmd, sizeof(cSqlCmd),
                "INSERT INTO %s (NAME, STATUS, PID, CLIPID) VALUES ('%s', %d, %d, %d);",
                QUAGGA_TABLE, cpcName, QUAGGA_ENABLE, INVALID_PID, INVALID_PID);

        return  dataBaseCmdExec(pDb, cSqlCmd);
    } else {
        bzero(cSqlCmd, sizeof(cSqlCmd));

        snprintf(cSqlCmd, sizeof(cSqlCmd),
                "UPDATE %s SET PID = %d, CLIPID = %d WHERE NAME = '%s';",
                QUAGGA_TABLE, INVALID_PID, INVALID_PID, cpcName);

        return  dataBaseCmdExec(pDb, cSqlCmd);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: quaggaPidGetFromDB
** ��������: �����ݿ��ȡ��̬·�ɽ��� pid ��Ϣ
** ��  ��  : cpcName   ��̬·��Э����
** ��  ��  : pPid      ��̬·�ɽ��� id ��
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaPidGetFromDB (CPCHAR  cpcName, pid_t  *pPid)
{
    INT               iRet;
    CHAR              cSelectSql[200]  = {0};
    CHAR            **cDbResult;
    CHAR             *pcErrMsg         = NULL;
    INT               iRow;
    INT               iColumn;
    sqlite3          *pDb               = NULL;

    if ((NULL == cpcName) || (NULL == pPid)) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE NAME='%s';", QUAGGA_TABLE, cpcName);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            *pPid = atoi(cDbResult[1 * iColumn + 2]);
        } else {
            *pPid = -1;
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: quaggaStatusGetFromDB
** ��������: �����ݿ��ȡ��̬·�ɽ���״̬
** ��  ��  : cpcName     ��̬·����
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaStatusGetFromDB (CPCHAR  cpcName)
{
    INT                   iRet;
    CHAR                  cSql[200]  = {0};
    CHAR                **cDbResult        = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;
    INT                   iStat        = 0;
    sqlite3              *pDb              = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSql, sizeof(cSql), "SELECT * FROM %s WHERE NAME = '%s';", QUAGGA_TABLE, cpcName);

    iRet = sqlite3_get_table(pDb, cSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            iStat = atoi(cDbResult[iColumn + 1]);
        } else {
            iStat = QUAGGA_DISABLE;
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iStat);
}
/*********************************************************************************************************
** ��������: quaggaDeleteDBItemByName
** ��������: ɾ�����ݿ���ָ����Ŀ
** ��  ��  : cpcName     ��̬·��Э����
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaDeleteDBItemByName (CPCHAR  cpcName)
{
    CHAR               cDelCmd[200] = {0};
    sqlite3           *pDb          = NULL;

    if (NULL == cpcName) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s WHERE NAME='%s';", QUAGGA_TABLE, cpcName);

    return  dataBaseCmdExec(pDb, cDelCmd);
}

/*********************************************************************************************************
** ��������: quaggaAddItemToDb
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pQuaggaInfo     QUAGGA ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaAddItemToDb (__PQUAGGA_HANDLE  pQuaggaInfo)
{
    INT           iRet;
    CHAR          cSqlCmd[200] = {0};
    sqlite3      *pDb          = NULL;
    INT           iFlag = 0;

    OPENRT_DB_HANDLE(pDb);
    if ((NULL == pDb) || (NULL == pQuaggaInfo)) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "SELECT * FROM %s WHERE NAME='%s';", QUAGGA_TABLE, pQuaggaInfo->cName);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (ERROR_NONE == iFlag) {
        quaggaDeleteDBItemByName(pQuaggaInfo->cName);
    }

    iRet = __quaggaInfoInsertDBItem(pDb, pQuaggaInfo);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: quaggaUpdatePidToDb
** ��������: ���� quagga ���� pid ���̺�
** ��  ��  : cpcName      ��̬·��Э����
**           pid          ���� id
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaUpdatePidToDb (CPCHAR  cpcName, pid_t  pid)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = NULL;
    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if ((NULL == pDb) || (NULL == cpcName)) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "UPDATE %s  SET PID = %d WHERE NAME = '%s';" ,
            QUAGGA_TABLE, pid, cpcName);

    iRet = dataBaseCmdExec(pDb, cSqlCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: quaggaTableCreate
** ��������: ��ʼ�� quagga ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaTableCreate (VOID)
{
    sqlite3  *pDb = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, QUAGGA_TABLE, QUAGGA_TABLE_HEAD));

    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "zebra"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "rip"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "ripng"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "ospf"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "isis"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "bgp"));
    INIT_CHECK(quaggaInitInfoInsertToDB(pDb, "pim"));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
