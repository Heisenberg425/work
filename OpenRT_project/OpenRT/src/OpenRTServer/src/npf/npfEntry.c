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
** 文   件   名: npfEntry.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 24 日
**
** 描        述: npf 配置操作入口
*********************************************************************************************************/
#include <SylixOS.h>
#include "npf_common.h"

/*********************************************************************************************************
** 函数名称: __npfCfgRule
** 函数功能: 配置 npf 规则
** 输    入: pNpfMsgBody  前端传入的参数
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __npfCfgRule (__PNPF_HANDLE  pNpfMsgBody)
{
    if (LW_NULL == pNpfMsgBody) {
        replyConfigMsg(OPENRT_CFG_NPF_CMD, PX_ERROR);

        return ;
    }

    replyConfigMsg(OPENRT_CFG_NPF_CMD, doCfgNpfRule(pNpfMsgBody));
}

/*********************************************************************************************************
** 函数名称: __npfGetRule
** 函数功能: 获取npf规则
** 输    入: pNpfMsgBody  前端传入的数据
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __npfGetRule (__PNPF_HANDLE  pNpfMsgBody)
{
    __PNPF_HANDLE pNpfArrayHeader = LW_NULL;
    INT           iRecordNum;

    if (LW_NULL == pNpfMsgBody) {
        replyConfigMsg(OPENRT_GET_NPF_CMD, PX_ERROR);
        return ;
    }

    pNpfArrayHeader = getDBNpfRule(pNpfMsgBody, &iRecordNum, pNpfMsgBody->cWhereLimit);
    replyQueryMsg(OPENRT_GET_NPF_CMD, sizeof(__NPF_HANDLE), iRecordNum, pNpfArrayHeader);

    if (LW_NULL != pNpfArrayHeader) {
        free(pNpfArrayHeader);
    }

    return ;
}

/*********************************************************************************************************
** 函数名称: npfHandleEntry
** 函数功能: npf配置操作入口
** 输    入: iCommand  服务命令
**           pArgBuf   前端传入的参数
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
VOID  npfHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_NPF_CMD:
         __npfCfgRule((__PNPF_HANDLE)pArgBuf);
         break;

    case OPENRT_GET_NPF_CMD:
         __npfGetRule((__PNPF_HANDLE)pArgBuf);
         break;

    default:
        break;
    }

    return ;
}
