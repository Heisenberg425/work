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
** ��   ��   ��: qos_common.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 15 ��
**
** ��        ��: QoS��Ϣͷ�ļ�
*********************************************************************************************************/
#ifndef __QOS_COMMON_H
#define __QOS_COMMON_H

#include "libqos_config.h"

/*********************************************************************************************************
  QoS ����Shell����
*********************************************************************************************************/
#define QOS_SHELL_CFG_ADD         "qosruleadd"                          /*  ���qos����                 */
#define QOS_SHELL_CFG_DEL         "qosruledel"                          /*  ɾ��qos����                 */
#define QOS_SHELL_CFG_SHOW        "qoss"                                /*  ��ʾqos����                 */

/*********************************************************************************************************
  Qos ���ݷ�Χ
*********************************************************************************************************/
#define QOS_CFG_PRIORITY_MAX      (7)                                   /*  ������ȼ�                  */
#define QOS_CFG_PRIORITY_MIN      (0)                                   /*  ������ȼ�                  */
#define QOS_CFG_RULE_IP           "ip"                                  /*  IP ����                     */
#define QOS_CFG_RULE_TCP          "tcp"                                 /*  TCP ����                    */
#define QOS_CFG_RULE_UDP          "udp"                                 /*  UDP ����                    */
#define QOS_CFG_CMPMETHOD_SRC     "SRC"                                 /*  SRC ����                    */
#define QOS_CFG_CMPMETHOD_DEST    "DEST"                                /*  DEST ����                   */
#define QOS_CFG_CMPMETHOD_BOTH    "BOTH"                                /*  BOTH                        */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    qosIsExists (__PQOS_HANDLE  pQosInfo);
PVOID  qosGetFromDB (INT*  piRecordNum);
INT    qosConfigToDb (__PQOS_HANDLE  pQosInfo);
INT    qosDeleteFromDb (__PQOS_HANDLE  pQosInfo);
INT    qosDoConfig (__PQOS_HANDLE  pQosInfo);
INT    qosDoDel (__PQOS_HANDLE  pQosInfo);
VOID   qosHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT    qosTableCreate (VOID);
INT    qosRecovery (VOID);

#endif                                                                  /*  __QOS_COMMON_H_             */
