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
** 文   件   名: dhcpserver_info.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 18 日
**
** 描        述: DHCP Server信息头文件
*********************************************************************************************************/
#ifndef __DHCPSERVER_INFO_H
#define __DHCPSERVER_INFO_H

#include "libdhcpserver_config.h"

/*********************************************************************************************************
** 函数名称: dhcpServerShow
** 功能描述: 查询dhcpServer信息
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : piRecordTotal      DHCP Server记录查询结果总数
** 返　回  : DHCP Server查询结果数组指针
*********************************************************************************************************/
PVOID  dhcpServerShow (__PDHCPSERVER_HANDLE  pDhcpServerHandle, INT*  piRecordTotal);
/*********************************************************************************************************
** 函数名称: dhcpServerConfig
** 功能描述: 配置DHCP Server
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT    dhcpServerConfig (__PDHCPSERVER_HANDLE  pDhcpServerHandle);
/*********************************************************************************************************
** 函数名称: dhcpServerDel
** 功能描述: 删除 DHCP Server
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT    dhcpServerDel (__PDHCPSERVER_HANDLE  pDhcpServerHandle);

#endif                                                                  /* __DHCPSERVER_INFO_H          */
