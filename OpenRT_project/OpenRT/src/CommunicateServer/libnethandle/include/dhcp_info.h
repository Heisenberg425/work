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
** 文   件   名: kidvpn_info.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: KidVPN 处理结构头文件
*********************************************************************************************************/
#ifndef __DHCP_INFO_H
#define __DHCP_INFO_H
#include "libdhcp_config.h"

/*********************************************************************************************************
** 函数名称: libDhcpClientHandle
** 功能描述: 使能 / 禁能网口 DHCP 客户端
** 输　入  : pcNetifName        网口名称
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libDhcpClientHandle (PCHAR  pcNetifName, CHAR  cHandle);
#endif                                                                  /*  __DHCP_INFO_H               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
