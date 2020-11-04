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
** 文   件   名: vlan_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: vlan 信息头文件
*********************************************************************************************************/

#ifndef __VLAN_COMMON_H
#define __VLAN_COMMON_H

#include "libvlan_config.h"
/*********************************************************************************************************
  VLAN 操作宏定义
*********************************************************************************************************/
#define VLAN_SHOW      "vlan"                                           /*查询vlan信息                  */
#define VLAN_ADD       "set"                                            /*增加或修改vlan信息            */
#define VLAN_DELETE    "clear"                                          /*删除vlan信息                  */
#define VLAN_UPDATE    "update"                                         /*更新vlan信息                  */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PVOID  vlanGetFromDB (INT* piRecordNum);
INT    vlanDoConfig (__PVLAN_HANDLE  pVlanInfo);
INT    vlanUpdateToDb (CPCHAR  operType, __PVLAN_HANDLE  pVlanInfo);
VOID   vlanHandleEntry (INT  iCommand, PVOID pArgBuf);
INT    vlanTableCreate (VOID);
INT    vlanRecovery (VOID);

#endif                                                                  /*  __VLAN_COMMON_H             */
