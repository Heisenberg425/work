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
** 文   件   名: libmonitor.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 16 日
**
** 描        述: monitor 信息头文件
*********************************************************************************************************/
#ifndef __LIBMONITOR_CONFIG_H
#define __LIBMONITOR_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  监控信息服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_MONITOR_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 1)  /*  配置监控                    */
#define OPENRT_CFG_MONITOR_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 2)  /*  配置监控应答                */

#define OPENRT_GET_MONITOR_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 3)  /*  获取监控信息                */
#define OPENRT_GET_MONITOR_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 4)  /*  获取监控信息应答            */

#define OPENRT_GET_CPU_MEM_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 5)  /*  获取 cpu/memory 信息        */
#define OPENRT_GET_CPU_MEM_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 6)  /*  获取 cpu/memory 信息应答    */

#define OPENRT_GET_INTERNET_CMD      (OPEN_RT_MONITOR_CONFIG_BASE | 7)  /*  获取互联网连接状态          */
#define OPENRT_GET_INTERNET_ACK_CMD  (OPEN_RT_MONITOR_CONFIG_BASE | 8)  /*  获取互联网连接状态应答      */
/*********************************************************************************************************
  用于监控显示的宏定义
*********************************************************************************************************/
#define TMT_TERM                     (1)                                /* ip (终端)                    */
#define TMT_APP                      (2)                                /* APP(应用)                    */
#define TMT_LINE                     (3)                                /* net interface (线路)         */
#define TMT_LINE_APP                 (4)                                /* net interface (应用)         */
#define TMT_WAN_TOTAL                (5)                                /* WAN 口总流量                 */
#define TMT_CPU_MEM_USER             (6)                                /* cpu/mem 占用率、user 统计    */

#define TMF_IP                       (1)                                /* IP                           */
#define TMF_MAC                      (2)                                /* MAC                          */
/*********************************************************************************************************
 互联网连接状态字段宏
*********************************************************************************************************/
#define INTERNET_OK                  (0)                                /*  连接成功                    */
#define INTERNET_FAILED              (1)                                /*  连接失败                    */
/*********************************************************************************************************
  App 类型
*********************************************************************************************************/
enum app_type
{
    APP_TYPE_HTTP = 1,                                                  /* HTTP 类                       */
    APP_TYPE_MAIL = 2,                                                  /* 邮件服务类                   */
    APP_TYPE_CHAT = 3,                                                  /* 聊天通信类                   */
    APP_TYPE_TRANSFER = 4,                                              /* 文件传输类                   */
    APP_TYPE_OTHERS = 5,                                                /* 其他类                       */
    APP_TYPE_MAX = 6
};
/*********************************************************************************************************
  资源使用情况
*********************************************************************************************************/
struct tm_cpu_mem {
	double      dCpuUsage;                                              /*  CPU 使用率                  */
	double      dMemoryUsage;                                           /*  内存使用率                  */
	UINT32      uiOnlineUsers;                                          /*  在线用户数目                */
};
typedef struct tm_cpu_mem   TM_CPU_MEM;
typedef struct tm_cpu_mem*  PTM_CPU_MEM;
/*********************************************************************************************************
  用于监控信息的结构
*********************************************************************************************************/
struct tm_stats {
    UCHAR           cType;                                              /* TERM (终端),APP (应用/线路)  */
    UCHAR           cEnable;                                            /* 1: 统计项使能 0: 统计项去使能*/
    UCHAR           cProtocol;                                          /* TCP/UDP                      */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* 应用知名端口号               */
    UINT16          usConns;                                            /* TCP 连接数                   */
    CHAR            pIpAddr[32];                                        /* IP 地址                      */
    CHAR            pMac[32];                                           /* MAC 地址                     */
    CHAR            pAlias[32];                                         /* 别名                         */
    CHAR            pIfname[NETIF_NAMESIZE];                            /* 网口名称                     */
    UINT32          iDuration;                                          /* 在线时长 (min)               */
    UINT32          iSendRate;                                          /* 发送速率 (B/s)               */
    UINT32          iRecvRate;                                          /* 接收速率 (B/s)               */
    UINT32          iSendCurrLen;                                       /* 速率统计周期内发送报文长度   */
    UINT32          iRecvCurrLen;                                       /* 速率统计周期内接收报文长度   */
    UINT64          iSendTotalLen;                                      /* 发送报文总长度(字节数)       */
    UINT64          iRecvTotalLen;                                      /* 接收报文总长度(字节数)       */
    UINT32          iSendDrops;                                         /* 发送丢包数                   */
    UINT32          iRecvDrops;                                         /* 接收丢包数                   */
    UINT64          iSendTotalPackets;                                  /* 发送总报文数                 */
    UINT64          iRecvTotalPackets;                                  /* 接收总报文数                 */
};
typedef struct tm_stats  TM_STATS;
typedef struct tm_stats  *PTM_STATS;
/*********************************************************************************************************
  流量相关结构
*********************************************************************************************************/
struct tm_stat_entry {
    LW_LIST_LINE    tmlist;                                             /*  流量相关链表                */
    TM_STATS        tm_stats;                                           /*  tm stats                    */
};
typedef struct tm_stat_entry  TM_STAT_ENTRY;
typedef struct tm_stat_entry*  PTM_STAT_ENTRY;
/*********************************************************************************************************
  互联网连接状态返回结构
*********************************************************************************************************/
struct internet_status {
    time_t      llTime;                                                 /*  连接时间                    */
    UINT8       ucStatus;                                               /*  连接状态                    */
};
typedef struct internet_status  __INTERNET_STATUS;
typedef struct internet_status  *__PINTERNET_STATUS;

#endif                                                                  /*  __LIBMONITOR_CONFIG_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
