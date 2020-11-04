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
** ��   ��   ��: vlan_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: vlan ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __VLAN_COMMON_H
#define __VLAN_COMMON_H

#include "libvlan_config.h"
/*********************************************************************************************************
  VLAN �����궨��
*********************************************************************************************************/
#define VLAN_SHOW      "vlan"                                           /*��ѯvlan��Ϣ                  */
#define VLAN_ADD       "set"                                            /*���ӻ��޸�vlan��Ϣ            */
#define VLAN_DELETE    "clear"                                          /*ɾ��vlan��Ϣ                  */
#define VLAN_UPDATE    "update"                                         /*����vlan��Ϣ                  */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
PVOID  vlanGetFromDB (INT* piRecordNum);
INT    vlanDoConfig (__PVLAN_HANDLE  pVlanInfo);
INT    vlanUpdateToDb (CPCHAR  operType, __PVLAN_HANDLE  pVlanInfo);
VOID   vlanHandleEntry (INT  iCommand, PVOID pArgBuf);
INT    vlanTableCreate (VOID);
INT    vlanRecovery (VOID);

#endif                                                                  /*  __VLAN_COMMON_H             */
