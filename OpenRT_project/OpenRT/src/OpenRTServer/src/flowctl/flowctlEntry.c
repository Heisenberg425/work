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
** ��   ��   ��: flowctlEntry.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 29 ��
**
** ��        ��: OpenRT �������ô������
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
#include "flowctl_common.h"
/*********************************************************************************************************
** ��������: flowctlConfigDeal
** ��������: �������ش�����
** �䡡��  : pFlowctlHandle     flowctl������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  flowctlConfigDeal (__PFLOWCTL_HANDLE  pFlowctlHandle)
{
    replyConfigMsg(OPENRT_CFG_FLOWCTL_ACK_CMD, flowctlDoConfig(pFlowctlHandle));
}

/*********************************************************************************************************
** ��������: flowctlShowDeal
** ��������: ��ѯ���ش�����
** �䡡��  : pFlowctlHandle     flowctl������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  flowctlShowDeal (__PFLOWCTL_HANDLE  pFlowctlHandle)
{
    __PFLOWCTL_HANDLE  pFlowctlHeader = LW_NULL;
    INT                iRecordNum     = 0;

    pFlowctlHeader = flowctlGetFromDB(pFlowctlHandle, &iRecordNum);

    replyQueryMsg(OPENRT_GET_FLOWCTL_ACK_CMD, sizeof(__FLOWCTL_HANDLE), iRecordNum , pFlowctlHeader);

    if (NULL != pFlowctlHeader) {
        free(pFlowctlHeader);
    }
}
/*********************************************************************************************************
** ��������: flowctlHandleEntry
** ��������: �����������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID flowctlHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_FLOWCTL_CMD:
        flowctlConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_FLOWCTL_CMD:
        flowctlShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}

