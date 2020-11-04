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
** ��   ��   ��: flowctlhandle.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 29 ��
**
** ��        ��:  �������ô���
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "flowctl_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: flowctlInfoFree
** ��������: �ͷ���������
** �䡡��  : pFlowctlArrayTableHead           ������Ϣ����
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  flowctlInfoFree (PVOID pFlowctlArrayTableHead)
{
    if (pFlowctlArrayTableHead) {
        free(pFlowctlArrayTableHead);
    }

    return;
}

/*********************************************************************************************************
** ��������: flowctlShow
** ��������: ��ѯ������Ϣ
** �䡡��  : pFlowctlHandle    ���������Ϣ
** �䡡��  : piRecordTotal     ���ؼ�¼��ѯ�������
** ������  : ���ز�ѯ�������ָ��
*********************************************************************************************************/
PVOID  flowctlShow (__PFLOWCTL_HANDLE  pFlowctlHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_FLOWCTL_CMD, pFlowctlHandle, sizeof(__FLOWCTL_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: flowctlConfig
** ��������: ��������
** �䡡��  : pFlowctlHandle      ���������Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  flowctlConfig (__PFLOWCTL_HANDLE pFlowctlHandle)
{
   return  requestConfigMsg (OPENRT_CFG_FLOWCTL_CMD, pFlowctlHandle, sizeof(__FLOWCTL_HANDLE));
}
