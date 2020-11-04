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
** ��   ��   ��: monitor_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 16 ��
**
** ��        ��: monitor��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __MONITOR_COMMON_H
#define __MONITOR_COMMON_H

#include "libmonitor_config.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT          monitorDoConfig (PTM_STATS  pMonitorInfo);
PTM_STATS    monitorDoShow(PTM_STATS     pMonitorHandle, INT   *piRecordNum);
PTM_CPU_MEM  cpuMemDoShow(PTM_STATS      pMonitorHandle, INT   *piRecordNum);
VOID         monitorHandleEntry (INT     iCommand,       PVOID  pArgBuf);
INT32        internetStatusGet (__PINTERNET_STATUS  pInternetStatus);

#endif                                                                  /* __MONITOR_COMMON_H           */
