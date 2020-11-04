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
** ��   ��   ��: dhcpServerEntry.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 19 ��
**
** ��        ��: DHCP Server���ô������
** 2018.09.20    DHCP SERVER ֧���ڶ������豸���ж��������
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
#include "dhcpserver_common.h"

/*********************************************************************************************************
** ��������: dhcpServerConfigDeal
** ��������: ����DHCP Server������
** �䡡��  : pDhcpServerHandle  DHCP Server������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpServerConfigDeal (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    replyConfigMsg(OPENRT_CFG_DHCPSERVER_ACK_CMD, dhcpServerDoConfig(pDhcpServerInfo));
}
/*********************************************************************************************************
** ��������: dhcpServerShowDeal
** ��������: ��ѯDHCP Server������
** �䡡��  : pDhcpServerHandle  DHCP Server ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpServerShowDeal (__PDHCPSERVER_HANDLE  pDhcpServerHandle)
{
    INT                   iNum = 0;
    __PDHCPSERVER_HANDLE  pDhcpServerInfo = LW_NULL;

    pDhcpServerInfo = dhcpServerDoShow(&iNum);                          /*  ��ȡ���ݿ�����              */

    replyQueryMsg(OPENRT_GET_DHCPSERVER_ACK_CMD, sizeof(__DHCPSERVER_HANDLE), iNum, pDhcpServerInfo);

    free(pDhcpServerInfo);
}
/*********************************************************************************************************
** ��������: dhcpServerDelDeal
** ��������: ɾ�� DHCP Server������
** �䡡��  : pDhcpServerHandle  DHCP Server������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpServerDelDeal (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    replyConfigMsg(OPENRT_DEL_DHCPSERVER_ACK_CMD, dhcpServerDoDel(pDhcpServerInfo));
}
/*********************************************************************************************************
** ��������: dhcpServerHandleEntry
** ��������: DHCP Server�������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpServerHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_DHCPSERVER_CMD:
        dhcpServerConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_DHCPSERVER_CMD:
        dhcpServerShowDeal(pArgBuf);
        break;

    case OPENRT_DEL_DHCPSERVER_CMD:
        dhcpServerDelDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
