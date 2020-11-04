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
** ��   ��   ��: udailExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 19 ��
**
** ��        ��: ���Ų���
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <spawn.h>
#include <wait.h>
#include "udialDB.h"
#include "log/log_common.h"
#include "intadaption/intada_common.h"
#include "common.h"
#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"
#include "network/lwip/netif.h"
#include "nat/natexec.h"
#include "net/lwip/lwip_iphook.h"
#include <net/lwip/tools/ppp/lwip_ppp.h>
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  ����ini�����ļ���غ�
*********************************************************************************************************/
#define UDIAL_INI_FILE      "/etc/ppp/ppp.ini"                          /*  ����ini�����ļ�����         */
#define UDIAL_FILE_MODE     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)     /*  ����ini�����ļ�����Ȩ��     */
#define UDIAL_INI_SECTION   "pppoe_"                                    /*  ����ini�����ļ�sectionǰ׺  */
/*********************************************************************************************************
  event �¼���
*********************************************************************************************************/
#define UDIAL_RESP_SIZE     1024                                        /*  event ����                  */
/*********************************************************************************************************
  ini�����ļ���section��Ӧ��key������udial.cԴ�룩
*********************************************************************************************************/
#define UKEY_IFNAME         "netif"
#define UKEY_MODEM          "modem"
#define UKEY_BAUD           "baud"
#define UKEY_INIT1          "init1"
#define UKEY_INIT2          "init2"
#define UKEY_INIT3          "init3"
#define UKEY_INIT4          "init4"
#define UKEY_INIT5          "init5"
#define UKEY_AREA_CODE      "area code"
#define UKEY_PHONE          "phone"
#define UKEY_USRNAME        "username"
#define UKEY_PASSWORD       "password"
#define UKEY_DIAL_CMD       "dial command"
#define UKEY_RESP_WAIT      "resp wait"
#define UKEY_DEFAULT_IF     "default interface"
#define UKEY_AUTO_RECONN    "auto reconnect"
/*********************************************************************************************************
 ppp netevent (������ udial ����)
*********************************************************************************************************/
typedef struct {
    int    evt_num;
    char   if_name[IF_NAMESIZE];
} ppp_evt_t;
/*********************************************************************************************************
  ȫ�ֱ��� _G_iNetifMaxNum û��������
*********************************************************************************************************/
static INT        _G_iNetifMaxNum;                                      /*  ��� netif num ֵ           */
static INT        _G_iUdialPppNum;                                      /*  ���Ŵ���                    */
static LW_HANDLE  _G_hUdialMutex;                                       /*  ���������ݽṹ�����ź���    */
/*********************************************************************************************************
** ��������: __udialIniFileHandle
** ��������: �� pppoe �� ini �����ļ����д���
** �䡡��  : pUdialInfo         ������Ϣ
**           iHandle            ����ʽ(1 : ���    0 �� ɾ��)
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static INT  __udialIniFileHandle (__PUDIAL_HANDLE   pUdialInfo, INT  iHandle)
{
    CHAR   cNewSecton[50]     = {0};
    CHAR  *pcIniNetifName     = LW_NULL;

    CHAR   cIniNetifBuf[50]   = {0};
    CHAR   cPassBuf[50]       = {0};
    CHAR   cUserBuf[50]       = {0};
    CHAR   cAutoBuf[50]       = {0};
    CHAR   cDefaultBuf[50]    = {0};

    INT    iSectionNum        = 0;
    INT    iSectionCnt        = 0;
    CHAR         cFlag        = 0;

    INT          iFd;
    FILE        *fppp  = LW_NULL;

    dictionary  *dict;
    INT          i;

    if (LW_NULL == pUdialInfo) {
        return  (PX_ERROR);
    }

    if(-1 == (access(UDIAL_INI_FILE, F_OK))) {                          /*  ����������ļ�              */

        if (PX_ERROR == access("/etc/ppp", F_OK)) {
            system("mkdir /etc/ppp/");                                  /*  �������ݿ�Ŀ¼              */
        }

        iFd = open(UDIAL_INI_FILE, O_RDWR | O_CREAT | O_TRUNC, UDIAL_FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }

        close(iFd);
    }

    dict = iniparser_load(UDIAL_INI_FILE);                              /*  ���������ļ�                */
    if (LW_NULL == dict) {
        printf("iniparser_load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  ��ȡsection����             */

    if (0 != iSectionNum) {                                             /*  �жϵ�ǰ�Ĳ�����Ϣ�Ƿ����  */
        for (i = 0; i < iSectionNum; i++) {
            sprintf(cIniNetifBuf, "%s:%s", iniparser_getsecname(dict, i), UKEY_IFNAME);

            pcIniNetifName = (CHAR *)iniparser_getstring(dict, cIniNetifBuf, "NULL");
            if (0 == strcmp(pcIniNetifName, pUdialInfo->cNetifname)) {  /*  �Ѿ����ڲ�����Ϣ            */
                iSectionCnt = i;
                cFlag = 1;                                              /*  �ж�Ӧ��Ϣ                  */
                break;
            }
        }
    }

    if (1 == iHandle) {                                                 /*  ��Ҫ��Ӳ�����Ϣ            */
        if (0 == cFlag) {                                               /*  ֮ǰû�ж�Ӧ�Ĳ�����Ϣ      */
            iniparser_freedict(dict);                                   /*  ��֮ǰ��dictionary��ɾ��    */

            fppp = fopen(UDIAL_INI_FILE, "r+");                         /*  r+ ��ʽ�������ļ�         */
            if (LW_NULL == fppp) {
                printf("Create file failed.\n");

                return  (PX_ERROR);
            }

            fseek(fppp, 0L, SEEK_END);                                  /*  �ƶ����ļ�ĩβ              */

            /*
             *  ���һ���µ�section
             */
            snprintf(cNewSecton, sizeof(cNewSecton), "%s%s", UDIAL_INI_SECTION, pUdialInfo->cNetifname);
            fprintf(fppp, "\n[%s]\n", cNewSecton);                      /*  ���ļ���д���� section      */

            fclose (fppp);

            dict = iniparser_load(UDIAL_INI_FILE);                      /*  ���»�ȡdictionary          */
            if (LW_NULL == dict) {
                printf("iniparser_load file error\n");

                return  (PX_ERROR);
            }

            iSectionCnt = iSectionNum;                                  /*  �������µ�section��λ��     */
        }

        /*
         *  ���ˣ��ҵ���Ӧ��section�ˣ����ڲ������ڵ��˺��������Ƿ���ini�ļ��е�һ�£���ֱ�Ӹ���һ��
         */
        snprintf(cIniNetifBuf, sizeof(cIniNetifBuf), "%s:%s",
                iniparser_getsecname(dict, iSectionCnt), UKEY_IFNAME);
        snprintf(cUserBuf, sizeof(cUserBuf), "%s:%s",
                iniparser_getsecname(dict, iSectionCnt), UKEY_USRNAME);
        snprintf(cPassBuf, sizeof(cPassBuf), "%s:%s",
                iniparser_getsecname(dict, iSectionCnt), UKEY_PASSWORD);
        snprintf(cAutoBuf, sizeof(cAutoBuf), "%s:%s",
                iniparser_getsecname(dict, iSectionCnt), UKEY_AUTO_RECONN);
        snprintf(cDefaultBuf, sizeof(cDefaultBuf), "%s:%s",
                iniparser_getsecname(dict, iSectionCnt), UKEY_DEFAULT_IF);

        iniparser_set(dict, cIniNetifBuf, pUdialInfo->cNetifname);
        iniparser_set(dict, cUserBuf, pUdialInfo->cUsrname);
        iniparser_set(dict, cPassBuf, pUdialInfo->cPassword);
        iniparser_set(dict, cAutoBuf, "yes");
        iniparser_set(dict, cDefaultBuf, "yes");

        fppp = LW_NULL;
        fppp = fopen(UDIAL_INI_FILE, "w+");                             /*  ����ļ���                */
        if (LW_NULL == fppp) {
            printf("Create file failed, ini has cleared.\n");
            iniparser_freedict(dict);                                   /*  �ͷ�dictionar               */

            return  (PX_ERROR);
        }

        iniparser_dump_ini(dict, fppp);                                 /*  ��dictionarд��ini�ļ�      */

        iniparser_freedict(dict);                                       /*  �ͷ�dictionary              */

        fclose (fppp);

        return  (ERROR_NONE);
    } else {                                                            /*  ��Ҫɾ��������Ϣ            */
        if (0 == cFlag) {                                               /*  ֮ǰû�ж�Ӧ�Ĳ�����Ϣ      */
            iniparser_freedict(dict);                                   /*  �ͷ�dictionary              */
            printf("Ini file has no msg.\n");

            return  (PX_ERROR);
        } else {
            fppp = fopen(UDIAL_INI_FILE, "w+");                         /*  ����ļ���                */
            if (LW_NULL == fppp) {
                printf("Create file failed.\n");
                iniparser_freedict(dict);                               /*  �ͷ�dictionar               */

                return  (PX_ERROR);
            }

            for (i = 0; i < iSectionNum; i++) {
                if (i != iSectionCnt) {                                 /*  ����Ҫɾ����section�޳�     */
                    iniparser_dumpsection_ini(dict, iniparser_getsecname(dict, i), fppp);
                }
            }

            iniparser_freedict(dict);                                   /*  �ͷ�dictionary              */

            fclose (fppp);

            return  (ERROR_NONE);
        }
    }
}
/*********************************************************************************************************
** ��������: accountInfoGetFromIniFile
** ��������: ������������ ini �����ļ��л�ȡ�˺ż�������Ϣ
** �䡡��  : cpcNetif         ������
** �䡡��  : pcAccount        �����˺���Ϣ������
**           pcPasswd         �����˺�������Ϣ������
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  accountInfoGetFromIniFile (CPCHAR  cpcNetif, PCHAR  pcAccount, PCHAR  pcPasswd)
{
    CHAR  *pcIniNetifName     = LW_NULL;
    CHAR  *pcValue            = LW_NULL;

    CHAR   cIniNetifBuf[50]   = {0};
    CHAR   cPassBuf[20]       = {0};
    CHAR   cUserBuf[20]       = {0};

    INT    iSectionNum        = 0;
    INT    iSectionCnt        = -1;

    dictionary  *dict         = LW_NULL;
    INT          i;

    if (LW_NULL == cpcNetif  ||
        LW_NULL == pcAccount ||
        LW_NULL == pcPasswd) {
        return  (PX_ERROR);
    }

    dict = iniparser_load(UDIAL_INI_FILE);                              /*  ���������ļ�                */
    if (LW_NULL == dict) {
        printf("iniparser load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  ��ȡsection����             */

    if (0 != iSectionNum) {                                             /*  �жϵ�ǰ�Ĳ�����Ϣ�Ƿ����  */
        for (i = 0; i < iSectionNum; i++) {
            sprintf(cIniNetifBuf, "%s:%s", iniparser_getsecname(dict, i), UKEY_IFNAME);

            pcIniNetifName = (CHAR *)iniparser_getstring(dict, cIniNetifBuf, "NULL");
            if (0 == strcmp(pcIniNetifName, cpcNetif)) {                /*  �Ѿ����ڲ�����Ϣ            */
                iSectionCnt = i;

                break;
            }
        }
    }

    if (iSectionCnt >= 0) {
        sprintf(cUserBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), UKEY_USRNAME);
        pcValue = (CHAR *)iniparser_getstring(dict, cUserBuf, "NULL");
        memcpy(pcAccount, pcValue, strlen(pcValue));

        sprintf(cPassBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), UKEY_PASSWORD);
        pcValue = (CHAR *)iniparser_getstring(dict, cPassBuf, "NULL");
        memcpy(pcPasswd, pcValue, strlen(pcValue));
    } else {
        iniparser_freedict(dict);                                       /*  �ͷ�dictionary              */

        return  (PX_ERROR);
    }

    iniparser_freedict(dict);                                           /*  �ͷ�dictionary              */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: netif_get_max_index
** ��������: ��õ�ǰ�������� num
** �䡡��  : NONE
**           NONE
** �䡡��  : NONE
** ������  : ucMaxIndex           ��ǰ���ڵ�������� num
*********************************************************************************************************/
int  netif_get_max_index (void)
{
    UCHAR  ucMaxIndex                 = 0;
    UCHAR  ucIndex                    = 0;
    CHAR   cNetifName[NETIF_NAMESIZE] = {0};

    struct netif *netif;

    if(netif_list != NULL) {
        bzero(cNetifName, sizeof(cNetifName));
        netif_get_name(netif_list, cNetifName);
        ucMaxIndex = netif_name_to_index(cNetifName);
    }

    NETIF_FOREACH(netif) {
        bzero(cNetifName, sizeof(cNetifName));
        netif_get_name(netif, cNetifName);
        ucIndex = netif_name_to_index(cNetifName);

        if (ucMaxIndex < ucIndex) {
            ucMaxIndex = ucIndex;
        }
    }

    return ucMaxIndex;
}

/*********************************************************************************************************
** ��������: __udial_parse_ppp_evt (���� udial ����)
** ��������: ��ȡ PPP �¼���Ϣ
** �䡡��  : evt_buf            �¼�����
** �䡡��  : out                ��� PPP �¼���Ϣ
** ������  : 0                  �� PPP �¼�
*********************************************************************************************************/
static INT __udial_parse_ppp_evt (ppp_evt_t  *out, char *evt_buf)
{
    unsigned int    evt;
    unsigned char  *buf = (unsigned char *)evt_buf;

    /*
     * evt is make in big-endian
     */
    evt = (((unsigned int)buf[0]) << 24)
        | (((unsigned int)buf[1]) << 16)
        | (((unsigned int)buf[2]) <<  8)
        | (((unsigned int)buf[3]) <<  0);

    if (evt >= NET_EVENT_PPP_DEAD && evt <= NET_EVENT_PPP_DISCONN) {
        out->evt_num = evt;
        memcpy(out->if_name, buf + 4, IF_NAMESIZE);
        return  0;
    }

    return -1;
}

/*********************************************************************************************************
** ��������: __udialPPPoEMonitorThread
** ��������: ����״̬�����߳�
** �䡡��  : arg                sqlite ���
** �䡡��  : NONE
** ������  : none
*********************************************************************************************************/
static PVOID __udialPPPoEMonitorThread (void *arg)
{
    int               fd;
    char              evt_buf[UDIAL_RESP_SIZE];
    ssize_t           read_len;
    ppp_evt_t         evt;
    INT               ret;
    INT               iInfoGet;

    __UDIAL_HANDLE    udialInfo;

    CHAR              cLogBuf[50]  = {0};


    fd = open("/dev/netevent", O_RDONLY);
    if (fd < 0) {
        printf("open /dev/netevent failed, can't monitor the ppp link status!\r\n");
        return   (LW_NULL);
    }

    while (1) {
        read_len = read(fd, evt_buf, UDIAL_RESP_SIZE);
        if (read_len > 0) {
            ret = __udial_parse_ppp_evt(&evt, evt_buf);                 /*  �жϵ�ǰ�ǲ�����ppp����¼� */

            if (ret == 0) {                                             /*  ����ppp����¼�             */
                bzero(&udialInfo, sizeof(udialInfo));

                /*
                 *  ͨ�� ppp ���ڻ�ȡ������Ϣ
                 */
                Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

                iInfoGet = udialGetSpecInfo(&udialInfo, "PPPNAME", evt.if_name, LW_NULL);
                if (iInfoGet == 0) {                                    /*  ��ȡ��Ϣ�ɹ�                */
                    udialInfo.iUdialStatus = evt.evt_num;

                    udialUpdateToDb(&udialInfo);                        /*  ���²���״̬                */

                    Lw_SemaphoreM_Post(_G_hUdialMutex);

                    bzero(cLogBuf, sizeof(cLogBuf));
                    snprintf(cLogBuf, sizeof(cLogBuf), "pppoe link status is %d", udialInfo.iUdialStatus);
                    udialLogInsert(udialInfo.cNetifname, cLogBuf);

                    if (NET_EVENT_PPP_RUN == udialInfo.iUdialStatus) {
                        bzero(cLogBuf, sizeof(cLogBuf));
                        snprintf(cLogBuf, sizeof(cLogBuf), "tm add line %s", evt.if_name);
                        system(cLogBuf);
                        printf("natWanCfg %s!\r\n", udialInfo.cPppname);
                        natWanCfg(udialInfo.cPppname);
                    }
                } else {
                    Lw_SemaphoreM_Post(_G_hUdialMutex);

                    continue;
                }
            } else {
                continue;
            }

        }

        if (read_len < 0) {
            printf("monidor netevent error, it seems the /dev/netevent device error!\r\n");
            close(fd);
            return   (LW_NULL);
        }
    }

    return   (LW_NULL);
}

/*********************************************************************************************************
** ��������: udialPppNumUpdate
** ��������: �����´�����PPP����index(��ʱЭ��ջû�з��������ṩ������netif num��
**           ��������ṩ����ӿڣ����������ط���ɾ������ʱ������һ�θ���)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
VOID udialPppNumUpdate (VOID)
{
    _G_iNetifMaxNum = netif_get_max_index();                            /*  ��¼�µ�ǰ���� num        */
}
/*********************************************************************************************************
** ��������: netIfMaxIndexGet
** ��������: ��ȡ����������������ֵ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ��������ֵ
*********************************************************************************************************/
INT  netIfMaxIndexGet (VOID)
{
   return  (_G_iNetifMaxNum);
}
/*********************************************************************************************************
** ��������: __udialProcessExitHandle
** ��������: udial �����˳�����
** ��  ��  : NONW
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __udialProcessExitHandle (INT  iPid)
{
    INT                iRet  = PX_ERROR;
    __UDIAL_HANDLE     udialInfo;


    /*
     *  ͨ��pid�������ݿ��ж��Ƿ��� UDIAL ���̣�����ǣ������ UDIAL ����Ϣɾ������
     */
    bzero(&udialInfo, sizeof(udialInfo));

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    iRet = udialGetSpecInfo(&udialInfo, "PID", LW_NULL, &iPid);
    if (ERROR_NONE == iRet) {
        udialPppNumUpdate();
        udialInfo.uiPid        = 0;
        udialInfo.iUdialStatus = NET_EVENT_PPP_DEAD;                    /*  ˵��ʱ�Ͽ�״̬              */
        udialUpdateToDb(&udialInfo);                                    /*  ����һ�� pid ˵����ɾ��     */

        Lw_SemaphoreM_Post(_G_hUdialMutex);

        _G_iUdialPppNum--;
    } else {
        Lw_SemaphoreM_Post(_G_hUdialMutex);
    }
}

/*********************************************************************************************************
** ��������: udialStatusInit
** ��������: ����״̬���³�ʼ��
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  udialStatusInit (VOID)
{
    pthread_t             tid;
    pthread_attr_t        attr;
    INT                   iRet;

    /*
     *  �������Ŵ������߳�
     */
    iRet = pthread_attr_init(&attr);
    if (iRet != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pthread attr init failed.\n");

        return  (PX_ERROR);
    }

    pthread_attr_setname(&attr, "t_udialUpdate");
    iRet = pthread_create(&tid, &attr, __udialPPPoEMonitorThread, NULL);
    if (iRet != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pthread create failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: udialStart
** ��������: ���в��Ų���
** ��  ��  : pUdialHandleInfo  ������Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT udialStart (__PUDIAL_HANDLE    pUdialHandleInfo)
{
    CHAR                  cSectionBuf[32]          = {0};
    CHAR                 *cCmdArg[7];

    struct sched_param    schedparam;
    posix_spawnattr_t     spawnattr;
    INT                   iRet;
    INT                   iNetifIndex;
    CHAR                  cPppName[NETIF_NAMESIZE] = {0};

    pid_t                 pid                      = 0;

    if (LW_NULL == pUdialHandleInfo || !strlen(pUdialHandleInfo->cNetifname)) {
        return  (PX_ERROR);
    }

    cCmdArg[0] = "udial";
    cCmdArg[1] = "-p";
    cCmdArg[2] = "-e";
    cCmdArg[3] = "-d";
    cCmdArg[4] = "-c";
    cCmdArg[5] = cSectionBuf;
    cCmdArg[6] = NULL;

    udialLogInsert(pUdialHandleInfo->cNetifname, "start pppoe udial\n");

    if (_G_iUdialPppNum >= LW_CFG_LWIP_NUM_PPP) {                       /*  ���Ŵ�������                */
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pppoe over\n");
        udialLogInsert(pUdialHandleInfo->cNetifname, "pppoe net interface over\n");

        return  (1);
    }

    iRet = __udialIniFileHandle(pUdialHandleInfo, 1);                   /*  ���������ļ�                */
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    /*
    * ��ʼ���������Զ���
    */
    if (posix_spawnattr_init(&spawnattr) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * �½��������ȼ�Ϊ NORMAL
     */
    schedparam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnattr, &schedparam);
    posix_spawnattr_setflags(&spawnattr, POSIX_SPAWN_SETSCHEDPARAM);

    /*
     *  ���첦����Ҫʹ�õ� ini section
     */
    bzero(cSectionBuf, sizeof(cSectionBuf));
    snprintf(cSectionBuf, sizeof(cSectionBuf), "pppoe_%s", pUdialHandleInfo->cNetifname);

    if (posix_spawnp(&pid, "udial", NULL,
                     &spawnattr, cCmdArg, NULL) != 0) {                 /* ��������                     */
        posix_spawnattr_destroy(&spawnattr);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "posix_spawnp error\n");

        return  (PX_ERROR);
    }

    _G_iUdialPppNum++;

    /*
     *  �������ݿ���Ϣ
     */
    iNetifIndex = netif_get_max_index();
    if (_G_iNetifMaxNum > iNetifIndex) {
        iNetifIndex = _G_iNetifMaxNum;
    } else {
        _G_iNetifMaxNum = iNetifIndex;
    }

    snprintf(cPppName, sizeof(cPppName), "pp%d", iNetifIndex);          /*  ����һ�� ppp ��������       */
    strncpy(pUdialHandleInfo->cPppname, cPppName, sizeof(pUdialHandleInfo->cPppname));
    pUdialHandleInfo->uiPid = pid;
    pUdialHandleInfo->iUdialStatus = NET_EVENT_PPP_DEAD;                /*  Ĭ������Ϊ�Ͽ�����״̬      */

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    udialUpdateToDb(pUdialHandleInfo);

    Lw_SemaphoreM_Post(_G_hUdialMutex);

    posix_spawnattr_destroy(&spawnattr);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: udialDel
** ��������: ɾ�����Ų���
** ��  ��  : pUdialHandleInfo  ������Ϣ
** ��  ��  : NONE
** ��  ��  : iStatLoc          ɾ��״̬
*********************************************************************************************************/
INT udialDel (CPCHAR  cpcNetifName)
{
    __UDIAL_HANDLE        udialHandleInfo;
    CHAR                  cKillCmd[50];
    INT                   iStatLoc = ERROR_NONE;
    INT                   iRet;

    udialLogInsert((PCHAR)cpcNetifName, "delet pppoe net interface\n");

    /*
     *  ��ȡ pid ==> ɾ�� ==> wait
     */
    bzero(&udialHandleInfo, sizeof(udialHandleInfo));

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    udialGetSpecInfo(&udialHandleInfo, "NETIF", (PCHAR)cpcNetifName, LW_NULL);
    if (0 != udialHandleInfo.uiPid) {
        _G_iNetifMaxNum = netif_get_max_index();                        /*  ��¼�µ�ǰ���� num        */
        /*
         *  ��ɾ�� ������� ��ppp���ڵĲ���
         */
        bzero(cKillCmd, sizeof(cKillCmd));
        snprintf(cKillCmd, sizeof(cKillCmd), "tm del line %s", udialHandleInfo.cPppname);
        system(cKillCmd);

        bzero(cKillCmd, sizeof(cKillCmd));
        snprintf(cKillCmd, sizeof(cKillCmd), "kill %d", udialHandleInfo.uiPid);

        iRet = system(cKillCmd);
        if (PX_ERROR == iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "kill udial error\n");

            return  (PX_ERROR);
        }

        waitpid(udialHandleInfo.uiPid, &iStatLoc, 0);

        udialHandleInfo.uiPid        = 0;
        udialHandleInfo.iUdialStatus = NET_EVENT_PPP_DEAD;          /*  ˵��ʱ�Ͽ�״̬              */
        udialUpdateToDb(&udialHandleInfo);                          /*  ����һ�� pid ˵����ɾ��     */

        Lw_SemaphoreM_Post(_G_hUdialMutex);

        _G_iUdialPppNum--;
    } else {
        Lw_SemaphoreM_Post(_G_hUdialMutex);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: udialInit
** ��������: ���ų�ʼ��
** ��  ��  : NONW
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  udialInit (VOID)
{
    /*
     * ע��һ�������������� udial �����˳�ʱ����
     */
    INIT_CHECK(subRecliamHandleAdd(__udialProcessExitHandle));

    _G_hUdialMutex = Lw_SemaphoreM_Create("kidvpn_fd_lock",            /*  �����ļ�����������������  */
                                           LW_PRIO_HIGH,
                                           LW_OPTION_WAIT_FIFO |
                                           LW_OPTION_OBJECT_LOCAL |
                                           LW_OPTION_INHERIT_PRIORITY |
                                           LW_OPTION_ERRORCHECK,
                                           LW_NULL);

    if (LW_OBJECT_HANDLE_INVALID == _G_hUdialMutex) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "mutex initialize failed.\n");

        return  (PX_ERROR);
    }


    return  udialStatusInit();                                          /*  �������ż���߳�            */
}
