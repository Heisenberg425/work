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
#ifndef __LIBVPN_CONFIG_H
#define __LIBVPN_CONFIG_H

#include "lwip/ip4_addr.h"
#include "libserver_type.h"

/*********************************************************************************************************
  VPN ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_ADD_VPN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 1)        /*  ��� VPN                    */
#define OPENRT_ADD_VPN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 2)        /*  ��� VPN   Ӧ��             */

#define OPENRT_DEL_VPN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 3)        /*  ɾ�� VPN                    */
#define OPENRT_DEL_VPN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 4)        /*  ɾ�� VPN Ӧ��               */

#define OPENRT_KEY_SAVE_CMD        (OPEN_RT_VPN_CONFIG_BASE | 5)        /*  ���� AES KEY ������         */
#define OPENRT_KEY_SAVE_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 6)        /*  ���� AES KEY ������Ӧ��     */

#define OPENRT_KEY_GEN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 7)        /*  ���� AES KEY                */
#define OPENRT_KEY_GEN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 8)        /*  ���� AES KEY Ӧ��           */

#define OPENRT_KEY_GET_CMD         (OPEN_RT_VPN_CONFIG_BASE | 9)        /*  ��ȡ���� AES KEY            */
#define OPENRT_KEY_GET_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 10)       /*  ��ȡ���� AES KEY Ӧ��       */

#define OPENRT_INFO_GET_CMD        (OPEN_RT_VPN_CONFIG_BASE | 11)       /*  ��ȡ VPN ��Ϣ               */
#define OPENRT_INFO_GET_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 12)       /*  ��ȡ VPN ��Ϣ Ӧ��          */

#define OPENRT_LINK_NUM_CMD        (OPEN_RT_VPN_CONFIG_BASE | 13)       /*  ��ȡ VPN ������Ϣ��Ŀ       */
#define OPENRT_LINK_NUM_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 14)       /*  ��ȡ VPN ������Ϣ��ĿӦ��   */

#define OPENRT_LINK_GET_CMD        (OPEN_RT_VPN_CONFIG_BASE | 15)       /*  ��ȡ VPN ������Ϣ           */
#define OPENRT_LINK_GET_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 16)       /*  ��ȡ VPN ������ϢӦ��       */

#define OPENRT_POSIT_GET_CMD       (OPEN_RT_VPN_CONFIG_BASE | 17)       /*  ��ȡ VPN ������Ϣ           */
#define OPENRT_POSIT_GET_ACK_CMD   (OPEN_RT_VPN_CONFIG_BASE | 18)       /*  ��ȡ VPN ������ϢӦ��       */
/*********************************************************************************************************
  KidVPN ��Ϣ cHandleType �ֶ�
*********************************************************************************************************/
#define TYPE_ADD_VPN_S             (0)                                  /*  ��� VPN ������             */
#define TYPE_ADD_VPN_C             (1)                                  /*  ��� VPN �ͻ���             */
#define TYPE_DEL_VPN               (2)                                  /*  ɾ�� VPN                    */
/*********************************************************************************************************
  KidVPN ��Ϣ�Ľṹ
*********************************************************************************************************/
struct kidvpn_handle {
    ip4_addr_t         uiIpAddr;                                        /*  IP ��ַ                     */
    INT                iVndID;                                          /*  �������� ID                 */
    INT                iMtu;                                            /*  MTU ֵ                      */
    INT                iPort;                                           /*  �˿���Ϣ                    */
    CHAR               cHandleType;                                     /*  ��������                    */
    CHAR               cStatus;                                         /*  VPN ״̬                    */
    CHAR               cPassWd[32];                                     /*  ����                        */
    CHAR               cPos[128];                                       /*  �������˵���λ����Ϣ        */
};
typedef struct kidvpn_handle   __KIDVPN_HANDLE;
typedef struct kidvpn_handle  *__PKIDVPN_HANDLE;
/*********************************************************************************************************
  KidVPN ���ݿ���Ϣ�Ľṹ
*********************************************************************************************************/
struct kidvpn_db {
    __KIDVPN_HANDLE    kidvpnHandle;                                    /*  VPN ��Ϣ                    */
    INT                iPid;                                            /*  VPN ���� ID                 */
    CHAR               cStatus;                                         /*  VPN ״̬                    */
};
typedef struct kidvpn_db   __KIDVPN_DB_INFO;
typedef struct kidvpn_db  *__PKIDVPN_DB_INFO;
/********************************************************************************************************
  ��ȡ KidVPN ��Ϣ�ṹ
*********************************************************************************************************/
struct vpn_info_handle {
    INT64       llStartIndex;                                           /*  ��ʼ���                    */
    INT64       llEndIndex;                                             /*  �������                    */
};
typedef struct vpn_info_handle   __VPN_INFO_HANDLE;
typedef struct vpn_info_handle  *__PVPN_INFO_HANDLE;
/*********************************************************************************************************
  KidVPN ���ݿ���Ϣ�Ľṹ
*********************************************************************************************************/
struct kidvpn_link {
    INT                iVndId;                                          /*  VND ID                      */
    CHAR               cIp[32];                                         /*  ���ӿͻ��� IP               */
    CHAR               cMac[32];                                        /*  ���ӿͻ��� MAC              */
    CHAR               cPos[128];                                       /*  �ͻ��˵���λ��              */
};
typedef struct kidvpn_link   __KIDVPN_LINK;
typedef struct kidvpn_link  *__PKIDVPN_LINK;
/*********************************************************************************************************
  Aes Key ��Ϣ�Ľṹ
*********************************************************************************************************/
struct aeskey_handle {
    INT                iVndID;                                          /*  �������� ID                 */
    CHAR               cAesKey[70];                                     /*  AES KEY                     */
};
typedef struct aeskey_handle   __AES_KEY_HANDLE;
typedef struct aeskey_handle  *__PAES_KEY_HANDLE;
/*********************************************************************************************************
 ���� �ļ������Ϣ�Ľṹ
*********************************************************************************************************/
struct fd_handle {
    INT  iVndId;                                                        /*  VND ID                      */
    INT  iPipeFd[2];                                                    /*  �ܵ��ļ����                */
};
typedef struct fd_handle  __FD_HANDLE;
typedef struct fd_handle  *__PFD_HANDLE;
/*********************************************************************************************************
  �ļ��������ڵ�
*********************************************************************************************************/
struct fd_node{
    LW_LIST_LINE          fdListLine;
    __FD_HANDLE           tNodeData;                                    /*  �ڵ�����                    */
};
typedef struct fd_node   __FD_NODE;
typedef struct fd_node  *__PFD_NODE;
/*********************************************************************************************************
  kidvpn ip ����λ��ƥ����Ϣ�ṹ��
*********************************************************************************************************/
struct kidvpn_ipPos {
    CHAR            cIp[32];                                            /*  KidVPN IP ��Ϣ              */
    CHAR            cPos[128];                                          /*  KidVPN λ����Ϣ             */
};
typedef struct kidvpn_ipPos   __KIDVPN_IPPOS;
typedef struct kidvpn_ipPos  *__PKIDVPN_IPPOS;
/*********************************************************************************************************
  kidvpn �ͻ��ˡ�����˵���λ�ýṹ��
*********************************************************************************************************/
struct kidvpn_pos {
    __KIDVPN_IPPOS  vpnSrc;                                             /*  KidVPN Դ����λ��           */
    __KIDVPN_IPPOS  vpnDst;                                             /*  KidVPN Ŀ�ĵ���λ��         */
};
typedef struct kidvpn_pos   __KIDVPN_POS;
typedef struct kidvpn_pos  *__PKIDVPN_POS;

#endif                                                                  /* __LIBVPN_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
