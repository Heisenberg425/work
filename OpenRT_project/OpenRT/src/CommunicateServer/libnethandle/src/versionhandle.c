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
** ��   ��   ��: versionhandle.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 8 ��
**
** ��        ��:  ϵͳversion����
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "version_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: versionShow
** ��������: ��ѯϵͳ�汾
** �䡡��  : pVersionHandle     version�����Ϣ
** �䡡��  : none
** ������  : version��ѯ�������ָ��
*********************************************************************************************************/
PVOID  versionShow (__PVERSION_HANDLE  pVersionHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_VERSION_CMD, pVersionHandle, sizeof(__VERSION_HANDLE), piRecordTotal);
}
