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
** ��   ��   ��: arphandle.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��:  arp������
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "arp_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: arpDynamicShow
** ��������: ��ѯ��̬arp��Ϣ
** �䡡��  : pArpHandle         arp�����Ϣ
** �䡡��  : piRecordTotal      arp��¼��ѯ�������
** ������  : arp��ѯ�������ָ��
*********************************************************************************************************/
PVOID  arpDynamicShow (__PARP_HANDLE  pArpHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ARP_DYNAMIC_CMD, pArpHandle, sizeof(__ARP_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: arpShow
** ��������: ��ѯarp��Ϣ
** �䡡��  : pArpHandle         arp�����Ϣ
** �䡡��  : piRecordTotal      arp��¼��ѯ�������
** ������  : arp��ѯ�������ָ��
*********************************************************************************************************/
PVOID  arpShow (__PARP_HANDLE  pArpHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ARP_CMD, pArpHandle, sizeof(__ARP_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: arpConfig
** ��������: ����arp
** �䡡��  : pArpHandle         arp�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  arpConfig (__PARP_HANDLE  pArpHandle)
{
    return  requestConfigMsg (OPENRT_CFG_ARP_CMD, pArpHandle, sizeof(__ARP_HANDLE));
}
