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
** ��   ��   ��: routehandle.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 22 ��
**
** ��        ��:  vlan������
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "vlan_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: vlanInfoFree
** ��������: �ͷ�vlan��Ϣ����
** �䡡��  : pVlanArrayHead           vlan��Ϣ����ͷ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vlanInfoFree (PVOID pVlanArrayHead)
{
    if (pVlanArrayHead) {
        free(pVlanArrayHead);
    }
    return;
}

/*********************************************************************************************************
** ��������: vlanShow
** ��������: ��ѯvlan��Ϣ
** �䡡��  : pVlanHandle       vlan�����Ϣ
** �䡡��  : piRecordTotal     vlan��¼��ѯ�������
** ������  : vlan��ѯ�������ָ��
*********************************************************************************************************/
PVOID  vlanShow (__PVLAN_HANDLE  pVlanHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_VLAN_CMD, pVlanHandle, sizeof(__VLAN_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: vlanConfig
** ��������: ����vlan
** �䡡��  : pVlanHandle      vlan�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  vlanConfig (__PVLAN_HANDLE pVlanHandle)
{
    return  requestConfigMsg (OPENRT_CFG_VLAN_CMD, pVlanHandle, sizeof(__VLAN_HANDLE));
}
