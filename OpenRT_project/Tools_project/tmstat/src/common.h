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
** 文   件   名: common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 11 日
**
** 描        述: 监控管理 common 头文件
*********************************************************************************************************/
#ifndef __TRAFFICSTATS_H
#define __TRAFFICSTATS_H

#include <time.h>
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"
/*********************************************************************************************************
  监控相关宏定义
*********************************************************************************************************/
#define TMT_TERM                     (1)                                /*  IP (终端)                   */
#define TMT_APP                      (2)                                /*  APP (应用)                  */
#define TMT_LINE                     (3)                                /*  net interface (线路)        */
#define TMT_LINE_APP                 (4)                                /*  net interface (应用)        */
#define TMT_WAN_TOTAL                (5)                                /*  WAN 口总流量                */
#define TMT_ONLINE_USERS             (6)                                /*  在线用户数                  */
#define TMT_CPU_MEM                  (7)                                /*  CPU/MEM 占用率统计          */

#define TMF_IP                       (1)                                /*  IP                          */
#define TMF_MAC                      (2)                                /*  MAC                         */
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define STAT_ITEM_PARAM_INVALID      (1)                                /*  参数无效                    */
#define STAT_ITEM_SHOULD_DELETE      (2)                                /*  应删除                      */
#define STAT_ITEM_HEADER_CHANGED     (3)                                /*  头部改变                    */
#define STAT_ITEM_IP_CHANGED         (4)                                /*  IP 改变                     */
#define STAT_ITEM_HEADER_DELETE      (5)                                /*  删除头部                    */
/*********************************************************************************************************
  速率统计周期宏定义
*********************************************************************************************************/
#define TM_CALC_PERIOD               (2)                                /*  速率统计周期为 2s           */
#define TM_STAT_PERIOD               (TM_CALC_PERIOD * LW_TICK_HZ)      /*  时间周期                    */
#define TM_STAT_COUNT_RESET_PERIOD   (LW_OPTION_ONE_HOUR * 24 * 7)      /*  统计计数清除周期为1周       */
/*********************************************************************************************************
  用于监控信息输出到 web 前端的结构
*********************************************************************************************************/
struct tm_stats {
    UCHAR           cType;                                              /* TERM(终端),APP(应用),(线路)  */
    UCHAR           cEnable;                                            /* 1: 统计项使能 0: 统计项去使能*/
    UCHAR           cProtocol;                                          /* TCP / UDP                    */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* 应用知名端口号               */
    UINT16          usConns;                                            /* TCP连接数                    */
    CHAR            pIpAddr[32];                                        /* IP地址                       */
    CHAR            pMac[32];                                           /* MAC地址                      */
    CHAR            pAlias[32];                                         /* 别名                         */
    CHAR            pIfname[16];                                        /* 网口名称                     */
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
  流量管理定义结构
*********************************************************************************************************/
struct tm_stats_inner {
    UCHAR           cType;                                              /* TERM(终端),APP(应用),(线路)  */
    UCHAR           cEnable;                                            /* 1: 统计项使能 0: 统计项去使能*/
    UCHAR           cProtocol;                                          /* TCP / UDP                    */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* 应用知名端口号               */
    UINT16          usConns;                                            /* TCP 连接数                   */
    ip4_addr_t      stIpAddr;                                           /* IP地址                       */
    UINT8           pMac[6];                                            /* MAC地址                      */
    CHAR            pAlias[32];                                         /* 别名                         */
    CHAR            pIfname[16];                                        /* 网口名称                     */
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
    INT64           iJoinTimestamp;                                     /* 统计项加入时间               */
};
typedef struct tm_stats_inner  TM_STATS_INNER;
typedef struct tm_stats_inner  *PTM_STATS_INNER;
/*********************************************************************************************************
  hash 节点成员结构
*********************************************************************************************************/
struct tm_stat_entry {
    LW_LIST_LINE    tmlist;                                             /*  链表                        */
    TM_STATS_INNER  tm_stats;                                           /* tm stats                     */
};

typedef struct tm_stat_entry  TM_STAT_ENTRY;
typedef struct tm_stat_entry*  PTM_STAT_ENTRY;
/*********************************************************************************************************
  全局变量声明
*********************************************************************************************************/
extern LW_OBJECT_HANDLE     _G_ulTmStatTimer;                           /*  流量统计定时器              */
extern LW_OBJECT_HANDLE     _G_ulTmStatCountResetTimer;                 /*  统计项清除定时器            */
/*********************************************************************************************************
  路由器上应用协议类型分类
*********************************************************************************************************/
enum app_type {
    APP_TYPE_HTTP = 1,                                                  /* HTTP类                       */
    APP_TYPE_MAIL = 2,                                                  /* 邮件服务类                   */
    APP_TYPE_CHAT = 3,                                                  /* 聊天通信类                   */
    APP_TYPE_TRANSFER = 4,                                              /* 文件传输类                   */
    APP_TYPE_OTHERS = 5,                                                /* 其他类                       */
    APP_TYPE_MAX = 6
};

#endif                                                                  /* __TRAFFICSTATS_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
