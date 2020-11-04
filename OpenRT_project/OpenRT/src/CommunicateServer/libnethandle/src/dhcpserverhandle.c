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
** 文   件   名: dhcpserverhandle.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 18 日
**
** 描        述:  DHCP Server管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "dhcpserver_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: dhcpServerShow
** 功能描述: 查询DHCP Server信息
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : piRecordTotal      DHCP Server记录查询结果总数
** 返　回  : DHCP Server查询结果数组指针
*********************************************************************************************************/
PVOID  dhcpServerShow (__PDHCPSERVER_HANDLE  pDhcpServerHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: dhcpServerConfig
** 功能描述: 配置DHCP Server
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  dhcpServerConfig (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    return  requestConfigMsg (OPENRT_CFG_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE));
}
/*********************************************************************************************************
** 函数名称: dhcpServerDel
** 功能描述: 删除 DHCP Server
** 输　入  : pDhcpServerHandle  DHCP Server入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT    dhcpServerDel (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    return  requestConfigMsg (OPENRT_DEL_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE));
}
