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
** 文   件   名: qos_common.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 15 日
**
** 描        述: QoS信息头文件
*********************************************************************************************************/
#ifndef __QOS_COMMON_H
#define __QOS_COMMON_H

#include "libqos_config.h"

/*********************************************************************************************************
  QoS 操作Shell命令
*********************************************************************************************************/
#define QOS_SHELL_CFG_ADD         "qosruleadd"                          /*  添加qos命令                 */
#define QOS_SHELL_CFG_DEL         "qosruledel"                          /*  删除qos命令                 */
#define QOS_SHELL_CFG_SHOW        "qoss"                                /*  显示qos命令                 */

/*********************************************************************************************************
  Qos 数据范围
*********************************************************************************************************/
#define QOS_CFG_PRIORITY_MAX      (7)                                   /*  最低优先级                  */
#define QOS_CFG_PRIORITY_MIN      (0)                                   /*  最高优先级                  */
#define QOS_CFG_RULE_IP           "ip"                                  /*  IP 规则                     */
#define QOS_CFG_RULE_TCP          "tcp"                                 /*  TCP 规则                    */
#define QOS_CFG_RULE_UDP          "udp"                                 /*  UDP 规则                    */
#define QOS_CFG_CMPMETHOD_SRC     "SRC"                                 /*  SRC 方法                    */
#define QOS_CFG_CMPMETHOD_DEST    "DEST"                                /*  DEST 方法                   */
#define QOS_CFG_CMPMETHOD_BOTH    "BOTH"                                /*  BOTH                        */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT    qosIsExists (__PQOS_HANDLE  pQosInfo);
PVOID  qosGetFromDB (INT*  piRecordNum);
INT    qosConfigToDb (__PQOS_HANDLE  pQosInfo);
INT    qosDeleteFromDb (__PQOS_HANDLE  pQosInfo);
INT    qosDoConfig (__PQOS_HANDLE  pQosInfo);
INT    qosDoDel (__PQOS_HANDLE  pQosInfo);
VOID   qosHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT    qosTableCreate (VOID);
INT    qosRecovery (VOID);

#endif                                                                  /*  __QOS_COMMON_H_             */
