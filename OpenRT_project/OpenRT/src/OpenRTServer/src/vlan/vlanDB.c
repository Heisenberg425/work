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
** 文   件   名: vlanDB.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: vlan 数据库操作
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
  端口映射配置信息
*********************************************************************************************************/
#define VLAN_TABLE                "vlan"                              /*  vlan表单名                  */
#define VLAN_TABLE_HEAD           "(id         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "vlanID     INT," \
                                   "iface      VARCHAR(16),"\
                                   "tag        INT," \
                                   "priority   INT)"          /*  vlan表单格式                */
/*********************************************************************************************************
** 函数名称: __vlanIsExists
** 功能描述: 判断 VLAN 是否已存在
** 输  入  : pVlanInfo     VLAN 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: vlanGetFromDB
** 功能描述: 从数据库获取 VLAN 信息
** 输  入  : NONE
** 输  出  : piRecordNum         VLAN 信息个数
** 返  回  : 成功返回 VLAN 信息，失败返回 NULL
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

    for (i = 1; i <= iRow; i++) {                                   /*  按行获取(过滤第一行)        */
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
** 函数名称: __vlanInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb         数据库句柄
**           pVlanInfo   VLAN 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
     *  将需要添加的信息加入数据库
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
** 函数名称: __vlanDeleteItem
** 功能描述: 删除一个条目
** 输  入  : pDb         数据库句柄
**           pVlanInfo   VLAN 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: __vlanUpdateItem
** 功能描述: 更新表中的一个条目
** 输  入  : pDb         数据库句柄
**           pVlanInfo   VLAN 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: vlanUpdateToDb
** 功能描述: 更新数据库
** 输  入  : cpcOperType       操作类型
**           pVlanInfo         VLAN 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: vlanTableCreate
** 功能描述: 初始化 VLAN 数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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

