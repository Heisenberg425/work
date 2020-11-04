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
** ��   ��   ��: dhcpserverExec.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 19 ��
**
** ��        ��: DHCP Server����
** 2018.09.20    DHCP SERVER ֧���ڶ������豸���ж��������
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
** ��������: dhcpServerProcessStart
** ��������: dhcp server ��������
** �䡡��  : cpcName        dhcpserver ����
**           ppcCmdArg      �����в���
** �䡡��  : pPid           ���� id ָ��
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerProcessStart (CPCHAR  cpcName, CHAR  **ppcCmdArg, pid_t  *pPid)
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
** ��������: __dhcpServerProcessKill
** ��������: ��ֹ dhcp server ����
** �䡡��  : pid       ���� id
** �䡡��  : NONE
** ������  : ERROR_CODE
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
     *  �˴���ʱ����Ҫ�ӣ���Ȼ kill ���̻��������
     */
    usleep(200000);

    waitpid(pid, &iStatLoc, 0);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __dhcpServerUpdateFile
** ��������: ��dictionary�ṹ���ݸ��µ������ļ�
** �䡡��  : dictionary         ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerUpdateFile (dictionary*  pIni)
{
    FILE  *pFp = LW_NULL;

    if (LW_NULL == pIni) {
        return;
    }

    /*
     * �Խضϵķ�ʽ�������ļ������֮ǰ������
     */
    pFp = fopen(DHCPSERVER_INI_FILE, "w");
    if (LW_NULL == pFp) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dhcp server config file fopen error.\n");
    } else {

        /*
         * ����Ϊ��λ������д���ļ�
         */
        iniparser_dumpsection_ini(pIni, "ipaddrpool", pFp);
        iniparser_dumpsection_ini(pIni, "filepath", pFp);
        iniparser_dumpsection_ini(pIni, "network", pFp);
        iniparser_dumpsection_ini(pIni, "opt", pFp);

        fclose(pFp);
    }
}
/*********************************************************************************************************
** ��������: __dhcpServerConfigFile
** ��������: ����DHCP Server�����ļ�
** �䡡��  : pDhcpServerInfo    DHCP Server������Ϣ
** �䡡��  : NONE
** ������  : NONE
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
     * ���������ļ�
     */
    pIni = iniparser_load(DHCPSERVER_INI_FILE);
    if (LW_NULL == pIni) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dhcp server: open config error\n");

        return;
    }

    /*
     * ���������ļ�����
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
    iniparser_set(pIni, DHCPSERVER_CMD_LEASE, cLease);                  /* �����ļ�ʱ�䵥λΪ100s       */

    iniparser_set(pIni, DHCPSERVER_CMD_T1, cT1);                        /* t1 Ĭ��Ϊlease��50%          */

    iniparser_set(pIni, DHCPSERVER_CMD_T2, cT2);                        /* t2 Ĭ��Ϊlease��87.5%        */

    __dhcpServerUpdateFile(pIni);                                       /* ���������ļ����ļ���         */

    iniparser_freedict(pIni);                                           /* �ͷ� dictionary �ռ�         */
}
/*********************************************************************************************************
** ��������: dhcpServerDoConfig
** ��������: ��ȡ dhcp server ��Ϣ
** �䡡��  : NONE
** �䡡��  : piNum     dhcpServer ��Ϣ��Ŀ
** ������  : �ɹ����� dhcp Server ��Ϣ��ʧ�ܷ��� NULL
*********************************************************************************************************/
PVOID  dhcpServerDoShow (INT  *piNum)
{
    return  dhcpServerGetFromDB(piNum);
}
/*********************************************************************************************************
** ��������: dhcpServerDoConfig
** ��������: ����dhcp
** �䡡��  : pDhcpServerInfo    DHCP Server������Ϣ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
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
     *  �������� dhcp server ����������ɱ���÷���������
     */
    dhcpServerPidGetFromDB(pDhcpServerInfo->cInterface, &pid);
    if (pid > 0) {
        __dhcpServerProcessKill(pid);
    }

    if (DHCPSERVER_ENABLE == pDhcpServerInfo->iEnable) {                /*  ������ dhcp server ������   */
        __dhcpServerConfigFile(pDhcpServerInfo);                        /*  ���� dhcp server �����ļ�   */

        /*
         *  ���� DHCP server
         */
        iRet = dhcpServerProcessStart(DHCPSERVER_PATH, cCmdArg, &pid);
        if (ERROR_NONE != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "DHCP Server start failed.\n");

            return (PX_ERROR);
        }

        dhcpServerAddItemToDb(pDhcpServerInfo);                         /*  �������� dhcp server ��Ϣ */
        dhcpServerUpdatePidToDb(pDhcpServerInfo->cInterface, pid);
    } else if (DHCPSERVER_DISABLE == pDhcpServerInfo->iEnable) {

        /*
         *  �����������ݿ�� dhcp server ��������Ϣ
         */
        dhcpServerAddItemToDb(pDhcpServerInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "argument is invalid.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: dhcpServerDoDel
** ��������: ɾ�� dhcp server
** �䡡��  : pDhcpServerInfo    DHCP Server������Ϣ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
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
     *  �������� dhcp server ����������ɱ���÷���������
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
** ��������: dhcpServerRecovery
** ��������: dhcp server �ָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
             *  1���޸� dhcp server �����ļ�
             *  2������ dhcp server ����
             *  3���������ݿ��� dhcp server �Ľ��� pid ��
             */
            __dhcpServerConfigFile(pDhcpServerTmp);
            iRet = dhcpServerProcessStart(DHCPSERVER_PATH, cCmdArg, &pid);
            if (ERROR_NONE == iRet) {
                dhcpServerUpdatePidToDb(pDhcpServerTmp->cInterface, pid);

                /*
                 *   ����ʱ����ֹͬһӦ�ö��֮����ع���
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
