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
** ��   ��   ��: dhcpEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT DHCP�������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "dhcpExec.h"
#include "libdhcp_config.h"
/*********************************************************************************************************
** ��������: __netifDhcpClientHandle
** ��������: ���� DHCP �ͻ��˲������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __netifDhcpClientHandle (PVOID  pArgBuf)
{
    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_DHCPC_HANDLE_ACK, PX_ERROR);

        return;
    }

    replyConfigMsg(OPENRT_DHCPC_HANDLE_ACK, netIfDhcpClient((__PDHCP_CLIENT_SET)pArgBuf));
}

/*********************************************************************************************************
** ��������: dhcpHandleEntry
** ��������: DHCP �������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID dhcpHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_DHCPC_HANDLE:
        __netifDhcpClientHandle(pArgBuf);
        break;

    default:
        break;
    }
}
