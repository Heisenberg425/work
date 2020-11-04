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
** ��   ��   ��: natStatus.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ��Ŀ¼��ȡ״̬��������ͷ�ļ�
*********************************************************************************************************/

#ifndef __NAT_STATUS_H
#define __NAT_STATUS_H

#include "libnat_config.h"
#include "nat_common.h"

/*********************************************************************************************************
  NAT ��ȡ��Ϣ��������
*********************************************************************************************************/
INT    natLanInfoGet (__PLAN_INFO   pLanInfo);
INT    natWanInfoGet (__PWAN_INFO   pWanInfo);
INT    natMapItemNumGet (VOID);
INT    natMapInfoGet (PVOID  pNatMapBuff, INT32  iStartIndex, INT32  iEndIndex);
INT    natIpFragGet (VOID);
INT    natAliasItemNumGet (VOID);
INT    natAliasInfoGet (PVOID  pNatAliasBuff, INT32  iStartIndex, INT32  iEndIndex);
INT    natAssNodeUseGet (__PNAT_ASSNODE_INFO  pAssNodeInfo);
#endif                                                                  /* __NAT_STATUS_H               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
