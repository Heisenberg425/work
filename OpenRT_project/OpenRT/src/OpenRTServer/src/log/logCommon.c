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
** 文   件   名: logCommon.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 24 日
**
** 描        述: 日志通用操作函数
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include "log_common.h"
#include "database/database_common.h"
#include <time.h>
/*********************************************************************************************************
** 函数名称: logTimeGen
** 功能描述: 产生一条 log 日志的时间
** 输　入  : NONE
** 输　出  : pcTimeBuf          时间字符串
** 返　回  : iTime              时间整形值
*********************************************************************************************************/
time_t  logTimeGen (PCHAR  pcTimeBuf)
{
    struct tm   logtm;
    time_t      iTime;

    bzero(&logtm, sizeof(logtm));
    iTime = time(NULL);

    ctime_r(&iTime, pcTimeBuf);

    return  (iTime);
}

/*********************************************************************************************************
** 函数名称: logGetSpecInfoCount
** 功能描述: 获取日志指定条件信息条目数
** 输　入  : iLogTableType      日志类型
**           logStartTm         开始时间
**           logEndTm           结束时间
**           pcOtherSpec        其他限制条件
** 输　出  : NONE
** 返　回  : iRet               条目个数
*********************************************************************************************************/
INT32  logGetSpecInfoCount (INT32   iLogTableType,
                            time_t  logStartTm,
                            time_t  logEndTm,
                            PCHAR   pcOtherSpec)
{
    sqlite3      *pDb              = LW_NULL;
    CHAR        **cDbResult        = LW_NULL;
    CHAR         *pcErrMsg         = LW_NULL;
    PCHAR         pcTableName      = LW_NULL;
    CHAR          cSelectSql[200]  = {0};
    INT32         iRet;
    INT32         IRow, IColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    /*
     *  获取日志表单名称
     */
    switch (iLogTableType) {
    case LOG_TYPE_UDIAL:
        pcTableName = UDIAL_LOG_TABLE;
        break;

    case LOG_TYPE_VPN:
        pcTableName = VPN_LOG_TABLE;
        break;

    default:
        break;
    }

    if (LW_NULL == pcTableName) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    if (LW_NULL != pcOtherSpec) {
        if (logStartTm < 0 && logEndTm < 0) {
            snprintf(cSelectSql, sizeof(cSelectSql),
                     "SELECT count(*) FROM %s WHERE %s;",
                     pcTableName, pcOtherSpec);
        } else if (logEndTm >= logStartTm) {
            snprintf(cSelectSql, sizeof(cSelectSql),
                     "SELECT count(*) FROM %s WHERE TIMEI >= %lld AND TIMEI <= %lld AND %s;",
                     pcTableName, logStartTm, logEndTm, pcOtherSpec);
        } else {
            return  (PX_ERROR);
        }
    } else {
        if (logStartTm < 0 && logEndTm < 0) {
            snprintf(cSelectSql, sizeof(cSelectSql), "SELECT count(*) FROM %s;", pcTableName);
        } else if (logEndTm >= logStartTm) {
            snprintf(cSelectSql, sizeof(cSelectSql),
                     "SELECT count(*) FROM %s WHERE TIMEI >= %lld AND TIMEI <= %lld;",
                     pcTableName, logStartTm, logEndTm);
        } else {
            return  (PX_ERROR);
        }
    }

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (0 == IRow) {
            iRet = 0;
        } else {
            iRet = atoi(cDbResult[1 * IColumn + 0]);
        }
    } else {
        iRet = PX_ERROR;
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base wan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: logTableDelet
** 功能描述: 删除日志数据库表单
** 输　入  : iLogTableType      日志数据库表单
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  logTableDelet (INT32  iLogTableType)
{
    sqlite3      *pDb         = LW_NULL;;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return;
    }

    switch (iLogTableType) {
    case LOG_TYPE_UDIAL:
        dataBaseTbInfoDelete(pDb, UDIAL_LOG_TABLE);
        break;

    case LOG_TYPE_VPN:
        dataBaseTbInfoDelete(pDb, VPN_LOG_TABLE);
        break;

    default:
        break;
    }
}

