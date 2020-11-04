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
** ��   ��   ��: tmStat.h
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 11 ��
**
** ��        ��: OpenRT ������ع���ͳ��ͷ�ļ�
*********************************************************************************************************/
#ifndef __TMSTAT_H
#define __TMSTAT_H

#include "common.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT       tmStatsStart (INT            iIpType,
                        INT            iHookType,
                        struct pbuf   *pBuf,
                        struct netif  *pinp,
                        struct netif  *poutp);
VOID      tmShowStats (INT  iType, ip4_addr_t* pip4Addr);
INT       tmStartStats (VOID);
INT       tmStatItemCount (PTM_STATS  pCondition, INT* piFilter);
PTM_STATS tmStatItemsCopy (PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
PTM_STATS tmStatItemsSumCopy (PTM_STATS*  ppTmStat, PTM_STATS pCondition);
VOID      tmWalkStats (INT    (*callback)(),
                       PVOID    pvArg0,
                       PVOID    pvArg1,
                       PVOID    pvArg2);
VOID      tmNotifyWanChange (PVOID  pvWan);
VOID      tmNotifyLanChange (PVOID  pvLan);

#endif                                                                  /* __TMSTAT_H                   */
