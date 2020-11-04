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
** ��   ��   ��: natDB.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ��Ŀ¼���ݿ⹫��ͷ�ļ�
*********************************************************************************************************/

#ifndef __NAT_DB_H
#define __NAT_DB_H

#include "nat_common.h"
/*********************************************************************************************************
  NAT ���ݿ⺯������
*********************************************************************************************************/
INT    natTableCreate (VOID);
INT    natLanUpdateToDB (VOID);
INT    natWanUpdateToDB (VOID);
INT    natMapUpdateToDB (VOID);
INT    natAliasUpdateToDB (VOID);
INT    natIpFragUpdateToDB (VOID);
INT    natLanGetFromDB (__PLAN_INFO   pLanInfo);
INT    natWanGetFromDB (__PWAN_INFO   pWanInfo);
PVOID  natMapGetDBInfo (VOID);
INT    natMapGetDBRow (VOID);
PVOID  natAliasGetDBInfo (VOID);
INT    natAliasGetDBRow (VOID);
INT    natIpFragGetFromDB (VOID);

#endif                                                                  /* __NAT_DB_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
