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
** ��   ��   ��: common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 11 ��
**
** ��        ��: ��ع��� common ͷ�ļ�
*********************************************************************************************************/
#ifndef __TRAFFICSTATS_H
#define __TRAFFICSTATS_H

#include <time.h>
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"
/*********************************************************************************************************
  �����غ궨��
*********************************************************************************************************/
#define TMT_TERM                     (1)                                /*  IP (�ն�)                   */
#define TMT_APP                      (2)                                /*  APP (Ӧ��)                  */
#define TMT_LINE                     (3)                                /*  net interface (��·)        */
#define TMT_LINE_APP                 (4)                                /*  net interface (Ӧ��)        */
#define TMT_WAN_TOTAL                (5)                                /*  WAN ��������                */
#define TMT_ONLINE_USERS             (6)                                /*  �����û���                  */
#define TMT_CPU_MEM                  (7)                                /*  CPU/MEM ռ����ͳ��          */

#define TMF_IP                       (1)                                /*  IP                          */
#define TMF_MAC                      (2)                                /*  MAC                         */
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define STAT_ITEM_PARAM_INVALID      (1)                                /*  ������Ч                    */
#define STAT_ITEM_SHOULD_DELETE      (2)                                /*  Ӧɾ��                      */
#define STAT_ITEM_HEADER_CHANGED     (3)                                /*  ͷ���ı�                    */
#define STAT_ITEM_IP_CHANGED         (4)                                /*  IP �ı�                     */
#define STAT_ITEM_HEADER_DELETE      (5)                                /*  ɾ��ͷ��                    */
/*********************************************************************************************************
  ����ͳ�����ں궨��
*********************************************************************************************************/
#define TM_CALC_PERIOD               (2)                                /*  ����ͳ������Ϊ 2s           */
#define TM_STAT_PERIOD               (TM_CALC_PERIOD * LW_TICK_HZ)      /*  ʱ������                    */
#define TM_STAT_COUNT_RESET_PERIOD   (LW_OPTION_ONE_HOUR * 24 * 7)      /*  ͳ�Ƽ����������Ϊ1��       */
/*********************************************************************************************************
  ���ڼ����Ϣ����� web ǰ�˵Ľṹ
*********************************************************************************************************/
struct tm_stats {
    UCHAR           cType;                                              /* TERM(�ն�),APP(Ӧ��),(��·)  */
    UCHAR           cEnable;                                            /* 1: ͳ����ʹ�� 0: ͳ����ȥʹ��*/
    UCHAR           cProtocol;                                          /* TCP / UDP                    */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* Ӧ��֪���˿ں�               */
    UINT16          usConns;                                            /* TCP������                    */
    CHAR            pIpAddr[32];                                        /* IP��ַ                       */
    CHAR            pMac[32];                                           /* MAC��ַ                      */
    CHAR            pAlias[32];                                         /* ����                         */
    CHAR            pIfname[16];                                        /* ��������                     */
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
  ����������ṹ
*********************************************************************************************************/
struct tm_stats_inner {
    UCHAR           cType;                                              /* TERM(�ն�),APP(Ӧ��),(��·)  */
    UCHAR           cEnable;                                            /* 1: ͳ����ʹ�� 0: ͳ����ȥʹ��*/
    UCHAR           cProtocol;                                          /* TCP / UDP                    */
    UCHAR           cPad1;
    UINT16          usPort;                                             /* Ӧ��֪���˿ں�               */
    UINT16          usConns;                                            /* TCP ������                   */
    ip4_addr_t      stIpAddr;                                           /* IP��ַ                       */
    UINT8           pMac[6];                                            /* MAC��ַ                      */
    CHAR            pAlias[32];                                         /* ����                         */
    CHAR            pIfname[16];                                        /* ��������                     */
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
    INT64           iJoinTimestamp;                                     /* ͳ�������ʱ��               */
};
typedef struct tm_stats_inner  TM_STATS_INNER;
typedef struct tm_stats_inner  *PTM_STATS_INNER;
/*********************************************************************************************************
  hash �ڵ��Ա�ṹ
*********************************************************************************************************/
struct tm_stat_entry {
    LW_LIST_LINE    tmlist;                                             /*  ����                        */
    TM_STATS_INNER  tm_stats;                                           /* tm stats                     */
};

typedef struct tm_stat_entry  TM_STAT_ENTRY;
typedef struct tm_stat_entry*  PTM_STAT_ENTRY;
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
extern LW_OBJECT_HANDLE     _G_ulTmStatTimer;                           /*  ����ͳ�ƶ�ʱ��              */
extern LW_OBJECT_HANDLE     _G_ulTmStatCountResetTimer;                 /*  ͳ���������ʱ��            */
/*********************************************************************************************************
  ·������Ӧ��Э�����ͷ���
*********************************************************************************************************/
enum app_type {
    APP_TYPE_HTTP = 1,                                                  /* HTTP��                       */
    APP_TYPE_MAIL = 2,                                                  /* �ʼ�������                   */
    APP_TYPE_CHAT = 3,                                                  /* ����ͨ����                   */
    APP_TYPE_TRANSFER = 4,                                              /* �ļ�������                   */
    APP_TYPE_OTHERS = 5,                                                /* ������                       */
    APP_TYPE_MAX = 6
};

#endif                                                                  /* __TRAFFICSTATS_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
