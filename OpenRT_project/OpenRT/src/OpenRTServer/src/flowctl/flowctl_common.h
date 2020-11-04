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
** ��   ��   ��: flowctl_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 29 ��
**
** ��        ��: flowctl ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __FLOWCTL_COMMON_H
#define __FLOWCTL_COMMON_H

#include "libflowctl_config.h"

/*********************************************************************************************************
  ������غ궨��
*********************************************************************************************************/
#define FLOWCTL_SHOW      "flowctl"                                     /* ���ز�ѯ                     */
#define FLOWCTL_ADD       "add"                                         /* ����������Ŀ                 */
#define FLOWCTL_DELETE    "del"                                         /* ɾ��������Ŀ                 */
#define FLOWCTL_CHANGE    "chg"                                         /* �޸�������Ŀ                 */

#define FC_PROTO_TCP      "tcp"                                         /* �������ƶ�TCPЭ����Ч        */
#define FC_PROTO_UDP      "udp"                                         /* �������ƶ�UDPЭ����Ч        */
#define FC_PROTO_IP       "all"                                         /* ������Э�����ƶ���Ч         */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    flowctlDoConfig(__PFLOWCTL_HANDLE   pFlowctlInfo);
PVOID  flowctlGetFromDB(__PFLOWCTL_HANDLE  pFlowctlHandle, INT  *piRecordNum);
INT    flowctlUpdateToDb(CPCHAR  cpcOperType, __PFLOWCTL_HANDLE  pFlowctlInfo);
VOID   flowctlHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    flowctlTableCreate(VOID);
INT    flowctlRecovery(VOID);

#endif                                                                  /*  __FLOWCTL_COMMON_H          */
