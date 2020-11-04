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
** ��   ��   ��: updatefirmware_common.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 3 ��
**
** ��        ��: update firmwareͷ�ļ�
*********************************************************************************************************/
#ifndef __UPDATEFIRMWARE_COMMON_H
#define __UPDATEFIRMWARE_COMMON_H

#include "libupdatefirmware_config.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    updateFirmwareRecovery (VOID);
INT    updateFirmwareDoConfig (VOID);
VOID   updateFirmwareHandleEntry (INT  iCommand, PVOID  pvArg);

#endif                                                                  /*  __UPDATEFIRMWARE_COMMON_H   */
