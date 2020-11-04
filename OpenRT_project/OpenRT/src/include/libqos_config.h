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
** ��   ��   ��: libqos_config.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 15 ��
**
** ��        ��: QoS ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBQOS_CONFIG_H
#define __LIBQOS_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  QoS ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_CFG_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 1)    /*  ���� QoS                    */
#define OPENRT_CFG_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 2)    /*  ���� QoS Ӧ��               */

#define OPENRT_DEL_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 3)    /*  ɾ�� QoS                    */
#define OPENRT_DEL_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 4)    /*  ɾ�� QoS Ӧ��               */

#define OPENRT_GET_QOS_CMD             (OPEN_RT_QOS_CONFIG_BASE | 5)    /*  ��ȡ QoS ��Ϣ               */
#define OPENRT_GET_QOS_ACK_CMD         (OPEN_RT_QOS_CONFIG_BASE | 6)    /*  ��ȡ QoS ��ϢӦ��           */
/*********************************************************************************************************
  ����QoS��Ϣ�Ľṹ
*********************************************************************************************************/
struct qos_handle {
    in_addr_t  uiIpStart;                                               /*  ��ʼ ip                     */
    in_addr_t  uiIpEnd;                                                 /*  ��ֹ ip                     */
    UINT16     uiPortStart;                                             /*  ��ʼ port                   */
    UINT16     uiPortEnd;                                               /*  ��ֹ port                   */
    UINT8      uiPriority;                                              /*  ���ȼ�                      */
    UINT8      uiDontDrop;                                              /*  ������                      */
    CHAR       cNetif[NETIF_NAMESIZE];                                  /*  �����ӿ�                    */
    CHAR       cWebName[NETIF_NAMESIZE];                                /*  �����ӿ�                    */
    CHAR       cRule[10];                                               /*  ����                        */
    CHAR       cCmpMethod[10];                                          /*  �ȽϷ���                    */
};
typedef struct qos_handle   __QOS_HANDLE;
typedef struct qos_handle  *__PQOS_HANDLE;

#endif                                                                  /*  __LIBQOS_CONFIG_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
