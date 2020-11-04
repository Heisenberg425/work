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
** ��   ��   ��: arpEntry.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: arp ���ô������
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
#include "arp_common.h"

/*********************************************************************************************************
** ��������: arpConfigDeal
** ��������: ���� arp ������
** �䡡��  : pArpHandle         arp ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpConfigDeal (__PARP_HANDLE  pArpInfo)
{
    replyConfigMsg(OPENRT_CFG_ARP_CMD, arpDoConfig(pArpInfo));
}
/*********************************************************************************************************
** ��������: arpShowDeal
** ��������: ��ѯ arp ������
** �䡡��  : pArpInfo   arp ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpShowDeal (__PARP_HANDLE  pArpInfo)
{
    __PARP_HANDLE  pArpArrayHeader = LW_NULL;
    INT            iRecordNum      = 0;

    pArpArrayHeader = arpGetFromDB(&iRecordNum);                        /*  ��ȡ���ݿ�����              */
    arpShellCheckStatus(pArpArrayHeader, iRecordNum);                   /*  �������ݿ�������Ч��        */

    replyQueryMsg(OPENRT_GET_ARP_CMD, sizeof(__ARP_HANDLE), iRecordNum , pArpArrayHeader);

    if (NULL != pArpArrayHeader) {
        free(pArpArrayHeader);
    }
}
/*********************************************************************************************************
** ��������: arpDynamicShowDeal
** ��������: ��ѯ��̬ arp ������
** �䡡��  : pArpInfo   arp ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpDynamicShowDeal (__PARP_HANDLE  pArpInfo)
{
    __PARP_HANDLE   pArpArrayHeader = LW_NULL;
    INT             iRecordNum      = 0;

    pArpArrayHeader = arpDynamicShellGet(&iRecordNum);                  /*  ��ȡ���ݿ�����              */

    replyQueryMsg(OPENRT_GET_ARP_CMD, sizeof(__ARP_HANDLE), iRecordNum , pArpArrayHeader);

    if (NULL != pArpArrayHeader) {
        free(pArpArrayHeader);
    }
}
/*********************************************************************************************************
** ��������: arpHandleEntry
** ��������: arp �������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_ARP_CMD:                                            /*  ���� ARP                    */
        arpConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_ARP_CMD:                                            /*  ��ȡ ARP ��Ϣ               */
        arpShowDeal(pArgBuf);
        break;

    case OPENRT_GET_ARP_DYNAMIC_CMD:                                    /*  ��ȡ��̬ ARP ��Ϣ           */
        arpDynamicShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}

