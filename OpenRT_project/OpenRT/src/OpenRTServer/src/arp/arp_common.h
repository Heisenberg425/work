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
** ��   ��   ��: arp_common.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: arp��Ϣͷ�ļ�
*********************************************************************************************************/
#ifndef __ARP_COMMON_H
#define __ARP_COMMON_H

#include "libarp_config.h"

/*********************************************************************************************************
  arp ����ṹ
*********************************************************************************************************/
struct arp_handle_node {
    __ARP_HANDLE               handle;
    struct arp_handle_node    *next;
};
typedef struct arp_handle_node   __ARP_HANDLE_NODE;
typedef struct arp_handle_node  *__PARP_HANDLE_NODE;
/*********************************************************************************************************
  arp ��غ궨��
*********************************************************************************************************/
#define ARP_SHOW                   "arp"                                /* ��ѯarp��Ϣ                  */
#define ARP_SET                    "set"                                /* ���ӻ��޸�arp��Ϣ            */
#define ARP_DELETE                 "clear"                              /* ɾ��arp��Ϣ                  */
#define ARP_UPDATE                 "update"                             /* ����arp��Ϣ                  */

#define ARP_STATUS_CONFIGURED      "configured"                         /* ��Ч����                     */
#define ARP_STATUS_INVALID         "invalid"                            /* ��Ч����                     */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
PCHAR  arpTypeTransform (__PARP_HANDLE  pArpInfo, PCHAR  pcMACData, UINT  uiMACSize );
VOID   arpShellCheckStatus (__PARP_HANDLE  pArpArrayHeader, INT  iRecordNum);
PVOID  arpGetFromDB (INT*  piRecordNum);
INT    arpDeleteItemByIp (__PARP_HANDLE  pArpInfo);
PVOID  arpDynamicShellGet (INT*  iRecordNum);
INT    arpDoConfig (__PARP_HANDLE  pArpInfo);
INT    arpUpdateToDb (CPCHAR  operType, __PARP_HANDLE  pArpInfo);
VOID   arpHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT    arpTableCreate (VOID);
INT    arpRecovery (VOID);

#endif                                                                  /*  __ARP_COMMON_H              */
