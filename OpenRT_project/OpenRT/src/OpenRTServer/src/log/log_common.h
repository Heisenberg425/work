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
** ��   ��   ��: interfaceExec.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ��Ŀ¼������������ͷ�ļ�
*********************************************************************************************************/

#ifndef __LOG_COMMON_H
#define __LOG_COMMON_H

#include "liblog_config.h"
/*********************************************************************************************************
  ��־��غ궨��
*********************************************************************************************************/
#define MAX_RECORD_NUM            (3000)                                /*  ��־����¼����            */
/*********************************************************************************************************
  ��־���ݿ�����
*********************************************************************************************************/
enum LOG_TYPE {
    LOG_TYPE_UDIAL,
    LOG_TYPE_VPN,
};

/*********************************************************************************************************
  ��־���ݿ������
*********************************************************************************************************/
#define OPER_LOG_TABLE            "log_oper"                            /*  ������־                    */
#define UDIAL_LOG_TABLE           "log_udial"                           /*  ���ű���                  */
#define VPN_LOG_TABLE             "log_vpn"                             /*  VPN ����                  */
/*********************************************************************************************************
  ��־���ݿ�ͨ�ò�������
*********************************************************************************************************/
time_t logTimeGen (PCHAR  pcTimeBuf);
VOID   logHandleEntry (INT  iCommand, PVOID  pArgBuf);
VOID   logTableDelet (INT32  iLogTableType);
INT32  logGetSpecInfoCount (INT32   iLogTableType,
                            time_t  logStartTm,
                            time_t  logEndTm,
                            PCHAR   pcOtherSpec);
/*********************************************************************************************************
  ������־���ݿ��������
*********************************************************************************************************/
INT    udialLogTableCreate(VOID);
VOID   udialLogInsert (PCHAR  pcNetifName, PCHAR  pcEventInfo);
INT    udialLogDel (VOID);
INT32  udialLogNumGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle);
INT32  udialLogGetSpecInfo (time_t      logStartTm,
                            time_t      logEndTm,
                            INT64       llStartIndex,
                            INT64       llEndIndex,
                            PVOID       pvLogBuff,
                            INT32       iBuffLen);
/*********************************************************************************************************
  VPN ��־���ݿ��������
*********************************************************************************************************/
INT    vpnLogTableCreate (VOID);
VOID   vpnLogInsert (INT32  iVndID, CPCHAR  cpcLogInfo);
INT    vpnLogDel (VOID);
INT32  vpnLogNumGet (__PVPN_LOG_HANDLE    pVpnLogHandle);
INT32  vpnLogGetSpecInfo (time_t      logStartTm,
                          time_t      logEndTm,
                          INT32       iVndID,
                          INT64       llStartIndex,
                          INT64       llEndIndex,
                          PVOID       pvLogBuff,
                          INT32       iBuffLen);

/*********************************************************************************************************
  OpenRT ������־���ݿ⺯��
*********************************************************************************************************/
INT    operLogTableCreate (VOID);
PVOID  operLogGetFromDB (__POPER_LOG pLoginRecord, INT* piRecordNum);
INT    operLog (__POPER_LOG pOperLog);
INT    operLogDel (VOID);

#endif /* SRC_LOG_LOG_COMMON_H_ */
