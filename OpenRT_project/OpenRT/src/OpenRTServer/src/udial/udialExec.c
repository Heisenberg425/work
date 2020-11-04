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
** 文   件   名: udailExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 19 日
**
** 描        述: 拨号操作
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
  拨号ini配置文件相关宏
*********************************************************************************************************/
#define UDIAL_INI_FILE      "/etc/ppp/ppp.ini"                          /*  拨号ini配置文件名称         */
#define UDIAL_FILE_MODE     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)     /*  拨号ini配置文件操作权限     */
#define UDIAL_INI_SECTION   "pppoe_"                                    /*  拨号ini配置文件section前缀  */
/*********************************************************************************************************
  event 事件宏
*********************************************************************************************************/
#define UDIAL_RESP_SIZE     1024                                        /*  event 长度                  */
/*********************************************************************************************************
  ini配置文件内section对应的key（拷贝udial.c源码）
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
 ppp netevent (拷贝自 udial 工程)
*********************************************************************************************************/
typedef struct {
    int    evt_num;
    char   if_name[IF_NAMESIZE];
} ppp_evt_t;
/*********************************************************************************************************
  全局变量 _G_iNetifMaxNum 没加锁！！
*********************************************************************************************************/
static INT        _G_iNetifMaxNum;                                      /*  最大 netif num 值           */
static INT        _G_iUdialPppNum;                                      /*  拨号次数                    */
static LW_HANDLE  _G_hUdialMutex;                                       /*  操作该数据结构互斥信号量    */
/*********************************************************************************************************
** 函数名称: __udialIniFileHandle
** 功能描述: 对 pppoe 的 ini 配置文件进行处理
** 输　入  : pUdialInfo         拨号信息
**           iHandle            处理方式(1 : 添加    0 ： 删除)
** 输　出  : NONE
** 返　回  : NONE
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

    if(-1 == (access(UDIAL_INI_FILE, F_OK))) {                          /*  不存在这个文件              */

        if (PX_ERROR == access("/etc/ppp", F_OK)) {
            system("mkdir /etc/ppp/");                                  /*  创建数据库目录              */
        }

        iFd = open(UDIAL_INI_FILE, O_RDWR | O_CREAT | O_TRUNC, UDIAL_FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }

        close(iFd);
    }

    dict = iniparser_load(UDIAL_INI_FILE);                              /*  加载配置文件                */
    if (LW_NULL == dict) {
        printf("iniparser_load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  获取section个数             */

    if (0 != iSectionNum) {                                             /*  判断当前的拨号信息是否存在  */
        for (i = 0; i < iSectionNum; i++) {
            sprintf(cIniNetifBuf, "%s:%s", iniparser_getsecname(dict, i), UKEY_IFNAME);

            pcIniNetifName = (CHAR *)iniparser_getstring(dict, cIniNetifBuf, "NULL");
            if (0 == strcmp(pcIniNetifName, pUdialInfo->cNetifname)) {  /*  已经存在拨号信息            */
                iSectionCnt = i;
                cFlag = 1;                                              /*  有对应信息                  */
                break;
            }
        }
    }

    if (1 == iHandle) {                                                 /*  需要添加拨号信息            */
        if (0 == cFlag) {                                               /*  之前没有对应的拨号信息      */
            iniparser_freedict(dict);                                   /*  把之前的dictionary先删除    */

            fppp = fopen(UDIAL_INI_FILE, "r+");                         /*  r+ 格式打开配置文件         */
            if (LW_NULL == fppp) {
                printf("Create file failed.\n");

                return  (PX_ERROR);
            }

            fseek(fppp, 0L, SEEK_END);                                  /*  移动到文件末尾              */

            /*
             *  添加一个新的section
             */
            snprintf(cNewSecton, sizeof(cNewSecton), "%s%s", UDIAL_INI_SECTION, pUdialInfo->cNetifname);
            fprintf(fppp, "\n[%s]\n", cNewSecton);                      /*  向文件中写入新 section      */

            fclose (fppp);

            dict = iniparser_load(UDIAL_INI_FILE);                      /*  重新获取dictionary          */
            if (LW_NULL == dict) {
                printf("iniparser_load file error\n");

                return  (PX_ERROR);
            }

            iSectionCnt = iSectionNum;                                  /*  设置最新的section的位置     */
        }

        /*
         *  至此，找到对应的section了，现在不管现在的账号与密码是否与ini文件中的一致，都直接更新一下
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
        fppp = fopen(UDIAL_INI_FILE, "w+");                             /*  清空文件打开                */
        if (LW_NULL == fppp) {
            printf("Create file failed, ini has cleared.\n");
            iniparser_freedict(dict);                                   /*  释放dictionar               */

            return  (PX_ERROR);
        }

        iniparser_dump_ini(dict, fppp);                                 /*  把dictionar写入ini文件      */

        iniparser_freedict(dict);                                       /*  释放dictionary              */

        fclose (fppp);

        return  (ERROR_NONE);
    } else {                                                            /*  需要删除拨号信息            */
        if (0 == cFlag) {                                               /*  之前没有对应的拨号信息      */
            iniparser_freedict(dict);                                   /*  释放dictionary              */
            printf("Ini file has no msg.\n");

            return  (PX_ERROR);
        } else {
            fppp = fopen(UDIAL_INI_FILE, "w+");                         /*  清空文件打开                */
            if (LW_NULL == fppp) {
                printf("Create file failed.\n");
                iniparser_freedict(dict);                               /*  释放dictionar               */

                return  (PX_ERROR);
            }

            for (i = 0; i < iSectionNum; i++) {
                if (i != iSectionCnt) {                                 /*  把需要删除的section剔除     */
                    iniparser_dumpsection_ini(dict, iniparser_getsecname(dict, i), fppp);
                }
            }

            iniparser_freedict(dict);                                   /*  释放dictionary              */

            fclose (fppp);

            return  (ERROR_NONE);
        }
    }
}
/*********************************************************************************************************
** 函数名称: accountInfoGetFromIniFile
** 功能描述: 根据网口名从 ini 配置文件中获取账号及密码信息
** 输　入  : cpcNetif         网口名
** 输　出  : pcAccount        保存账号信息缓冲区
**           pcPasswd         保存账号密码信息缓冲区
** 返　回  : ERROR_CODE
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

    dict = iniparser_load(UDIAL_INI_FILE);                              /*  加载配置文件                */
    if (LW_NULL == dict) {
        printf("iniparser load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  获取section个数             */

    if (0 != iSectionNum) {                                             /*  判断当前的拨号信息是否存在  */
        for (i = 0; i < iSectionNum; i++) {
            sprintf(cIniNetifBuf, "%s:%s", iniparser_getsecname(dict, i), UKEY_IFNAME);

            pcIniNetifName = (CHAR *)iniparser_getstring(dict, cIniNetifBuf, "NULL");
            if (0 == strcmp(pcIniNetifName, cpcNetif)) {                /*  已经存在拨号信息            */
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
        iniparser_freedict(dict);                                       /*  释放dictionary              */

        return  (PX_ERROR);
    }

    iniparser_freedict(dict);                                           /*  释放dictionary              */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: netif_get_max_index
** 功能描述: 获得当前最大的网卡 num
** 输　入  : NONE
**           NONE
** 输　出  : NONE
** 返　回  : ucMaxIndex           当前存在的网卡最大 num
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
** 函数名称: __udial_parse_ppp_evt (拷贝 udial 工程)
** 功能描述: 获取 PPP 事件信息
** 输　入  : evt_buf            事件内容
** 输　出  : out                输出 PPP 事件信息
** 返　回  : 0                  是 PPP 事件
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
** 函数名称: __udialPPPoEMonitorThread
** 功能描述: 拨号状态更新线程
** 输　入  : arg                sqlite 句柄
** 输　出  : NONE
** 返　回  : none
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
            ret = __udial_parse_ppp_evt(&evt, evt_buf);                 /*  判断当前是不是与ppp相关事件 */

            if (ret == 0) {                                             /*  是与ppp相关事件             */
                bzero(&udialInfo, sizeof(udialInfo));

                /*
                 *  通过 ppp 网口获取拨号信息
                 */
                Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

                iInfoGet = udialGetSpecInfo(&udialInfo, "PPPNAME", evt.if_name, LW_NULL);
                if (iInfoGet == 0) {                                    /*  获取信息成功                */
                    udialInfo.iUdialStatus = evt.evt_num;

                    udialUpdateToDb(&udialInfo);                        /*  更新拨号状态                */

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
** 函数名称: udialPppNumUpdate
** 功能描述: 更新新创建的PPP网口index(暂时协议栈没有方法可以提供接下来netif num，
**           因此这里提供这个接口，用于其他地方在删除网卡时，进行一次更新)
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
VOID udialPppNumUpdate (VOID)
{
    _G_iNetifMaxNum = netif_get_max_index();                            /*  记录下当前最大的 num        */
}
/*********************************************************************************************************
** 函数名称: netIfMaxIndexGet
** 功能描述: 获取保存的网卡索引最大值
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : 网卡索引值
*********************************************************************************************************/
INT  netIfMaxIndexGet (VOID)
{
   return  (_G_iNetifMaxNum);
}
/*********************************************************************************************************
** 函数名称: __udialProcessExitHandle
** 功能描述: udial 进程退出操作
** 输  入  : NONW
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __udialProcessExitHandle (INT  iPid)
{
    INT                iRet  = PX_ERROR;
    __UDIAL_HANDLE     udialInfo;


    /*
     *  通过pid，查数据库判断是否是 UDIAL 进程，如果是，则进行 UDIAL 的信息删除操作
     */
    bzero(&udialInfo, sizeof(udialInfo));

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    iRet = udialGetSpecInfo(&udialInfo, "PID", LW_NULL, &iPid);
    if (ERROR_NONE == iRet) {
        udialPppNumUpdate();
        udialInfo.uiPid        = 0;
        udialInfo.iUdialStatus = NET_EVENT_PPP_DEAD;                    /*  说明时断开状态              */
        udialUpdateToDb(&udialInfo);                                    /*  更新一下 pid 说明被删除     */

        Lw_SemaphoreM_Post(_G_hUdialMutex);

        _G_iUdialPppNum--;
    } else {
        Lw_SemaphoreM_Post(_G_hUdialMutex);
    }
}

/*********************************************************************************************************
** 函数名称: udialStatusInit
** 功能描述: 拨号状态更新初始化
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  udialStatusInit (VOID)
{
    pthread_t             tid;
    pthread_attr_t        attr;
    INT                   iRet;

    /*
     *  创建拨号处理子线程
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
** 函数名称: udialStart
** 功能描述: 进行拨号操作
** 输  入  : pUdialHandleInfo  拨号信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
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

    if (_G_iUdialPppNum >= LW_CFG_LWIP_NUM_PPP) {                       /*  拨号次数超过                */
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pppoe over\n");
        udialLogInsert(pUdialHandleInfo->cNetifname, "pppoe net interface over\n");

        return  (1);
    }

    iRet = __udialIniFileHandle(pUdialHandleInfo, 1);                   /*  更新配置文件                */
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    /*
    * 初始化进程属性对象
    */
    if (posix_spawnattr_init(&spawnattr) != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "init posix_spawnattr_t failed\n");

        return  (PX_ERROR);
    }

    /*
     * 新建进程优先级为 NORMAL
     */
    schedparam.sched_priority = PX_PRIORITY_CONVERT(LW_PRIO_NORMAL);
    posix_spawnattr_setschedparam(&spawnattr, &schedparam);
    posix_spawnattr_setflags(&spawnattr, POSIX_SPAWN_SETSCHEDPARAM);

    /*
     *  构造拨号需要使用的 ini section
     */
    bzero(cSectionBuf, sizeof(cSectionBuf));
    snprintf(cSectionBuf, sizeof(cSectionBuf), "pppoe_%s", pUdialHandleInfo->cNetifname);

    if (posix_spawnp(&pid, "udial", NULL,
                     &spawnattr, cCmdArg, NULL) != 0) {                 /* 启动进程                     */
        posix_spawnattr_destroy(&spawnattr);
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "posix_spawnp error\n");

        return  (PX_ERROR);
    }

    _G_iUdialPppNum++;

    /*
     *  更新数据库信息
     */
    iNetifIndex = netif_get_max_index();
    if (_G_iNetifMaxNum > iNetifIndex) {
        iNetifIndex = _G_iNetifMaxNum;
    } else {
        _G_iNetifMaxNum = iNetifIndex;
    }

    snprintf(cPppName, sizeof(cPppName), "pp%d", iNetifIndex);          /*  构建一个 ppp 网卡名称       */
    strncpy(pUdialHandleInfo->cPppname, cPppName, sizeof(pUdialHandleInfo->cPppname));
    pUdialHandleInfo->uiPid = pid;
    pUdialHandleInfo->iUdialStatus = NET_EVENT_PPP_DEAD;                /*  默认设置为断开连接状态      */

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    udialUpdateToDb(pUdialHandleInfo);

    Lw_SemaphoreM_Post(_G_hUdialMutex);

    posix_spawnattr_destroy(&spawnattr);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: udialDel
** 功能描述: 删除拨号操作
** 输  入  : pUdialHandleInfo  拨号信息
** 输  出  : NONE
** 返  回  : iStatLoc          删除状态
*********************************************************************************************************/
INT udialDel (CPCHAR  cpcNetifName)
{
    __UDIAL_HANDLE        udialHandleInfo;
    CHAR                  cKillCmd[50];
    INT                   iStatLoc = ERROR_NONE;
    INT                   iRet;

    udialLogInsert((PCHAR)cpcNetifName, "delet pppoe net interface\n");

    /*
     *  获取 pid ==> 删除 ==> wait
     */
    bzero(&udialHandleInfo, sizeof(udialHandleInfo));

    Lw_SemaphoreM_Wait(_G_hUdialMutex, LW_OPTION_WAIT_INFINITE);

    udialGetSpecInfo(&udialHandleInfo, "NETIF", (PCHAR)cpcNetifName, LW_NULL);
    if (0 != udialHandleInfo.uiPid) {
        _G_iNetifMaxNum = netif_get_max_index();                        /*  记录下当前最大的 num        */
        /*
         *  先删除 流量监控 对ppp网口的操作
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
        udialHandleInfo.iUdialStatus = NET_EVENT_PPP_DEAD;          /*  说明时断开状态              */
        udialUpdateToDb(&udialHandleInfo);                          /*  更新一下 pid 说明被删除     */

        Lw_SemaphoreM_Post(_G_hUdialMutex);

        _G_iUdialPppNum--;
    } else {
        Lw_SemaphoreM_Post(_G_hUdialMutex);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: udialInit
** 功能描述: 拨号初始化
** 输  入  : NONW
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  udialInit (VOID)
{
    /*
     * 注册一个处理函数，用于 udial 进程退出时处理
     */
    INIT_CHECK(subRecliamHandleAdd(__udialProcessExitHandle));

    _G_hUdialMutex = Lw_SemaphoreM_Create("kidvpn_fd_lock",            /*  创建文件句柄链表操作互斥锁  */
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


    return  udialStatusInit();                                          /*  启动拨号检测线程            */
}
