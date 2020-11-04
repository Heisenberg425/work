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
** 文   件   名: routeDB.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 静态路由数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "libroute_config.h"
#include "route_common.h"
#include "common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  静态路由配置信息
*********************************************************************************************************/
#define ROUTE_TABLE                "route"                              /*  路由表单名                  */
#define ROUTE_TABLE_HEAD           "(id         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "destination VARCHAR(64)," \
                                   "gateway     VARCHAR(64)," \
                                   "genmask     VARCHAR(20)," \
                                   "flags       VARCHAR(12)," \
                                   "prefix      INT,"\
                                   "metric      INT,"\
                                   "ref         INT,"\
                                   "use         INT,"\
                                   "iface       VARCHAR(16)," \
                                   "ip_famliy   VARCHAR(8),"\
                                   "route_type  VARCHAR(12))"          /*  路由表单格式                */
/*********************************************************************************************************
** 函数名称: __routeDeleteItem
** 功能描述: 删除路由表中的一个条目
** 输  入  : pDb   数据库句柄
**           pRouteInfo 路由信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __routeDeleteItem (sqlite3  *pDb, __PROUTE_HANDLE  pRouteInfo)
{
    CHAR          cSqlCmd[256]  = {0};
    INT           iRet;
    PCHAR         pcErrMsg      = LW_NULL;

    if (LW_NULL == pRouteInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof (cSqlCmd), "delete from %s "
                                        "where destination = '%s' and genmask = '%s' "
                                        " and gateway = '%s' and iface = '%s';" ,
                                        ROUTE_TABLE,
                                        pRouteInfo->cDestination,
                                        pRouteInfo->cGenmask,
                                        pRouteInfo->cGateway,
                                        pRouteInfo->cIface);

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: staticRouteCleanFromDbByIfName
** 功能描述: 通过网卡名删除路由表中的一个条目
** 输  入  : cpcIfName     网口名
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  staticRouteCleanFromDbByIfName (CPCHAR  cpcIfName)
{
    CHAR          cSqlCmd[256]  = {0};
    sqlite3      *pDb           = NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == cpcIfName) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof (cSqlCmd), "DELETE FROM %s WHERE iface = '%s';", ROUTE_TABLE, cpcIfName);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** 函数名称: __routeInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb   数据库句柄
**           pRouteInfo 路由信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __routeInsertItem (sqlite3  *pDb, __PROUTE_HANDLE  pRouteInfo)
{
    CHAR          cTbInsertCmd[200]  = {0};
    sqlite3_stmt *pstmt              = LW_NULL;
    const CHAR   *pcTail             = LW_NULL;
    INT           iRet;

    if (LW_NULL == pRouteInfo) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd), "INSERT INTO %s (destination, genmask, "
                                                 "gateway, flags, metric, iface, route_type)"
                                                 " VALUES(?, ?, ?, ?, ?, ?, ?);", ROUTE_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base prepare failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  将需要添加的信息加入数据库
     */
    sqlite3_bind_text(pstmt, 1, pRouteInfo->cDestination, strlen(pRouteInfo->cDestination), LW_NULL);
    sqlite3_bind_text(pstmt, 2, pRouteInfo->cGenmask, strlen(pRouteInfo->cGenmask), LW_NULL);
    sqlite3_bind_text(pstmt, 3, pRouteInfo->cGateway, strlen(pRouteInfo->cGateway), LW_NULL);
    sqlite3_bind_text(pstmt, 4, pRouteInfo->cFlags, strlen(pRouteInfo->cFlags), LW_NULL);
    sqlite3_bind_int (pstmt, 5, pRouteInfo->iMetric);
    sqlite3_bind_text(pstmt, 6, pRouteInfo->cIface, strlen(pRouteInfo->cIface), LW_NULL);
    sqlite3_bind_text(pstmt, 7, pRouteInfo->cRouteType, strlen(pRouteInfo->cRouteType), LW_NULL);

    iRet = sqlite3_step(pstmt);

    sqlite3_reset(pstmt);
    sqlite3_finalize(pstmt);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: __routeUpdateDefaultRoute
** 功能描述: 更新路由表中默认路由
** 输  入  : pDb   数据库句柄
**           pRouteInfo 路由信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __routeUpdateDefaultRoute (sqlite3 *pDb, __PROUTE_HANDLE  pRouteInfo)
{
    INT           iRet;
    PCHAR         pcErrMsg      = LW_NULL;
    CHAR          cSqlCmd[200]  = {0};

    if (LW_NULL == pRouteInfo) {
        return  (PX_ERROR);
    }

    /*
     *  如果存在则删除
     */
    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "delete from %s where route_type = '%s' ;", ROUTE_TABLE, DEFAULT_ROUTE);
    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "sqlite3_exec failed.\n");

        return  (PX_ERROR);
    }

    iRet += __routeInsertItem(pDb, pRouteInfo);

    sqlite3_free(pcErrMsg);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: routeGetFromDB
** 功能描述: 从数据库获取路由信息，内存由调用者释放
** 输  入  : NONE
** 输  出  : piRecordNum         路由信息总条数
** 返  回  : 路由信息链表头
*********************************************************************************************************/
PVOID  routeGetFromDB (INT  *piRecordNum)
{
    INT                iRet;
    __PROUTE_HANDLE    pRouteNode      = LW_NULL;
    __PROUTE_HANDLE    pTmpNode        = LW_NULL;
    sqlite3           *pDb             = LW_NULL;
    CHAR             **cDbResult       = LW_NULL;
    CHAR              *pcErrMsg        = LW_NULL;
    CHAR               cSelectSql[200] = {0};
    INT                iRow            = 0;
    INT                iColumn         = 0;
    INT                i;

    *piRecordNum = -1;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (LW_NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", ROUTE_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        goto  Error_Handle;
    }

    if (iRow == 0) {
        *piRecordNum = 0;

        goto  Error_Handle;
    }

    pRouteNode = (__PROUTE_HANDLE)malloc(sizeof(__ROUTE_HANDLE) * iRow);
    if (pRouteNode == LW_NULL) {
        goto  Error_Handle;
    }

    bzero(pRouteNode, sizeof(__ROUTE_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {
        pTmpNode = (__PROUTE_HANDLE)(pRouteNode + (i - 1));

        strncpy(pTmpNode->cDestination, cDbResult[i * iColumn + 1], sizeof(pTmpNode->cDestination));
        strncpy(pTmpNode->cGateway,     cDbResult[i * iColumn + 2], sizeof(pTmpNode->cGateway));
        strncpy(pTmpNode->cGenmask,     cDbResult[i * iColumn + 3], sizeof(pTmpNode->cGenmask));
        strncpy(pTmpNode->cFlags,       cDbResult[i * iColumn + 4], sizeof(pTmpNode->cFlags));

        pTmpNode->iMetric = atoi(cDbResult[i * iColumn + 6]);

        strncpy(pTmpNode->cIface,     cDbResult[i * iColumn + 9],  sizeof(pTmpNode->cIface));
        strncpy(pTmpNode->cRouteType, cDbResult[i * iColumn + 11], sizeof(pTmpNode->cRouteType));
    }

    *piRecordNum = iRow;

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pRouteNode);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: routeUpdateToDb
** 功能描述: 更新数据表操作，包括增删改操作
** 输  入  : cpcOperType         操作类型字符串
**           pRouteInfo          路由信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  routeUpdateToDb (CPCHAR  cpcOperType, __PROUTE_HANDLE  pRouteInfo)
{
    sqlite3      *pDb = NULL;

    if (LW_NULL == pRouteInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DB connection is null.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    if (!strcmp(cpcOperType, ROUTE_ADD)) {
        __routeInsertItem(pDb, pRouteInfo);
    } else if (!strcmp(cpcOperType, ROUTE_DEL)) {
        __routeDeleteItem(pDb, pRouteInfo);
    } else if  (!strcmp(cpcOperType, ROUTE_CHG)) {
        __routeUpdateDefaultRoute(pDb, pRouteInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "unknown operation.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: routeTableCreate
** 功能描述: 路由表创建
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  routeTableCreate (VOID)
{
    sqlite3   *pDb  = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, ROUTE_TABLE, ROUTE_TABLE_HEAD);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
