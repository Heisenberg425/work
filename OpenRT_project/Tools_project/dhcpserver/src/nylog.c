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
** 文   件   名: nylog.c
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年1月10日
**
** 描        述: 简易log系统实现
*********************************************************************************************************/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "log.h"
#include "configini.h"

#ifdef  NYSLOG

#define DEBUGLOG        0                                               /* 调试log                      */
#define ERRORLOG        1                                               /* 报错log                      */
#define INFOLOG         2                                               /* 信息log                      */
#define WARNINGLOG      3                                               /* 警告log                      */
#define YEARBASE        1900                                            /* 年份从1900开始计算           */
#define MONBASE         1                                               /* 月份从1开始计算              */
#define AF_UNIX_PATH    "/root/AF_UNIX/"

static  INT                 GilogClientFd;                              /* 日志客户端句柄               */
static  LOGDEV              GlogdevServer;                              /* 服务端设备全局结构体         */
static  pthread_mutex_t     GClientSocketMutex;                         /* 客户端套接字锁               */
static  pthread_once_t      GOnce = PTHREAD_ONCE_INIT;                  /* 一次性初始化全局变量         */

LOGFILE                     GLogFile;                                   /* 全局文件结构体               */
pthread_t                   GLogServerTid           = 0;                /* 日志服务线程ID号             */
pthread_t                   GFileSyncTid            = 0;                /* 文件同步线程ID号             */

/*********************************************************************************************************
** 函数名称: filebackcheck
** 功能描述: 文件备份检查
** 输    入: PVOID  Argv
** 输    出: NONE
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
VOID  filebackcheck (VOID)
{
    CHAR  cCmd[128] = {0};

    if (GLogFile.uiFileSize >= LOGFILESPACE*1024*1024) {
        fflush(GLogFile.pLogFilefd);
        sync();
        fclose(GLogFile.pLogFilefd);
        memset(cCmd, 0, sizeof(cCmd));

        snprintf(cCmd, sizeof(cCmd), "rm %s%s", GserverConfig.pclogfile,".old");
        system(cCmd);

        snprintf(cCmd, sizeof(cCmd),
                "mv %s %s%s",
                GserverConfig.pclogfile,
                GserverConfig.pclogfile,
                ".old");
        system(cCmd);

        GLogFile.uiFileSize = 0;
        GLogFile.pLogFilefd = NULL;
        GLogFile.pLogFilefd = fopen(GserverConfig.pclogfile, "a+");
        if (GLogFile.pLogFilefd == NULL) {
            GlogdevServer.iOpenFlag = 0;
        }
    }

    return;
}

/*********************************************************************************************************
** 函数名称: logToFile
** 功能描述: log写入文件
** 输    入: CPCHAR  pcMsg  日志内容
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT logToFile (CPCHAR  pcMsg)
{
    INT        iRet;
    static INT iWriteCount = 0;

    iRet = fprintf(GLogFile.pLogFilefd,"%s",pcMsg);
    if (iRet < 0) {
       return (PX_ERROR);
    }
    fflush(GLogFile.pLogFilefd);

    GLogFile.uiFileSize += iRet;
    iWriteCount ++;
    if (iWriteCount >= LOGCOUNTMAX) {
        iWriteCount = 0;
        sync();
    }

    filebackcheck();
    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: logFormat
** 功能描述: log打印添加详细信息
** 输    入: INT     iLevel       日志等级
             CPCHAR  pcMsg        日志内容
             PCHAR   pcFormatMsg  输出buff
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
**           输出格式户后的日志内容
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logFormat (INT     iLevel,
                       CPCHAR  pcMsg,
                       PCHAR   pcFormatMsg)
{
    struct tm  timeNow;
    INT64      iTime;

    iTime = time(NULL);
    localtime_r(&iTime,&timeNow);                                       /* 获取系统时间                 */

    if (pcMsg!= NULL && pcFormatMsg!= NULL) {
        switch (iLevel) {                                               /* 根据日志等级拼接日志字符串   */
        case DEBUGLOG:
            snprintf(pcFormatMsg,
                     LOGBUFFMAX,
                     "%04d-%02d-%02d %02d:%02d:%02d[DEBG]:%s",
                     timeNow.tm_year + YEARBASE,
                     timeNow.tm_mon  + MONBASE,
                     timeNow.tm_mday,
                     timeNow.tm_hour,
                     timeNow.tm_min,
                     timeNow.tm_sec,
                     pcMsg);
            break;
        case ERRORLOG:
            snprintf(pcFormatMsg,
                     LOGBUFFMAX,
                     "%04d-%02d-%02d %02d:%02d:%02d[EROR]:%s",
                     timeNow.tm_year + YEARBASE,
                     timeNow.tm_mon  + MONBASE,
                     timeNow.tm_mday,
                     timeNow.tm_hour,
                     timeNow.tm_min,
                     timeNow.tm_sec,
                     pcMsg);
            break;
        case INFOLOG:
            snprintf(pcFormatMsg,
                     LOGBUFFMAX,
                     "%04d-%02d-%02d %02d:%02d:%02d[INFO]:%s",
                     timeNow.tm_year + YEARBASE,
                     timeNow.tm_mon  + MONBASE,
                     timeNow.tm_mday,
                     timeNow.tm_hour,
                     timeNow.tm_min,
                     timeNow.tm_sec,
                     pcMsg);
            break;
        case WARNINGLOG:
            snprintf(pcFormatMsg,
                     LOGBUFFMAX,
                     "%04d-%02d-%02d %02d:%02d:%02d[WARN]:%s",
                     timeNow.tm_year + YEARBASE,
                     timeNow.tm_mon  + MONBASE,
                     timeNow.tm_mday,
                     timeNow.tm_hour,
                     timeNow.tm_min,
                     timeNow.tm_sec,
                     pcMsg);
            break;
        default:
            return (PX_ERROR);
        }
    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: logServer
** 功能描述: logServer线程
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
PVOID  logServer (PVOID  Argv)
{
    CHAR                    cBuf[LOGBUFFMAX] = {0};
    struct sockaddr_un      recv_address;
    INT                     recv_len         = sizeof(recv_address);

    GLogFile.pLogFilefd   = NULL;
    GLogFile.pLogFilefd   = fopen(GserverConfig.pclogfile, "a+");

    if (GLogFile.pLogFilefd != NULL) {
        fseek(GLogFile.pLogFilefd, 0, SEEK_END);
        GLogFile.uiFileSize = (UINT)ftell(GLogFile.pLogFilefd);         /* 追踪到文件尾                 */
        while (GlogdevServer.iOpenFlag) {
            recvfrom(GlogdevServer.iSocketfd,
                     cBuf,
                     sizeof(cBuf),
                     0,
                     (struct sockaddr *) &recv_address,
                     (socklen_t*)&recv_len);
#ifdef LOGSTDOUT
            fprintf(stdout, "%s", cBuf);
#endif
            logToFile(cBuf);
            memset(cBuf, 0, sizeof(cBuf));
        }
    } else {
        GlogdevServer.iOpenFlag = 0;
    }
    return (NULL);
}

/*********************************************************************************************************
** 函数名称: logServerStart
** 功能描述: logServer线程启动
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logServerStart (VOID)
{
    INT             iRet;
    pthread_attr_t  attr;

    GlogdevServer.iOpenFlag = 1;

    iRet = pthread_attr_init(&attr);
    if (iRet != ERROR_NONE) {
        return (PX_ERROR);
    }

    iRet = pthread_attr_setname(&attr, "dhcpServerlog");
    if (iRet != ERROR_NONE) {
        return (PX_ERROR);
    }
    iRet = pthread_create(&GLogServerTid, &attr, logServer, NULL);
    if (iRet != LW_OBJECT_HANDLE_INVALID) {
        return (PX_ERROR);
    }

    if (GlogdevServer.iOpenFlag == 0) {
        return (PX_ERROR);
    }
    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: logServerInit
** 功能描述: logServer初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logServerInit (VOID)
{
    INT  iRet;

    GlogdevServer.uiLen                          = sizeof (GlogdevServer.server_address);
    GlogdevServer.server_address.sun_family      = AF_UNIX;
    sprintf (GlogdevServer.server_address.sun_path, AF_UNIX_PATH"dhcp_log_server_%s",GserverConfig.pcInteface);
    unlink (GlogdevServer.server_address.sun_path);

    if (GlogdevServer.iOpenFlag != 1) {
        GlogdevServer.iSocketfd = socket(GlogdevServer.server_address.sun_family, SOCK_DGRAM, 0);
        if (GlogdevServer.iSocketfd < 0) {
            return (PX_ERROR);
        }

        iRet = bind(GlogdevServer.iSocketfd,
                    (struct sockaddr *) &GlogdevServer.server_address,
                    GlogdevServer.uiLen);
        if (iRet < 0) {
            return (PX_ERROR);
        }

        iRet = logServerStart();
        if (iRet < 0) {
            GlogdevServer.iOpenFlag = 0;
            return (PX_ERROR);
        }
    }
    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: logClientInit
** 功能描述: logClient初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logClientInit (VOID)
{
    struct sockaddr_un    unix_client;

    GilogClientFd = socket(AF_UNIX, SOCK_DGRAM, 0);                     /*  初始化client套接字          */
    if (GilogClientFd < 0) {
        return (PX_ERROR);
    }

    unix_client.sun_family = AF_UNIX;
    sprintf(unix_client.sun_path, AF_UNIX_PATH"dhcp_log_client_%s",GserverConfig.pcInteface);
    unlink (unix_client.sun_path);
    bind(GilogClientFd,
         (struct sockaddr *) &unix_client,
         sizeof (unix_client));

    if (pthread_mutex_init(&GClientSocketMutex, NULL) != 0) {           /*  初始化互斥锁                */
        printf("mutex init failed!\n");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: logClose
** 功能描述: log关闭
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logClose (VOID)
{

    if (&GlogdevServer.iOpenFlag) {
        GlogdevServer.iOpenFlag = 0;
    }

    fflush(GLogFile.pLogFilefd);
    sync();

    return (ERROR_NONE);
}


/*********************************************************************************************************
** 函数名称: logExit
** 功能描述: log退出
** 输    入: NONE
** 输    出: NONE
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
VOID  logExit (VOID)
{
    logClose();
}
/*********************************************************************************************************
** 函数名称: logInit
** 功能描述: log初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  logInit (VOID)
{
    INT               iRet;

    mkdir(AF_UNIX_PATH, S_IRWXU|S_IRGRP|S_IROTH);

    iRet = logServerInit();
    if (iRet < 0) {
        return (PX_ERROR);
    }

    iRet = logClientInit();
    if (iRet < 0) {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: nyslog
** 功能描述: log线程安全方法
** 输    入: INT     iLevel   log等级
**           CPCHAR  pcMsg    log内容
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 全局变量: NONE
** 调用模块: API
*********************************************************************************************************/
INT  nyslog (INT     iLevel,
             CPCHAR  pcMsg,
             ...)
{
    INT      iRet;
    va_list  ap;
    CHAR     cMsg[LOGBUFFMAX] = {0};
    CHAR     cLogBuff[LOGBUFFMAX] = {0};

    iRet = pthread_once(&GOnce, (PVOID)logInit);

    if (iRet < 0) {
        return (PX_ERROR);
    }

    if (pcMsg == NULL) {
        return (PX_ERROR);
    }
#ifndef LOGDEBUG
    if (iLevel == LOG_DEBUG) {
        return (ERROR_NONE);
    }
#endif
    if (GlogdevServer.iOpenFlag) {
        va_start(ap, pcMsg);
        vsnprintf(cMsg, LOGBUFFMAX, pcMsg, ap);                         /* 不定长参数处理               */
        va_end(ap);

        iRet = logFormat(iLevel, cMsg, cLogBuff);
        if (iRet < 0) {
            return (PX_ERROR);
        }

        pthread_mutex_lock(&GClientSocketMutex);

        iRet = sendto(GilogClientFd,                                   /* 将日志字符串发送出去         */
                      cLogBuff,
                      strlen(cLogBuff),
                      0,
                      (struct sockaddr*)&GlogdevServer.server_address,
                      GlogdevServer.uiLen);

        pthread_mutex_unlock(&GClientSocketMutex);

        if (iRet < 0) {
            return (PX_ERROR);
        }

    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
