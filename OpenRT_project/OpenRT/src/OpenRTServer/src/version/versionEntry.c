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
** ��   ��   ��: versionEntry.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 8 ��
**
** ��        ��: version�������
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
#include "version_common.h"

/*********************************************************************************************************
** ��������: versionShowDeal
** ��������: ��ѯversion������
** �䡡��  : pVersionHandle     version��Ϣ
** �䡡��  : none
** ������  : none
*********************************************************************************************************/
VOID  versionShowDeal (__PVERSION_HANDLE  pVersionHandle)
{
    __VERSION_HANDLE  versionHeader;

    memset(versionHeader.cVer, 0, sizeof(versionHeader.cVer));
    strncpy(versionHeader.cVer, OPENRT_VERSION, sizeof(versionHeader.cVer));

    replyQueryMsg(OPENRT_GET_VERSION_CMD, sizeof(__VERSION_HANDLE), 1 , &versionHeader);
}

/*********************************************************************************************************
** ��������: versionHandleEntry
** ��������: version ���
** �䡡��  : iCommand           ��������
**           pvArg              ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID versionHandleEntry (INT  iCommand, PVOID  pvArg)
{
    switch (iCommand) {
    case OPENRT_GET_VERSION_CMD:
        versionShowDeal(pvArg);
        break;

    default:
        break;
    }
}

