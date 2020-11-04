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
** 文   件   名: dhcpEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: OpenRT DHCP操作入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "dhcpExec.h"
#include "libdhcp_config.h"
/*********************************************************************************************************
** 函数名称: __netifDhcpClientHandle
** 功能描述: 网口 DHCP 客户端操作入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __netifDhcpClientHandle (PVOID  pArgBuf)
{
    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_DHCPC_HANDLE_ACK, PX_ERROR);

        return;
    }

    replyConfigMsg(OPENRT_DHCPC_HANDLE_ACK, netIfDhcpClient((__PDHCP_CLIENT_SET)pArgBuf));
}

/*********************************************************************************************************
** 函数名称: dhcpHandleEntry
** 功能描述: DHCP 操作入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID dhcpHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_DHCPC_HANDLE:
        __netifDhcpClientHandle(pArgBuf);
        break;

    default:
        break;
    }
}
