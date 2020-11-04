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
** 文   件   名: userAdmin.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 8 月 28 日
**
** 描        述: 用户管理
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "goahead.h"
#include "userAdmin.h"
#include "webHandle.h"
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static ONLINE_USER  _G_OpenRT_Online_Users[MAX_ONLINE_USERS] = {{0}};   /*  OpenRT 在线用户信息         */
static INT          _G_OnlineUsers = 0;                                 /*  OpenRT 在线用户个数         */
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern BOOL  userAuthCheck (INT32  iModule, UINT8  ucOperaType);
/*********************************************************************************************************
** 函数名称: isUserLogin
** 功能描述: 检查用户是否已登陆
** 输　入  : wp                 Web 句柄
** 输　出  : NONE
** 返　回  : LW_TRUE 已通过鉴权，LW_FALSE 未通过鉴权
*********************************************************************************************************/
bool isUserLogin(Webs *wp)
{
#if 1
	WebsUser *pUser      = NULL;
	PCHAR     pcUserName = NULL;

    if (LW_NULL == wp) {
        return (LW_FALSE);
    }

    pcUserName = (PCHAR)websGetSessionVar(wp, WEBS_SESSION_USER, 0);
    if (NULL == pcUserName) {
        return (LW_FALSE);
    }

    pUser = websLookupUser(pcUserName);
    if (NULL == pUser) {
        printf("user is null \n");
        return (LW_FALSE);
    }

    if (NULL == pUser->roles) {
        printf("user roles is null \n");
        return (LW_FALSE);
    }
#endif
    return (LW_TRUE);
#if 0
    if (smatch(pUser->roles, ROLE_ADMIN)) {
        return (LW_TRUE);
    } else if (smatch(pUser->roles, ROLE_OPERATOR)) {

    } else if (smatch(pUser->roles, ROLE_VISITOR)) {
    }
#endif
}

/*********************************************************************************************************
** 函数名称: userLoginAuth
** 功能描述: 用户登陆及权限监测
** 输　入  : wp                 Web 句柄
**           iModule            功能模块(枚举类型值)
**           ucOperaType        操作类型(ACTION_OPERAT / ACTION_VIEW)
** 输　出  : NONE
** 返　回  : LW_TRUE 已通过鉴权，LW_FALSE 未通过鉴权
*********************************************************************************************************/
BOOL userLoginAuth(Webs *wp, INT32  iModule, UINT8  ucOperaType)
{
    WebsUser  *pUser      = NULL;
    PCHAR      pcUserName = NULL;

    if (LW_NULL == wp) {
        goto  __redirct_401;
    }

    pcUserName = (PCHAR)websGetSessionVar(wp, WEBS_SESSION_USER, 0);
    if (NULL == pcUserName) {
        goto  __redirct_401;
    }

    pUser = websLookupUser(pcUserName);
    if (NULL == pUser) {
        goto  __redirct_401;
    }

    /*
     *  进行权限认证
     */
    if (smatch(pUser->roles, ROLE_ADMIN)) {
        return  (LW_TRUE);
    } else if (smatch(pUser->roles, ROLE_OPERATOR)) {
        if (LW_FALSE == userAuthCheck(iModule, ACTION_OPERAT)) {
            goto  __error_403;
        } else {
            return  (LW_TRUE);
        }
    } else if (smatch(pUser->roles, ROLE_VISITOR)) {
        if (ACTION_OPERAT == ucOperaType) {                             /*  观察者不允许进行配置操作    */
            goto  __error_403;
        }

        if (LW_FALSE == userAuthCheck(iModule, ACTION_VIEW)) {
            goto  __error_403;
        } else {
            return  (LW_TRUE);
        }
    } else {
        return  (LW_FALSE);
    }

__redirct_401:
    webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
    webBackHandle(wp, NULL);

    return  (LW_FALSE);

__error_403:
    webHeadHandle(wp, HTTP_CODE_FORBIDDEN);
    webBackHandle(wp, NULL);

    return  (LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: getUserOnlineSessionID
** 功能描述: 获取用户在线 session ID
** 输　入  : pcUserName       用户名
** 输　出  : NONE
** 返　回  : 成功返回 session ID 指针，失败返回 NULL
*********************************************************************************************************/
PCHAR  getUserOnlineSessionID (PCHAR  pcUserName)
{
    INT i;

    for (i = 0; i< MAX_ONLINE_USERS; i++) {
        if (_G_OpenRT_Online_Users[i].user.name) {
            if (smatch(_G_OpenRT_Online_Users[i].user.name, pcUserName)) {
               return  (_G_OpenRT_Online_Users[i].sessionID);
            }
        }
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: userOnline
** 功能描述: 用户上线
** 输　入  : pcSessionID          sessionID
**           pcUserName           用户名
** 输　出  : NONE
** 返　回  : 记录结果
*********************************************************************************************************/
BOOL  userOnline (PCHAR  pcSessionID, PCHAR  pcUserName)
{
    INT  i;

    /*
     *  同时在线用户数达到最大值, 不允许再登陆
     */
    if (_G_OnlineUsers == MAX_ONLINE_USERS) {
        return  (LW_FALSE);
    }

    for (i = 0; i< MAX_ONLINE_USERS; i++) {
        if (_G_OpenRT_Online_Users[i].user.name == NULL) {
             break;
         }
    }

    if (MAX_ONLINE_USERS == i) {
        return  (LW_FALSE);
    }

    _G_OpenRT_Online_Users[i].sessionID = pcSessionID;

    bzero(&_G_OpenRT_Online_Users[i].user, sizeof(_G_OpenRT_Online_Users[i].user));
    _G_OpenRT_Online_Users[i].user.name = pcUserName;

    _G_OnlineUsers++;

    return  (LW_TRUE);
}
/*********************************************************************************************************
** 函数名称: userOffline
** 功能描述: 将用户剔除下线
** 输　入  : pcUserName    用户名
** 输　出  : NONE
** 返　回  : 剔除结果
*********************************************************************************************************/
INT userOffline (PCHAR  pcUserName)
{
    BOOL  iExist = LW_FALSE;
    INT   i;

    if (NULL == pcUserName) {
        return  (LW_FALSE);
    }

    if (_G_OnlineUsers == 0) {
        return  (LW_FALSE);
    }

    for (i = 0; i< MAX_ONLINE_USERS; i++) {
        if (NULL == _G_OpenRT_Online_Users[i].user.name) {
            continue;
        }

        if(!strcmp(_G_OpenRT_Online_Users[i].user.name, pcUserName)) {
            iExist = LW_TRUE;

            break;
        }
    }

    if (iExist) {
        _G_OnlineUsers--;

        _G_OpenRT_Online_Users[i].sessionID = NULL;

        bzero(&_G_OpenRT_Online_Users[i].user, sizeof(_G_OpenRT_Online_Users[i].user));
    }

    return  (LW_TRUE);
}

