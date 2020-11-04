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
** ��   ��   ��: vpnLog.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 24 ��
**
** ��        ��: VPN ��־ʵ��
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include <time.h>
#include "liblog_config.h"
#include "log_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  VPN ��־���ݿ�������Ϣ
*********************************************************************************************************/
#define VPN_LOG_TABLE_HEAD         "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "TIMES       TEXT,"                               \
                                   "TIMEI       BIGINT,"                             \
                                   "VNDID       INT,"                                \
                                   "LOG         TEXT)"                  /*   VPN ����ʽ               */
/*********************************************************************************************************
** ��������: __vpnLogNumGet
** ��������: ��ȡ VPN ��־��Ŀ�������
** �䡡��  : pVpnLogHandle      ��ȡ��Ϣ�ṹ
** �䡡��  : NONE
** ������  : iItermNum          ���ظ���
*********************************************************************************************************/
INT32  vpnLogNumGet (__PVPN_LOG_HANDLE    pVpnLogHandle)
{
    CHAR                 cOthSpecBuff[30] = {0};
    INT32                iItermNum        = 0;

    if (LW_NULL == pVpnLogHandle) {
        return  (0);
    }

    if (pVpnLogHandle->iVndID < 0) {                                    /*  û��������������            */
        iItermNum = logGetSpecInfoCount(LOG_TYPE_VPN,
                                        pVpnLogHandle->logStartTm,
                                        pVpnLogHandle->logEndTm,
                                        LW_NULL);
    } else {
        snprintf(cOthSpecBuff, sizeof(cOthSpecBuff), "VNDID = %d", pVpnLogHandle->iVndID);
        iItermNum = logGetSpecInfoCount(LOG_TYPE_VPN,
                                        pVpnLogHandle->logStartTm,
                                        pVpnLogHandle->logEndTm,
                                        cOthSpecBuff);
    }

    if (iItermNum <= 0) {
        return  (0);
    }

    return  (iItermNum);
}
/*********************************************************************************************************
** ��������: vpnLogGetSpecInfo
** ��������: ��ȡ VPN ָ����־��Ϣ
** �䡡��  : logStartTm         ��ʼʱ��
**           logEndTm           ����ʱ��
**           llStartIterm       ��ʼ���
**           llEndIndex         �������
** �䡡��  : pcTimeBuf          ʱ���ַ���
** ������  : ERROR_CODE
*********************************************************************************************************/
INT32  vpnLogGetSpecInfo (time_t      logStartTm,
                          time_t      logEndTm,
                          INT32       iVndID,
                          INT64       llStartIndex,
                          INT64       llEndIndex,
                          PVOID       pvLogBuff,
                          INT32       iBuffLen)
{
    sqlite3            *pDb              = LW_NULL;
    CHAR              **cDbResult        = LW_NULL;
    CHAR               *pcErrMsg         = LW_NULL;
    CHAR                cSelectSql[200]  = {0};
    INT32               iRet;
    INT32               IRow, IColumn;
    INT32               i;

    __PKIDVPN_LOG_INFO  pVpnLogNode      = LW_NULL;
    PVOID               pBuffTmp         = pvLogBuff;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb ||
        LW_NULL == pvLogBuff ||
        logStartTm > logEndTm) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    if (iVndID > -1) {
        snprintf(cSelectSql,
                 sizeof(cSelectSql),
                 "SELECT *FROM %s WHERE TIMEI >= %lld AND TIMEI <= %lld AND VNDID = %d order by ID desc;",
                 VPN_LOG_TABLE,
                 logStartTm,
                 logEndTm,
                 iVndID);
    } else {
        snprintf(cSelectSql,
                 sizeof(cSelectSql),
                 "SELECT *FROM %s WHERE TIMEI >= %lld AND TIMEI <= %lld order by ID desc;",
                 VPN_LOG_TABLE,
                 logStartTm,
                 logEndTm);
    }

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            if (i < llStartIndex) {
                continue;
            }

            if (i > llEndIndex) {
                break;
            }

            pVpnLogNode = (__PKIDVPN_LOG_INFO)pvLogBuff;
            pVpnLogNode->llTime  = atoi(cDbResult[i * IColumn + 2]);
            pVpnLogNode->iVndID  = atoi(cDbResult[i * IColumn + 3]);

            strncpy(pVpnLogNode->cLogInfo, cDbResult[i * IColumn + 4], sizeof(pVpnLogNode->cLogInfo));

            pvLogBuff = (PCHAR)pvLogBuff + sizeof(__KIDVPN_LOG_INFO);
            if (pvLogBuff - pBuffTmp > iBuffLen) {                      /*  �ڴ治��                    */
                break;
            }
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get log data base vpn info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: vpnLogInsert
** ��������: ���һ�� vpn ��־����
** �䡡��  : iVndID             VND id
**           cpcLogInfo         ������־��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vpnLogInsert (INT32  iVndID, CPCHAR  cpcLogInfo)
{
    CHAR               cTbInsertCmd[LOG_INFO_MAX_LEN]  = {0};
    time_t             iTime;
    CHAR               cTImeString[30]    = {0};                        /*  ����Ҫ���� 26 ���ֽ�        */

    sqlite3            *pDb               = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == cpcLogInfo)) {
        return;
    }

    iTime = logTimeGen(cTImeString);

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (TIMES, TIMEI, VNDID, LOG) "
             "VALUES ('%s', %lld, %d, '%s');",
             VPN_LOG_TABLE,
             cTImeString,
             iTime,
             iVndID,
             cpcLogInfo);

    dataBaseCmdExec(pDb, cTbInsertCmd);

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "DELETE FROM %s WHERE %s NOT IN (SELECT %s FROM %s ORDER BY %s DESC LIMIT %d);",
             VPN_LOG_TABLE, "ID", "ID", VPN_LOG_TABLE, "ID", MAX_RECORD_NUM);

    dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: vpnLogDel
** ��������: ��� VPN ��־
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  vpnLogDel (VOID)
{
    CHAR               cTbCmd[64] = {0};
    sqlite3            *pDb       = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb)  {
        return  (PX_ERROR);
    }

    snprintf(cTbCmd, sizeof(cTbCmd), "DELETE FROM %s;", VPN_LOG_TABLE);

    return  dataBaseCmdExec(pDb, cTbCmd);
}
/*********************************************************************************************************
** ��������: vpnLogTableCreate
** ��������: ��ʼ�� vpn ��־���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  vpnLogTableCreate (VOID)
{
    sqlite3      *pDb         = LW_NULL;;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, VPN_LOG_TABLE, VPN_LOG_TABLE_HEAD);
}
