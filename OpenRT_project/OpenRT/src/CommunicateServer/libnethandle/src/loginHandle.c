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
** 文   件   名: loginHandle.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述:  登录管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "liblogin_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: loginInfoQuery
** 功能描述: 查询登录信息
** 输　入  : NONE
** 输　出  : piRecordTotal     登录信息查询结果总数
** 返　回  : 登录信息数组指针
*********************************************************************************************************/
PVOID  loginInfoQuery (INT  *piRecordTotal)
{
    return requestQueryMsg(OPENRT_LOGIN_INFO_GET_CMD, NULL, 0, piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: loginChangePasswd
** 功能描述: 修改密码
** 输　入  : pLoginHandle      登录入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  loginChangePasswd (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_CHANGE_PASSWD_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** 函数名称: userAdd
** 功能描述: 增加一个用户
** 输　入  : pLoginHandle      用户信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  userAdd (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_ADD_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** 函数名称: userDelete
** 功能描述: 增加一个用户
** 输　入  : pLoginHandle      用户信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  userDelete (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_DEL_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** 函数名称: userDisable
** 功能描述: 禁用一个用户
** 输　入  : pLoginHandle      用户信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  userDisable (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_DISABLE_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** 函数名称: loginRecordWrite
** 功能描述: 用户行为记录
** 输　入  : pLoginRecord      用户行为入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  loginRecordWrite (__PLOGIN_RECORD  pLoginRecord)
{
   return  requestConfigMsg (OPENRT_LOGIN_RECORD_ADD_CMD, pLoginRecord, sizeof(__LOGIN_RECORD));
}

/*********************************************************************************************************
** 函数名称: loginRecordQuery
** 功能描述: 查询登录信息
** 输　入  : pLoginRecord      用户行为查询条件
** 输　出  : piRecordTotal     用户行为查询结果总数
** 返　回  : 用户行为数组指针
*********************************************************************************************************/
PVOID  loginRecordQuery (__PLOGIN_RECORD  pLoginRecord, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_LOGIN_RECORD_GET_CMD,
                           pLoginRecord,
                           sizeof(__LOGIN_RECORD),
                           piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: libuserAuthHandle
** 功能描述: 用户认证信息设置
** 输　入  : userAuth           用户配置信息
** 输　出  : NONE
** 返　回  : 认证信息设置是否成功
*********************************************************************************************************/
INT32  libuserAuthHandle (__PUSER_AUTH_SET  pUserAuth)
{
    return  requestConfigMsg (OPENRT_USER_AUTH_SET_CMD, pUserAuth, sizeof(*pUserAuth));
}

/*********************************************************************************************************
** 函数名称: libuserAuthGet
** 功能描述: 用户认证权限信息获取(需要用户释放内存)
** 输　入  : pcRole             角色名称
** 输　出  : NONE
** 返　回  : 权限信息
*********************************************************************************************************/
PVOID  libuserAuthGet (PCHAR  pcRole)
{
    INT   iNum = 0;
    if (LW_NULL == pcRole) {
        return  (LW_NULL);
    }
    return requestQueryMsg(OPENRT_USER_AUTH_GET_CMD, pcRole, AUTH_BUFF_LEN, &iNum);
}

