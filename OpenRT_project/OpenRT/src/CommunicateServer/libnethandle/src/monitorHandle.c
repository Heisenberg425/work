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
** ��   ��   ��: monitorHandle.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 16 ��
**
** ��        ��:  �������ô���
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "monitor_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: monitorInfoFree
** ��������: �ͷż������
** �䡡��  : pMonitorArrayTableHead           ������Ϣ����
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  monitorInfoFree (PVOID pMonitorArrayTableHead)
{
    if (pMonitorArrayTableHead) {
        free(pMonitorArrayTableHead);
    }

    return;
}

/*********************************************************************************************************
** ��������: monitorShow
** ��������: ��ѯ�����Ϣ
** �䡡��  : pMonitorHandle    ��������Ϣ
** �䡡��  : piRecordTotal     ��ؼ�¼��ѯ�������
** ������  : ��ز�ѯ�������ָ��
*********************************************************************************************************/
PVOID  monitorShow (PTM_STATS  pMonitorHandle, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_GET_MONITOR_CMD, pMonitorHandle, sizeof(TM_STATS), piRecordTotal);
}

/*********************************************************************************************************
** ��������: cpuMemShow
** ��������: ��ѯCPU/MEMORY/�����û�����Ϣ
** �䡡��  : pMonitorHandle    ��������Ϣ
** �䡡��  : piRecordTotal     ��ؼ�¼��ѯ�������
** ������  : ��ز�ѯ�������ָ��
*********************************************************************************************************/
PVOID  cpuMemShow (PTM_STATS  pMonitorHandle, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_GET_CPU_MEM_CMD, pMonitorHandle, sizeof(TM_STATS), piRecordTotal);
}

/*********************************************************************************************************
** ��������: monitorConfig
** ��������: ���ü��
** �䡡��  : pMonitorHandle      ��������Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  monitorConfig (PTM_STATS  pMonitorHandle)
{
    return  requestConfigMsg(OPENRT_CFG_MONITOR_CMD, pMonitorHandle, sizeof(TM_STATS));
}

/*********************************************************************************************************
** ��������: libInternetStatusMonitor
** ��������: ��ȡ����������״̬
** �䡡��  : pMonitorHandle      ��������Ϣ
** �䡡��  : pInternetStatus     ���������ӽ��
** ������  : ERROR_CODE
*********************************************************************************************************/
INT32  libInternetStatusMonitor (__PINTERNET_STATUS  pInternetStatus)
{
    __PINTERNET_STATUS  pTmp = LW_NULL;
    INT32               iNum = 0;
    if (LW_NULL == pInternetStatus) {
        return  (PX_ERROR);
    }

    pTmp = requestQueryMsg(OPENRT_GET_INTERNET_CMD, NULL, 0, &iNum);
    if (LW_NULL == pTmp || 0 == iNum) {
        return  (PX_ERROR);
    }

    memcpy(pInternetStatus, pTmp, sizeof(__INTERNET_STATUS));

    if (NULL != pTmp) {
        free(pTmp);
    }

    return  (ERROR_NONE);
}
