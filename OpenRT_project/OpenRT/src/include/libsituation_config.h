/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: libsituation_config.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 18 日
**
** 描        述: 情景模式信息头文件
*********************************************************************************************************/

#ifndef __LIBSITATION_CONFIG_H
#define __LIBSITATION_CONFIG_H

#include <netinet/in.h>
#include "lwip/netif.h"
#include "libserver_type.h"

/*********************************************************************************************************
  情景模式相关宏定义 (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_SAVE_SIT_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 1)  /*  保存情景模式信息命令        */
#define OPENRT_SAVE_SIT_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 2)  /*  保存情景模式信息命令应答    */

#define OPENRT_SAVE_WAN_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 3)  /*  保存 WAN 信息命令           */
#define OPENRT_SAVE_WAN_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 4)  /*  保存 WAN 信息命令应答       */

#define OPENRT_SAVE_VPNSER_INFO_CMD      (OPEN_RT_SIT_CONFIG_BASE | 5)  /*  保存 VPN Server 信息命令    */
#define OPENRT_SAVE_VPNSER_INFO_CMD_ACK  (OPEN_RT_SIT_CONFIG_BASE | 6)  /*  保存 VPN Server 信息命令应答*/

#define OPENRT_CLEAN_VPNSER_INFO_CMD     (OPEN_RT_SIT_CONFIG_BASE | 7)  /*  保存 VPN Server 信息命令    */
#define OPENRT_CLEAN_VPNSER_INFO_CMD_ACK (OPEN_RT_SIT_CONFIG_BASE | 8)  /*  保存 VPN Server 信息命令应答*/

#define OPENRT_CANCEL_KIDVPN_CMD         (OPEN_RT_SIT_CONFIG_BASE | 9)  /*  取消 Kidvpn 命令            */
#define OPENRT_CANCEL_KIDVPN_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 10) /*  取消 Kidvpn 命令应答        */

#define OPENRT_SAVE_VPNSER_CONF_CMD      (OPEN_RT_SIT_CONFIG_BASE | 11) /*  保存 VPN Server 信息命令    */
#define OPENRT_SAVE_VPNSER_CONF_CMD_ACK  (OPEN_RT_SIT_CONFIG_BASE | 12) /*  保存 VPN Server 信息命令应答*/

#define OPENRT_SAVE_CLI_INFO_CMD         (OPEN_RT_SIT_CONFIG_BASE | 13) /*  保存 VPN Client 信息命令    */
#define OPENRT_SAVE_CLI_INFO_CMD_ACK     (OPEN_RT_SIT_CONFIG_BASE | 14) /*  保存 VPN Client 信息命令应答*/

#define OPENRT_CANCEL_OPTFUNC_CMD        (OPEN_RT_SIT_CONFIG_BASE | 15) /*  取消可选功能命令            */
#define OPENRT_CANCEL_OPTFUNC_CMD_ACK    (OPEN_RT_SIT_CONFIG_BASE | 16) /*  取消可选功能命令应答        */
/*********************************************************************************************************
  情景模式信息结构定义
*********************************************************************************************************/
struct situation_info {
    CHAR     ucSituationMode[40];                                       /*  情景模式                    */
    CHAR     ucPosition[40];                                            /*  引导框定位                  */
    INT32    iFunc;                                                     /*  情景模式复选功能            */
    CHAR     cStatus[40];                                               /*  情景模式状态                */
};
typedef struct situation_info   __SITUATION_INFO;
typedef struct situation_info  *__PSITUATION_INFO;
/*********************************************************************************************************
  WAN 信息结构定义
*********************************************************************************************************/
struct wanpage_info {
    CHAR      cNetifName[NETIF_NAMESIZE];                               /*  WAN 网口名称                */
    UINT8     ucMode;                                                   /*  接入外网模式                */
    CHAR      cPppoeUsr[30];                                            /*  拨号账号                    */
    CHAR      cPppoePwd[30];                                            /*  拨号密码                    */
    CHAR      cIpAddr[40];                                              /*  IP 地址                     */
    CHAR      cNetMask[40];                                             /*  掩码地址                    */
    CHAR      cGateWay[40];                                             /*  网关地址                    */
};
typedef struct wanpage_info   __WANPAGE_INFO;
typedef struct wanpage_info  *__PWANPAGE_INFO;
/*********************************************************************************************************
  KidVPN Server 信息结构定义
*********************************************************************************************************/
struct kidvpn_server_info {
    CHAR      cPublicIp[32];                                            /*  公网 ip                     */
    CHAR      cVndIpAddr[32];                                           /*  VND ip                      */
    CHAR      cVndNetMask[32];                                          /*  VND 掩码                    */
    CHAR      cVndGateway[32];                                          /*  VND 网关                    */
    CHAR      cPasswd[32];                                              /*  KidVPN 密码                 */
    CHAR      cAesKey[70];                                              /*  KidVPN 密钥                 */
    INT       iVndId;                                                   /*  VND ID                      */
};
typedef struct kidvpn_server_info   __KIDVPN_SERVER_INFO;
typedef struct kidvpn_server_info  *__PKIDVPN_SERVER_INFO;
/*********************************************************************************************************
  KidVPN Client 信息结构定义
*********************************************************************************************************/
struct kidvpn_client_info {
    CHAR      cPublicIp[32];                                            /*  公网 ip                     */

    CHAR      cVndIpAddr[32];                                           /*  VND ip                      */
    CHAR      cVndNetMask[32];                                          /*  VND 掩码                    */

    CHAR      cPasswd[32];                                              /*  KidVPN 密码                 */
    CHAR      cAesKey[70];                                              /*  KidVPN 密钥                 */
    INT       iVndId;                                                   /*  VND ID                      */
};
typedef struct kidvpn_client_info   __KIDVPN_CLIENT_INFO;
typedef struct kidvpn_client_info  *__PKIDVPN_CLIENT_INFO;

#endif                                                                  /* __LIBSITATION_CONFIG_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
