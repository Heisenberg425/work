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
** ��   ��   ��: liblog_config.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 23 ��
**
** ��        ��: LOG ����ͷ�ļ�
*********************************************************************************************************/
#ifndef _LIBLOG_CONFIG_H_
#define _LIBLOG_CONFIG_H_

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"
/*********************************************************************************************************
  LOG ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_UDIAL_LOG_GET         (OPEN_RT_LOG_CONFIG_BASE | 1)      /*  ��ȡ������־                */
#define OPENRT_UDIAL_LOG_GET_ACK     (OPEN_RT_LOG_CONFIG_BASE | 2)      /*  ��ȡ������־Ӧ��            */

#define OPENRT_UDIAL_LOG_NUM         (OPEN_RT_LOG_CONFIG_BASE | 3)      /*  ��ȡ������־����            */
#define OPENRT_UDIAL_LOG_NUM_ACK     (OPEN_RT_LOG_CONFIG_BASE | 4)      /*  ��ȡ������־����Ӧ��        */

#define OPENRT_OPER_LOG_GET          (OPEN_RT_LOG_CONFIG_BASE | 5)      /*  ��ȡ������־                */
#define OPENRT_OPER_LOG_GET_ACK      (OPEN_RT_LOG_CONFIG_BASE | 6)      /*  ��ȡ������־Ӧ��            */

#define OPENRT_OPER_LOG_PUT          (OPEN_RT_LOG_CONFIG_BASE | 7)      /*  ��¼������־                */
#define OPENRT_OPER_LOG_PUT_ACK      (OPEN_RT_LOG_CONFIG_BASE | 8)      /*  ��¼������־Ӧ��            */

#define OPENRT_OPER_LOG_NUM          (OPEN_RT_LOG_CONFIG_BASE | 9)      /*  ��ȡ������־����            */
#define OPENRT_OPER_LOG_NUM_ACK      (OPEN_RT_LOG_CONFIG_BASE | 10)     /*  ��ȡ������־����Ӧ��        */

#define OPENRT_VPN_LOG_GET           (OPEN_RT_LOG_CONFIG_BASE | 11)     /*  ��ȡ VPN ��־����           */
#define OPENRT_VPN_LOG_GET_ACK       (OPEN_RT_LOG_CONFIG_BASE | 12)     /*  ��ȡ VPN ��־����Ӧ��       */

#define OPENRT_VPN_LOG_NUM           (OPEN_RT_LOG_CONFIG_BASE | 13)     /*  ��ȡ VPN ��־����           */
#define OPENRT_VPN_LOG_NUM_ACK       (OPEN_RT_LOG_CONFIG_BASE | 14)     /*  ��ȡ VPN ��־����Ӧ��       */

#define OPENRT_UDIAL_LOG_DEL         (OPEN_RT_LOG_CONFIG_BASE | 15)     /*  ���������־                */
#define OPENRT_UDIAL_LOG_DEL_ACK     (OPEN_RT_LOG_CONFIG_BASE | 16)     /*  ���������־Ӧ��            */

#define OPENRT_OPER_LOG_DEL          (OPEN_RT_LOG_CONFIG_BASE | 17)     /*  ���������־                */
#define OPENRT_OPER_LOG_DEL_ACK      (OPEN_RT_LOG_CONFIG_BASE | 18)     /*  ���������־Ӧ��            */

#define OPENRT_VPN_LOG_DEL           (OPEN_RT_LOG_CONFIG_BASE | 19)     /*  ��� VPN ��־               */
#define OPENRT_VPN_LOG_DEL_ACK       (OPEN_RT_LOG_CONFIG_BASE | 20)     /*  ��� VPN ��־Ӧ��           */
/********************************************************************************************************
  ��ȡ������־��Ϣ�ṹ
*********************************************************************************************************/
struct udial_log_handle {
    time_t      logStartTm;                                             /*  ��ʼʱ��                    */
    time_t      logEndTm;                                               /*  ����ʱ��                    */
    INT64       llStartIndex;                                           /*  ��ʼ���                    */
    INT64       llEndIndex;                                             /*  �������                    */
};
typedef struct udial_log_handle   __UDIAL_LOG_HANDLE;
typedef struct udial_log_handle  *__PUDIAL_LOG_HANDLE;
/*********************************************************************************************************
  ������־��Ϣ��غ궨��
*********************************************************************************************************/
#define EVENT_BUFF_LEN             (50)                                 /*  ÿһ���¼�buff����          */
/*********************************************************************************************************
  ������־��Ϣ���ؽṹ
*********************************************************************************************************/
struct udial_log_info {
    time_t      llTime;                                                 /*  ʱ��                        */
    CHAR        cNetifName[NETIF_NAMESIZE];                             /*  ��������                    */
    CHAR        cEvent[EVENT_BUFF_LEN];                                 /*  �¼�����                    */
};
typedef struct udial_log_info   __UDIAL_LOG_INFO;
typedef struct udial_log_info  *__PUDIAL_LOG_INFO;

/*********************************************************************************************************
  KidVPN Log ��־����
*********************************************************************************************************/
#define LOG_INFO_MAX_LEN      (512)                                     /*  ÿ����־����󳤶�          */
/********************************************************************************************************
  ��ȡ KidVPN��־��Ϣ�ṹ
*********************************************************************************************************/
struct vpn_log_handle {
    time_t      logStartTm;                                             /*  ��ʼʱ��                    */
    time_t      logEndTm;                                               /*  ����ʱ��                    */
    INT32       iVndID;                                                 /*  VND ID                      */
    INT64       llStartIndex;                                           /*  ��ʼ���                    */
    INT64       llEndIndex;                                             /*  �������                    */
};
typedef struct vpn_log_handle   __VPN_LOG_HANDLE;
typedef struct vpn_log_handle  *__PVPN_LOG_HANDLE;
/*********************************************************************************************************
  KidVPN Log ��Ϣ���ؽṹ
*********************************************************************************************************/
struct kidvpn_log_info {
    time_t      llTime;                                                 /*  ʱ��                        */
    INT32       iVndID;                                                 /*  VND ���                    */
    CHAR        cLogInfo[LOG_INFO_MAX_LEN];                             /*  Log ����                    */
};
typedef struct kidvpn_log_info   __KIDVPN_LOG_INFO;
typedef struct kidvpn_log_info  *__PKIDVPN_LOG_INFO;
/*********************************************************************************************************
  ������־��Ϣ�Ľṹ
*********************************************************************************************************/
struct oper_log {
    time_t   logStartTm;                                               /*  ��ʼʱ��                     */
    time_t   logEndTm;                                                 /*  ����ʱ��                     */
    UINT32   llStartIndex;                                             /*  ��ʼ���                     */
    UINT32   llEndIndex;                                               /*  �������                     */
    time_t   llTime;                                                   /*  ʱ��                         */
    CHAR     cClientIP[64];                                            /*  �ͻ��� IP                    */
    CHAR     cOperator[32];                                            /*  �����û�                     */
    CHAR     cModuleName[32];                                          /*  ģ������                     */
    INT      iResult;                                                  /*  �������  0:�ɹ�,1:ʧ��      */
    CHAR     cLog[128];                                                /*  ������־                     */
    UINT32   iTotal;                                                   /*  ������־��ѯ����             */
};
typedef struct oper_log   __OPER_LOG;
typedef struct oper_log  *__POPER_LOG;

#endif                                                                  /* _LIBLOG_CONFIG_H_            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
