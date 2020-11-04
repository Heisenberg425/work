/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: npfEntry.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 24 ��
**
** ��        ��: npf ���ò������
*********************************************************************************************************/
#include <SylixOS.h>
#include "npf_common.h"

/*********************************************************************************************************
** ��������: __npfCfgRule
** ��������: ���� npf ����
** ��    ��: pNpfMsgBody  ǰ�˴���Ĳ���
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: __npfGetRule
** ��������: ��ȡnpf����
** ��    ��: pNpfMsgBody  ǰ�˴��������
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: npfHandleEntry
** ��������: npf���ò������
** ��    ��: iCommand  ��������
**           pArgBuf   ǰ�˴���Ĳ���
** ��    ��: NONE
** ��    ��: NONE
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
