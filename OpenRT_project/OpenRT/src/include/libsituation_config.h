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
** ��   ��   ��: libsituation_config.h
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 18 ��
**
** ��        ��: �龰ģʽ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBSITATION_CONFIG_H
#define __LIBSITATION_CONFIG_H

#include <netinet/in.h>
#include "lwip/netif.h"
#include "libserver_type.h"

/*********************************************************************************************************
  �龰ģʽ��غ궨�� (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_SAVE_SIT_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 1)  /*  �����龰ģʽ��Ϣ����        */
#define OPENRT_SAVE_SIT_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 2)  /*  �����龰ģʽ��Ϣ����Ӧ��    */

#define OPENRT_SAVE_WAN_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 3)  /*  ���� WAN ��Ϣ����           */
#define OPENRT_SAVE_WAN_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 4)  /*  ���� WAN ��Ϣ����Ӧ��       */

#define OPENRT_SAVE_VPNSER_INFO_CMD      (OPEN_RT_SIT_CONFIG_BASE | 5)  /*  ���� VPN Server ��Ϣ����    */
#define OPENRT_SAVE_VPNSER_INFO_CMD_ACK  (OPEN_RT_SIT_CONFIG_BASE | 6)  /*  ���� VPN Server ��Ϣ����Ӧ��*/

#define OPENRT_CLEAN_VPNSER_INFO_CMD     (OPEN_RT_SIT_CONFIG_BASE | 7)  /*  ���� VPN Server ��Ϣ����    */
#define OPENRT_CLEAN_VPNSER_INFO_CMD_ACK (OPEN_RT_SIT_CONFIG_BASE | 8)  /*  ���� VPN Server ��Ϣ����Ӧ��*/

#define OPENRT_CANCEL_KIDVPN_CMD         (OPEN_RT_SIT_CONFIG_BASE | 9)  /*  ȡ�� Kidvpn ����            */
#define OPENRT_CANCEL_KIDVPN_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 10) /*  ȡ�� Kidvpn ����Ӧ��        */

#define OPENRT_SAVE_VPNSER_CONF_CMD      (OPEN_RT_SIT_CONFIG_BASE | 11) /*  ���� VPN Server ��Ϣ����    */
#define OPENRT_SAVE_VPNSER_CONF_CMD_ACK  (OPEN_RT_SIT_CONFIG_BASE | 12) /*  ���� VPN Server ��Ϣ����Ӧ��*/

#define OPENRT_SAVE_CLI_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 13) /*  ���� VPN Client ��Ϣ����    */
#define OPENRT_SAVE_CLI_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 14) /*  ���� VPN Client ��Ϣ����Ӧ��*/

#define OPENRT_CANCEL_OPTFUNC_CMD        (OPEN_RT_SIT_CONFIG_BASE | 15) /*  ȡ����ѡ��������            */
#define OPENRT_CANCEL_OPTFUNC_CMD_ACK    (OPEN_RT_SIT_CONFIG_BASE | 16) /*  ȡ����ѡ��������Ӧ��        */
/*********************************************************************************************************
  �龰ģʽ��Ϣ�ṹ����
*********************************************************************************************************/
struct situation_info {
    CHAR     ucSituationMode[40];                                       /*  �龰ģʽ                    */
    CHAR     ucPosition[40];                                            /*  ������λ                  */
    INT32    iFunc;                                                     /*  �龰ģʽ��ѡ����            */
    CHAR     cStatus[40];                                               /*  �龰ģʽ״̬                */
};
typedef struct situation_info   __SITUATION_INFO;
typedef struct situation_info  *__PSITUATION_INFO;
/*********************************************************************************************************
  WAN ��Ϣ�ṹ����
*********************************************************************************************************/
struct wanpage_info {
    CHAR      cNetifName[NETIF_NAMESIZE];                               /*  WAN ��������                */
    UINT8     ucMode;                                                   /*  ��������ģʽ                */
    CHAR      cPppoeUsr[30];                                            /*  �����˺�                    */
    CHAR      cPppoePwd[30];                                            /*  ��������                    */
    CHAR      cIpAddr[40];                                              /*  IP ��ַ                     */
    CHAR      cNetMask[40];                                             /*  �����ַ                    */
    CHAR      cGateWay[40];                                             /*  ���ص�ַ                    */
};
typedef struct wanpage_info   __WANPAGE_INFO;
typedef struct wanpage_info  *__PWANPAGE_INFO;
/*********************************************************************************************************
  KidVPN Server ��Ϣ�ṹ����
*********************************************************************************************************/
struct kidvpn_server_info {
    CHAR      cPublicIp[32];                                            /*  ���� ip                     */
    CHAR      cVndIpAddr[32];                                           /*  VND ip                      */
    CHAR      cVndNetMask[32];                                          /*  VND ����                    */
    CHAR      cVndGateway[32];                                          /*  VND ����                    */
    CHAR      cPasswd[32];                                              /*  KidVPN ����                 */
    CHAR      cAesKey[70];                                              /*  KidVPN ��Կ                 */
    INT       iVndId;                                                   /*  VND ID                      */
};
typedef struct kidvpn_server_info   __KIDVPN_SERVER_INFO;
typedef struct kidvpn_server_info  *__PKIDVPN_SERVER_INFO;
/*********************************************************************************************************
  KidVPN Client ��Ϣ�ṹ����
*********************************************************************************************************/
struct kidvpn_client_info {
    CHAR      cPublicIp[32];                                            /*  ���� ip                     */

    CHAR      cVndIpAddr[32];                                           /*  VND ip                      */
    CHAR      cVndNetMask[32];                                          /*  VND ����                    */

    CHAR      cPasswd[32];                                              /*  KidVPN ����                 */
    CHAR      cAesKey[70];                                              /*  KidVPN ��Կ                 */
    INT       iVndId;                                                   /*  VND ID                      */
};
typedef struct kidvpn_client_info   __KIDVPN_CLIENT_INFO;
typedef struct kidvpn_client_info  *__PKIDVPN_CLIENT_INFO;

#endif                                                                  /* __LIBSITATION_CONFIG_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
