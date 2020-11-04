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
** 文   件   名: flowctl_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 29 日
**
** 描        述: flowctl 信息头文件
*********************************************************************************************************/

#ifndef __FLOWCTL_COMMON_H
#define __FLOWCTL_COMMON_H

#include "libflowctl_config.h"

/*********************************************************************************************************
  流控相关宏定义
*********************************************************************************************************/
#define FLOWCTL_SHOW      "flowctl"                                     /* 流控查询                     */
#define FLOWCTL_ADD       "add"                                         /* 增加流控条目                 */
#define FLOWCTL_DELETE    "del"                                         /* 删除流控条目                 */
#define FLOWCTL_CHANGE    "chg"                                         /* 修改流控条目                 */

#define FC_PROTO_TCP      "tcp"                                         /* 流控限制对TCP协议生效        */
#define FC_PROTO_UDP      "udp"                                         /* 流控限制对UDP协议生效        */
#define FC_PROTO_IP       "all"                                         /* 对所有协议限制都生效         */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT    flowctlDoConfig(__PFLOWCTL_HANDLE   pFlowctlInfo);
PVOID  flowctlGetFromDB(__PFLOWCTL_HANDLE  pFlowctlHandle, INT  *piRecordNum);
INT    flowctlUpdateToDb(CPCHAR  cpcOperType, __PFLOWCTL_HANDLE  pFlowctlInfo);
VOID   flowctlHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    flowctlTableCreate(VOID);
INT    flowctlRecovery(VOID);

#endif                                                                  /*  __FLOWCTL_COMMON_H          */
