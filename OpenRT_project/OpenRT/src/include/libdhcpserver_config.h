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
** ��   ��   ��: libdhcpserver_config.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 18 ��
**
** ��        ��: DHCP Server ��Ϣͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBDHCPSERVER_CONFIG_H
#define __LIBDHCPSERVER_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  DHCP Server ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_CFG_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 1)  /*  ���� DHCP Server            */
#define OPENRT_CFG_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 2)  /*  ���� DHCP Server Ӧ��       */

#define OPENRT_GET_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 3)  /*  ��ȡ DHCP Server ��Ϣ       */
#define OPENRT_GET_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 4)  /*  ��ȡ DHCP Server ��ϢӦ��   */

#define OPENRT_DEL_DHCPSERVER_CMD      (OPEN_RT_DHCPSER_CONF_BASE | 5)  /*  ɾ�� DHCP Server ��Ϣ       */
#define OPENRT_DEL_DHCPSERVER_ACK_CMD  (OPEN_RT_DHCPSER_CONF_BASE | 6)  /*  ɾ�� DHCP Server ��ϢӦ��   */
/*********************************************************************************************************
 ���� DHCP Server ��Ϣ�Ľṹ
*********************************************************************************************************/
struct dhcpserver_handle {
    INT                 iEnable;                                        /*  ����״̬��1 ���� 0 ��ֹ     */
    in_addr_t           uiIpStart;                                      /*  ��ַ�ؿ�ʼ��ַ              */
    in_addr_t           uiIpEnd;                                        /*  ��ַ�ؽ�����ַ              */
    CHAR                cInterface[NETIF_NAMESIZE];                     /*  ����ʹ�õ���������          */
    in_addr_t           uiIpDns1;                                       /*  �� DNS                      */
    in_addr_t           uiIpDns2;                                       /*  ���� DSN                    */
    in_addr_t           uiIpMask;                                       /*  ����                        */
    in_addr_t           uiIpGateway;                                    /*  ����                        */
    UINT                uiLease;                                        /*  ���� (s)                    */
};
typedef struct dhcpserver_handle   __DHCPSERVER_HANDLE;
typedef struct dhcpserver_handle  *__PDHCPSERVER_HANDLE;

#endif                                                                  /* __LIBDHCPSERVER_CONFIG_H     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
