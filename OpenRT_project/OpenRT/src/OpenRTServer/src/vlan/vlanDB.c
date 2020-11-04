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
** ��   ��   ��: vlanDB.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: vlan ���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "vlan_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  �˿�ӳ��������Ϣ
*********************************************************************************************************/
#define VLAN_TABLE                "vlan"                              /*  vlan����                  */
#define VLAN_TABLE_HEAD           "(id         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "vlanID     INT," \
                                   "iface      VARCHAR(16),"\
                                   "tag        INT," \
                                   "priority   INT)"          /*  vlan����ʽ                */
/*********************************************************************************************************
** ��������: __vlanIsExists
** ��������: �ж� VLAN �Ƿ��Ѵ���
** ��  ��  : pVlanInfo     VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __vlanIsExists( __PVLAN_HANDLE  pVlanInfo)
{
    sqlite3       *pDb;

    CHAR           cSelectSql[200]  = {0};
    CHAR         **cDbResult;
    CHAR          *pcErrMsg         = LW_NULL;
    INT            iRow             = 0;
    INT            iColumn          = 0;
    INT            isExist          = LW_FALSE;
    INT            iRet;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE iface='%s' ;",
             VLAN_TABLE, pVlanInfo->cIface);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        isExist = LW_TRUE;
    }
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);

    return  (isExist);
}
/*********************************************************************************************************
** ��������: vlanGetFromDB
** ��������: �����ݿ��ȡ VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : piRecordNum         VLAN ��Ϣ����
** ��  ��  : �ɹ����� VLAN ��Ϣ��ʧ�ܷ��� NULL
*********************************************************************************************************/
PVOID  vlanGetFromDB (INT  *piRecordNum)
{
    CHAR           cSelectSql[200]  = {0};
    INT            iRet;
    sqlite3        *pDb;
    CHAR           **cDbResult;
    CHAR           *pcErrMsg        = LW_NULL;
    INT            iRow, iColumn;
    INT            i;
    __PVLAN_HANDLE  pVlanHeader     = LW_NULL;
    __PVLAN_HANDLE  pTmpNode        = LW_NULL;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", VLAN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || 0 == iRow) {
        goto Error_Handle;
    }

    pVlanHeader = (__PVLAN_HANDLE)malloc(sizeof(__VLAN_HANDLE) * iRow);
    if (LW_NULL == pVlanHeader){
        goto Error_Handle;
    }

    bzero(pVlanHeader, sizeof(__VLAN_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
        pTmpNode = (__PVLAN_HANDLE)(pVlanHeader + (i - 1));

        pTmpNode->iVlanID   = atoi(cDbResult[i * iColumn + 1]);
        pTmpNode->iTag      = atoi(cDbResult[i * iColumn + 3]);
        pTmpNode->iPriority = atoi(cDbResult[i * iColumn + 4]);

        strncpy(pTmpNode->cIface, cDbResult[i * iColumn + 2], sizeof(pTmpNode->cIface));
    }

    *piRecordNum = iRow;
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pVlanHeader);
Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    *piRecordNum = 0;

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __vlanInsertItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb         ���ݿ���
**           pVlanInfo   VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __vlanInsertItem (sqlite3  *pDb, __PVLAN_HANDLE  pVlanInfo)
{
    CHAR          cTbInsertCmd[200]  = {0};
    sqlite3_stmt *pstmt              = LW_NULL;
    const CHAR   *pcTail             = LW_NULL;
    INT           iRet;

    if (NULL == pDb || NULL == pVlanInfo) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd),
             "INSERT INTO %s (vlanID, iface, tag, priority) VALUES(?, ?, ?, ?);", VLAN_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ����Ҫ��ӵ���Ϣ�������ݿ�
     */
    sqlite3_bind_int(pstmt,  1, pVlanInfo->iVlanID);
    sqlite3_bind_text(pstmt, 2, pVlanInfo->cIface, strlen(pVlanInfo->cIface), LW_NULL);
    sqlite3_bind_int(pstmt,  3, pVlanInfo->iVlanID);
    sqlite3_bind_int(pstmt,  4, 0);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vlanDeleteItem
** ��������: ɾ��һ����Ŀ
** ��  ��  : pDb         ���ݿ���
**           pVlanInfo   VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __vlanDeleteItem (sqlite3 *pDb, __PVLAN_HANDLE  pVlanInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = LW_NULL;

    if (NULL == pDb || NULL == pVlanInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "delete from %s where vlanID = '%d' and iface = '%s';",
                                       VLAN_TABLE,
                                       pVlanInfo->iVlanID,
                                       pVlanInfo->cIface);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (iRet);
}
/*********************************************************************************************************
** ��������: __vlanUpdateItem
** ��������: ���±��е�һ����Ŀ
** ��  ��  : pDb         ���ݿ���
**           pVlanInfo   VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __vlanUpdateItem (sqlite3 *pDb, __PVLAN_HANDLE  pVlanInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = LW_NULL;

    if (NULL == pDb || NULL == pVlanInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "update %s set vlanID = '%d', tag = '%d' where iface = '%s';",
             VLAN_TABLE, pVlanInfo->iVlanID, pVlanInfo->iVlanID, pVlanInfo->cIface);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (iRet);
}
/*********************************************************************************************************
** ��������: vlanUpdateToDb
** ��������: �������ݿ�
** ��  ��  : cpcOperType       ��������
**           pVlanInfo         VLAN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  vlanUpdateToDb (CPCHAR  cpcOperType, __PVLAN_HANDLE  pVlanInfo)
{
    sqlite3      *pDb;
    INT           iRet;

    if (NULL == cpcOperType || NULL == pVlanInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    if (!strcmp(cpcOperType, VLAN_ADD)) {
        iRet = __vlanIsExists(pVlanInfo);
        if (LW_TRUE == iRet) {
            __vlanUpdateItem(pDb, pVlanInfo);
        } else if (LW_FALSE == iRet) {
            __vlanInsertItem(pDb, pVlanInfo);
        } else {
            sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
            return  (PX_ERROR);
        }
    } else if (!strcmp(cpcOperType, VLAN_DELETE)) {
        __vlanDeleteItem(pDb, pVlanInfo);
    } else if (!strcmp(cpcOperType, VLAN_UPDATE)) {
        __vlanUpdateItem(pDb, pVlanInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "unknown operation.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: vlanTableCreate
** ��������: ��ʼ�� VLAN ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT vlanTableCreate (VOID)
{
    sqlite3   *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, VLAN_TABLE, VLAN_TABLE_HEAD);
}

