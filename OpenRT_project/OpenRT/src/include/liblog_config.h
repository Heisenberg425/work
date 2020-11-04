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
** 文   件   名: liblog_config.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: LOG 配置头文件
*********************************************************************************************************/
#ifndef _LIBLOG_CONFIG_H_
#define _LIBLOG_CONFIG_H_

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"
/*********************************************************************************************************
  LOG 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_UDIAL_LOG_GET         (OPEN_RT_LOG_CONFIG_BASE | 1)      /*  获取拨号日志                */
#define OPENRT_UDIAL_LOG_GET_ACK     (OPEN_RT_LOG_CONFIG_BASE | 2)      /*  获取拨号日志应答            */

#define OPENRT_UDIAL_LOG_NUM         (OPEN_RT_LOG_CONFIG_BASE | 3)      /*  获取拨号日志个数            */
#define OPENRT_UDIAL_LOG_NUM_ACK     (OPEN_RT_LOG_CONFIG_BASE | 4)      /*  获取拨号日志个数应答        */

#define OPENRT_OPER_LOG_GET          (OPEN_RT_LOG_CONFIG_BASE | 5)      /*  获取操作日志                */
#define OPENRT_OPER_LOG_GET_ACK      (OPEN_RT_LOG_CONFIG_BASE | 6)      /*  获取操作日志应答            */

#define OPENRT_OPER_LOG_PUT          (OPEN_RT_LOG_CONFIG_BASE | 7)      /*  记录操作日志                */
#define OPENRT_OPER_LOG_PUT_ACK      (OPEN_RT_LOG_CONFIG_BASE | 8)      /*  记录操作日志应答            */

#define OPENRT_OPER_LOG_NUM          (OPEN_RT_LOG_CONFIG_BASE | 9)      /*  获取操作日志个数            */
#define OPENRT_OPER_LOG_NUM_ACK      (OPEN_RT_LOG_CONFIG_BASE | 10)     /*  获取操作日志个数应答        */

#define OPENRT_VPN_LOG_GET           (OPEN_RT_LOG_CONFIG_BASE | 11)     /*  获取 VPN 日志内容           */
#define OPENRT_VPN_LOG_GET_ACK       (OPEN_RT_LOG_CONFIG_BASE | 12)     /*  获取 VPN 日志内容应答       */

#define OPENRT_VPN_LOG_NUM           (OPEN_RT_LOG_CONFIG_BASE | 13)     /*  获取 VPN 日志个数           */
#define OPENRT_VPN_LOG_NUM_ACK       (OPEN_RT_LOG_CONFIG_BASE | 14)     /*  获取 VPN 日志个数应答       */

#define OPENRT_UDIAL_LOG_DEL         (OPEN_RT_LOG_CONFIG_BASE | 15)     /*  清除拨号日志                */
#define OPENRT_UDIAL_LOG_DEL_ACK     (OPEN_RT_LOG_CONFIG_BASE | 16)     /*  清除拨号日志应答            */

#define OPENRT_OPER_LOG_DEL          (OPEN_RT_LOG_CONFIG_BASE | 17)     /*  清除操作日志                */
#define OPENRT_OPER_LOG_DEL_ACK      (OPEN_RT_LOG_CONFIG_BASE | 18)     /*  清除操作日志应答            */

#define OPENRT_VPN_LOG_DEL           (OPEN_RT_LOG_CONFIG_BASE | 19)     /*  清除 VPN 日志               */
#define OPENRT_VPN_LOG_DEL_ACK       (OPEN_RT_LOG_CONFIG_BASE | 20)     /*  清除 VPN 日志应答           */
/********************************************************************************************************
  获取拨号日志信息结构
*********************************************************************************************************/
struct udial_log_handle {
    time_t      logStartTm;                                             /*  开始时间                    */
    time_t      logEndTm;                                               /*  结束时间                    */
    INT64       llStartIndex;                                           /*  开始序号                    */
    INT64       llEndIndex;                                             /*  结束序号                    */
};
typedef struct udial_log_handle   __UDIAL_LOG_HANDLE;
typedef struct udial_log_handle  *__PUDIAL_LOG_HANDLE;
/*********************************************************************************************************
  拨号日志信息相关宏定义
*********************************************************************************************************/
#define EVENT_BUFF_LEN             (50)                                 /*  每一个事件buff长度          */
/*********************************************************************************************************
  拨号日志信息返回结构
*********************************************************************************************************/
struct udial_log_info {
    time_t      llTime;                                                 /*  时间                        */
    CHAR        cNetifName[NETIF_NAMESIZE];                             /*  网口名称                    */
    CHAR        cEvent[EVENT_BUFF_LEN];                                 /*  事件内容                    */
};
typedef struct udial_log_info   __UDIAL_LOG_INFO;
typedef struct udial_log_info  *__PUDIAL_LOG_INFO;

/*********************************************************************************************************
  KidVPN Log 日志参数
*********************************************************************************************************/
#define LOG_INFO_MAX_LEN      (512)                                     /*  每条日志的最大长度          */
/********************************************************************************************************
  获取 KidVPN日志信息结构
*********************************************************************************************************/
struct vpn_log_handle {
    time_t      logStartTm;                                             /*  开始时间                    */
    time_t      logEndTm;                                               /*  结束时间                    */
    INT32       iVndID;                                                 /*  VND ID                      */
    INT64       llStartIndex;                                           /*  开始序号                    */
    INT64       llEndIndex;                                             /*  结束序号                    */
};
typedef struct vpn_log_handle   __VPN_LOG_HANDLE;
typedef struct vpn_log_handle  *__PVPN_LOG_HANDLE;
/*********************************************************************************************************
  KidVPN Log 信息返回结构
*********************************************************************************************************/
struct kidvpn_log_info {
    time_t      llTime;                                                 /*  时间                        */
    INT32       iVndID;                                                 /*  VND 编号                    */
    CHAR        cLogInfo[LOG_INFO_MAX_LEN];                             /*  Log 内容                    */
};
typedef struct kidvpn_log_info   __KIDVPN_LOG_INFO;
typedef struct kidvpn_log_info  *__PKIDVPN_LOG_INFO;
/*********************************************************************************************************
  操作日志信息的结构
*********************************************************************************************************/
struct oper_log {
    time_t   logStartTm;                                               /*  开始时间                     */
    time_t   logEndTm;                                                 /*  结束时间                     */
    UINT32   llStartIndex;                                             /*  开始序号                     */
    UINT32   llEndIndex;                                               /*  结束序号                     */
    time_t   llTime;                                                   /*  时间                         */
    CHAR     cClientIP[64];                                            /*  客户端 IP                    */
    CHAR     cOperator[32];                                            /*  操作用户                     */
    CHAR     cModuleName[32];                                          /*  模块名称                     */
    INT      iResult;                                                  /*  操作结果  0:成功,1:失败      */
    CHAR     cLog[128];                                                /*  操作日志                     */
    UINT32   iTotal;                                                   /*  操作日志查询条数             */
};
typedef struct oper_log   __OPER_LOG;
typedef struct oper_log  *__POPER_LOG;

#endif                                                                  /* _LIBLOG_CONFIG_H_            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
