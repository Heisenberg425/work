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
** ��   ��   ��: intAdaptionExec.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ��Ŀ¼������������ͷ�ļ�
*********************************************************************************************************/

#ifndef __INTADAPTION_EXEC_H
#define __INTADAPTION_EXEC_H

#include "libintada_config.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  ������������
*********************************************************************************************************/
INT  intAdaptionHandle (__PINT_ADA_SET  pcIntAdaptionInfo);
INT  intAdaptionIsExist (INT  iWebIndex);
INT  intAdaptionCheck (VOID);
VOID intAdaptionClear (VOID);
INT  intAdaptionGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName);
INT  intAdaptionGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName);
INT  intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex);

#endif                                                                  /* __INTADAPTION_EXEC_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
