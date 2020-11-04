/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: loginDB.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述: login数据库操作
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
  数据库配置信息
*********************************************************************************************************/
#define LOGIN_TABLE                "login"                              /*  登录表单名                  */
#define LOGIN_TABLE_HEAD           "(id         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "username    VARCHAR(64),"                       \
                                   "passwd      VARCHAR(64),"                       \
                                   "role        INT,"                               \
                                   "status      INT)"                   /*  登录表单格式                */
/*********************************************************************************************************
  记录web用户登出操作，记录用户名、操作时间、clientIP，操作结果等信息
*********************************************************************************************************/
#define LOGIN_RECORD_TABLE         "login_record"                       /*  登录记录表单名              */
#define LOGIN_RECORD_TABLE_HEAD    "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "USERNAME    VARCHAR(64),"                        \
                                   "CLIENT_IP   VARCHAR(64),"                        \
                                   "TIMES       VARCHAR(64),"                        \
                                   "TIMEI       BIGINT,"                             \
                                   "RESULT      INT,"                                \
                                   "TYPE        INT)"                   /*  type: login/logout          */
/*********************************************************************************************************
  认证信息
*********************************************************************************************************/
#define OPERA_AUTH_TABLE           "operaauth"                          /*  操作者表单名                */
#define VIEW_AUTH_TABLE            "viewauth"                           /*  观察者表单名                */
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
** 函数名称: loginRecordGetFromDB
** 功能描述: 从数据库获取用户信息,内存由调用者释放
** 输  入  : NONE
** 输  出  : piRecordNum        用户总条数
** 返  回  : pLoginHead         用户链表头
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
** 函数名称: loginRecordInsert
** 功能描述: 增加一条登录行为记录
** 输　入  : pLoginRecord 登录行为信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  loginRecordInsert (__PLOGIN_RECORD  pLoginRecord)
{

    CHAR     cTbInsertCmd[256]  = {0};
    time_t   iTime;
    CHAR     cTImeString[30]    = {0};                        /*  必须要大于 26 个字节        */
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
** 函数名称: loginInsert
** 功能描述: 增加一个用户数据
** 输　入  : pLogin 用户信息
** 输　出  : NONE
** 返　回  : 操作结果
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
** 函数名称: loginDelete
** 功能描述: 删除一个用户数据
** 输　入  : pLogin 用户信息
** 输　出  : NONE
** 返　回  : 操作结果
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
** 函数名称: disableUserLogin
** 功能描述: 禁止用户登录
** 输　入  : pLogin 用户信息
** 输　出  : NONE
** 返　回  : 操作结果
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
** 函数名称: changePasswd
** 功能描述: 增加一个用户数据
** 输　入  : pLogin       用户信息
** 输　出  : NONE
** 返　回  : NONE
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
** 函数名称: loginGetFromDB
** 功能描述: 从数据库获取用户信息,内存由调用者释放
** 输  入  : NONE
** 输  出  : piRecordNum        用户总条数
** 返  回  : pLoginHead         用户链表头
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
** 函数名称: authDBInfoGet
** 功能描述: 认证权限表格信息获取
** 输　入  : pcRoleName         角色名称
**           pcBuff             存放权限信息的buff
**           iBuffLen           buff长度
** 输　出  : NONE
** 返　回  : 操作结果
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
** 函数名称: authDBUpdate
** 功能描述: 认证权限表格信息更新
** 输　入  : pAuthSet           更新信息
** 输　出  : NONE
** 返　回  : 操作结果
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
** 函数名称: loginTableCreate
** 功能描述: 登录管理表创建
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
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
     *  创建权限认证表格
     */
    dataBaseTbCreate(pDb, OPERA_AUTH_TABLE, AUTH_TABLE_HEAD);
    dataBaseTbCreate(pDb, VIEW_AUTH_TABLE, AUTH_TABLE_HEAD);

    dataBaseGetTbRowCol(pDb, OPERA_AUTH_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        dataBaseCmdExec(pDb, pcOperaInserSql);                          /*  初始化数据库值              */
    }

    dataBaseGetTbRowCol(pDb, VIEW_AUTH_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        dataBaseCmdExec(pDb, pcViewInserSql);                           /*  初始化数据库值              */
    }

    return  (ERROR_NONE);
}

