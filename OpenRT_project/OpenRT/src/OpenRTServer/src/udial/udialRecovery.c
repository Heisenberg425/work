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
** ��   ��   ��: udailRecovery.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 19 ��
**
** ��        ��: ����״̬�ָ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <spawn.h>
#include "common.h"
#include "udialExec.h"
#include "udialDB.h"
#include <net/lwip/event/lwip_netevent.h>
#include "network/lwip/netif.h"
#include "libudial_config.h"
#include "database/database_common.h"
/*********************************************************************************************************
  �������ݿ�������Ϣ
*********************************************************************************************************/
#define UDIAL_TABLE                "udial"                              /*  ���ű���                  */
#define UDIAL_TABLE_HEAD           "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "USERNAME    TEXT,"                               \
                                   "PASSWORD    TEXT,"                               \
                                   "NETIF       TEXT,"                               \
                                   "PPPNAME     TEXT,"                               \
                                   "PID         INT,"                                \
                                   "STATUS      TEXT)"                  /*  ���ű���ʽ                */
/*********************************************************************************************************
** ��������: udialRecovery
** ��������: �ָ�����ǰ�Ĳ���״̬
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  udialRecovery (VOID)
{
    CHAR                  cSelectSql[200]  = {0};
    sqlite3               *pDb             = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRet;
    CHAR                **cDbResult;
    INT                   IRow, IColumn;

    INT                   i;

    __UDIAL_HANDLE        udialHandleInfo;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", UDIAL_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            bzero(&udialHandleInfo, sizeof(udialHandleInfo));
            strncpy(udialHandleInfo.cUsrname,
                    cDbResult[i * IColumn + 1],
                    sizeof(udialHandleInfo.cUsrname));
            strncpy(udialHandleInfo.cPassword,
                    cDbResult[i * IColumn + 2],
                    sizeof(udialHandleInfo.cPassword));
            strncpy(udialHandleInfo.cNetifname,
                    cDbResult[i * IColumn + 3],
                    sizeof(udialHandleInfo.cNetifname));

            /*
             *  ֮ǰ�Ѿ����ӳɹ��Ľ��лָ�
             */
            if (NET_EVENT_PPP_RUN == atoi(cDbResult[i * IColumn + 6])) {
                iRet = udialStart(&udialHandleInfo);                    /*  ���в��Ų���                */
            } else {
                /*
                 *  ����������Щ���λָ�ǰû�гɹ����ŵ����ڣ������в��Żָ�
                 *  ���������ｫ���ǵ� pid ��PP ���ڡ� ״̬���ƻָ�ΪĬ��ֵ��
                 */
                udialHandleInfo.uiPid        = 0;
                udialHandleInfo.iUdialStatus = NET_EVENT_PPP_DEAD;
                udialUpdateToDb(&udialHandleInfo);
            }
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base lan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
