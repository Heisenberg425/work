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
** 文   件   名: srouteThread.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: OpenRTvlan配置处理入口
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
#include "vlan_common.h"

/*********************************************************************************************************
** 函数名称: vlanConfigDeal
** 功能描述: 配置 vlan 处理函数
** 输　入  : pVlanHandle vlan 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vlanConfigDeal (__PVLAN_HANDLE  pVlanInfo)
{
    replyConfigMsg(OPENRT_CFG_VLAN_ACK_CMD, vlanDoConfig(pVlanInfo));
}
/*********************************************************************************************************
** 函数名称: vlanShowDeal
** 功能描述: 查询 vlan 处理函数
** 输　入  : pVlanHandle vlan 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vlanShowDeal (__PVLAN_HANDLE  pVlanHandle)
{
    __PVLAN_HANDLE pVlanArrayHeader = LW_NULL;
    INT            iRecordNum       = 0;

    pVlanArrayHeader = vlanGetFromDB(&iRecordNum);

    replyQueryMsg(OPENRT_GET_VLAN_ACK_CMD, sizeof(__VLAN_HANDLE), iRecordNum , pVlanArrayHeader);

    if (NULL != pVlanArrayHeader) {
        free(pVlanArrayHeader);
    }
}
/*********************************************************************************************************
** 函数名称: vlanHandleEntry
** 功能描述: vlan 配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID vlanHandleEntry (INT  iCommand, PVOID pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_VLAN_CMD:
        vlanConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_VLAN_CMD:
        vlanShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}

