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
** ��   ��   ��: loginDB.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��: login���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "login_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  ���ݿ�������Ϣ
*********************************************************************************************************/
#define LOGIN_TABLE                "login"                              /*  ��¼����                  */
#define LOGIN_TABLE_HEAD           "(id         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "username    VARCHAR(64),"                       \
                                   "passwd      VARCHAR(64),"                       \
                                   "role        INT,"                               \
                                   "status      INT)"                   /*  ��¼����ʽ                */
/*********************************************************************************************************
  ��¼web�û��ǳ���������¼�û���������ʱ�䡢clientIP�������������Ϣ
*********************************************************************************************************/
#define LOGIN_RECORD_TABLE         "login_record"                       /*  ��¼��¼����              */
#define LOGIN_RECORD_TABLE_HEAD    "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "USERNAME    VARCHAR(64),"                        \
                                   "CLIENT_IP   VARCHAR(64),"                        \
                                   "TIMES       VARCHAR(64),"                        \
                                   "TIMEI       BIGINT,"                             \
                                   "RESULT      INT,"                                \
                                   "TYPE        INT)"                   /*  type: login/logout          */
/*********************************************************************************************************
  ��֤��Ϣ
*********************************************************************************************************/
#define OPERA_AUTH_TABLE           "operaauth"                          /*  �����߱���                */
#define VIEW_AUTH_TABLE            "viewauth"                           /*  �۲��߱���                */
#define AUTH_TABLE_HEAD            "(MONITOR          INT,"   \
                                   "UDIAL             INT,"   \
                                   "INTERFACE         INT,"   \
                                   "DHCPSERVER        INT,"   \
                                   "DNS               INT,"   \
                                   "ROUTE             INT,"   \
                                   "VLAN              INT,"   \
                                   "VPN               INT,"   \
                                   "VND               INT,"   \
                                   "NAT               INT,"   \
                                   "QoS               INT,"   \
                                   "DEVNAME           INT,"   \
                                   "TIMESET           INT,"   \
                                   "ACCOUNT           INT,"   \
                                   "AUTH              INT,"   \
                                   "UPDATEFIRMWARE    INT,"   \
                                   "HWINFO            INT,"   \
                                   "RESET             INT,"   \
                                   "INTADA            INT,"   \
                                   "FLOWCTL           INT,"   \
                                   "BEHAVIOUR         INT,"   \
                                   "ARP               INT,"   \
                                   "ARP_DEFENSE       INT,"   \
                                   "NPF               INT,"   \
                                   "PING_TEST         INT,"   \
                                   "SPEEDTEST         INT,"   \
                                   "UDIAL_LOG         INT,"   \
                                   "OPERA_LOG         INT,"   \
                                   "VPN_LOG           INT,"   \
                                   "ASDEFENSE_LOG     INT,"   \
                                   "DHCPSERVER_LOG    INT,"   \
                                   "QUAGGA            INT)"
extern time_t  logTimeGen (PCHAR  pcTimeBuf);
/*********************************************************************************************************
** ��������: loginRecordGetFromDB
** ��������: �����ݿ��ȡ�û���Ϣ,�ڴ��ɵ������ͷ�
** ��  ��  : NONE
** ��  ��  : piRecordNum        �û�������
** ��  ��  : pLoginHead         �û�����ͷ
*********************************************************************************************************/
PVOID  loginRecordGetFromDB (__PLOGIN_RECORD  pLoginRecord, INT  *piRecordNum)
{
#define SQL_BUFF_LEN  256
    __PLOGIN_RECORD pLoginRecordHead = LW_NULL;
    __PLOGIN_RECORD pTmpNode         = LW_NULL;
    CHAR            cSelectSql[SQL_BUFF_LEN]  = {0};
    sqlite3        *pDb;
    CHAR          **cDbResult;
    CHAR           *pcErrMsg        = LW_NULL;
    INT             iRet;
    INT             iRow            = 0;
    INT             iColumn         = 0;
    INT             i;
    INT             k;
    INT             iStrlen;
    INT             iRestLength;
    INT             iRecordCount    = 0;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iStrlen = snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE 1=1 ", LOGIN_RECORD_TABLE);
    if ((pLoginRecord->logStartTm > 0 ) && (pLoginRecord->logEndTm >0 )) {
        iRestLength = SQL_BUFF_LEN - iStrlen;
        iStrlen += snprintf(cSelectSql + iStrlen, iRestLength,  "and  TIMEI >= %lld AND TIMEI <= %lld ",
                            pLoginRecord->logStartTm, pLoginRecord->logEndTm);
    }

    if (strlen(pLoginRecord->cClientIp) > 0 ) {
        iRestLength = SQL_BUFF_LEN - iStrlen;
        iStrlen += snprintf(cSelectSql + iStrlen, iRestLength,  "and  CLIENT_IP like '%%%s%%' ",
                            pLoginRecord->cClientIp);
    }

    iRestLength = SQL_BUFF_LEN - iStrlen;
    iStrlen += snprintf(cSelectSql + iStrlen, iRestLength, " order by ID desc;");

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || iRow == 0) {
        goto Error_Handle;
    }

    for (i = 1; i <= iRow; i++) {
        if (i < pLoginRecord->llStartIndex) {
            continue;
        }

        if (i > pLoginRecord->llEndIndex) {
            break;
        }

        iRecordCount++;
    }

    pLoginRecordHead = (__PLOGIN_RECORD)malloc(sizeof(__LOGIN_RECORD) * iRecordCount);
    if (pLoginRecordHead == LW_NULL) {
        goto Error_Handle;
    }

    bzero(pLoginRecordHead, sizeof(__LOGIN_RECORD) * iRecordCount);

    for (i = 1, k = 0; (i <= iRow)&&(k < iRecordCount); i++) {
        if (i < pLoginRecord->llStartIndex) {
            continue;
        }

        if (i > pLoginRecord->llEndIndex) {
            break;
        }

        pTmpNode = (__PLOGIN_RECORD)(pLoginRecordHead + k);
        pTmpNode->iTotal = iRow;
        k = k + 1;

        strncpy(pTmpNode->cUsername, cDbResult[i * iColumn + 1], sizeof(pTmpNode->cUsername));
        strncpy(pTmpNode->cClientIp, cDbResult[i * iColumn + 2], sizeof(pTmpNode->cClientIp));
        strncpy(pTmpNode->cTime,     cDbResult[i * iColumn + 3], sizeof(pTmpNode->cTime));
        pTmpNode->iResult = atoi(cDbResult[i * iColumn + 5]);
        pTmpNode->iType   = atoi(cDbResult[i * iColumn + 6]);
    }

    *piRecordNum = iRecordCount;
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pLoginRecordHead);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
    *piRecordNum = 0;

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: loginRecordInsert
** ��������: ����һ����¼��Ϊ��¼
** �䡡��  : pLoginRecord ��¼��Ϊ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  loginRecordInsert (__PLOGIN_RECORD  pLoginRecord)
{

    CHAR     cTbInsertCmd[256]  = {0};
    time_t   iTime;
    CHAR     cTImeString[30]    = {0};                        /*  ����Ҫ���� 26 ���ֽ�        */
    sqlite3  *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iTime = logTimeGen(cTImeString);
    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (USERNAME, CLIENT_IP, TIMES, TIMEI, RESULT, TYPE) "
             "VALUES ('%s', '%s', '%s', %lld, %d, %d);",
             LOGIN_RECORD_TABLE,
             pLoginRecord->cUsername,
             pLoginRecord->cClientIp,
             cTImeString,
             iTime,
             pLoginRecord->iResult,
             pLoginRecord->iType);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: loginInsert
** ��������: ����һ���û�����
** �䡡��  : pLogin �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  loginInsert (__PLOGIN_HANDLE  pLogin)
{

    CHAR     cTbInsertCmd[256]  = {0};
    sqlite3  *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (username, passwd, status, role) "
             "VALUES ('%s', '%s', %d, %d);",
             LOGIN_TABLE,
             pLogin->cUsername,
             pLogin->cPasswd,
             pLogin->iStatus,
			 pLogin->iRole);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: loginDelete
** ��������: ɾ��һ���û�����
** �䡡��  : pLogin �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  loginDelete (__PLOGIN_HANDLE  pLogin)
{
    CHAR     cTbDeleteCmd[200]  = {0};
    sqlite3  *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cTbDeleteCmd,
             sizeof(cTbDeleteCmd),
             "DELETE FROM %s  WHERE username = '%s' ",
             LOGIN_TABLE, pLogin->cUsername);

    return  dataBaseCmdExec(pDb, cTbDeleteCmd);
}
/*********************************************************************************************************
** ��������: disableUserLogin
** ��������: ��ֹ�û���¼
** �䡡��  : pLogin �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  disableUserLogin (__PLOGIN_HANDLE  pLogin)
{
    CHAR      cTbDisableCmd[256] = {0};
    sqlite3  *pDb                = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cTbDisableCmd,
             sizeof(cTbDisableCmd),
             "UPDATE %s set status='%d' "
             "where username ='%s';",
             LOGIN_TABLE,
             pLogin->iStatus,
             pLogin->cUsername);

   return  dataBaseCmdExec(pDb, cTbDisableCmd);
}
/*********************************************************************************************************
** ��������: changePasswd
** ��������: ����һ���û�����
** �䡡��  : pLogin       �û���Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  changePasswd (__PLOGIN_HANDLE  pLogin)
{
    CHAR     cTbInsertCmd[500]  = {0};
    sqlite3  *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "UPDATE %s set passwd= '%s', status='%d' "
             "where username ='%s' and passwd = '%s';",
             LOGIN_TABLE,
             pLogin->cNewPasswd,
             pLogin->iStatus,
             pLogin->cUsername,
             pLogin->cPasswd);

   return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: loginGetFromDB
** ��������: �����ݿ��ȡ�û���Ϣ,�ڴ��ɵ������ͷ�
** ��  ��  : NONE
** ��  ��  : piRecordNum        �û�������
** ��  ��  : pLoginHead         �û�����ͷ
*********************************************************************************************************/
PVOID  loginGetFromDB (INT  *piRecordNum)
{
    __PLOGIN_HANDLE pLoginHead      = LW_NULL;
    __PLOGIN_HANDLE pTmpNode        = LW_NULL;
    CHAR            cSelectSql[200] = {0};
    sqlite3        *pDb;
    CHAR          **cDbResult;
    CHAR           *pcErrMsg        = LW_NULL;
    INT             iRet;
    INT             iRow            = 0;
    INT             iColumn         = 0;
    INT             i;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s where status = 0;", LOGIN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || iRow == 0) {
        goto Error_Handle;
    }

    pLoginHead = (__PLOGIN_HANDLE)malloc(sizeof(__LOGIN_HANDLE) * iRow);
    if (pLoginHead == LW_NULL) {
        goto Error_Handle;
    }

    bzero(pLoginHead, sizeof(__LOGIN_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {
        pTmpNode = (__PLOGIN_HANDLE)(pLoginHead + (i - 1));

        strncpy(pTmpNode->cUsername, cDbResult[i * iColumn + 1], sizeof(pTmpNode->cUsername));
        strncpy(pTmpNode->cPasswd,   cDbResult[i * iColumn + 2], sizeof(pTmpNode->cPasswd));
        pTmpNode->iRole   = atoi(cDbResult[i * iColumn + 3]);
        pTmpNode->iStatus = atoi(cDbResult[i * iColumn + 4]);
    }

    *piRecordNum = iRow;

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
    return pLoginHead;

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
    *piRecordNum = 0;

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: authDBInfoGet
** ��������: ��֤Ȩ�ޱ����Ϣ��ȡ
** �䡡��  : pcRoleName         ��ɫ����
**           pcBuff             ���Ȩ����Ϣ��buff
**           iBuffLen           buff����
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  authDBInfoGet (PCHAR  pcRoleName, PCHAR  pcBuff, INT32  iBuffLen)
{
    CHAR            cSelectSql[200]  = {0};
    INT             iRet;
    CHAR          **cDbResult;
    INT             IRow, IColumn;
    CHAR           *pcErrMsg         = LW_NULL;
    sqlite3        *pDb              = LW_NULL;
    INT             i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pcBuff || iBuffLen < OPEN_RT_MAX_INDEX) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql,
             sizeof(cSelectSql),
             "SELECT * FROM %s;",
             strcmp(pcRoleName, OPERA_AUTH) ? VIEW_AUTH_TABLE : OPERA_AUTH_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        for (i = 0; i < IColumn; i++) {
            pcBuff[i] = atoi(cDbResult[1 * IColumn + i]);
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: authDBUpdate
** ��������: ��֤Ȩ�ޱ����Ϣ����
** �䡡��  : pAuthSet           ������Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  authDBUpdate (__PUSER_AUTH_SET  pAuthSet)
{
    INT       iRet;
    sqlite3  *pDb;
    CHAR      cSqlCmd[200] = {0};

    OPENRT_DB_HANDLE(pDb);

    if (LW_NULL == pAuthSet || LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "update %s set %s = %d;",
             strcmp(pAuthSet->cRole, OPERA_AUTH) ? VIEW_AUTH_TABLE : OPERA_AUTH_TABLE,
             pAuthSet->cTypeName,
             pAuthSet->cHandle);

    iRet = dataBaseCmdExec(pDb, cSqlCmd);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: loginTableCreate
** ��������: ��¼�������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  loginTableCreate (VOID)
{
    sqlite3   *pDb             = LW_NULL;
    PCHAR      pcOperaInserSql = "INSERT INTO "OPERA_AUTH_TABLE" VALUES (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                        "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                        "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                        "0, 0);";
    PCHAR      pcViewInserSql = "INSERT INTO "VIEW_AUTH_TABLE" VALUES (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                      "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
                                                                      "0, 0);";
    INT32      iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, LOGIN_TABLE, LOGIN_TABLE_HEAD));
    INIT_CHECK(dataBaseTbCreate(pDb, LOGIN_RECORD_TABLE, LOGIN_RECORD_TABLE_HEAD));

    /*
     *  ����Ȩ����֤���
     */
    dataBaseTbCreate(pDb, OPERA_AUTH_TABLE, AUTH_TABLE_HEAD);
    dataBaseTbCreate(pDb, VIEW_AUTH_TABLE, AUTH_TABLE_HEAD);

    dataBaseGetTbRowCol(pDb, OPERA_AUTH_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  ֻ���ֶ���                  */
        dataBaseCmdExec(pDb, pcOperaInserSql);                          /*  ��ʼ�����ݿ�ֵ              */
    }

    dataBaseGetTbRowCol(pDb, VIEW_AUTH_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  ֻ���ֶ���                  */
        dataBaseCmdExec(pDb, pcViewInserSql);                           /*  ��ʼ�����ݿ�ֵ              */
    }

    return  (ERROR_NONE);
}

