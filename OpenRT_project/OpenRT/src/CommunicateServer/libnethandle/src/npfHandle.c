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
** 文   件   名: npfHandle.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 25 日
**
** 描        述:  NPF管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "npf_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: npfInfoFree
** 功能描述: 释放 NPF 信息数组
** 输　入  : pNpfArrayHead  NPF信息数组头
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  npfInfoFree (PVOID  pNpfArrayHead)
{
    if (NULL != pNpfArrayHead) {
        free(pNpfArrayHead);
    }
}
/*********************************************************************************************************
** 函数名称: npfShow
** 功能描述: 查询 NPF 信息
** 输　入  : pNpfHandle     NPF 入参信息
** 输　出  : piRecordTotal  NPF 记录查询结果总数
** 返　回  : NPF 查询结果数组指针
*********************************************************************************************************/
PVOID  npfShow (__PNPF_HANDLE  pNpfHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_NPF_CMD, pNpfHandle, sizeof(__NPF_HANDLE), piRecordTotal);
}
/*********************************************************************************************************
** 函数名称: npfConfig
** 功能描述: 配置 NPF
** 输　入  : pNpfHandle  NPF 入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  npfConfig (__PNPF_HANDLE  pNpfHandle)
{
    return  requestConfigMsg(OPENRT_CFG_NPF_CMD, (PVOID)pNpfHandle, sizeof(__NPF_HANDLE));
}
