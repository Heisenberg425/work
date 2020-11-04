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
** 文   件   名: quaggaExec.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 9 月 4 日
**
** 描        述: quagga 操作
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
  quagga 路由信息
*********************************************************************************************************/
static CHAR  _G_cQuaggaProtoName[QUAGGA_PROTO_NUM][8] = {"rip",   \
                                                         "ripng", \
                                                         "pim",   \
                                                         "ospf",  \
                                                         "isis",  \
                                                         "bgp",   \
                                                         "zebra"};
/*********************************************************************************************************
** 函数名称: quaggaProcessStart
** 功能描述: quagga 进程启动
** 输　入  : cpcName        动态路由协议名
**           ppcCmdArg      命令行参数
** 输　出  : pPid           进程 id 指针
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  quaggaProcessStart (CPCHAR  cpcName, CHAR  **ppcCmdArg, pid_t  *pPid)
{
    struct sched_param          schedParam;
    posix_spawnattr_t           spawnAttr;

    if ((NULL == cpcName) || (NULL == pPid)) {
        return  (PX_ERROR);
    }

    /*
     * 初始化进程属性对象
     */
    if (posix_spawnattr_init(&spawnAttr) != 0) {
        fprintf(stderr, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * 新建进程优先级为 NORMAL
     */
    schedParam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnAttr, &schedParam);
    posix_spawnattr_setflags(&spawnAttr, POSIX_SPAWN_SETSCHEDPARAM);


    /*
     *  启动进程
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
** 函数名称: __quaggaProcessKill
** 功能描述: 终止 quagga 进程
** 输　入  : cpcName   进程名
**           pid       进程 id
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  此处延时必须要加，不然 kill 进程会出现问题
     */
    usleep(200000);

    waitpid(pid, &iStatLoc, 0);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: quaggaProcDestroy
** 功能描述: 销毁 quagga 相关进程
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : NONE
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
** 函数名称: quaggaRecovery
** 功能描述: 恢复重启前的 quagga 连接
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
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
     *  首先恢复启动 zebra 进程，并更新 pid
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
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
            bzero(&quaggaInfo, sizeof(quaggaInfo));
            bzero(cProcName, sizeof(cProcName));

            strncpy(quaggaInfo.cName, cDbResult[i * IColumn + 0], sizeof(quaggaInfo.cName));
            quaggaInfo.iStatus = atoi(cDbResult[i * IColumn + 1]);

            if (QUAGGA_DISABLE == quaggaInfo.iStatus) {
                continue;
            }

            if (!strcmp(quaggaInfo.cName, "zebra") ||
                 strlen(quaggaInfo.cName) >= (sizeof(cProcName) - 2)) { /*  防止数组越界                */
                continue;
            }

            /*
             *  根据动态路由协议名获取 quagga 进程名
             */
            snprintf(cProcName, sizeof(cProcName), "%s", quaggaInfo.cName);
            cProcName[strlen(cProcName)] = 'd';

            ppcCmdArg[0] = cProcName;
            ppcCmdArg[1] = LW_NULL;

            /*
             *  启动 quagga 动态路由进程，并更新 pid 信息到数据库
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
