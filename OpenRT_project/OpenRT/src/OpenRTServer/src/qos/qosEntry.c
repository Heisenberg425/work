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
** ��   ��   ��: qosEntry.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 15 ��
**
** ��        ��: QoS���ô������
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
#include "qos_common.h"

/*********************************************************************************************************
** ��������: qosConfigDeal
** ��������: ���� QoS ������
** �䡡��  : pQosInfo     QoS ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  qosConfigDeal (__PQOS_HANDLE  pQosInfo)
{
    replyConfigMsg(OPENRT_CFG_QOS_CMD, qosDoConfig(pQosInfo));
}
/*********************************************************************************************************
** ��������: qosShowDeal
** ��������: ��ѯQoS������
** �䡡��  : pQosInfo   QoS ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  qosShowDeal (__PQOS_HANDLE  pQosInfo)
{
    __PQOS_HANDLE  pQosArrayHeader = LW_NULL;
    INT            iRecordNum      = 0;

    pQosArrayHeader = qosGetFromDB(&iRecordNum);                        /*  ��ȡ����                    */

    replyQueryMsg(OPENRT_GET_QOS_CMD, sizeof(__QOS_HANDLE), iRecordNum , pQosArrayHeader);

    if (NULL != pQosArrayHeader) {
        free(pQosArrayHeader);
    }
}
/*********************************************************************************************************
** ��������: qosDelDeal
** ��������: ɾ�� QoS ������
** �䡡��  : pQosInfo     QoS ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  qosDelDeal (__PQOS_HANDLE  pQosInfo)
{
    replyConfigMsg(OPENRT_DEL_QOS_CMD, qosDoDel(pQosInfo));
}
/*********************************************************************************************************
** ��������: qosHandleEntry
** ��������: QoS�������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID qosHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_QOS_CMD:
        qosConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_QOS_CMD:
        qosShowDeal(pArgBuf);
        break;

    case OPENRT_DEL_QOS_CMD:
        qosDelDeal(pArgBuf);
        break;

    default:
        break;
    }
}
