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
** 文   件   名: dhcpserverExec.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 19 日
**
** 描        述: DHCP Server操作
** 2018.09.20    DHCP SERVER 支持在多网口设备运行多个服务器
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <spawn.h>
#include <wait.h>
#include "dhcpserver_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "iniparser/iniparser.h"
/*********************************************************************************************************
** 函数名称: dhcpServerProcessStart
** 功能描述: dhcp server 进程启动
** 输　入  : cpcName        dhcpserver 名称
**           ppcCmdArg      命令行参数
** 输　出  : pPid           进程 id 指针
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerProcessStart (CPCHAR  cpcName, CHAR  **ppcCmdArg, pid_t  *pPid)
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
** 函数名称: __dhcpServerProcessKill
** 功能描述: 终止 dhcp server 进程
** 输　入  : pid       进程 id
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpServerProcessKill (pid_t  pid)
{
    INT                   iRet;
    CHAR                  cKillCmd[32] = {0};
    INT                   iStatLoc     = PX_ERROR;

    snprintf(cKillCmd, sizeof(cKillCmd), "kill %d", pid);
    iRet = API_TShellExec(cKillCmd);
    if (PX_ERROR == iRet) {
        fprintf(stderr, "kill %s error\n", "dhcpserver");

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
** 函数名称: __dhcpServerUpdateFile
** 功能描述: 将dictionary结构内容更新到配置文件
** 输　入  : dictionary         参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerUpdateFile (dictionary*  pIni)
{
    FILE  *pFp = LW_NULL;

    if (LW_NULL == pIni) {
        return;
    }

    /*
     * 以截断的方式打开配置文件，清除之前的配置
     */
    pFp = fopen(DHCPSERVER_INI_FILE, "w");
    if (LW_NULL == pFp) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dhcp server config file fopen error.\n");
    } else {

        /*
         * 以组为单位将数据写入文件
         */
        iniparser_dumpsection_ini(pIni, "ipaddrpool", pFp);
        iniparser_dumpsection_ini(pIni, "filepath", pFp);
        iniparser_dumpsection_ini(pIni, "network", pFp);
        iniparser_dumpsection_ini(pIni, "opt", pFp);

        fclose(pFp);
    }
}
/*********************************************************************************************************
** 函数名称: __dhcpServerConfigFile
** 功能描述: 更新DHCP Server配置文件
** 输　入  : pDhcpServerInfo    DHCP Server参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID __dhcpServerConfigFile (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    dictionary      *pIni       = LW_NULL;
    CHAR             cLease[16] = {0};
    CHAR             cT1[16]    = {0};
    CHAR             cT2[16]    = {0};

    if (LW_NULL == pDhcpServerInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dhcp server: config file error\n");
        return;
    }

    sprintf(cLease, "%d", pDhcpServerInfo->uiLease);
    sprintf(cT1, "%d", pDhcpServerInfo->uiLease /2);
    sprintf(cT2, "%d", pDhcpServerInfo->uiLease * 7 / 8);

    /*
     * 加载配置文件
     */
    pIni = iniparser_load(DHCPSERVER_INI_FILE);
    if (LW_NULL == pIni) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dhcp server: open config error\n");

        return;
    }

    /*
     * 更新配置文件内容
     */
    iniparser_set(pIni, DHCPSERVER_CMD_START,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpStart));
    iniparser_set(pIni, DHCPSERVER_CMD_END,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpEnd));
    iniparser_set(pIni, DHCPSERVER_CMD_LEASEFILE, DHCPSERVER_DEF_LEASEFILE);
    iniparser_set(pIni, DHCPSERVER_CMD_LOGFILE, DHCPSERVER_DEF_LOGFILE);
    iniparser_set(pIni, DHCPSERVER_CMD_INTERFACE, pDhcpServerInfo->cInterface);
    iniparser_set(pIni, DHCPSERVER_CMD_DNS1,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns1));
    iniparser_set(pIni, DHCPSERVER_CMD_DNS2,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns2));
    iniparser_set(pIni, DHCPSERVER_CMD_MASK,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpMask));
    iniparser_set(pIni, DHCPSERVER_CMD_GATEWAY,
                        inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpGateway));
    iniparser_set(pIni, DHCPSERVER_CMD_LEASE, cLease);                  /* 配置文件时间单位为100s       */

    iniparser_set(pIni, DHCPSERVER_CMD_T1, cT1);                        /* t1 默认为lease的50%          */

    iniparser_set(pIni, DHCPSERVER_CMD_T2, cT2);                        /* t2 默认为lease的87.5%        */

    __dhcpServerUpdateFile(pIni);                                       /* 更新配置文件到文件中         */

    iniparser_freedict(pIni);                                           /* 释放 dictionary 空间         */
}
/*********************************************************************************************************
** 函数名称: dhcpServerDoConfig
** 功能描述: 获取 dhcp server 信息
** 输　入  : NONE
** 输　出  : piNum     dhcpServer 信息数目
** 返　回  : 成功返回 dhcp Server 信息，失败返回 NULL
*********************************************************************************************************/
PVOID  dhcpServerDoShow (INT  *piNum)
{
    return  dhcpServerGetFromDB(piNum);
}
/*********************************************************************************************************
** 函数名称: dhcpServerDoConfig
** 功能描述: 配置dhcp
** 输　入  : pDhcpServerInfo    DHCP Server参数信息
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT  dhcpServerDoConfig (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    INT     iRet;
    pid_t   pid  = INVALID_PID;
    CHAR   *cCmdArg[4];

    cCmdArg[0] = DHCPSERVER_PATH;
    cCmdArg[1] = "-c";
    cCmdArg[2] = DHCPSERVER_INI_FILE;
    cCmdArg[3] = LW_NULL;

    if (LW_NULL == pDhcpServerInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DHCP Server data is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  若已启动 dhcp server 服务器，则杀死该服务器进程
     */
    dhcpServerPidGetFromDB(pDhcpServerInfo->cInterface, &pid);
    if (pid > 0) {
        __dhcpServerProcessKill(pid);
    }

    if (DHCPSERVER_ENABLE == pDhcpServerInfo->iEnable) {                /*  需启用 dhcp server 服务器   */
        __dhcpServerConfigFile(pDhcpServerInfo);                        /*  更新 dhcp server 配置文件   */

        /*
         *  启动 DHCP server
         */
        iRet = dhcpServerProcessStart(DHCPSERVER_PATH, cCmdArg, &pid);
        if (ERROR_NONE != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DHCP Server start failed.\n");

            return (PX_ERROR);
        }

        dhcpServerAddItemToDb(pDhcpServerInfo);                         /*  插入或更新 dhcp server 信息 */
        dhcpServerUpdatePidToDb(pDhcpServerInfo->cInterface, pid);
    } else if (DHCPSERVER_DISABLE == pDhcpServerInfo->iEnable) {

        /*
         *  插入或更新数据库的 dhcp server 服务器信息
         */
        dhcpServerAddItemToDb(pDhcpServerInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "argument is invalid.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dhcpServerDoDel
** 功能描述: 删除 dhcp server
** 输　入  : pDhcpServerInfo    DHCP Server参数信息
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT  dhcpServerDoDel (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    INT     iRet = ERROR_NONE;
    pid_t   pid  = INVALID_PID;

    if (LW_NULL == pDhcpServerInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DHCP Server data is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  若已启动 dhcp server 服务器，则杀死该服务器进程
     */
    iRet = dhcpServerPidGetFromDB(pDhcpServerInfo->cInterface, &pid);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DHCP Server data is null.\n");

       return  (PX_ERROR);
    }

    if (pid > 0) {
        __dhcpServerProcessKill(pid);
    }

    return  dhcpServerDelItemFromDb(pDhcpServerInfo->cInterface);
}
/*********************************************************************************************************
** 函数名称: dhcpServerRecovery
** 功能描述: dhcp server 恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT dhcpServerRecovery (VOID)
{
    __PDHCPSERVER_HANDLE  pDhcpServerHeader = LW_NULL;
    __PDHCPSERVER_HANDLE  pDhcpServerTmp    = LW_NULL;
    CHAR                 *cCmdArg[4];
    pid_t                 pid;
    INT                   iNum = 0;
    INT                   iRet;
    INT                   i;

    pDhcpServerHeader = dhcpServerGetFromDB(&iNum);
    if (LW_NULL == pDhcpServerHeader) {
        return  (ERROR_NONE);
    }

    cCmdArg[0] = DHCPSERVER_PATH;
    cCmdArg[1] = "-c";
    cCmdArg[2] = DHCPSERVER_INI_FILE;
    cCmdArg[3] = LW_NULL;

    for (i = 0; i < iNum; i++) {
        pDhcpServerTmp = pDhcpServerHeader + i;

        if (DHCPSERVER_ENABLE == pDhcpServerTmp->iEnable) {

            /*
             *  1、修改 dhcp server 配置文件
             *  2、启动 dhcp server 进程
             *  3、更新数据库中 dhcp server 的进程 pid 号
             */
            __dhcpServerConfigFile(pDhcpServerTmp);
            iRet = dhcpServerProcessStart(DHCPSERVER_PATH, cCmdArg, &pid);
            if (ERROR_NONE == iRet) {
                dhcpServerUpdatePidToDb(pDhcpServerTmp->cInterface, pid);

                /*
                 *   加延时，防止同一应用多次之间加载过快
                 */
                usleep(20000);
            } else {
                dhcpServerUpdatePidToDb(pDhcpServerTmp->cInterface, -1);
            }
        }
    }

    free(pDhcpServerHeader);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
