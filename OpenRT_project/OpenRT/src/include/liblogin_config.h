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
** ��   ��   ��: liblogin_config.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��: login ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBLOGIN_CONFIG_H
#define __LIBLOGIN_CONFIG_H

#include "libserver_type.h"

/*********************************************************************************************************
  login ����ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_LOGIN_INFO_GET_CMD         (OPEN_RT_LOGIN_BASE | 1)      /*  ��¼��Ϣ��ȡ                */
#define OPENRT_LOGIN_INFO_GET_ACK_CMD     (OPEN_RT_LOGIN_BASE | 2)      /*  ��¼��Ϣ��ȡӦ��            */

#define OPENRT_CHANGE_PASSWD_CMD          (OPEN_RT_LOGIN_BASE | 3)      /*  �޸�����                    */
#define OPENRT_CHANGE_PASSWD_ACK_CMD      (OPEN_RT_LOGIN_BASE | 4)      /*  �޸�����Ӧ��                */

#define OPENRT_LOGIN_RECORD_ADD_CMD       (OPEN_RT_LOGIN_BASE | 5)      /*  �����û���Ϊ��¼            */
#define OPENRT_LOGIN_RECORD_ADD_ACK_CMD   (OPEN_RT_LOGIN_BASE | 6)      /*  �����û���Ϊ��¼Ӧ��        */

#define OPENRT_LOGIN_RECORD_GET_CMD       (OPEN_RT_LOGIN_BASE | 7)      /*  ��ѯ�û���Ϊ��¼            */
#define OPENRT_LOGIN_RECORD_GET_ACK_CMD   (OPEN_RT_LOGIN_BASE | 8)      /*  ��ѯ�û���Ϊ��¼Ӧ��        */

#define OPENRT_LOGIN_INFO_ADD_CMD         (OPEN_RT_LOGIN_BASE | 9)      /*  ����һ���û�                */
#define OPENRT_LOGIN_INFO_ADD_ACK_CMD     (OPEN_RT_LOGIN_BASE | 10)     /*  ����һ���û�Ӧ��            */

#define OPENRT_LOGIN_INFO_DEL_CMD         (OPEN_RT_LOGIN_BASE | 11)     /*  ɾ��һ���û�                */
#define OPENRT_LOGIN_INFO_DEL_ACK_CMD     (OPEN_RT_LOGIN_BASE | 12)     /*  ɾ��һ���û�Ӧ��            */

#define OPENRT_LOGIN_INFO_DISABLE_CMD     (OPEN_RT_LOGIN_BASE | 13)     /*  ����һ���û�                */
#define OPENRT_LOGIN_INFO_DISABLE_ACK_CMD (OPEN_RT_LOGIN_BASE | 14)     /*  ����һ���û�Ӧ��            */

#define OPENRT_USER_AUTH_SET_CMD          (OPEN_RT_LOGIN_BASE | 15)     /*  ������֤��Ϣ                */
#define OPENRT_USER_AUTH_SET_ACK_CMD      (OPEN_RT_LOGIN_BASE | 16)     /*  ������֤��ϢӦ��            */

#define OPENRT_USER_AUTH_GET_CMD          (OPEN_RT_LOGIN_BASE | 17)     /*  ��ȡ��֤��Ϣ                */
#define OPENRT_USER_AUTH_GET_ACK_CMD      (OPEN_RT_LOGIN_BASE | 18)     /*  ��ȡ��֤��ϢӦ��            */
/*********************************************************************************************************
  ģ������ö��ֵ
*********************************************************************************************************/
enum OPEN_RT_MODULE_INDEX {
    OPEN_RT_MONITOR = 0,
    OPEN_RT_UDAIL,
    OPEN_RT_INTERFACE,
    OPEN_RT_DHCPSERVER,
    OPEN_RT_DNS,
    OPEN_RT_ROUTE,
    OPEN_RT_VLAN,
    OPEN_RT_VPN,
    OPEN_RT_VND,
    OPEN_RT_NAT,
    OPEN_RT_QoS,
    OPEN_RT_DEVNAME,
    OPEN_RT_TIMESET,
    OPEN_RT_ACCOUNT,
    OPEN_RT_AUTH,
    OPEN_RT_UPDATEFIRMWARE,
    OPEN_RT_HWINFO,
    OPEN_RT_RESET,
    OPEN_RT_INTADA,
    OPEN_RT_FLOWCTL,
    OPEN_RT_BEHAVIOUR,
    OPEN_RT_ARP,
    OPEN_RT_ARP_DEFENSE,
    OPEN_RT_NPF,
    OPEN_RT_PING_TEST,
    OPEN_RT_SPEEDTEST,
    OPEN_RT_UDIA_LOG,
    OPEN_RT_OPERA_LOG,
    OPEN_RT_VPN_LOG,
    OPEN_RT_ASDEFENSE_LOG,
    OPEN_RT_DHCPSERVER_LOG,
    OPEN_RT_QUAGGA,
    OPEN_RT_USR_SCRIPT,

    OPEN_RT_MAX_INDEX
};
/*********************************************************************************************************
  handle ��
*********************************************************************************************************/
#define AUTH_HANDLE_ON              (1)                                 /*  ����Ȩ��                    */
#define AUTH_HANDLE_OFF             (0)                                 /*  ��ֹȨ��                    */
/*********************************************************************************************************
  ��½������
*********************************************************************************************************/
#define TYPE_LOGIN                  (1)
#define TYPE_LOGOUT                 (2)
/*********************************************************************************************************
  ��½״̬��
*********************************************************************************************************/
#define LRESULT_SUCCESS             (0)
#define LRESULT_FAIL                (1)
/*********************************************************************************************************
  Ȩ����֤���ƺ�
*********************************************************************************************************/
#define OPERA_AUTH                  "operaauth"                         /*  ������                      */
#define VIEW_AUTH                   "viewauth"                          /*  �۲���                      */
#define AUTH_BUFF_LEN               (32)                                /*  ��֤�ṹ buff ����          */
/*********************************************************************************************************
  ���ڵ�¼�û���Ϣ�Ľṹ
*********************************************************************************************************/
struct login_handle {
    CHAR    cUsername[64];                                              /*  �û���                      */
	CHAR    cNewPasswd[64];                                             /*  ������                      */
    CHAR    cPasswd[64];                                                /*  ����                        */
    UINT32  iRole;                                                      /*  ��ɫȨ�� 1:admin
                                                                                     2: operator
                                                                                     3: visitor        */
    UINT32  iStatus;                                                    /*  ״̬ 0�����á�1������       */
};
typedef struct login_handle  __LOGIN_HANDLE;
typedef struct login_handle  *__PLOGIN_HANDLE;
/*********************************************************************************************************
  ���ڵ�¼��Ϊ����Ľṹ
*********************************************************************************************************/
struct login_record {
    time_t  logStartTm;                                                 /*  ��ʼʱ��                    */
    time_t  logEndTm;                                                   /*  ����ʱ��                    */
    UINT32  llStartIndex;                                               /*  ��ʼ���                    */
    UINT32  llEndIndex;                                                 /*  �������                    */
    CHAR    cUsername[64];                                              /*  �û���                      */
    CHAR    cClientIp[64];                                              /*  �ͻ���IP��ַ                */
    CHAR    cTime[64];                                                  /*  ����ʱ��                    */
    UINT32  iResult;                                                    /*  �������                    */
    UINT32  iType;                                                      /*  ���ͣ�1, login 2, logout    */
    UINT32  iTotal;                                                     /*  ��ѯ��¼������              */
};
typedef struct login_record  __LOGIN_RECORD;
typedef struct login_record  *__PLOGIN_RECORD;
/*********************************************************************************************************
  ���ڵ�¼��Ϊ����Ľṹ
*********************************************************************************************************/
struct user_auth_set {
    CHAR    cHandle;                                                     /*  ����ʽ                   */
    CHAR    cRole[AUTH_BUFF_LEN];                                        /*  ��ɫ����                   */
    CHAR    cTypeName[AUTH_BUFF_LEN];                                    /*  ��������                   */
};
typedef struct user_auth_set   __USER_AUTH_SET;
typedef struct user_auth_set  *__PUSER_AUTH_SET;

#endif                                                                  /*  __LIBLOGIN_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
