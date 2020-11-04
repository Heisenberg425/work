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
** 文   件   名: liblogin_config.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述: login 信息头文件
*********************************************************************************************************/

#ifndef __LIBLOGIN_CONFIG_H
#define __LIBLOGIN_CONFIG_H

#include "libserver_type.h"

/*********************************************************************************************************
  login 服务ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_LOGIN_INFO_GET_CMD         (OPEN_RT_LOGIN_BASE | 1)      /*  登录信息获取                */
#define OPENRT_LOGIN_INFO_GET_ACK_CMD     (OPEN_RT_LOGIN_BASE | 2)      /*  登录信息获取应答            */

#define OPENRT_CHANGE_PASSWD_CMD          (OPEN_RT_LOGIN_BASE | 3)      /*  修改密码                    */
#define OPENRT_CHANGE_PASSWD_ACK_CMD      (OPEN_RT_LOGIN_BASE | 4)      /*  修改密码应答                */

#define OPENRT_LOGIN_RECORD_ADD_CMD       (OPEN_RT_LOGIN_BASE | 5)      /*  增加用户行为记录            */
#define OPENRT_LOGIN_RECORD_ADD_ACK_CMD   (OPEN_RT_LOGIN_BASE | 6)      /*  增加用户行为记录应答        */

#define OPENRT_LOGIN_RECORD_GET_CMD       (OPEN_RT_LOGIN_BASE | 7)      /*  查询用户行为记录            */
#define OPENRT_LOGIN_RECORD_GET_ACK_CMD   (OPEN_RT_LOGIN_BASE | 8)      /*  查询用户行为记录应答        */

#define OPENRT_LOGIN_INFO_ADD_CMD         (OPEN_RT_LOGIN_BASE | 9)      /*  新增一个用户                */
#define OPENRT_LOGIN_INFO_ADD_ACK_CMD     (OPEN_RT_LOGIN_BASE | 10)     /*  新增一个用户应答            */

#define OPENRT_LOGIN_INFO_DEL_CMD         (OPEN_RT_LOGIN_BASE | 11)     /*  删除一个用户                */
#define OPENRT_LOGIN_INFO_DEL_ACK_CMD     (OPEN_RT_LOGIN_BASE | 12)     /*  删除一个用户应答            */

#define OPENRT_LOGIN_INFO_DISABLE_CMD     (OPEN_RT_LOGIN_BASE | 13)     /*  禁用一个用户                */
#define OPENRT_LOGIN_INFO_DISABLE_ACK_CMD (OPEN_RT_LOGIN_BASE | 14)     /*  禁用一个用户应答            */

#define OPENRT_USER_AUTH_SET_CMD          (OPEN_RT_LOGIN_BASE | 15)     /*  设置认证信息                */
#define OPENRT_USER_AUTH_SET_ACK_CMD      (OPEN_RT_LOGIN_BASE | 16)     /*  设置认证信息应答            */

#define OPENRT_USER_AUTH_GET_CMD          (OPEN_RT_LOGIN_BASE | 17)     /*  获取认证信息                */
#define OPENRT_USER_AUTH_GET_ACK_CMD      (OPEN_RT_LOGIN_BASE | 18)     /*  获取认证信息应答            */
/*********************************************************************************************************
  模块类型枚举值
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
  handle 宏
*********************************************************************************************************/
#define AUTH_HANDLE_ON              (1)                                 /*  允许权限                    */
#define AUTH_HANDLE_OFF             (0)                                 /*  禁止权限                    */
/*********************************************************************************************************
  登陆操作宏
*********************************************************************************************************/
#define TYPE_LOGIN                  (1)
#define TYPE_LOGOUT                 (2)
/*********************************************************************************************************
  登陆状态宏
*********************************************************************************************************/
#define LRESULT_SUCCESS             (0)
#define LRESULT_FAIL                (1)
/*********************************************************************************************************
  权限认证名称宏
*********************************************************************************************************/
#define OPERA_AUTH                  "operaauth"                         /*  操作者                      */
#define VIEW_AUTH                   "viewauth"                          /*  观察者                      */
#define AUTH_BUFF_LEN               (32)                                /*  认证结构 buff 长度          */
/*********************************************************************************************************
  用于登录用户信息的结构
*********************************************************************************************************/
struct login_handle {
    CHAR    cUsername[64];                                              /*  用户名                      */
	CHAR    cNewPasswd[64];                                             /*  新密码                      */
    CHAR    cPasswd[64];                                                /*  密码                        */
    UINT32  iRole;                                                      /*  角色权限 1:admin
                                                                                     2: operator
                                                                                     3: visitor        */
    UINT32  iStatus;                                                    /*  状态 0：启用、1：禁用       */
};
typedef struct login_handle  __LOGIN_HANDLE;
typedef struct login_handle  *__PLOGIN_HANDLE;
/*********************************************************************************************************
  用于登录行为管理的结构
*********************************************************************************************************/
struct login_record {
    time_t  logStartTm;                                                 /*  开始时间                    */
    time_t  logEndTm;                                                   /*  结束时间                    */
    UINT32  llStartIndex;                                               /*  开始序号                    */
    UINT32  llEndIndex;                                                 /*  结束序号                    */
    CHAR    cUsername[64];                                              /*  用户名                      */
    CHAR    cClientIp[64];                                              /*  客户端IP地址                */
    CHAR    cTime[64];                                                  /*  操作时间                    */
    UINT32  iResult;                                                    /*  操作结果                    */
    UINT32  iType;                                                      /*  类型：1, login 2, logout    */
    UINT32  iTotal;                                                     /*  查询记录总条数              */
};
typedef struct login_record  __LOGIN_RECORD;
typedef struct login_record  *__PLOGIN_RECORD;
/*********************************************************************************************************
  用于登录行为管理的结构
*********************************************************************************************************/
struct user_auth_set {
    CHAR    cHandle;                                                     /*  处理方式                   */
    CHAR    cRole[AUTH_BUFF_LEN];                                        /*  角色名称                   */
    CHAR    cTypeName[AUTH_BUFF_LEN];                                    /*  命令名称                   */
};
typedef struct user_auth_set   __USER_AUTH_SET;
typedef struct user_auth_set  *__PUSER_AUTH_SET;

#endif                                                                  /*  __LIBLOGIN_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
