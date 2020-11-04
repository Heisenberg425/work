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
** ��   ��   ��: pingEntry.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ PING �������
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
#include "ping_common.h"

/*********************************************************************************************************
** ��������: pingTestDeal
** ��������: PING ���Դ�����
** �䡡��  : pPingInfo  ping ���Բ�����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  pingTestDeal (__PPING_HANDLE  pPingInfo)
{
    PCHAR  pcPingResult = NULL;

    pcPingResult = pingTestDoDeal(pPingInfo);
    if (NULL != pcPingResult) {
        replyQueryMsg(OPENRT_PING_TEST_ACK_CMD, PING_TEST_RESULT_LEN, 1, pcPingResult);

        free(pcPingResult);

        pcPingResult = NULL;
    }

    return;
}
/*********************************************************************************************************
** ��������: pingHandleEntry
** ��������: PING ���������������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID pingHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_PING_TEST_CMD:
        pingTestDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
