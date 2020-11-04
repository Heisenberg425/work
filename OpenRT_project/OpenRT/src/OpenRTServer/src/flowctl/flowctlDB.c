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
** 文   件   名: flowctlDB.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: flowctl数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "flowctl_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  端口映射配置信息
*********************************************************************************************************/
#define FLOWCTL_TABLE              "flowctl"                              /*  flowctl表名               */
#define FLOWCTL_TABLE_HEAD         "(id   INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "fc_type INT,"   \
                                   "fc_enable VARCHAR(8)," \
                                   "fc_proto VARCHAR(8),"  \
                                   "fc_start VARCHAR(64)," \
                                   "fc_end VARCHAR(64)," \
                                   "fc_sport INT,"  \
                                   "fc_eport INT,"  \
                                   "fc_ifname VARCHAR(16)," \
                                   "fc_uprate BIGINT,"   \
                                   "fc_downrate BIGINT," \
                                   "fc_bufsize BIGINT)"                 /*  flowctl表单格式             */
/*********************************************************************************************************
** 函数名称: flowctlGetFromDB
** 功能描述: 从数据库获取路由信息
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : pRouteHead          路由信息链表头
*********************************************************************************************************/
PVOID  flowctlGetFromDB (__PFLOWCTL_HANDLE  pFlowctlHandle, INT  *piRecordNum)
{
    INT                iRet;
    sqlite3           *pDb;
    CHAR             **cDbResult;
    CHAR               cSelectSql[200]  = {0};
    CHAR              *pcErrMsg         = LW_NULL;
    INT                iRow;
    INT                iColumn;
    INT                i;

    __PFLOWCTL_HANDLE  pFlowctlHead     = LW_NULL;
    __PFLOWCTL_HANDLE  pTmpNode         = LW_NULL;

    *piRecordNum = -1;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (LW_NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    if (LW_NULL != pFlowctlHandle) {
        snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE fc_type= %d ;",
                                                  FLOWCTL_TABLE, pFlowctlHandle->iType);
    } else {
        snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s ;", FLOWCTL_TABLE);
    }

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        goto Error_Handle;
    }

    if (iRow == 0) {
        *piRecordNum = 0;

        goto Error_Handle;
    }

    pFlowctlHead = (__PFLOWCTL_HANDLE)malloc(sizeof(__FLOWCTL_HANDLE) * iRow);
    if (LW_NULL == pFlowctlHead) {
           goto Error_Handle;
    }

    bzero(pFlowctlHead, sizeof(__FLOWCTL_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {
        pTmpNode = (__PFLOWCTL_HANDLE)(pFlowctlHead + (i - 1));

        pTmpNode->iType = atoi(cDbResult[i * iColumn + 1]);

        strncpy(pTmpNode->cEnable,   cDbResult[i * iColumn + 2], sizeof(pTmpNode->cEnable));
        strncpy(pTmpNode->cProtocol, cDbResult[i * iColumn + 3], sizeof(pTmpNode->cProtocol));
        strncpy(pTmpNode->cStartIp,  cDbResult[i * iColumn + 4], sizeof(pTmpNode->cStartIp));
        strncpy(pTmpNode->cEndIp,    cDbResult[i * iColumn + 5], sizeof(pTmpNode->cEndIp));

        pTmpNode->iStartPort = atoi(cDbResult[i * iColumn + 6]);
        pTmpNode->iEndPort   = atoi(cDbResult[i * iColumn + 7]);

        strncpy(pTmpNode->cIfname, cDbResult[i * iColumn + 8], sizeof(pTmpNode->cIfname));

        pTmpNode->ulUpRate   = atol(cDbResult[i * iColumn + 9]);
        pTmpNode->ulDownRate = atol(cDbResult[i * iColumn + 10]);
        pTmpNode->ulBufSize  = atol(cDbResult[i * iColumn + 11]);
    }

    *piRecordNum = iRow;

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pFlowctlHead);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (LW_NULL);
}

/*********************************************************************************************************
** 函数名称: __flowctlInsertItem
** 功能描述: 向流控表中添加一个条目
** 输  入  : pDb            数据库连接句柄
**           pFlowctlInfo   流控入参
** 输  出  : NONE
** 返  回  : 操作结果
*********************************************************************************************************/
static INT  __flowctlInsertItem (sqlite3  *pDb, __PFLOWCTL_HANDLE  pFlowctlInfo)
{
    CHAR          cTbInsertCmd[512]  = {0};
    PCHAR         pcErrMsg           = LW_NULL;
    INT           iRet;

    if (LW_NULL == pFlowctlInfo) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (fc_type, fc_enable, fc_proto, fc_start, "
             "fc_end,fc_sport, fc_eport, fc_ifname, fc_uprate,"
             "fc_downrate, fc_bufsize)"
             " VALUES( %d , '%s', '%s', '%s','%s',"
             " %d, %d, '%s', %lu, %lu, %lu);",
             FLOWCTL_TABLE,pFlowctlInfo->iType, pFlowctlInfo->cEnable,
             pFlowctlInfo->cProtocol, pFlowctlInfo->cStartIp,
             pFlowctlInfo->cEndIp, pFlowctlInfo->iStartPort,
             pFlowctlInfo->iEndPort, pFlowctlInfo->cIfname,
             pFlowctlInfo->ulUpRate, pFlowctlInfo->ulDownRate,
             pFlowctlInfo->ulBufSize);


   iRet = sqlite3_exec(pDb, cTbInsertCmd, 0, 0, &pcErrMsg);
   if (SQLITE_OK != iRet) {
       sqlite3_free(pcErrMsg);
       __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command %s, error_code =%d.\n",
                                             cTbInsertCmd, iRet);

       return  (PX_ERROR);
  }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __flowctlDeleteItem
** 功能描述: 删除流控表中的一个条目
** 输  入  : pDb            数据库连接句柄
**           pFlowctlInfo   流控入参
** 输  出  : NONE
** 返  回  : 操作结果
*********************************************************************************************************/
static INT  __flowctlDeleteItem (sqlite3  *pDb, __PFLOWCTL_HANDLE  pFlowctlInfo)
{
    INT           iRet;
    CHAR          cSqlCmd[256]  = {0};
    PCHAR         pcErrMsg      = LW_NULL;

    if (LW_NULL == pFlowctlInfo) {
        return  (PX_ERROR);
    }

    if (FCT_IP == pFlowctlInfo->iType) {
        snprintf(cSqlCmd,
                 sizeof(cSqlCmd),
                 "delete from %s where fc_type = %d and fc_start='%s' and fc_end='%s' "
                 "and fc_proto = '%s' and fc_ifname='%s';",
                 FLOWCTL_TABLE,
                 pFlowctlInfo->iType,
                 pFlowctlInfo->cStartIp,
                 pFlowctlInfo->cEndIp,
                 pFlowctlInfo->cProtocol,
                 pFlowctlInfo->cIfname);
    } else {
        snprintf(cSqlCmd,
                 sizeof(cSqlCmd),
                 "delete from %s where fc_type = %d and fc_ifname='%s';",
                 FLOWCTL_TABLE,
                 pFlowctlInfo->iType,
                 pFlowctlInfo->cIfname);
    }

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command %s, error_code =%d.\n",
                                               cSqlCmd, iRet);

        return  (PX_ERROR);
    }

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: __flowctlUpdateItem
** 功能描述: 删除流控表中的一个条目
** 输  入  : pDb            数据库连接句柄
**           pFlowctlInfo   流控入参
** 输  出  : NONE
** 返  回  : 操作结果
*********************************************************************************************************/
static INT  __flowctlUpdateItem (sqlite3  *pDb, __PFLOWCTL_HANDLE  pFlowctlInfo)
{
    INT           iRet;
    CHAR          cSqlCmd[512]  = {0};
    PCHAR         pcErrMsg      = LW_NULL;

    if (LW_NULL == pFlowctlInfo) {
        return  (PX_ERROR);
    }

    if (FCT_IP == pFlowctlInfo->iType) {
        if (0 == pFlowctlInfo->ulBufSize) {
            snprintf(cSqlCmd,
                     sizeof(cSqlCmd),
                     "update %s set fc_uprate=%lu, fc_downrate=%lu "
                     "where fc_type = %d and fc_start='%s' and fc_end='%s' "
                     "and fc_proto = '%s' and fc_ifname='%s';",
                     FLOWCTL_TABLE,
                     pFlowctlInfo->ulUpRate,
                     pFlowctlInfo->ulDownRate,
                     pFlowctlInfo->iType,
                     pFlowctlInfo->cStartIp,
                     pFlowctlInfo->cEndIp,
                     pFlowctlInfo->cProtocol,
                     pFlowctlInfo->cIfname);
        } else {
            snprintf(cSqlCmd,
                    sizeof(cSqlCmd),
                    "update %s set fc_uprate=%lu,"
                    " fc_downrate=%lu, fc_bufsize=%lu "
                    "where fc_type = %d and fc_start='%s' and fc_end='%s' "
                    "and fc_proto = '%s' and fc_ifname='%s';",
                    FLOWCTL_TABLE,
                    pFlowctlInfo->ulUpRate,
                    pFlowctlInfo->ulDownRate,
                    pFlowctlInfo->ulBufSize,
                    pFlowctlInfo->iType,
                    pFlowctlInfo->cStartIp,
                    pFlowctlInfo->cEndIp,
                    pFlowctlInfo->cProtocol,
                    pFlowctlInfo->cIfname);
        }
    } else {
        if (0 == pFlowctlInfo->ulBufSize) {
            snprintf(cSqlCmd,
                     sizeof(cSqlCmd),
                     "update %s set fc_uprate=%lu, fc_downrate=%lu"
                     " where fc_type = %d and fc_ifname='%s';",
                     FLOWCTL_TABLE,
                     pFlowctlInfo->ulUpRate,
                     pFlowctlInfo->ulDownRate,
                     pFlowctlInfo->iType,
                     pFlowctlInfo->cIfname);
        } else {
            snprintf(cSqlCmd,
                     sizeof(cSqlCmd),
                     "update %s set fc_uprate=%lu, "
                     "fc_downrate=%lu，fc_bufsize=%lu "
                     "where fc_type = %d and fc_ifname='%s';",
                     FLOWCTL_TABLE,
                     pFlowctlInfo->ulUpRate,
                     pFlowctlInfo->ulDownRate,
                     pFlowctlInfo->ulBufSize,
                     pFlowctlInfo->iType,
                     pFlowctlInfo->cIfname);
        }
    }

    iRet = sqlite3_exec(pDb, cSqlCmd, 0, 0, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        sqlite3_free(pcErrMsg);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command %s, error_code =%d.\n",
                                               cSqlCmd, iRet);

        return  (PX_ERROR);
    }

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: flowctlUpdateToDb
** 功能描述: 向流控表中更新一个条目
** 输  入  : cpcOperType      操作类型
**           pFlowctlInfo     流控参数信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  flowctlUpdateToDb (CPCHAR  cpcOperType, __PFLOWCTL_HANDLE  pFlowctlInfo)
{
    sqlite3      *pDb;

    if (LW_NULL == pFlowctlInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    if (!strcmp(cpcOperType, FLOWCTL_ADD)) {
        __flowctlInsertItem(pDb, pFlowctlInfo);
    } else if (!strcmp(cpcOperType, FLOWCTL_DELETE)) {
        __flowctlDeleteItem(pDb, pFlowctlInfo);
    } else if  (!strcmp(cpcOperType, FLOWCTL_CHANGE)) {
        __flowctlUpdateItem(pDb, pFlowctlInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "unknown operation.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: flowctlTableCreate
** 功能描述: 流控表不存在时，创建流控表
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  flowctlTableCreate (VOID)
{
    sqlite3         *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, FLOWCTL_TABLE, FLOWCTL_TABLE_HEAD);
}

