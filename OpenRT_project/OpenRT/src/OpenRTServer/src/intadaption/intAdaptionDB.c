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
** ��   ��   ��: intAdaptionDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��: ����˳��λ���ݿ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include "intAdaptionExec.h"
#include "intada_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  ����Ӧ��λ���ݿ�������Ϣ
*********************************************************************************************************/
#define INTADAPTION_TABLE          "intadaption"                        /*  ���ݿ����                */
#define INTADAPTION_TABLE_HEAD     "(ID       INTEGER PRIMARY KEY AUTOINCREMENT,"\
                                   "WEBINDEX  INT,"   \
                                   "WEBNAME   TEXT,"  \
                                   "NETNAME   TEXT,"  \
                                   "STATUS    TEXT)"                    /*  ���ݿ����ʽ              */
/*********************************************************************************************************
** ��������: intAdaptionDBClear
** ��������: ������ݿ�����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID intAdaptionDBClear (VOID)
{
    CHAR          cCmdSql[200]  = {0};
    sqlite3       *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return;
    }

    dataBaseTbInfoDelete(pDb, INTADAPTION_TABLE);

    snprintf(cCmdSql,
            sizeof(cCmdSql),
            "UPDATE sqlite_sequence SET seq = 0 WHERE name='%s';",
            INTADAPTION_TABLE);
    dataBaseCmdExec(pDb, cCmdSql);
}

/*********************************************************************************************************
** ��������: intAdaptionDBFindFinish
** ��������: ������ݿ��Ƿ���״̬Ϊ finished ����Ŀ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           �д�����
**           ERROR_NONE         �޴�����
*********************************************************************************************************/
INT intAdaptionDBFindFinish (VOID)
{
    CHAR          cSelectSql[200]  = {0};
    INT           iRet;

    sqlite3       *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSelectSql,
            sizeof(cSelectSql),
            "select * from %s where STATUS = '%s';",
            INTADAPTION_TABLE, NET_FINISH);

    iRet = dataBaseItemCheck(pDb, cSelectSql);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: intAdaptionDBGetInfo
** ��������: ��ȡ���ݿ��ж�Ӧ��Ϣ
** �䡡��  : pNetInfo           ��ȡ����Ϣ
**           cFlag              0��ͨ��webindex��ȡ��Ϣ
**                              1��ͨ��netif���ƻ�ȡ��Ϣ
**                              2��ͨ��web netif ���ƻ�ȡ��Ϣ
** �䡡��  : NONE
** ������  : pNetInfo           ��ȡ����Ϣ
*********************************************************************************************************/
INT  intAdaptionDBGetInfo (__PINTADAPTION_HANDLE_INFO   pNetInfo, CHAR  cFlag)
{
    CHAR          cSelectSql[200]  = {0};
    INT           iRet             = PX_ERROR;
    sqlite3      *pDb              = LW_NULL;
    sqlite3_stmt *pstmt            = LW_NULL;
    const CHAR   *pcTail           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pNetInfo || LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    if (0 == cFlag) {
        snprintf(cSelectSql,
                 sizeof(cSelectSql),
                 "select * from %s where WEBINDEX = '%d';",
                 INTADAPTION_TABLE,
                 pNetInfo->iWebIndex);
    } else if (1 == cFlag) {
        snprintf(cSelectSql,
                 sizeof(cSelectSql),
                 "select * from %s where NETNAME = '%s';",
                 INTADAPTION_TABLE,
                 pNetInfo->cNetIfName);
    } else if (2 == cFlag) {
        snprintf(cSelectSql,
                 sizeof(cSelectSql),
                 "select * from %s where WEBNAME = '%s';",
                 INTADAPTION_TABLE,
                 pNetInfo->cWebName);
    } else {
        return  (PX_ERROR);
    }

    iRet = sqlite3_prepare(pDb, cSelectSql, strlen(cSelectSql), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ֻҪ����һ����˵�����      */
        pNetInfo->iWebIndex = sqlite3_column_int(pstmt, 1);

        strncpy(pNetInfo->cWebName,  (CPCHAR)sqlite3_column_text(pstmt, 2), sizeof(pNetInfo->cWebName));
        strncpy(pNetInfo->cNetIfName,(CPCHAR)sqlite3_column_text(pstmt, 3), sizeof(pNetInfo->cNetIfName));
        strncpy(pNetInfo->cStatus,   (CPCHAR)sqlite3_column_text(pstmt, 4), sizeof(pNetInfo->cStatus));

        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    }

    /*
     *  ֻҪ��һ�������ھ�˵��û���
     */
    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: intadaStatusGetFromDbByName
** ��������: ������������ȡ����״̬��Ϣ
** �䡡��  : cpcNetIfName    ������
** �䡡��  : piStatus        ����״̬
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  intadaStatusGetFromDbByName (CPCHAR  cpcNetIfName, INT  *piStatus)
{
    CHAR          cSelectSql[200]  = {0};
    INT           iRet             = PX_ERROR;
    sqlite3      *pDb              = LW_NULL;
    sqlite3_stmt *pstmt            = LW_NULL;
    const CHAR   *pcTail           = LW_NULL;
    PCHAR         pcStatus         = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb          ||
        LW_NULL == cpcNetIfName ||
        LW_NULL == piStatus) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    *piStatus = 0;

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql),
             "select * from %s where NETNAME = '%s';",
             INTADAPTION_TABLE,
             cpcNetIfName);

    iRet = sqlite3_prepare(pDb, cSelectSql, strlen(cSelectSql), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ֻҪ����һ����˵�����      */
        pcStatus = (PCHAR)sqlite3_column_text(pstmt, 4);
        if (0 == strcmp(pcStatus, NET_UNUSED) || '\0' == pcStatus[0]) {
            *piStatus = 0;
        } else {
            *piStatus = 1;
        }

        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    } else {                                                            /*  ֻҪ��һ�������ھ�˵��û���*/
        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: intAdaptionDBSetFinish
** ��������: ����������Ϊ���óɹ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT intAdaptionDBSetFinish (VOID)
{
    CHAR          cCmdSql[200]  = {0};
    INT           iRet;

    sqlite3       *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cCmdSql, sizeof(cCmdSql), "update %s set status = '%s';", INTADAPTION_TABLE, NET_FINISH);
    iRet = dataBaseCmdExec(pDb, cCmdSql);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: intAdaptionDBUpdate
** ��������: ���� Web ������Ϣ�����ݿ�
** �䡡��  : pIntAdaptionInfo   web ������Ϣ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT intAdaptionDBUpdate (__PINTADAPTION_HANDLE_INFO  pIntAdaptionInfo)
{
    CHAR          cCmdSql[200]  = {0};
    INT           iRet;
    sqlite3       *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    if (LW_NULL == pIntAdaptionInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "argv null\n");

        return  (PX_ERROR);
    }

    /*
     *  ����� id ���õ� sq ����������
     */
    snprintf(cCmdSql,
            sizeof(cCmdSql),
            "insert into %s values (null, %d, '%s', '%s', '%s' );",
            INTADAPTION_TABLE,
            pIntAdaptionInfo->iWebIndex,
            pIntAdaptionInfo->cWebName,
            pIntAdaptionInfo->cNetIfName,
            pIntAdaptionInfo->cStatus);

    iRet = dataBaseCmdExec(pDb, cCmdSql);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: intAdaptionTableCreate
** ��������: ��ʼ�� intadaption ���ݿ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT intAdaptionTableCreate (VOID)
{
    sqlite3  *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, INTADAPTION_TABLE, INTADAPTION_TABLE_HEAD);
}
