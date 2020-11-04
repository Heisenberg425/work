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
** ��   ��   ��: tmStatItem.h
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 11 ��
**
** ��        ��: OpenRT ������ع���ͳ����ͷ�ļ�
*********************************************************************************************************/

#ifndef __TMSTATITEM_H
#define __TMSTATITEM_H

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT   tmStatTermUserAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatAppAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, INT  iProtocol, UINT16  usPort);
INT   tmStatLineAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatItemAdd (INT  iArgC, PCHAR  *ppcArgV);
INT   tmStatItemDelete (INT  iArgC, PCHAR  *ppcArgV);
INT   tmStatAllLinesInstall (PCHAR  pcIfName);
VOID  tmStatLineAllAppsInstall (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatTermUserAutoInstall (VOID);
INT   tmStatItemAllTermRemove (VOID);
INT   tmStatAllLinesIpUpdate (PCHAR  pcIfName, ip4_addr_t  *pip4Addr);

#endif                                                                  /* __TMSTATITEM_H               */
