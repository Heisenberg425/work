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
** ��   ��   ��: srouteThread.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: OpenRTvlan���ô������
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
** ��������: vlanConfigDeal
** ��������: ���� vlan ������
** �䡡��  : pVlanHandle vlan ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vlanConfigDeal (__PVLAN_HANDLE  pVlanInfo)
{
    replyConfigMsg(OPENRT_CFG_VLAN_ACK_CMD, vlanDoConfig(pVlanInfo));
}
/*********************************************************************************************************
** ��������: vlanShowDeal
** ��������: ��ѯ vlan ������
** �䡡��  : pVlanHandle vlan ������Ϣ
** �䡡��  : NONE
** ������  : NONE
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
** ��������: vlanHandleEntry
** ��������: vlan �������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
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

