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
** ��   ��   ��: libmonitor.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 16 ��
**
** ��        ��: monitor ��Ϣͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBMONITOR_CONFIG_H
#define __LIBMONITOR_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  �����Ϣ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_CFG_MONITOR_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 1)  /*  ���ü��                    */
#define OPENRT_CFG_MONITOR_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 2)  /*  ���ü��Ӧ��                */

#define OPENRT_GET_MONITOR_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 3)  /*  ��ȡ�����Ϣ                */
#define OPENRT_GET_MONITOR_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 4)  /*  ��ȡ�����ϢӦ��            */

#define OPENRT_GET_CPU_MEM_CMD       (OPEN_RT_MONITOR_CONFIG_BASE | 5)  /*  ��ȡ cpu/memory ��Ϣ        */
#define OPENRT_GET_CPU_MEM_ACK_CMD   (OPEN_RT_MONITOR_CONFIG_BASE | 6)  /*  ��ȡ cpu/memory ��ϢӦ��    */

#define OPENRT_GET_INTERNET_CMD      (OPEN_RT_MONITOR_CONFIG_BASE | 7)  /*  ��ȡ����������״̬          */
#define OPENRT_GET_INTERNET_ACK_CMD  (OPEN_RT_MONITOR_CONFIG_BASE | 8)  /*  ��ȡ����������״̬Ӧ��      */
/*********************************************************************************************************
  ���ڼ����ʾ�ĺ궨��
*********************************************************************************************************/
#define TMT_TERM                     (1)                                /* ip (�ն�)                    */
#define TMT_APP                      (2)                                /* APP(Ӧ��)                    */
#define TMT_LINE                     (3)                                /* net interface (��·)         */
#define TMT_LINE_APP                 (4)                                /* net interface (Ӧ��)         */
#define TMT_WAN_TOTAL                (5)                                /* WAN ��������                 */
#define TMT_CPU_MEM_USER             (6)                                /* cpu/mem ռ���ʡ�user ͳ��    */

#define TMF_IP                       (1)                                /* IP                           */
#define TMF_MAC                      (2)                                /* MAC                          */
/*********************************************************************************************************
 ����������״̬�ֶκ�
*********************************************************************************************************/
#define INTERNET_OK                  (0)                                /*  ���ӳɹ�                    */
#define INTERNET_FAILED              (1)                                /*  ����ʧ��                    */
/*********************************************************************************************************
  App ����
*********************************************************************************************************/
enum app_type
{
    APP_TYPE_HTTP = 1,                                                  /* HTTP ��                       */
    APP_TYPE_MAIL = 2,                                                  /* �ʼ�������                   */
    APP_TYPE_CHAT = 3,                                                  /* ����ͨ����                   */
    APP_TYPE_TRANSFER = 4,                                              /* �ļ�������                   */
    APP_TYPE_OTHERS = 5,                                                /* ������                       */
    APP_TYPE_MAX = 6
};
/*********************************************************************************************************
  ��Դʹ�����
*********************************************************************************************************/
struct tm_cpu_mem {
	double      dCpuUsage;                                              /*  CPU ʹ����                  */
	double      dMemoryUsage;                                           /*  �ڴ�ʹ����                  */
	UINT32      uiOnlineUsers;                                          /*  �����û���Ŀ                */
};
typedef struct tm_cpu_mem   TM_CPU_MEM;
typedef struct tm_cpu_mem*  PTM_CPU_MEM;
/*********************************************************************************************************
  ���ڼ����Ϣ�Ľṹ
*********************************************************************************************************/
struct tm_stats {
    UCHAR           cType;                                              /* TERM (�ն�),APP (Ӧ��/��·)  */
    UCHAR           cEnable;                                            /* 1: ͳ����ʹ�� 0: ͳ����ȥʹ��*/
    UCHAR           cProtocol;                                          /* TCP/UDP                      */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* Ӧ��֪���˿ں�               */
    UINT16          usConns;                                            /* TCP ������                   */
    CHAR            pIpAddr[32];                                        /* IP ��ַ                      */
    CHAR            pMac[32];                                           /* MAC ��ַ                     */
    CHAR            pAlias[32];                                         /* ����                         */
    CHAR            pIfname[NETIF_NAMESIZE];                            /* ��������                     */
    UINT32          iDuration;                                          /* ����ʱ�� (min)               */
    UINT32          iSendRate;                                          /* �������� (B/s)               */
    UINT32          iRecvRate;                                          /* �������� (B/s)               */
    UINT32          iSendCurrLen;                                       /* ����ͳ�������ڷ��ͱ��ĳ���   */
    UINT32          iRecvCurrLen;                                       /* ����ͳ�������ڽ��ձ��ĳ���   */
    UINT64          iSendTotalLen;                                      /* ���ͱ����ܳ���(�ֽ���)       */
    UINT64          iRecvTotalLen;                                      /* ���ձ����ܳ���(�ֽ���)       */
    UINT32          iSendDrops;                                         /* ���Ͷ�����                   */
    UINT32          iRecvDrops;                                         /* ���ն�����                   */
    UINT64          iSendTotalPackets;                                  /* �����ܱ�����                 */
    UINT64          iRecvTotalPackets;                                  /* �����ܱ�����                 */
};
typedef struct tm_stats  TM_STATS;
typedef struct tm_stats  *PTM_STATS;
/*********************************************************************************************************
  ������ؽṹ
*********************************************************************************************************/
struct tm_stat_entry {
    LW_LIST_LINE    tmlist;                                             /*  �����������                */
    TM_STATS        tm_stats;                                           /*  tm stats                    */
};
typedef struct tm_stat_entry  TM_STAT_ENTRY;
typedef struct tm_stat_entry*  PTM_STAT_ENTRY;
/*********************************************************************************************************
  ����������״̬���ؽṹ
*********************************************************************************************************/
struct internet_status {
    time_t      llTime;                                                 /*  ����ʱ��                    */
    UINT8       ucStatus;                                               /*  ����״̬                    */
};
typedef struct internet_status  __INTERNET_STATUS;
typedef struct internet_status  *__PINTERNET_STATUS;

#endif                                                                  /*  __LIBMONITOR_CONFIG_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
