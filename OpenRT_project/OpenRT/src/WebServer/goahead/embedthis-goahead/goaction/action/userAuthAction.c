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
** 文   件   名: userAuthAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 9 月 30 日
**
** 描        述: OpenRT 用户权限管理
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "../webHandle.h"
#include "../operLog.h"
#include "liblogin_config.h"
/*********************************************************************************************************
  全局权限表
*********************************************************************************************************/
static PCHAR  _G_pcOperatorAuthMap;                                     /*  操作员权限                  */
static PCHAR  _G_pcViewAuthMap;                                         /*  观察员权限                  */
extern const char *__G_pOpenRTModuleName[OPEN_RT_MAX_INDEX];            /*  OpenRT 模块名               */
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE               "code"                                   /*  返回的状态码                */
#define JSON_SUCCESS_CODE      (200)                                    /*  成功返回Json code值         */
#define JSON_FAILED_CODE       (500)                                    /*  失败返回Json code值         */
#define OPERA_ARRAY_NAME       "operadata"                              /*  Json字段：操作者权限数据    */
#define VIEW_ARRAY_NAME        "viewdata"                               /*  Json字段：观察者权限数据    */
/*********************************************************************************************************
  Jason key
*********************************************************************************************************/
#define WEB_USER_AUTH_TYPE     "type"                                   /*  Json字段：模块类型          */
#define WEB_USER_AUTH_ROLE     "role"                                   /*  Json字段：角色              */
#define WEB_USER_AUTH_HANDLE   "handle"                                 /*  Json字段：处理类型          */
/*********************************************************************************************************
  handle 宏
*********************************************************************************************************/
#define HANDLE_ON              "on"                                     /*  允许权限                    */
#define HANDLE_OFF             "off"                                    /*  禁止权限                    */
/*********************************************************************************************************
** 函数名称: authMapUpdate
** 功能描述: 更新认证信息表
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __authMapUpdate (VOID)
{
    PVOID  pvInfo = LW_NULL;

    pvInfo = libuserAuthGet(OPERA_AUTH);                                /*  获取操作者权限              */
    if (NULL != pvInfo) {
        memcpy(_G_pcOperatorAuthMap, pvInfo, OPEN_RT_MAX_INDEX);

        free(pvInfo);

        pvInfo = NULL;
    }

    pvInfo = libuserAuthGet(VIEW_AUTH);                                 /*  获取观察者权限              */
    if (NULL != pvInfo) {
        memcpy(_G_pcViewAuthMap, pvInfo, OPEN_RT_MAX_INDEX);

        free(pvInfo);

        pvInfo = NULL;
    }
}

/*********************************************************************************************************
** 函数名称: __userAuthHandleAction
** 功能描述: 用户权限显示后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __userAuthShowAction (Webs *wp)
{
    cJSON            *pSendjRes         = LW_NULL;
    PCHAR             pcOperatorAuthMap = LW_NULL;
    PCHAR             pcViewAuthMap     = LW_NULL;
    INT32             i;

    /*
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pcOperatorAuthMap = libuserAuthGet(OPERA_AUTH);                     /*  获取操作者权限              */
    pcViewAuthMap     = libuserAuthGet(VIEW_AUTH);                      /*  获取观察者权限              */
    if (LW_NULL == pcOperatorAuthMap || LW_NULL == pcViewAuthMap) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto __error;
    }

    cJSON  *pjOperatorAuth = cJSON_CreateObject();                      /*  创建一条映射 Json 对象      */
    if(LW_NULL == pjOperatorAuth) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto __error;
    }

    cJSON  *pjViewAuth = cJSON_CreateObject();                          /*  创建一条映射 Json 对象      */
    if(LW_NULL == pjViewAuth) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
        cJSON_Delete(pjOperatorAuth);

        goto __error;
    }

    for (i = 0; i < OPEN_RT_MAX_INDEX; i++) {
        cJSON_AddStringToObject(pjOperatorAuth,
                                __G_pOpenRTModuleName[i],
                                pcOperatorAuthMap[i] ? "on" : "off");
        cJSON_AddStringToObject(pjViewAuth,
                                __G_pOpenRTModuleName[i],
                                pcViewAuthMap[i] ? "on" : "off");
    }

    cJSON_AddItemToObject(pSendjRes, OPERA_ARRAY_NAME, pjOperatorAuth);
    cJSON_AddItemToObject(pSendjRes, VIEW_ARRAY_NAME, pjViewAuth);
    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);                  /*  操作成功                    */

__error:
    free(pcOperatorAuthMap);
    free(pcViewAuthMap);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __userAuthHandleAction
** 功能描述: 用户权限设置后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __userAuthHandleAction (Webs *wp)
{
    cJSON            *pRecvjRes                           = LW_NULL;
    cJSON            *pSendjRes                           = LW_NULL;
    cJSON            *pjType                              = LW_NULL;
    cJSON            *pjRole                              = LW_NULL;
    cJSON            *pjHandle                            = LW_NULL;
    CHAR             *pcType                              = LW_NULL;
    CHAR             *pcHandle                            = LW_NULL;
    CHAR             *pcRole                              = LW_NULL;

    __USER_AUTH_SET   userAuth;

    INT               iRet;

    /*
     * 用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);
    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjType   = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_TYPE);
    pjHandle = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_HANDLE);
    pjRole   = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_ROLE);
    if (LW_NULL == pjType || LW_NULL == pjHandle || LW_NULL == pjRole) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcType   = pjType->valuestring;
    pcRole   = pjRole->valuestring;
    pcHandle = pjHandle->valuestring;

    bzero(&userAuth, sizeof(userAuth));
    userAuth.cHandle = (strcmp(pcHandle, HANDLE_OFF)) ? AUTH_HANDLE_ON : AUTH_HANDLE_OFF;
    strncpy(userAuth.cRole, pcRole, sizeof(userAuth.cRole));
    strncpy(userAuth.cTypeName, pcType, sizeof(userAuth.cTypeName));

    iRet = libuserAuthHandle(&userAuth);
    if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
    }

    __authMapUpdate();
__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** 函数名称: __userAuthActionDefine
** 功能描述: 用户权限后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __userAuthActionDefine (VOID)
{
    websDefineAction("userAuthHandleAction", __userAuthHandleAction);
    websDefineAction("userAuthShowAction", __userAuthShowAction);
}

/*********************************************************************************************************
** 函数名称: userAuthCheck
** 功能描述: 用户权限验证
** 输　入  : iModule            模块
**           ucOperaType        操作者 / 观察者
** 输　出  : NONE
** 返　回  : 是否通过
*********************************************************************************************************/
BOOL  userAuthCheck (INT32  iModule, UINT8  ucOperaType)
{
    if (ACTION_OPERAT == ucOperaType && _G_pcOperatorAuthMap[iModule]) {
        return  (LW_TRUE);
    } else if (ACTION_VIEW == ucOperaType && _G_pcViewAuthMap[iModule]) {
        return  (LW_TRUE);
    } else {
        return  (LW_FALSE);
    }
}

/*********************************************************************************************************
** 函数名称: userAuthActionInit
** 功能描述: 用户权限后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  userAuthActionInit (VOID)
{
    _G_pcOperatorAuthMap = _G_pcViewAuthMap = LW_NULL;
    _G_pcOperatorAuthMap = libuserAuthGet(OPERA_AUTH);                  /*  获取操作者权限              */
    _G_pcViewAuthMap     = libuserAuthGet(VIEW_AUTH);                   /*  获取观察者权限              */
    __userAuthActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
