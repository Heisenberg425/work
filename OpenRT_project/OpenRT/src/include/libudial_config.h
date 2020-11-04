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
** ��   ��   ��: libudial_config.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 23 ��
**
** ��        ��: ������Ϣ����ͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBUDIAL_CONFIG_H
#define __LIBUDIAL_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  ���ڶ�λ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_UDIAL_START         (OPEN_RT_UDAIL_CONFIG_BASE | 1)      /*  ��������                    */
#define OPENRT_UDIAL_START_ACK     (OPEN_RT_UDAIL_CONFIG_BASE | 2)      /*  �������� Ӧ��               */

#define OPENRT_UDIAL_DEL           (OPEN_RT_UDAIL_CONFIG_BASE | 3)      /*  ɾ������                    */
#define OPENRT_UDIAL_DEL_ACK       (OPEN_RT_UDAIL_CONFIG_BASE | 4)      /*  ɾ������ Ӧ��               */

#define OPENRT_UDIAL_INFO          (OPEN_RT_UDAIL_CONFIG_BASE | 5)      /*  ��ȡ������Ϣ                */
#define OPENRT_UDIAL_INFO_ACK      (OPEN_RT_UDAIL_CONFIG_BASE | 6)      /*  ��ȡ������Ϣ Ӧ��           */
/*********************************************************************************************************
 �������ò�����Ϣ�Ľṹ
*********************************************************************************************************/
struct udial_handle {
    CHAR               cUsrname[20];                                    /*  �����˺�                    */
    CHAR               cPassword[20];                                   /*  ��������                    */
    CHAR               cNetifname[NETIF_NAMESIZE];                      /*  ��������������              */
    CHAR               cPppname[NETIF_NAMESIZE];                        /*  ���� PPP ������             */
    INT                iUdialStatus;                                    /*  ����״̬                    */
    UINT32             uiPid;                                           /*  ���Ž��� PID                */
};
typedef struct udial_handle   __UDIAL_HANDLE;
typedef struct udial_handle  *__PUDIAL_HANDLE;

#endif                                                                  /*  __LIBUDIAL_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
