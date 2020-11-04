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
** ��   ��   ��: dhcpserverhandle.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 18 ��
**
** ��        ��:  DHCP Server������
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "dhcpserver_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: dhcpServerShow
** ��������: ��ѯDHCP Server��Ϣ
** �䡡��  : pDhcpServerHandle  DHCP Server�����Ϣ
** �䡡��  : piRecordTotal      DHCP Server��¼��ѯ�������
** ������  : DHCP Server��ѯ�������ָ��
*********************************************************************************************************/
PVOID  dhcpServerShow (__PDHCPSERVER_HANDLE  pDhcpServerHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: dhcpServerConfig
** ��������: ����DHCP Server
** �䡡��  : pDhcpServerHandle  DHCP Server�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  dhcpServerConfig (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    return  requestConfigMsg (OPENRT_CFG_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE));
}
/*********************************************************************************************************
** ��������: dhcpServerDel
** ��������: ɾ�� DHCP Server
** �䡡��  : pDhcpServerHandle  DHCP Server�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT    dhcpServerDel (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    return  requestConfigMsg (OPENRT_DEL_DHCPSERVER_CMD, pDhcpServerHandle, sizeof(__DHCPSERVER_HANDLE));
}
