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
** ��   ��   ��: udial_common.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 23 ��
**
** ��        ��: ���Ŵ�����ͷ�ļ�
*********************************************************************************************************/
#ifndef __UDIAL_COMMON_H
#define __UDIAL_COMMON_H

#include <SylixOS.h>
#include "libudial_config.h"
/*********************************************************************************************************
  EXPORT ���Ų�����������
*********************************************************************************************************/
INT         udialInit (VOID);
INT         natWanCfg (CPCHAR  cpcWanName);
INT         udialRecovery (VOID);
INT         udialStatusInit (VOID);
INT         udialTableCreate (VOID);
VOID        udialPppNumUpdate (VOID);
INT         netIfMaxIndexGet (VOID);
VOID        udialHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT         netif_get_max_index (VOID);
/*********************************************************************************************************
  EXPORT �������ݿ⺯������
*********************************************************************************************************/
INT32  udialGetSpecInfo (__PUDIAL_HANDLE  pUdialInfo, CPCHAR  cpcField, PCHAR  pcArg, INT  *piArg);

INT    accountInfoGetFromIniFile (CPCHAR  cpcNetif, PCHAR  pcAccount, PCHAR  pcPasswd);

#endif                                                                  /*  __UDIAL_COMMON_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
