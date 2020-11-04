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
** ��   ��   ��: nylog.c
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��1��10��
**
** ��        ��: ����logϵͳʵ��
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

#define DEBUGLOG        0                                               /* ����log                      */
#define ERRORLOG        1                                               /* ����log                      */
#define INFOLOG         2                                               /* ��Ϣlog                      */
#define WARNINGLOG      3                                               /* ����log                      */
#define YEARBASE        1900                                            /* ��ݴ�1900��ʼ����           */
#define MONBASE         1                                               /* �·ݴ�1��ʼ����              */
#define AF_UNIX_PATH    "/root/AF_UNIX/"

static  INT                 GilogClientFd;                              /* ��־�ͻ��˾��               */
static  LOGDEV              GlogdevServer;                              /* ������豸ȫ�ֽṹ��         */
static  pthread_mutex_t     GClientSocketMutex;                         /* �ͻ����׽�����               */
static  pthread_once_t      GOnce = PTHREAD_ONCE_INIT;                  /* һ���Գ�ʼ��ȫ�ֱ���         */

LOGFILE                     GLogFile;                                   /* ȫ���ļ��ṹ��               */
pthread_t                   GLogServerTid           = 0;                /* ��־�����߳�ID��             */
pthread_t                   GFileSyncTid            = 0;                /* �ļ�ͬ���߳�ID��             */

/*********************************************************************************************************
** ��������: filebackcheck
** ��������: �ļ����ݼ��
** ��    ��: PVOID  Argv
** ��    ��: NONE
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: logToFile
** ��������: logд���ļ�
** ��    ��: CPCHAR  pcMsg  ��־����
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: logFormat
** ��������: log��ӡ�����ϸ��Ϣ
** ��    ��: INT     iLevel       ��־�ȼ�
             CPCHAR  pcMsg        ��־����
             PCHAR   pcFormatMsg  ���buff
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
**           �����ʽ�������־����
** ȫ�ֱ���: NONE
** ����ģ��: API
*********************************************************************************************************/
static INT  logFormat (INT     iLevel,
                       CPCHAR  pcMsg,
                       PCHAR   pcFormatMsg)
{
    struct tm  timeNow;
    INT64      iTime;

    iTime = time(NULL);
    localtime_r(&iTime,&timeNow);                                       /* ��ȡϵͳʱ��                 */

    if (pcMsg!= NULL && pcFormatMsg!= NULL) {
        switch (iLevel) {                                               /* ������־�ȼ�ƴ����־�ַ���   */
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
** ��������: logServer
** ��������: logServer�߳�
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
        GLogFile.uiFileSize = (UINT)ftell(GLogFile.pLogFilefd);         /* ׷�ٵ��ļ�β                 */
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
** ��������: logServerStart
** ��������: logServer�߳�����
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: logServerInit
** ��������: logServer��ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: logClientInit
** ��������: logClient��ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
*********************************************************************************************************/
static INT  logClientInit (VOID)
{
    struct sockaddr_un    unix_client;

    GilogClientFd = socket(AF_UNIX, SOCK_DGRAM, 0);                     /*  ��ʼ��client�׽���          */
    if (GilogClientFd < 0) {
        return (PX_ERROR);
    }

    unix_client.sun_family = AF_UNIX;
    sprintf(unix_client.sun_path, AF_UNIX_PATH"dhcp_log_client_%s",GserverConfig.pcInteface);
    unlink (unix_client.sun_path);
    bind(GilogClientFd,
         (struct sockaddr *) &unix_client,
         sizeof (unix_client));

    if (pthread_mutex_init(&GClientSocketMutex, NULL) != 0) {           /*  ��ʼ��������                */
        printf("mutex init failed!\n");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: logClose
** ��������: log�ر�
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: logExit
** ��������: log�˳�
** ��    ��: NONE
** ��    ��: NONE
** ȫ�ֱ���: NONE
** ����ģ��: API
*********************************************************************************************************/
VOID  logExit (VOID)
{
    logClose();
}
/*********************************************************************************************************
** ��������: logInit
** ��������: log��ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
** ��������: nyslog
** ��������: log�̰߳�ȫ����
** ��    ��: INT     iLevel   log�ȼ�
**           CPCHAR  pcMsg    log����
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ȫ�ֱ���: NONE
** ����ģ��: API
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
        vsnprintf(cMsg, LOGBUFFMAX, pcMsg, ap);                         /* ��������������               */
        va_end(ap);

        iRet = logFormat(iLevel, cMsg, cLogBuff);
        if (iRet < 0) {
            return (PX_ERROR);
        }

        pthread_mutex_lock(&GClientSocketMutex);

        iRet = sendto(GilogClientFd,                                   /* ����־�ַ������ͳ�ȥ         */
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
