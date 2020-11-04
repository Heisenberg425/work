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
** ��   ��   ��: libvpn_config.h
**
** ��   ��   ��: Li.xiaocheng (��Т��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: OpenRT ���ù��ߺ�ǰ�˷������ļ�
*********************************************************************************************************/
#ifndef __LIBVND_CONFIG_H
#define __LIBVND_CONFIG_H

#include "lwip/ip4_addr.h"
#include "libserver_type.h"

/*********************************************************************************************************
  VPN ����ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_ADD_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 1)        /*  ��� VND                    */
#define OPENRT_ADD_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 2)        /*  ��� VND   Ӧ��             */

#define OPENRT_DEL_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 3)        /*  ɾ�� VND                    */
#define OPENRT_DEL_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 4)        /*  ɾ�� VND Ӧ��               */

#define OPENRT_GET_VND_CMD         (OPEN_RT_VND_CONFIG_BASE | 5)        /*  ��ȡ VND ��Ϣ               */
#define OPENRT_GET_VND_ACK_CMD     (OPEN_RT_VND_CONFIG_BASE | 6)        /*  ��ȡ VND ��Ϣ Ӧ��          */

#define OPENRT_GET_IFINDEX_CMD     (OPEN_RT_VND_CONFIG_BASE | 7)        /*  ��ȡ NETIF INDEX ��Ϣ       */
#define OPENRT_GET_IFINDEX_ACK_CMD (OPEN_RT_VND_CONFIG_BASE | 8)        /*  ��ȡ NETIF INDEX ��ϢӦ��   */

#define OPENRT_GET_VNDID_CMD       (OPEN_RT_VND_CONFIG_BASE | 9)        /*  ��ȡ VND ID ��Ϣ            */
#define OPENRT_GET_VNDID_ACK_CMD   (OPEN_RT_VND_CONFIG_BASE | 10)       /*  ��ȡ VND ID ��ϢӦ��        */
/*********************************************************************************************************
  vnd ��Ϣ�Ľṹ
*********************************************************************************************************/
struct vnd_handle {
    INT                iVndId;                                          /*  �������� ID                 */
    ip4_addr_t         uiIpAddr;                                        /*  IP ��ַ                     */
    ip4_addr_t         uiNetMask;                                       /*  ��������                    */
    ip4_addr_t         uiGateWay;                                       /*  ���ص�ַ                    */
    UCHAR              ucHwAddr[6];                                     /*  MAC ��ַ                    */
    INT                iMtu;                                            /*  MTU ֵ                      */
    INT                iNetIfIndex;                                     /*  ����ֵ���� vn7              */
};
typedef struct vnd_handle   __VND_HANDLE;
typedef struct vnd_handle  *__PVND_HANDLE;

#endif                                                                  /* __LIBVND_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
