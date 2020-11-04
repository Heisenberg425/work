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
** ��   ��   ��: updatefirmwareEntry.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 3 ��
**
** ��        ��: update firmware �������
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
#include "updatefirmware_common.h"

/*********************************************************************************************************
** ��������: updateFirmwareConfigDeal
** ��������: ���¹̼�������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  updateFirmwareConfigDeal (VOID)
{
    replyConfigMsg(OPENRT_CFG_UPDATEFIRMWARE_CMD, updateFirmwareDoConfig());
}
/*********************************************************************************************************
** ��������: updateFirmwareHandleEntry
** ��������: update firmware �������
** �䡡��  : iCommand           ��������
**           pvArg              ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID updateFirmwareHandleEntry (INT  iCommand, PVOID  pvArg)
{
    switch (iCommand) {
    case OPENRT_CFG_UPDATEFIRMWARE_CMD:
        updateFirmwareConfigDeal();
        break;

    default:
        break;
    }
}

