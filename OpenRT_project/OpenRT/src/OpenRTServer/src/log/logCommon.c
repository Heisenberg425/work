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
** ��   ��   ��: logCommon.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 24 ��
**
** ��        ��: ��־ͨ�ò�������
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
** ��������: logTimeGen
** ��������: ����һ�� log ��־��ʱ��
** �䡡��  : NONE
** �䡡��  : pcTimeBuf          ʱ���ַ���
** ������  : iTime              ʱ������ֵ
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
** ��������: logGetSpecInfoCount
** ��������: ��ȡ��־ָ��������Ϣ��Ŀ��
** �䡡��  : iLogTableType      ��־����
**           logStartTm         ��ʼʱ��
**           logEndTm           ����ʱ��
**           pcOtherSpec        ������������
** �䡡��  : NONE
** ������  : iRet               ��Ŀ����
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
     *  ��ȡ��־������
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
** ��������: logTableDelet
** ��������: ɾ����־���ݿ��
** �䡡��  : iLogTableType      ��־���ݿ��
** �䡡��  : NONE
** ������  : NONE
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

