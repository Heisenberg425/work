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
** 文   件   名: log.h
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年2月27日
**
** 描        述: dhcp服务log头文件
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

#define NYSLOG                                                          /* 定义使用NYSLOG系统           */

#ifdef  NYSLOG                                                          /* 是否启动NYSLOG系统           */
//#define LOGSTDOUT                                                       /* 是否输出LOG到终端            */
#define LOGBUFFMAX      128                                             /* log缓冲区最大大小            */
#define LOGCOUNTMAX     128                                             /* log刷出前最大缓冲数          */
#define LOGFILESPACE    16                                              /* log单个文件最大的空间        */
#define LOG_WARNING     3                                               /* WARNING LOG                  */
#define LOG_ERR         1                                               /* ERROR   LOG                  */
#define LOG_INFO        2                                               /* INFO    LOG                  */
#define LOG_DEBUG       0                                               /* DEBUG   LOG                  */
#define LOG(level, str, args...) nyslog(level, str, ## args)
#else
/*
 *  定义LOG输出函数
 */
#define LOG_WARNING     "warning"                                       /* WARNING LOG                  */
#define LOG_ERR         "error"                                         /* ERROR   LOG                  */
#define LOG_INFO        "info"                                          /* INFO    LOG                  */
#define LOG_DEBUG       "debug"                                         /* DEBUG   LOG                  */
#define LOG(level, str, args...) do { printf("%s: ", level); \
                                      printf(str, ## args);} while(0)
#endif

/*********************************************************************************************************
  log设备结构体
*********************************************************************************************************/
struct logstruct {
    struct sockaddr_un      server_address;                             /* AF_UNIX地址                   */
    INT                     iSocketfd;                                  /* udp套接字                    */
    INT                     iOpenFlag;                                  /* 开关状态位                   */
    UINT                    uiLen;                                      /* 长度                         */
};
/*********************************************************************************************************
  log文件结构体
*********************************************************************************************************/
struct logfile {
    UINT                    uiFileSize;                                 /* 文件大小                     */
    FILE                   *pLogFilefd;                                 /* 文件指针                     */
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
