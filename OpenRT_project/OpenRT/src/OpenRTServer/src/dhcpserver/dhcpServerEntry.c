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
** 文   件   名: dhcpServerEntry.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 19 日
**
** 描        述: DHCP Server配置处理入口
** 2018.09.20    DHCP SERVER 支持在多网口设备运行多个服务器
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "dhcpserver_common.h"

/*********************************************************************************************************
** 函数名称: dhcpServerConfigDeal
** 功能描述: 配置DHCP Server处理函数
** 输　入  : pDhcpServerHandle  DHCP Server参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpServerConfigDeal (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    replyConfigMsg(OPENRT_CFG_DHCPSERVER_ACK_CMD, dhcpServerDoConfig(pDhcpServerInfo));
}
/*********************************************************************************************************
** 函数名称: dhcpServerShowDeal
** 功能描述: 查询DHCP Server处理函数
** 输　入  : pDhcpServerHandle  DHCP Server 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpServerShowDeal (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    INT                   iNum = 0;
    __PDHCPSERVER_HANDLE  pDhcpServerInfo = LW_NULL;

    pDhcpServerInfo = dhcpServerDoShow(&iNum);                          /*  获取数据库数据              */

    replyQueryMsg(OPENRT_GET_DHCPSERVER_ACK_CMD, sizeof(__DHCPSERVER_HANDLE), iNum, pDhcpServerInfo);

    free(pDhcpServerInfo);
}
/*********************************************************************************************************
** 函数名称: dhcpServerDelDeal
** 功能描述: 删除 DHCP Server处理函数
** 输　入  : pDhcpServerHandle  DHCP Server参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpServerDelDeal (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    replyConfigMsg(OPENRT_DEL_DHCPSERVER_ACK_CMD, dhcpServerDoDel(pDhcpServerInfo));
}
/*********************************************************************************************************
** 函数名称: dhcpServerHandleEntry
** 功能描述: DHCP Server配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpServerHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_DHCPSERVER_CMD:
        dhcpServerConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_DHCPSERVER_CMD:
        dhcpServerShowDeal(pArgBuf);
        break;

    case OPENRT_DEL_DHCPSERVER_CMD:
        dhcpServerDelDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
