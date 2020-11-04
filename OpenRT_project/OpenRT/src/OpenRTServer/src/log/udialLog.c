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
** ��   ��   ��: udialLog.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 16 ��
**
** ��        ��: ������־ʵ��
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
  ������־���ݿ�������Ϣ
*********************************************************************************************************/
#define UDIAL_LOG_TABLE_HEAD       "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "TIMES       TEXT,"                               \
                                   "TIMEI       BIGINT,"                             \
                                   "NETIF       TEXT,"                               \
                                   "EVENT       TEXT)"                  /*  ���ű���ʽ                */
/*********************************************************************************************************
** ��������: udialLogNumGet
** ��������: ��ȡ��־��Ŀ�������
** �䡡��  : pUdialLogHandle    ��ȡ��Ϣ�ṹ
** �䡡��  : NONE
** ������  : iItermNum          ����
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
** ��������: udialLogGet
** ��������: ��ȡָ��������־��Ϣ
** �䡡��  : logStartTm         ��ʼʱ��
**           logEndTm           ����ʱ��
**           llStartIterm       ��ʼ���
**           llEndIndex         �������
** �䡡��  : pcTimeBuf          ʱ���ַ���
** ������  : ERROR_CODE
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
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
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
            if (pvLogBuff - pBuffTmp > iBuffLen) {                      /*  �ڴ治��                    */
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
** ��������: udialLogInsert
** ��������: ���һ�� udial ��־����
** �䡡��  : pcNetifName    ������
**           pcEventInfo    ����ʱ����Ϣָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  udialLogInsert (PCHAR  pcNetifName, PCHAR  pcEventInfo)
{
    CHAR               cTbInsertCmd[500]  = {0};
    time_t             iTime;
    CHAR               cTImeString[30]    = {0};                        /*  ����Ҫ���� 26 ���ֽ�        */
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
** ��������: udialLogDel
** ��������: ���������־
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: udialTableCreate
** ��������: ��ʼ�� udial ��־���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
