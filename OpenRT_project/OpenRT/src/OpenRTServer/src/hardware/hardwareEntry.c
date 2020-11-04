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
** ��   ��   ��: hardwareEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 2 ��
**
** ��        ��: OpenRT Ӳ����Ϣ��ȡ���
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "msgcomm.h"
#include "hardware_common.h"
/*********************************************************************************************************
** ��������: __hwInfoGet
** ��������: ��ȡӲ����Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __hwInfoGet (PVOID  pArgBuf)
{
    __HW_INFO   hwInfo;

    bzero(&hwInfo, sizeof(hwInfo));
    getHwInfo(&hwInfo);
    replyQueryMsg(OPENRT_HW_INFO_GET_ACK_CMD, sizeof(__HW_INFO), 1, &hwInfo);
}

/*********************************************************************************************************
** ��������: hwInfoHandleEntry
** ��������: Ӳ����Ϣ��ȡ���
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID hwInfoHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_HW_INFO_GET_CMD:
        __hwInfoGet(pArgBuf);
        break;

    default:
        break;
    }
}
