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
** 文   件   名: arphandle.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述:  arp管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "arp_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: arpDynamicShow
** 功能描述: 查询动态arp信息
** 输　入  : pArpHandle         arp入参信息
** 输　出  : piRecordTotal      arp记录查询结果总数
** 返　回  : arp查询结果数组指针
*********************************************************************************************************/
PVOID  arpDynamicShow (__PARP_HANDLE  pArpHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ARP_DYNAMIC_CMD, pArpHandle, sizeof(__ARP_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: arpShow
** 功能描述: 查询arp信息
** 输　入  : pArpHandle         arp入参信息
** 输　出  : piRecordTotal      arp记录查询结果总数
** 返　回  : arp查询结果数组指针
*********************************************************************************************************/
PVOID  arpShow (__PARP_HANDLE  pArpHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ARP_CMD, pArpHandle, sizeof(__ARP_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: arpConfig
** 功能描述: 配置arp
** 输　入  : pArpHandle         arp入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  arpConfig (__PARP_HANDLE  pArpHandle)
{
    return  requestConfigMsg (OPENRT_CFG_ARP_CMD, pArpHandle, sizeof(__ARP_HANDLE));
}
