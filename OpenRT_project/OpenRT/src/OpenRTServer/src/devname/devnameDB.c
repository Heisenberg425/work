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
** 文   件   名: devnameDB.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: device name 数据库操作
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
  端口映射配置信息
*********************************************************************************************************/
#define DEV_NAME_TABLE            "devname"                             /*  device name 表单名          */
#define DEV_NAME_TABLE_HEAD       "(devname      VARCHAR(32))"          /*  device name 表单格式        */
/*********************************************************************************************************
** 函数名称: devNameGetFromDB
** 功能描述: 从数据库获取设备名信息
** 输  入  : piNum       设备名信息条数
** 输  出  : NONE
** 返  回  : 设备名结构指针
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
** 函数名称: __devNameInfoInsert
** 功能描述: 插入数据到数据库
** 输  入  : pDb           数据库句柄
**           cpcDevName    设备名指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: __devNameUpdateItem
** 功能描述: 更新数据库
** 输  入  : pDb         数据库句柄
**           pDevName    设备名结构指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: devNameUpdateToDb
** 功能描述: 更新数据库
** 输  入  : pDevName    设备名结构指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
** 函数名称: devNameTableCreate
** 功能描述: 初始化 device name 数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
