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
** ��   ��   ��: dataBaseHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��:  ���ݿ�ͨ�ò���
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "database_common.h"
/*********************************************************************************************************
** ��������: dataBaseGetTbRowCol
** ��������: �����ݿ����л�ȡ��Ϣ����������
** ��  ��  : pDb               ���ݿ����Ӿ��
**           cpcTbName         �������
** ��  ��  : piRow             ����(�����ֶ�)
**           piColumn          ����
** ��  ��  : ERROR_CODE
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
** ��������: dataBaseCmdExec
** ��������: ִ�����ݿ����
** ��  ��  : pDb               ���ݿ����Ӿ��
**           cpcSqCmd          sqlite ���
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
     * ִ�� Sql ���
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
** ��������: dataBaseTbInfoDelete
** ��������: ɾ������������Ϣ
** ��  ��  : pDb               ���ݿ����Ӿ��
**           cpcTbName         ����
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: dataBaseTbCreate
** ��������: �������ݿ���
** �䡡��  : pDb                ���ݿ����Ӿ��
**           cpcTableName       ���ݿ�������
**           cpcTbFormatDef     ���ݿ����ʽ
** �䡡��  : NONE
** ������  : ���ݿ�ִ�н��
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
** ��������: dataBaseItemCheck
** ��������: ȷ�����ݿ����Ƿ����һ�ж�Ӧ����Ŀ
** ��  ��  : pDb               ���ݿ����Ӿ��
**           cpcSqCmd        sqlite ���
** ��  ��  : NONE
** ��  ��  : PX_ERROR        û�ҵ����߲���ʧ��
**           ERROR_NONE      ���ڶ�Ӧ��Ŀ
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ֻҪ����һ����˵���ҵ���    */
        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    }

    /*
     *  ֻҪ��һ�������ھ�˵��û�ж�Ӧ����
     */
    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (PX_ERROR);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
