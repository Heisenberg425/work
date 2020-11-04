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
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: device name ���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "devname_common.h"
#include "common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  �˿�ӳ��������Ϣ
*********************************************************************************************************/
#define DEV_NAME_TABLE            "devname"                             /*  device name ����          */
#define DEV_NAME_TABLE_HEAD       "(devname      VARCHAR(32))"          /*  device name ����ʽ        */
/*********************************************************************************************************
** ��������: devNameGetFromDB
** ��������: �����ݿ��ȡ�豸����Ϣ
** ��  ��  : piNum       �豸����Ϣ����
** ��  ��  : NONE
** ��  ��  : �豸���ṹָ��
*********************************************************************************************************/
PVOID  devNameGetFromDB (INT  *piNum)
{
    CHAR               cSelectSql[200]  = {0};
    INT                iRet;
    sqlite3            *pDb;
    CHAR               **cDbResult;
    CHAR               *pcErrMsg      = NULL;
    INT                iRow, iColumn;
    __PDEVNAME_HANDLE  pDevNameHeader = NULL;

    *piNum = -1;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", DEV_NAME_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        goto Error_Handle;
    }

    if (0 == iRow) {
        *piNum = 0;

        goto Error_Handle;
    } else if (1 == iRow) {
        pDevNameHeader = (__PDEVNAME_HANDLE)malloc(sizeof(__DEVNAME_HANDLE));
        if (pDevNameHeader == NULL) {
            goto Error_Handle;
        }

        *piNum = 1;
        bzero(pDevNameHeader, sizeof(__DEVNAME_HANDLE));

        lib_memcpy(pDevNameHeader->cdevName, cDbResult[1], sizeof(__DEVNAME_HANDLE));

        sqlite3_free(pcErrMsg);
        sqlite3_free_table(cDbResult);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (pDevNameHeader);
    }

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __devNameInfoInsert
** ��������: �������ݵ����ݿ�
** ��  ��  : pDb           ���ݿ���
**           cpcDevName    �豸��ָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __devNameInfoInsert (sqlite3  *pDb, CPCHAR  cpcDevName)
{
    INT    iRet;
    CHAR   cSqlCmd[200] = {0};
    PCHAR  pcErrMsg     = NULL;

    if (NULL == cpcDevName) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "insert into %s (devname) values ('%s');",
             DEV_NAME_TABLE,
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
** ��������: __devNameUpdateItem
** ��������: �������ݿ�
** ��  ��  : pDb         ���ݿ���
**           pDevName    �豸���ṹָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __devNameUpdateItem (sqlite3  *pDb, __PDEVNAME_HANDLE  pDevName)
{
    INT    iRet;
    CHAR   cSqlCmd[200] = {0};
    PCHAR  pcErrMsg     = NULL;

    if (NULL == pDevName) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "update %s set devname = '%s';", DEV_NAME_TABLE, pDevName->cdevName);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: devNameUpdateToDb
** ��������: �������ݿ�
** ��  ��  : pDevName    �豸���ṹָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  devNameUpdateToDb (__PDEVNAME_HANDLE  pDevName)
{
    sqlite3  *pDb;

    if (NULL == pDevName) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    __devNameUpdateItem(pDb, pDevName);

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: devNameTableCreate
** ��������: ��ʼ�� device name ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT devNameTableCreate (VOID)
{
    sqlite3  *pDb    = NULL;
    PVOID     pPoint = NULL;
    INT       iNum;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, DEV_NAME_TABLE, DEV_NAME_TABLE_HEAD));

    pPoint = devNameGetFromDB(&iNum);
    if (NULL == pPoint) {
        if (0 == iNum) {
            __devNameInfoInsert (pDb, DEFAULT_DEV_NAME);
        } else if (-1 == iNum) {
            return  (PX_ERROR);
        }
    } else {
        free(pPoint);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
