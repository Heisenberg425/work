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
** ��   ��   ��: log.h
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��2��27��
**
** ��        ��: dhcp����logͷ�ļ�
*********************************************************************************************************/
#ifndef LOG_H_
#define LOG_H_

#include "stdio.h"
#include <semaphore.h>
#include <socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/un.h>

#define NYSLOG                                                          /* ����ʹ��NYSLOGϵͳ           */

#ifdef  NYSLOG                                                          /* �Ƿ�����NYSLOGϵͳ           */
//#define LOGSTDOUT                                                       /* �Ƿ����LOG���ն�            */
#define LOGBUFFMAX      128                                             /* log����������С            */
#define LOGCOUNTMAX     128                                             /* logˢ��ǰ��󻺳���          */
#define LOGFILESPACE    16                                              /* log�����ļ����Ŀռ�        */
#define LOG_WARNING     3                                               /* WARNING LOG                  */
#define LOG_ERR         1                                               /* ERROR   LOG                  */
#define LOG_INFO        2                                               /* INFO    LOG                  */
#define LOG_DEBUG       0                                               /* DEBUG   LOG                  */
#define LOG(level, str, args...) nyslog(level, str, ## args)
#else
/*
 *  ����LOG�������
 */
#define LOG_WARNING     "warning"                                       /* WARNING LOG                  */
#define LOG_ERR         "error"                                         /* ERROR   LOG                  */
#define LOG_INFO        "info"                                          /* INFO    LOG                  */
#define LOG_DEBUG       "debug"                                         /* DEBUG   LOG                  */
#define LOG(level, str, args...) do { printf("%s: ", level); \
                                      printf(str, ## args);} while(0)
#endif

/*********************************************************************************************************
  log�豸�ṹ��
*********************************************************************************************************/
struct logstruct {
    struct sockaddr_un      server_address;                             /* AF_UNIX��ַ                   */
    INT                     iSocketfd;                                  /* udp�׽���                    */
    INT                     iOpenFlag;                                  /* ����״̬λ                   */
    UINT                    uiLen;                                      /* ����                         */
};
/*********************************************************************************************************
  log�ļ��ṹ��
*********************************************************************************************************/
struct logfile {
    UINT                    uiFileSize;                                 /* �ļ���С                     */
    FILE                   *pLogFilefd;                                 /* �ļ�ָ��                     */
};

typedef struct logstruct    LOGDEV;
typedef struct logfile      LOGFILE;

#ifdef  NYSLOG
INT   nyslog (INT     iLevel,
              CPCHAR  pcMsg,
              ...);
VOID  logExit (VOID);
#endif

#endif /* LOG_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
