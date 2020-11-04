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
** 文   件   名: qosDB.c
**
** 创   建   人: Wang.JingShi ( 王京石)
**
** 文件创建日期: 2018 年 8 月 15 日
**
** 描        述: QoS数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include "qos_common.h"
#include "common.h"
#include "intadaption/intAdaptionExec.h"
#include "database/database_common.h"

/*********************************************************************************************************
  QoS配置信息
*********************************************************************************************************/
#define QOS_TABLE                "qos"                                  /*  QoS表单名                   */
#define QOS_TABLE_HEAD           "(ID        INTEGER PRIMARY KEY AUTOINCREMENT," \
                                 "NETIF      TEXT," \
                                 "RULE       TEXT," \
                                 "IPS        TEXT," \
                                 "IPE        TEXT," \
                                 "PORTS      INTEGER," \
                                 "PORTE      INTEGER," \
                                 "CMPMETHOD  TEXT," \
                                 "PRIO       TINYINT," \
                                 "DONTDROP   TINYINT)"                  /*  QoS表单格式                 */

/*********************************************************************************************************
** 函数名称: qosIsExists
** 功能描述: 判断数据库中是否已存在此条目
** 输  入  : pQosInfo          QoS信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  qosIsExists (__PQOS_HANDLE  pQosInfo)
{
    CHAR           cSelectSql[512]  = {0};
    CHAR           cIpStart[20]     = {0};
    CHAR           cIpEnd[20]       = {0};
    CHAR         **cDbResult        = LW_NULL;
    CHAR          *pcErrMsg         = LW_NULL;
    sqlite3       *pDb              = LW_NULL;
    INT            iRow             = 0;
    INT            iColumn          = 0;
    INT            isExist          = LW_FALSE;
    INT            iRet;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    strncpy(cIpStart, inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpStart), sizeof(cIpStart));
    strncpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpEnd), sizeof(cIpEnd));

    snprintf(cSelectSql, sizeof(cSelectSql),
             "SELECT * FROM %s WHERE NETIF='%s' AND RULE='%s' AND " \
             "IPS='%s' AND IPE='%s' AND PORTS=%u AND PORTE=%u AND " \
             "CMPMETHOD='%s' AND PRIO=%u AND DONTDROP=%u;",
             QOS_TABLE, pQosInfo->cNetif, pQosInfo->cRule,
             cIpStart, cIpEnd, pQosInfo->uiPortStart, pQosInfo->uiPortEnd,
             pQosInfo->cCmpMethod, pQosInfo->uiPriority, pQosInfo->uiDontDrop);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        isExist = LW_TRUE;
    }

    if (SQLITE_OK != iRet) {
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        sqlite3_free(pcErrMsg);
        sqlite3_free_table(cDbResult);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);

    return  (isExist);
}
/*********************************************************************************************************
** 函数名称: qosGetFromDB
** 功能描述: 从数据库获取 QoS 信息
** 输  入  : NONE
** 输  出  : piRecordNum   QoS 信息个数
** 返  回  : QoS 信息
*********************************************************************************************************/
PVOID  qosGetFromDB (INT  *piRecordNum)
{
    CHAR            cSelectSql[64] = {0};
    CHAR          **cDbResult      = LW_NULL;
    CHAR           *pcErrMsg       = LW_NULL;
    sqlite3        *pDb            = LW_NULL;
    __PQOS_HANDLE   pQosHeader     = LW_NULL;
    __PQOS_HANDLE   pTmpNode       = LW_NULL;
    INT             iRet;
    INT             iRow, iColumn;
    INT             i;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", QOS_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || iRow == 0) {
        goto Error_Handle;
    }

    pQosHeader = (__PQOS_HANDLE)malloc(sizeof(__QOS_HANDLE) * iRow);
    if (pQosHeader == LW_NULL) {
        goto Error_Handle;
    }

    bzero(pQosHeader, sizeof(__QOS_HANDLE) * iRow);

    /*
     *  从数据库中获取 QoS 信息
     */
    for (i = 1; i <= iRow; i++) {                                       /*  按行获取(过滤第一行)        */
        pTmpNode = (__PQOS_HANDLE)(pQosHeader + (i - 1));

        strncpy(pTmpNode->cNetif, cDbResult[i * iColumn + 1], sizeof(pTmpNode->cNetif));
        intAdaptionGetWebNetif(pTmpNode->cNetif, pTmpNode->cWebName);

        strncpy(pTmpNode->cRule,      cDbResult[i * iColumn + 2], sizeof(pTmpNode->cRule));
        strncpy(pTmpNode->cCmpMethod, cDbResult[i * iColumn + 7], sizeof(pTmpNode->cCmpMethod));

        pTmpNode->uiIpStart   = inet_addr(cDbResult[i * iColumn + 3]);
        pTmpNode->uiIpEnd     = inet_addr(cDbResult[i * iColumn + 4]);
        pTmpNode->uiPortStart = atoi(cDbResult[i * iColumn + 5]);
        pTmpNode->uiPortEnd   = atoi(cDbResult[i * iColumn + 6]);
        pTmpNode->uiPriority  = atoi(cDbResult[i * iColumn + 8]);
        pTmpNode->uiDontDrop  = atoi(cDbResult[i * iColumn + 9]);
    }

    *piRecordNum = iRow;

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pQosHeader);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    *piRecordNum = 0;

    return  (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: __qosInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb       数据库句柄
**           pQosInfo  QoS 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __qosInsertItem (sqlite3  *pDb, __PQOS_HANDLE  pQosInfo)
{
    CHAR          cTbInsertCmd[512] = {0};
    CHAR          cIpStart[20]      = {0};
    CHAR          cIpEnd[20]        = {0};
    sqlite3_stmt *pstmt             = LW_NULL;
    const CHAR   *pcTail            = LW_NULL;
    INT           iRet;

    if (NULL == pDb || NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd),
             "INSERT INTO %s (NETIF, RULE, IPS, IPE, PORTS, PORTE, CMPMETHOD, PRIO, DONTDROP) " \
             "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);", QOS_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  将需要添加的信息加入数据库
     */
    strncpy(cIpStart, inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpStart), sizeof(cIpStart));
    strncpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpEnd),   sizeof(cIpEnd));

    sqlite3_bind_text(pstmt,  1,  pQosInfo->cNetif, strlen(pQosInfo->cNetif), LW_NULL);
    sqlite3_bind_text(pstmt,  2,  pQosInfo->cRule,  strlen(pQosInfo->cRule),  LW_NULL);
    sqlite3_bind_text(pstmt,  3,  cIpStart, strlen(cIpStart), LW_NULL);
    sqlite3_bind_text(pstmt,  4,  cIpEnd,   strlen(cIpEnd),   LW_NULL);
    sqlite3_bind_int(pstmt,   5,  pQosInfo->uiPortStart);
    sqlite3_bind_int(pstmt,   6,  pQosInfo->uiPortEnd);
    sqlite3_bind_text(pstmt,  7,  pQosInfo->cCmpMethod, strlen(pQosInfo->cCmpMethod), LW_NULL);
    sqlite3_bind_int(pstmt,   8,  pQosInfo->uiPriority);
    sqlite3_bind_int(pstmt,   9,  pQosInfo->uiDontDrop);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __qosDeleteItem
** 功能描述: 删除 QoS 中的一个条目
** 输  入  : pDb       数据库句柄
**           pQosInfo  QoS 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __qosDeleteItem (sqlite3  *pDb, __PQOS_HANDLE  pQosInfo)
{
    CHAR          cSqlCmd[512] = {0};
    CHAR          cIpStart[20] = {0};
    CHAR          cIpEnd[20]   = {0};
    PCHAR         pcErrMsg     = LW_NULL;
    INT           iRet;

    if (NULL == pDb || NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    strcpy(cIpStart, inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpStart));
    strcpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpEnd));

    if (0 == stricmp(pQosInfo->cRule, "ip")) {
        snprintf(cSqlCmd, sizeof(cSqlCmd), "DELETE FROM %s WHERE " \
                                           "NETIF='%s'      AND "  \
                                           "RULE='%s'       AND "  \
                                           "IPS='%s'        AND "  \
                                           "IPE='%s'        AND "  \
                                           "CMPMETHOD='%s'  AND "  \
                                           "PRIO=%u         AND "  \
                                           "DONTDROP=%u;",
                                           QOS_TABLE, pQosInfo->cNetif, pQosInfo->cRule,
                                           cIpStart, cIpEnd, pQosInfo->cCmpMethod,
                                           pQosInfo->uiPriority, pQosInfo->uiDontDrop);
    } else {
        snprintf(cSqlCmd, sizeof(cSqlCmd), "DELETE FROM %s WHERE " \
                                           "NETIF='%s'      AND "  \
                                           "RULE='%s'       AND "  \
                                           "IPS='%s'        AND "  \
                                           "IPE='%s'        AND "  \
                                           "PORTS=%u        AND "  \
                                           "PORTE=%u        AND "  \
                                           "CMPMETHOD='%s'  AND "  \
                                           "PRIO=%u         AND "  \
                                           "DONTDROP=%u;",
                                           QOS_TABLE, pQosInfo->cNetif, pQosInfo->cRule,
                                           cIpStart, cIpEnd, pQosInfo->uiPortStart,
                                           pQosInfo->uiPortEnd, pQosInfo->cCmpMethod,
                                           pQosInfo->uiPriority, pQosInfo->uiDontDrop);
    }

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: qosConfigToDb
** 功能描述: 向数据库中添加一个条目
** 输  入  : pQosInfo  QoS 参数信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  qosConfigToDb (__PQOS_HANDLE  pQosInfo)
{
    sqlite3  *pDb;
    INT       iRet;

    if (LW_NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = qosIsExists(pQosInfo);
    if (LW_FALSE == iRet) {
        __qosInsertItem(pDb, pQosInfo);
    } else if (PX_ERROR == iRet) {
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: qosDeleteFromDb
** 功能描述: 从数据库中删除一个条目
** 输  入  : pQosInfo    QoS 参数信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  qosDeleteFromDb (__PQOS_HANDLE  pQosInfo)
{
    sqlite3      *pDb;
    INT           iRet;

    if (LW_NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = __qosDeleteItem(pDb, pQosInfo);
    if (PX_ERROR == iRet) {
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: qosDataBaseInit
** 功能描述: 初始化 QoS 数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  qosTableCreate (VOID)
{
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, QOS_TABLE, QOS_TABLE_HEAD);
}

