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
** ��   ��   ��: libarp_config.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 11 ��
**
** ��        ��: ARP ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBARP_CONFIG_H
#define __LIBARP_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"

/*********************************************************************************************************
  ARP ����ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_CFG_ARP_CMD             (OPEN_RT_ARP_CONFIG_BASE | 1)    /*  ���� ARP                    */
#define OPENRT_CFG_ARP_ACK_CMD         (OPEN_RT_ARP_CONFIG_BASE | 2)    /*  ���� ARP Ӧ��               */

#define OPENRT_GET_ARP_CMD             (OPEN_RT_ARP_CONFIG_BASE | 3)    /*  ��ȡ ARP ��Ϣ               */
#define OPENRT_GET_ARP_ACK_CMD         (OPEN_RT_ARP_CONFIG_BASE | 4)    /*  ��ȡ ARP ��ϢӦ��           */

#define OPENRT_GET_ARP_DYNAMIC_CMD     (OPEN_RT_ARP_CONFIG_BASE | 5)    /*  ��ȡ��̬ ARP ��Ϣ           */
#define OPENRT_GET_ARP_DYNAMIC_ACK_CMD (OPEN_RT_ARP_CONFIG_BASE | 6)    /*  ��ȡ��̬ ARP ��ϢӦ��       */
/*********************************************************************************************************
 ���� ARP ��Ϣ�Ľṹ
*********************************************************************************************************/
struct arp_handle {
    in_addr_t              uiIpAddr;                                    /*  IP ��ַ                     */
    UCHAR                  ucHwAddr[6];                                 /*  MAC ��ַ                    */
    CHAR                   cOperType[64];                               /*  ��������                    */
    CHAR                   cStatus[20];                                 /*  ��ǰ״̬                    */
};
typedef struct arp_handle   __ARP_HANDLE;
typedef struct arp_handle  *__PARP_HANDLE;

#endif                                                                  /*  __LIBARP_CONFIG_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
