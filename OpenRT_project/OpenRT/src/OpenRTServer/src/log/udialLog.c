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
** 文   件   名: udialLog.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 16 日
**
** 描        述: 拨号日志实现
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include <time.h>
#include "liblog_config.h"
#include "log_common.h"
#include "intadaption/intada_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  拨号日志数据库配置信息
*********************************************************************************************************/
#define UDIAL_LOG_TABLE_HEAD       "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "TIMES       TEXT,"                               \
                                   "TIMEI       BIGINT,"                             \
                                   "NETIF       TEXT,"                               \
                                   "EVENT       TEXT)"                  /*  拨号表单格式                */
/*********************************************************************************************************
** 函数名称: udialLogNumGet
** 功能描述: 获取日志条目个数入口
** 输　入  : pUdialLogHandle    获取信息结构
** 输　出  : NONE
** 返　回  : iItermNum          个数
*********************************************************************************************************/
INT32  udialLogNumGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle)
{
    INT32  iItermNum = 0;

    if (LW_NULL == pUdialLogHandle) {
        return  (0);
    }

    iItermNum = logGetSpecInfoCount(LOG_TYPE_UDIAL,
                                    pUdialLogHandle->logStartTm,
                                    pUdialLogHandle->logEndTm,
                                    LW_NULL);

    return  ((iItermNum >= 0) ? iItermNum : 0);
}

/*********************************************************************************************************
** 函数名称: udialLogGet
** 功能描述: 获取指定拨号日志信息
** 输　入  : logStartTm         开始时间
**           logEndTm           结束时间
**           llStartIterm       开始序号
**           llEndIndex         结束序号
** 输　出  : pcTimeBuf          时间字符串
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT32  udialLogGetSpecInfo (time_t      logStartTm,
                            time_t      logEndTm,
                            INT64       llStartIndex,
                            INT64       llEndIndex,
                            PVOID       pvLogBuff,
                            INT32       iBuffLen)
{
    sqlite3           *pDb              = LW_NULL;
    CHAR             **cDbResult        = LW_NULL;
    CHAR              *pcErrMsg         = LW_NULL;
    CHAR               cSelectSql[200]  = {0};
    INT32              iRet;
    INT32              IRow, IColumn;
    INT32              i;

    __PUDIAL_LOG_INFO  pUdialLogNode    = LW_NULL;
    PVOID              pBuffTmp         = pvLogBuff;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pvLogBuff) {
        return  (PX_ERROR);
    }

    if (logStartTm < 0 && logEndTm < 0) {
        snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s  order by ID desc;", UDIAL_LOG_TABLE);
    } else if (logEndTm >= logStartTm) {
        snprintf(cSelectSql, sizeof(cSelectSql),
                 "SELECT * FROM %s WHERE TIMEI >= %lld AND TIMEI <= %lld  order by ID desc;",
                 UDIAL_LOG_TABLE, logStartTm, logEndTm);
    } else {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
            if (i < llStartIndex) {
                continue;
            }

            if (i > llEndIndex) {
                break;
            }

            pUdialLogNode          = (__PUDIAL_LOG_INFO)pvLogBuff;
            pUdialLogNode->llTime  = atoi(cDbResult[i * IColumn + 2]);

            strncpy(pUdialLogNode->cNetifName,
                    cDbResult[i * IColumn + 3],
                    sizeof(pUdialLogNode->cNetifName));
            strncpy(pUdialLogNode->cEvent,
                    cDbResult[i * IColumn + 4],
                    sizeof(pUdialLogNode->cEvent));

            pvLogBuff = (PCHAR)pvLogBuff + sizeof(__UDIAL_LOG_INFO);
            if (pvLogBuff - pBuffTmp > iBuffLen) {                      /*  内存不够                    */
                break;
            }
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get log data base udial info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: udialLogInsert
** 功能描述: 填充一条 udial 日志数据
** 输　入  : pcNetifName    网口名
**           pcEventInfo    拨号时间信息指针
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  udialLogInsert (PCHAR  pcNetifName, PCHAR  pcEventInfo)
{
    CHAR               cTbInsertCmd[500]  = {0};
    time_t             iTime;
    CHAR               cTImeString[30]    = {0};                        /*  必须要大于 26 个字节        */
    CHAR               cWebName[50]       = {0};
    INT32              iWebIndex;

    sqlite3            *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == pcNetifName)) {
        return;
    }

    intAdaptionGetWebName(pcNetifName, cWebName, &iWebIndex);
    iTime = logTimeGen(cTImeString);

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (TIMES, TIMEI, NETIF, EVENT) "
             "VALUES ('%s', %lld, '%s', '%s');",
             UDIAL_LOG_TABLE,
             cTImeString,
             iTime,
             cWebName,
             pcEventInfo);

    dataBaseCmdExec(pDb, cTbInsertCmd);

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "DELETE FROM %s WHERE %s NOT IN (SELECT %s FROM %s ORDER BY %s DESC LIMIT %d);",
             UDIAL_LOG_TABLE, "ID", "ID", UDIAL_LOG_TABLE, "ID", MAX_RECORD_NUM);

    dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** 函数名称: udialLogDel
** 功能描述: 清除拨号日志
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  udialLogDel (VOID)
{
    CHAR               cTbCmd[64] = {0};
    sqlite3            *pDb       = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb)  {
        return  (PX_ERROR);
    }

    snprintf(cTbCmd, sizeof(cTbCmd), "DELETE FROM %s;", UDIAL_LOG_TABLE);

    return  dataBaseCmdExec(pDb, cTbCmd);
}
/*********************************************************************************************************
** 函数名称: udialTableCreate
** 功能描述: 初始化 udial 日志数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  udialLogTableCreate (VOID)
{
    sqlite3      *pDb = LW_NULL;;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, UDIAL_LOG_TABLE, UDIAL_LOG_TABLE_HEAD);
}
