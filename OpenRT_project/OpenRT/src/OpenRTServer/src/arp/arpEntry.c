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
** 文   件   名: arpEntry.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: arp 配置处理入口
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
#include "arp_common.h"

/*********************************************************************************************************
** 函数名称: arpConfigDeal
** 功能描述: 配置 arp 处理函数
** 输　入  : pArpHandle         arp 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpConfigDeal (__PARP_HANDLE  pArpInfo)
{
    replyConfigMsg(OPENRT_CFG_ARP_CMD, arpDoConfig(pArpInfo));
}
/*********************************************************************************************************
** 函数名称: arpShowDeal
** 功能描述: 查询 arp 处理函数
** 输　入  : pArpInfo   arp 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpShowDeal (__PARP_HANDLE  pArpInfo)
{
    __PARP_HANDLE  pArpArrayHeader = LW_NULL;
    INT            iRecordNum      = 0;

    pArpArrayHeader = arpGetFromDB(&iRecordNum);                        /*  获取数据库数据              */
    arpShellCheckStatus(pArpArrayHeader, iRecordNum);                   /*  测试数据库数据有效性        */

    replyQueryMsg(OPENRT_GET_ARP_CMD, sizeof(__ARP_HANDLE), iRecordNum , pArpArrayHeader);

    if (NULL != pArpArrayHeader) {
        free(pArpArrayHeader);
    }
}
/*********************************************************************************************************
** 函数名称: arpDynamicShowDeal
** 功能描述: 查询动态 arp 处理函数
** 输　入  : pArpInfo   arp 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpDynamicShowDeal (__PARP_HANDLE  pArpInfo)
{
    __PARP_HANDLE   pArpArrayHeader = LW_NULL;
    INT             iRecordNum      = 0;

    pArpArrayHeader = arpDynamicShellGet(&iRecordNum);                  /*  获取数据库数据              */

    replyQueryMsg(OPENRT_GET_ARP_CMD, sizeof(__ARP_HANDLE), iRecordNum , pArpArrayHeader);

    if (NULL != pArpArrayHeader) {
        free(pArpArrayHeader);
    }
}
/*********************************************************************************************************
** 函数名称: arpHandleEntry
** 功能描述: arp 配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_ARP_CMD:                                            /*  配置 ARP                    */
        arpConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_ARP_CMD:                                            /*  获取 ARP 信息               */
        arpShowDeal(pArgBuf);
        break;

    case OPENRT_GET_ARP_DYNAMIC_CMD:                                    /*  获取动态 ARP 信息           */
        arpDynamicShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}

