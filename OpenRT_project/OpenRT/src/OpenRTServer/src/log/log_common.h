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
** 文   件   名: interfaceExec.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录操作函数公用头文件
*********************************************************************************************************/

#ifndef __LOG_COMMON_H
#define __LOG_COMMON_H

#include "liblog_config.h"
/*********************************************************************************************************
  日志相关宏定义
*********************************************************************************************************/
#define MAX_RECORD_NUM            (3000)                                /*  日志最大记录条数            */
/*********************************************************************************************************
  日志数据库类型
*********************************************************************************************************/
enum LOG_TYPE {
    LOG_TYPE_UDIAL,
    LOG_TYPE_VPN,
};

/*********************************************************************************************************
  日志数据库表单名称
*********************************************************************************************************/
#define OPER_LOG_TABLE            "log_oper"                            /*  操作日志                    */
#define UDIAL_LOG_TABLE           "log_udial"                           /*  拨号表单名                  */
#define VPN_LOG_TABLE             "log_vpn"                             /*  VPN 表单名                  */
/*********************************************************************************************************
  日志数据库通用操作函数
*********************************************************************************************************/
time_t logTimeGen (PCHAR  pcTimeBuf);
VOID   logHandleEntry (INT  iCommand, PVOID  pArgBuf);
VOID   logTableDelet (INT32  iLogTableType);
INT32  logGetSpecInfoCount (INT32   iLogTableType,
                            time_t  logStartTm,
                            time_t  logEndTm,
                            PCHAR   pcOtherSpec);
/*********************************************************************************************************
  拨号日志数据库操作函数
*********************************************************************************************************/
INT    udialLogTableCreate(VOID);
VOID   udialLogInsert (PCHAR  pcNetifName, PCHAR  pcEventInfo);
INT    udialLogDel (VOID);
INT32  udialLogNumGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle);
INT32  udialLogGetSpecInfo (time_t      logStartTm,
                            time_t      logEndTm,
                            INT64       llStartIndex,
                            INT64       llEndIndex,
                            PVOID       pvLogBuff,
                            INT32       iBuffLen);
/*********************************************************************************************************
  VPN 日志数据库操作函数
*********************************************************************************************************/
INT    vpnLogTableCreate (VOID);
VOID   vpnLogInsert (INT32  iVndID, CPCHAR  cpcLogInfo);
INT    vpnLogDel (VOID);
INT32  vpnLogNumGet (__PVPN_LOG_HANDLE    pVpnLogHandle);
INT32  vpnLogGetSpecInfo (time_t      logStartTm,
                          time_t      logEndTm,
                          INT32       iVndID,
                          INT64       llStartIndex,
                          INT64       llEndIndex,
                          PVOID       pvLogBuff,
                          INT32       iBuffLen);

/*********************************************************************************************************
  OpenRT 操作日志数据库函数
*********************************************************************************************************/
INT    operLogTableCreate (VOID);
PVOID  operLogGetFromDB (__POPER_LOG pLoginRecord, INT* piRecordNum);
INT    operLog (__POPER_LOG pOperLog);
INT    operLogDel (VOID);

#endif /* SRC_LOG_LOG_COMMON_H_ */
