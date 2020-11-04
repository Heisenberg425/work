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
** 文   件   名: routehandle.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 22 日
**
** 描        述:  vlan管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "vlan_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: vlanInfoFree
** 功能描述: 释放vlan信息数组
** 输　入  : pVlanArrayHead           vlan信息数组头
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vlanInfoFree (PVOID pVlanArrayHead)
{
    if (pVlanArrayHead) {
        free(pVlanArrayHead);
    }
    return;
}

/*********************************************************************************************************
** 函数名称: vlanShow
** 功能描述: 查询vlan信息
** 输　入  : pVlanHandle       vlan入参信息
** 输　出  : piRecordTotal     vlan记录查询结果总数
** 返　回  : vlan查询结果数组指针
*********************************************************************************************************/
PVOID  vlanShow (__PVLAN_HANDLE  pVlanHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_VLAN_CMD, pVlanHandle, sizeof(__VLAN_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: vlanConfig
** 功能描述: 配置vlan
** 输　入  : pVlanHandle      vlan入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  vlanConfig (__PVLAN_HANDLE pVlanHandle)
{
    return  requestConfigMsg (OPENRT_CFG_VLAN_CMD, pVlanHandle, sizeof(__VLAN_HANDLE));
}
