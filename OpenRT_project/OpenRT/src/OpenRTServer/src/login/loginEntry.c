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
** 文   件   名: loginEntry.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述: OpenRT 登录管理处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "liblogin_config.h"
#include "login_common.h"

/*********************************************************************************************************
** 函数名称: loginRecordAdd
** 功能描述: 修改登录信息
** 输　入  : pLoginHandle           前端传入的登录信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginRecordAdd (__PLOGIN_RECORD  pLoginRecord)
{
    replyConfigMsg(OPENRT_LOGIN_RECORD_ADD_ACK_CMD, loginRecordInsert(pLoginRecord));
}
/*********************************************************************************************************
** 函数名称: loginRecordGet
** 功能描述: 获取用户信息
** 输　入  : NONE
** 输　出  : pLoginRecord   登陆信息
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginRecordGet (__PLOGIN_RECORD pLoginRecord)
{
    __PLOGIN_RECORD pLoginRecordArrayHead = LW_NULL;
    INT             iRecordNum            = 0;

    pLoginRecordArrayHead = loginRecordGetFromDB(pLoginRecord, &iRecordNum);
    replyQueryMsg(OPENRT_LOGIN_RECORD_GET_ACK_CMD,
                  sizeof(__LOGIN_RECORD),
                  iRecordNum ,
                  pLoginRecordArrayHead);

    if (NULL != pLoginRecordArrayHead) {
        free(pLoginRecordArrayHead);
    }
}
/*********************************************************************************************************
** 函数名称: loginInfoChange
** 功能描述: 修改登录信息
** 输　入  : pLoginHandle           前端传入的登录信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginInfoChange (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_CHANGE_PASSWD_ACK_CMD, changePasswd(pLoginHandle));
}
/*********************************************************************************************************
** 函数名称: loginInfoGet
** 功能描述: 获取用户信息
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginInfoGet (VOID)
{
    __PLOGIN_HANDLE pLoginArrayHead = LW_NULL;
    INT             iRecordNum      = 0;

    pLoginArrayHead = loginGetFromDB(&iRecordNum);

    replyQueryMsg(OPENRT_LOGIN_INFO_GET_ACK_CMD, sizeof(__LOGIN_HANDLE), iRecordNum , pLoginArrayHead);

    if (NULL != pLoginArrayHead) {
        free(pLoginArrayHead);
    }
}
/*********************************************************************************************************
** 函数名称: loginUserAdd
** 功能描述: 增加登录用户
** 输　入  : pLoginHandle           前端传入的登录信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginUserAdd (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_ADD_ACK_CMD, loginInsert(pLoginHandle));
}
/*********************************************************************************************************
** 函数名称: loginUserDel
** 功能描述: 删除一个用户
** 输　入  : pLoginHandle           前端传入的用户信息信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginUserDel (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_DEL_ACK_CMD, loginDelete(pLoginHandle));
}
/*********************************************************************************************************
** 函数名称: loginInfoDoDisable
** 功能描述: 禁用一个用户
** 输　入  : pLoginHandle           前端传入的用户信息信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginUserDisable (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_DISABLE_ACK_CMD, disableUserLogin(pLoginHandle));
}
/*********************************************************************************************************
** 函数名称: userAuthHanle
** 功能描述: 用户权限认证处理
** 输　入  : pArgBuf            参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  userAuthHanle (PVOID  pArgBuf)
{
    __PUSER_AUTH_SET  pUserAuth = pArgBuf;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_USER_AUTH_SET_ACK_CMD, PX_ERROR);
    }

    replyConfigMsg(OPENRT_USER_AUTH_SET_ACK_CMD, authDBUpdate(pUserAuth));
}
/*********************************************************************************************************
** 函数名称: userAuthInfoGet
** 功能描述: 用户权限认证获取
** 输　入  : pArgBuf            参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  userAuthInfoGet (PVOID  pArgBuf)
{
    INT               iRet                         = PX_ERROR;
    CHAR              cInfoBuff[OPEN_RT_MAX_INDEX] = {0};

    if (LW_NULL == pArgBuf) {
        replyQueryMsg(OPENRT_USER_AUTH_GET_ACK_CMD, 0, 0, LW_NULL);
    }

    iRet = authDBInfoGet(pArgBuf, cInfoBuff, OPEN_RT_MAX_INDEX);
    if (ERROR_NONE == iRet) {
        replyQueryMsg(OPENRT_USER_AUTH_GET_ACK_CMD, sizeof(cInfoBuff), 1, cInfoBuff);
    } else {
        replyQueryMsg(OPENRT_USER_AUTH_GET_ACK_CMD, 0, 0, LW_NULL);
    }
}
/*********************************************************************************************************
** 函数名称: loginHandleEntry
** 功能描述: 登录配置入口
** 输　入  : pArgBuf            参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID loginHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_LOGIN_INFO_GET_CMD:
        loginInfoGet();
        break;

    case OPENRT_LOGIN_INFO_ADD_CMD:
        loginUserAdd(pArgBuf);
        break;

    case OPENRT_LOGIN_INFO_DEL_CMD:
        loginUserDel(pArgBuf);
        break;

    case OPENRT_LOGIN_INFO_DISABLE_CMD:
        loginUserDisable(pArgBuf);
        break;

    case OPENRT_CHANGE_PASSWD_CMD:
        loginInfoChange(pArgBuf);
        break;

    case OPENRT_LOGIN_RECORD_ADD_CMD:
        loginRecordAdd(pArgBuf);
        break;

    case OPENRT_LOGIN_RECORD_GET_CMD:
        loginRecordGet(pArgBuf);
        break;

    case OPENRT_USER_AUTH_SET_CMD:
        userAuthHanle(pArgBuf);
        break;

    case OPENRT_USER_AUTH_GET_CMD:
        userAuthInfoGet(pArgBuf);
        break;

    default:
        break;
    }
}
