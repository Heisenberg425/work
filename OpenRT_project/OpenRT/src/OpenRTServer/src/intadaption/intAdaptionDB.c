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
** 文   件   名: intAdaptionDB.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述: 网口顺序定位数据库操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include "intAdaptionExec.h"
#include "intada_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  自适应定位数据库配置信息
*********************************************************************************************************/
#define INTADAPTION_TABLE          "intadaption"                        /*  数据库表单名                */
#define INTADAPTION_TABLE_HEAD     "(ID       INTEGER PRIMARY KEY AUTOINCREMENT,"\
                                   "WEBINDEX  INT,"   \
                                   "WEBNAME   TEXT,"  \
                                   "NETNAME   TEXT,"  \
                                   "STATUS    TEXT)"                    /*  数据库表单格式              */
/*********************************************************************************************************
** 函数名称: intAdaptionDBClear
** 功能描述: 清空数据库内容
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
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
** 函数名称: intAdaptionDBFindFinish
** 功能描述: 监测数据库是否有状态为 finished 的条目
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           有此内容
**           ERROR_NONE         无此内容
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
** 函数名称: intAdaptionDBGetInfo
** 功能描述: 获取数据库中对应信息
** 输　入  : pNetInfo           获取的信息
**           cFlag              0：通过webindex获取信息
**                              1：通过netif名称获取信息
**                              2：通过web netif 名称获取信息
** 输　出  : NONE
** 返　回  : pNetInfo           获取的信息
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  只要存在一个就说明完成      */
        pNetInfo->iWebIndex = sqlite3_column_int(pstmt, 1);

        strncpy(pNetInfo->cWebName,  (CPCHAR)sqlite3_column_text(pstmt, 2), sizeof(pNetInfo->cWebName));
        strncpy(pNetInfo->cNetIfName,(CPCHAR)sqlite3_column_text(pstmt, 3), sizeof(pNetInfo->cNetIfName));
        strncpy(pNetInfo->cStatus,   (CPCHAR)sqlite3_column_text(pstmt, 4), sizeof(pNetInfo->cStatus));

        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    }

    /*
     *  只要有一个不存在就说明没完成
     */
    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: intadaStatusGetFromDbByName
** 功能描述: 根据网口名获取网口状态信息
** 输　入  : cpcNetIfName    网口名
** 输　出  : piStatus        网口状态
** 返　回  : ERROR_CODE
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  只要存在一个就说明完成      */
        pcStatus = (PCHAR)sqlite3_column_text(pstmt, 4);
        if (0 == strcmp(pcStatus, NET_UNUSED) || '\0' == pcStatus[0]) {
            *piStatus = 0;
        } else {
            *piStatus = 1;
        }

        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    } else {                                                            /*  只要有一个不存在就说明没完成*/
        sqlite3_finalize(pstmt);
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** 函数名称: intAdaptionDBSetFinish
** 功能描述: 更新所有列为设置成功
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
** 函数名称: intAdaptionDBUpdate
** 功能描述: 更新 Web 网口信息到数据库
** 输　入  : pIntAdaptionInfo   web 网口信息
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
     *  这里的 id 是用的 sq 的自增属性
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
** 函数名称: intAdaptionTableCreate
** 功能描述: 初始化 intadaption 数据库表格
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
