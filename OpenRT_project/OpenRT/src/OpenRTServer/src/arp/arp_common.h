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
** 文   件   名: arp_common.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: arp信息头文件
*********************************************************************************************************/
#ifndef __ARP_COMMON_H
#define __ARP_COMMON_H

#include "libarp_config.h"

/*********************************************************************************************************
  arp 链表结构
*********************************************************************************************************/
struct arp_handle_node {
    __ARP_HANDLE               handle;
    struct arp_handle_node    *next;
};
typedef struct arp_handle_node   __ARP_HANDLE_NODE;
typedef struct arp_handle_node  *__PARP_HANDLE_NODE;
/*********************************************************************************************************
  arp 相关宏定义
*********************************************************************************************************/
#define ARP_SHOW                   "arp"                                /* 查询arp信息                  */
#define ARP_SET                    "set"                                /* 增加或修改arp信息            */
#define ARP_DELETE                 "clear"                              /* 删除arp信息                  */
#define ARP_UPDATE                 "update"                             /* 更新arp信息                  */

#define ARP_STATUS_CONFIGURED      "configured"                         /* 有效配置                     */
#define ARP_STATUS_INVALID         "invalid"                            /* 无效配置                     */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PCHAR  arpTypeTransform (__PARP_HANDLE  pArpInfo, PCHAR  pcMACData, UINT  uiMACSize );
VOID   arpShellCheckStatus (__PARP_HANDLE  pArpArrayHeader, INT  iRecordNum);
PVOID  arpGetFromDB (INT*  piRecordNum);
INT    arpDeleteItemByIp (__PARP_HANDLE  pArpInfo);
PVOID  arpDynamicShellGet (INT*  iRecordNum);
INT    arpDoConfig (__PARP_HANDLE  pArpInfo);
INT    arpUpdateToDb (CPCHAR  operType, __PARP_HANDLE  pArpInfo);
VOID   arpHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT    arpTableCreate (VOID);
INT    arpRecovery (VOID);

#endif                                                                  /*  __ARP_COMMON_H              */
