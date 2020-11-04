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
** ��   ��   ��: quaggaExec.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 9 �� 4 ��
**
** ��        ��: quagga ����
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <wait.h>
#include "common.h"
#include "quagga_common.h"
/*********************************************************************************************************
  quagga ·����Ϣ
*********************************************************************************************************/
static CHAR  _G_cQuaggaProtoName[QUAGGA_PROTO_NUM][8] = {"rip",   \
                                                         "ripng", \
                                                         "pim",   \
                                                         "ospf",  \
                                                         "isis",  \
                                                         "bgp",   \
                                                         "zebra"};
/*********************************************************************************************************
** ��������: quaggaProcessStart
** ��������: quagga ��������
** �䡡��  : cpcName        ��̬·��Э����
**           ppcCmdArg      �����в���
** �䡡��  : pPid           ���� id ָ��
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaProcessStart (CPCHAR  cpcName, CHAR  **ppcCmdArg, pid_t  *pPid)
{
    struct sched_param          schedParam;
    posix_spawnattr_t           spawnAttr;

    if ((NULL == cpcName) || (NULL == pPid)) {
        return  (PX_ERROR);
    }

    /*
     * ��ʼ���������Զ���
     */
    if (posix_spawnattr_init(&spawnAttr) != 0) {
        fprintf(stderr, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * �½��������ȼ�Ϊ NORMAL
     */
    schedParam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnAttr, &schedParam);
    posix_spawnattr_setflags(&spawnAttr, POSIX_SPAWN_SETSCHEDPARAM);


    /*
     *  ��������
     */
    if (posix_spawnp(pPid, cpcName, NULL, &spawnAttr, ppcCmdArg, NULL) != 0) {
        posix_spawnattr_destroy(&spawnAttr);

        perror("posix_spawnp process failed.\n");

        return  (PX_ERROR);
    }

    posix_spawnattr_destroy(&spawnAttr);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __quaggaProcessKill
** ��������: ��ֹ quagga ����
** �䡡��  : cpcName   ������
**           pid       ���� id
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __quaggaProcessKill (CPCHAR  cpcName, pid_t  pid)
{
    INT                   iRet;
    CHAR                  cKillCmd[32] = {0};
    INT                   iStatLoc     = PX_ERROR;

    if (NULL == cpcName) {
        return  (PX_ERROR);
    }

    snprintf(cKillCmd, sizeof(cKillCmd), "kill %d", pid);
    iRet = API_TShellExec(cKillCmd);
    if (PX_ERROR == iRet) {
        fprintf(stderr, "kill %s error\n", cpcName);

        return  (PX_ERROR);
    }

    /*
     *  �˴���ʱ����Ҫ�ӣ���Ȼ kill ���̻��������
     */
    usleep(200000);

    waitpid(pid, &iStatLoc, 0);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: quaggaProcDestroy
** ��������: ���� quagga ��ؽ���
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
VOID  quaggaProcDestroy (VOID)
{
    pid_t  pid = -1;
    INT    i   = 0;

    for (i = 0; i < QUAGGA_PROTO_NUM; i++) {
        quaggaPidGetFromDB(_G_cQuaggaProtoName[i], &pid);
        if (pid > 0) {
            __quaggaProcessKill(_G_cQuaggaProtoName[i], pid);
            quaggaUpdatePidToDb(_G_cQuaggaProtoName[i], pid);
        }
    }
}
/*********************************************************************************************************
** ��������: quaggaRecovery
** ��������: �ָ�����ǰ�� quagga ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaRecovery (VOID)
{
    CHAR                  cSelectSql[200]  = {0};
    INT                   iRet;
    CHAR                **cDbResult;
    INT                   IRow, IColumn;
    sqlite3              *pDb              = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;

    INT                   i;
    INT                   iStat;
    pid_t                 pid;
    __QUAGGA_HANDLE       quaggaInfo;
    CHAR                  cProcName[32]  = {0};
    CHAR                 *ppcCmdArg[2];

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iStat = quaggaStatusGetFromDB("zebra");
    if (QUAGGA_DISABLE == iStat) {
        return  (ERROR_NONE);
    }

    /*
     *  ���Ȼָ����� zebra ���̣������� pid
     */
    ppcCmdArg[0] = "zebra";
    ppcCmdArg[1] = LW_NULL;

    quaggaProcessStart("zebra", ppcCmdArg, &pid);
    quaggaUpdatePidToDb("zebra", pid);

    sleep(1);

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    sprintf(cSelectSql, "SELECT * FROM %s;", QUAGGA_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            bzero(&quaggaInfo, sizeof(quaggaInfo));
            bzero(cProcName, sizeof(cProcName));

            strncpy(quaggaInfo.cName, cDbResult[i * IColumn + 0], sizeof(quaggaInfo.cName));
            quaggaInfo.iStatus = atoi(cDbResult[i * IColumn + 1]);

            if (QUAGGA_DISABLE == quaggaInfo.iStatus) {
                continue;
            }

            if (!strcmp(quaggaInfo.cName, "zebra") ||
                 strlen(quaggaInfo.cName) >= (sizeof(cProcName) - 2)) { /*  ��ֹ����Խ��                */
                continue;
            }

            /*
             *  ���ݶ�̬·��Э������ȡ quagga ������
             */
            snprintf(cProcName, sizeof(cProcName), "%s", quaggaInfo.cName);
            cProcName[strlen(cProcName)] = 'd';

            ppcCmdArg[0] = cProcName;
            ppcCmdArg[1] = LW_NULL;

            /*
             *  ���� quagga ��̬·�ɽ��̣������� pid ��Ϣ�����ݿ�
             */
            quaggaProcessStart(cProcName, ppcCmdArg, &pid);
            quaggaUpdatePidToDb(quaggaInfo.cName, pid);
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
