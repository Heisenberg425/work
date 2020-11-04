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
** 文   件   名: loginAction.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 26 日
**
** 描        述: 登录管理后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "login_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                      "code"                            /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: 登陆信息
*********************************************************************************************************/
#define WEB_LOGIN_SESSION_ID          "id"                              /*  Json字段：SessionID         */
#define WEB_LOGIN_USERNAME            "username"                        /*  Json字段：用户名            */
#define WEB_LOGIN_PASSWD              "passwd"                          /*  Json字段：密码              */
#define WEB_LOGIN_STATUS              "status"                          /*  Json字段：状态              */
#define WEB_LOGIN_ROLE                "role"                            /*  Json字段：角色              */
#define WEB_LOGIN_NEW_PASSWD          "new_passwd"                      /*  Json字段：新密码            */
#define WEB_LOGIN_CLIENT_IP           "clientIp"                        /*  Json字段：客户端IP          */

#define WEB_LOGIN_TIME                "time"                            /*  Json字段：操作时间          */
#define WEB_LOGIN_RESULT              "result"                          /*  Json字段：操作结果          */
#define WEB_LOGIN_TYPE                "type"                            /*  Json字段：操作类型          */
#define WEB_LOGIN_LOG_STARTT          "starttime"                       /*  Json字段：开始时间          */
#define WEB_LOGIN_LOG_ENDT            "endtime"                         /*  Json字段：结束时间          */
#define WEB_LOGIN_LOG_PAGE            "page"                            /*  Json字段：页码              */
#define WEB_LOGIN_LOG_PAGECOUNT       "pagecount"                       /*  Json字段：每页个数          */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                      "page"                            /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                     "total"                           /*  Json字段：总共的条目数      */
#define ARRAY_NAME                    "data"                            /*  Json字段：内网端口          */
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static PCHAR  _G_pcInitPassWd = "21232f297a57a5a743894a0e4a801fc3";     /*  初始化密码                  */
/*********************************************************************************************************
** 函数名称: __changePasswdAction
** 功能描述: 修改密码
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __changePasswdAction (Webs *wp)
{
    __LOGIN_HANDLE    loginHandle = {{0}};
    cJSON            *pjSend      = LW_NULL;
    cJSON            *pRecvjRes   = LW_NULL;
    INT               iResult     = 0;

    cJSON            *pjUserName  = LW_NULL;
    cJSON            *pjPassWd    = LW_NULL;
    cJSON            *pjNewPassWd = LW_NULL;

    PCHAR             pcUserName  = LW_NULL;
    PCHAR             pcPassWd    = LW_NULL;
    PCHAR             pcNewPassWd = LW_NULL;
    WebsUser         *pUser       = LW_NULL;

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

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjUserName  = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    pjPassWd    = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_PASSWD);
    pjNewPassWd = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_NEW_PASSWD);
    if (NULL == pjUserName || NULL == pjPassWd || NULL == pjNewPassWd) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    pcUserName  = pjUserName->valuestring;
    pcPassWd    = pjPassWd->valuestring;
    pcNewPassWd = pjNewPassWd->valuestring;
    if (NULL == pcUserName || NULL == pcPassWd || NULL == pcNewPassWd) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    strncpy(loginHandle.cUsername,  pcUserName,  sizeof(loginHandle.cUsername));
    strncpy(loginHandle.cPasswd,    pcPassWd,    sizeof(loginHandle.cPasswd));
    strncpy(loginHandle.cNewPasswd, pcNewPassWd, sizeof(loginHandle.cNewPasswd));

    iResult = loginChangePasswd(&loginHandle);
    if (ERROR_NONE != iResult) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto __error;
    } else {
        pUser = websLookupUser(loginHandle.cUsername);
        if (LW_NULL == pUser) {
            printf("verifyUser: Unknown user \"%s\"\n", wp->username);

            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto __error;
        }

        if (!smatch(pUser->password, loginHandle.cPasswd)) {
            printf("old password does not match\n");

            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto __error;
        }

        wfree(pUser->password);
        pUser->password = sclone(loginHandle.cNewPasswd);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __websLoginUser
** 功能描述: 登录用户认证后台接口
** 输　入  : wp            服务器句柄
**           pcUserName    用户名
**           pcPassWd      密码
** 输　出  : NONE
** 返　回  : 成功返回 LW_TURE，失败返回 LW_FALSE
*********************************************************************************************************/
static BOOL  __websLoginUser(Webs *wp, PCHAR  pcUserName, PCHAR  pcPassWd)
{
    if (NULL == wp->route) {
        printf("websLoginUser route is null\n");

        return  (LW_FALSE);
    }

    if (NULL == wp->route->verify) {
        printf("websLoginUser verify is null\n");

        return  (LW_FALSE);
    }

    wfree(wp->username);
    wp->username = sclone(pcUserName);
    wfree(wp->password);
    wp->password = sclone(pcPassWd);

    if (0 == (wp->user = websLookupUser(wp->username))) {
       printf("verifyUser: Unknown user \"%s\"\n", wp->username);

       return  (LW_FALSE);
    }

    if (!smatch(wp->password, wp->user->password)) {
        printf("Password does not match\n");

        return  (LW_FALSE);
    }

    printf("Login successful for %s, roles=%s\n", wp->user->name, wp->user->roles);

    return  (LW_TRUE);
}
/*********************************************************************************************************
** 函数名称: __loginAction
** 功能描述: 登录的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __loginAction (Webs *wp)
{
    __LOGIN_HANDLE        loginHandle      = {{0}};
    __LOGIN_RECORD        loginRecord      = {0};
    cJSON                *pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjUserName       = LW_NULL;
    cJSON                *pjPassWd         = LW_NULL;

    PCHAR                 pcUserName       = LW_NULL;
    PCHAR                 pcPassWd         = LW_NULL;

    BOOL                  bSuccess         = LW_FALSE;
    PCHAR                 pcSessionID      = LW_NULL;
    PCHAR                 pcOldSessionID   = LW_NULL;
    WebsUser             *pUser            = LW_NULL;

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjUserName = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    pjPassWd   = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_PASSWD);
    if (NULL == pjUserName || NULL == pjPassWd) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    pcUserName = pjUserName->valuestring;
    pcPassWd   = pjPassWd->valuestring;
    if (NULL == pcUserName || NULL == pcPassWd) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    strncpy(loginHandle.cUsername, pcUserName, sizeof(loginHandle.cUsername));
    strncpy(loginHandle.cPasswd,   pcPassWd,   sizeof(loginHandle.cPasswd));
    strncpy(loginRecord.cUsername, loginHandle.cUsername,    sizeof(loginRecord.cUsername));
    strncpy(loginRecord.cClientIp, websGetRequestIpAddr(wp), sizeof(loginRecord.cClientIp));

    loginRecord.iType = TYPE_LOGIN;

    bSuccess = __websLoginUser(wp, loginHandle.cUsername, loginHandle.cPasswd);
    if (bSuccess) {
        websSetSessionVar(wp, "loginStatus", "ok");

        /*
         *  使用初始密码登录时, code 填充1000, 提醒用户修改密码
         */
        if (smatch(loginHandle.cPasswd, _G_pcInitPassWd)){
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 1000);
        } else {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);
        }

        pcSessionID    = websGetSessionID(wp);
        pUser          = websLookupUser(wp->username);
        pcOldSessionID = getUserOnlineSessionID(wp->username);

        /*
         *  已经有同一个用户已经登录
         */
        if (NULL != pcOldSessionID &&
            NULL != pcSessionID    &&
            (!smatch(pcSessionID, pcOldSessionID)) &&
            NULL != pUser) {
            userOffline(pUser->name);
            websSetSessionInValidate(pcOldSessionID);
        }

        if (NULL != pUser) {
            websSetSessionVar(wp, WEBS_SESSION_USER, pUser->name);

            userOnline(pcSessionID, pUser->name);

            cJSON_AddStringToObject(pjSend, WEB_LOGIN_ROLE, pUser->roles);
        }

        cJSON_AddStringToObject(pjSend, WEB_LOGIN_SESSION_ID, pcSessionID);

        loginRecord.iResult = LRESULT_SUCCESS;
    } else {
        websSetSessionVar(wp, "loginStatus", "failed");
        cJSON_AddNumberToObject(pjSend, WEB_CODE, HTTP_CODE_UNAUTHORIZED);
        loginRecord.iResult = LRESULT_FAIL;
    }

    cJSON_AddStringToObject(pjSend, WEB_LOGIN_USERNAME, loginHandle.cUsername);

    /*
     *  记录 web 上线日志
     */
    loginRecordWrite(&loginRecord);

 __error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __logoutAction
** 功能描述: 注销后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __logoutAction (Webs *wp)
{
    cJSON                *pjSend        = LW_NULL;
    __LOGIN_RECORD        loginRecord   = {0};
    PCHAR                 pcSessionID   = LW_NULL;
    PCHAR                 pcUserName    = LW_NULL;

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    pcUserName = (PCHAR)websGetSessionVar(wp, WEBS_SESSION_USER, 0);
    if (NULL != pcUserName) {
        strncpy(loginRecord.cUsername, pcUserName, sizeof(loginRecord.cUsername));
    }

    strncpy(loginRecord.cClientIp, websGetRequestIpAddr(wp), sizeof(loginRecord.cClientIp));

    loginRecord.iType   = TYPE_LOGOUT;
    loginRecord.iResult = LRESULT_SUCCESS;

    if (NULL != pcUserName) {

        pcSessionID = getUserOnlineSessionID(pcUserName);
        if (NULL != pcSessionID) {
            websRemoveSessionVar(wp, pcSessionID);
        }

        userOffline(pcUserName);
    }

    websRemoveSessionVar(wp, WEBS_SESSION_USER);

    /*
     *  记录 web 上线日志
     */
    loginRecordWrite(&loginRecord);

    webBackHandle(wp, pjSend);

    websSetSessionInValidate(pcSessionID);

    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __loadUser
** 功能描述: 加载本地合法用户
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __loadUser (VOID)
{
    __PLOGIN_HANDLE pLoginHead = NULL;
    __PLOGIN_HANDLE pLogin     = NULL;
    __LOGIN_HANDLE  login      = {{0}};
    PCHAR           pcRole     = NULL;
    INT             iCount     = 0;
    INT             i;

    pLoginHead = loginInfoQuery(&iCount);
    if (0 == iCount) {
        strncpy(login.cUsername,"admin",        sizeof(login.cUsername));
        strncpy(login.cPasswd, _G_pcInitPassWd, sizeof(login.cPasswd));

        login.iRole   = 1;
        login.iStatus = 0;

        userAdd(&login);

        /*
         *  将 buff 释放掉，再次获取一次新的信息
         */
        if (NULL != pLoginHead) {
            free(pLoginHead);

            pLoginHead = NULL;
        }

        pLoginHead = loginInfoQuery(&iCount);
    }

    if (NULL == pLoginHead) {
        return;
    }

    for (i = 0; i < iCount; i++) {
        pLogin = pLoginHead + i;

        if (NULL != pLogin) {
            if (pLogin->iRole == 1) {
                pcRole = ROLE_ADMIN;
            } else if (pLogin->iRole == 2) {
                pcRole = ROLE_OPERATOR;
            } else if (pLogin->iRole == 3) {
                pcRole = ROLE_VISITOR;
            }

            websAddUser(pLogin->cUsername, pLogin->cPasswd, pcRole);    /*  将用户信息加入web server    */
        }
    }
}
/*********************************************************************************************************
** 函数名称: __loginRecordQuery
** 功能描述: 查询用户登录信息后台接口定义
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __loginRecordQuery (Webs *wp)
{
    __PLOGIN_RECORD        pLoginRecordHead = LW_NULL;
    __LOGIN_RECORD         loginRecord      = {0};
    __PLOGIN_RECORD        pTmpNode         = LW_NULL;
    cJSON*                 pJsonVlanArry    = LW_NULL;
    cJSON*                 pjSend           = LW_NULL;
    cJSON                 *pRecvjRes        = LW_NULL;
    INT                    iRecordTotal     = 0;
    INT                    i;
    CHAR                  *pcStartTime      = LW_NULL;
    CHAR                  *pcEndTime        = LW_NULL;
    INT                    iPage            = 0;
    INT                    iPageCount       = 0;

    struct tm              tmStartTime;
    struct tm              tmEndTime;

    cJSON                 *pjStartTime      = LW_NULL;
    cJSON                 *pjEndTime        = LW_NULL;
    cJSON                 *pjPage           = LW_NULL;
    cJSON                 *pjPageCount      = LW_NULL;
    INT                    iCount           = 0;

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

    pjSend        = cJSON_CreateObject();                               /*  创建返回 Json 对象          */
    pJsonVlanArry = cJSON_CreateArray();                                /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVlanArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_ENDT);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGECOUNT);
    if (NULL == pjPage || NULL == pjPageCount) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    if ((LW_NULL != pjStartTime) && (LW_NULL != pjEndTime)) {
        pcStartTime = pjStartTime->valuestring;
        pcEndTime   = pjEndTime->valuestring;

        bzero(&tmEndTime,   sizeof(tmEndTime));
        bzero(&tmStartTime, sizeof(tmStartTime));
        strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
        strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

        loginRecord.logStartTm   = mktime(&tmStartTime);
        loginRecord.logEndTm     = mktime(&tmEndTime);
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount               = pjPageCount->valueint;
    loginRecord.llStartIndex = (iPage - 1) * iPageCount + 1;
    loginRecord.llEndIndex   = iPage * iPageCount;

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_CLIENT_IP)) {
        strncpy(loginRecord.cClientIp,
                cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_CLIENT_IP)->valuestring,
                sizeof (loginRecord.cClientIp));
    }

    /*
     *  调用查询请求接口
     */
    pLoginRecordHead = loginRecordQuery(&loginRecord, &iRecordTotal);

    /*
     *  循环设置返回的 Json 内容
     */
    if (LW_NULL != pLoginRecordHead) {
        iCount = pLoginRecordHead->iTotal;

        for (i = 0; i < iRecordTotal; i++) {
            pTmpNode = pLoginRecordHead + i;

            cJSON  *pjLoginRec = cJSON_CreateObject();                  /*  创建一条映射 Json 对象      */
            if (LW_NULL == pjLoginRec) {
                break;
            }

            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_USERNAME,  pTmpNode->cUsername);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_CLIENT_IP, pTmpNode->cClientIp);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_TIME,      pTmpNode->cTime);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_RESULT,    pTmpNode->iResult);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_TYPE,      pTmpNode->iType);

            cJSON_AddItemToArray(pJsonVlanArry, pjLoginRec);            /*  将此条目加入数组            */
        }
    }

    if (NULL != pLoginRecordHead) {
        free(pLoginRecordHead);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonVlanArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iCount);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVlanArry);
}
/*********************************************************************************************************
** 函数名称: __queryUsersAction
** 功能描述: 查询用户信息后台接口定义
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __queryUsersAction (Webs *wp)
{
    __PLOGIN_HANDLE        pUsersHead      = LW_NULL;
    __PLOGIN_HANDLE        pTmpNode        = LW_NULL;
    INT                    iRecords        = 0;
    cJSON                 *pjSend          = LW_NULL;
    cJSON                 *pRecvjRes       = LW_NULL;
    cJSON                 *pJsonUserArry   = LW_NULL;
    INT                    i               = 0;
    INT                    iPage           = 0;

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

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    if(LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGE)) {
        iPage = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGE)->valueint;
    }

    /*
     *  设置返回的 Json 内容
     */
    pUsersHead = loginInfoQuery(&iRecords);
    if (LW_NULL != pUsersHead) {
        pJsonUserArry = cJSON_CreateArray();                            /*  创建 Json 映射信息数组      */
        if (LW_NULL == pJsonUserArry) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto  __error;
        }

        for (i = 0; i < iRecords; i++) {
            pTmpNode = pUsersHead + i;

            cJSON  *pjLoginRec = cJSON_CreateObject();                  /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjLoginRec) {
                break;
            }

            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_USERNAME, pTmpNode->cUsername);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_PASSWD,   pTmpNode->cPasswd);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_STATUS,   pTmpNode->iStatus);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_ROLE,     pTmpNode->iRole);

            cJSON_AddItemToArray(pJsonUserArry, pjLoginRec);            /*  将此条目加入数组            */
        }
    }

    if (NULL != pUsersHead) {
        free(pUsersHead);
    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonUserArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iRecords);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __addUserAction
** 功能描述: 增加用户后台接口定义
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __addUserAction (Webs *wp)
{
    __LOGIN_HANDLE   loginHandle   = {{0}};
    cJSON           *pRecvjRes     = LW_NULL;
    cJSON           *pSendjRes     = LW_NULL;
    cJSON           *pjUserName    = LW_NULL;
    cJSON           *pjPassword    = LW_NULL;
    cJSON           *pjRole        = LW_NULL;
    cJSON           *pjStatus      = LW_NULL;
    PCHAR            pcRole        = LW_NULL;
    INT              iRet;

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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjUserName = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    pjPassword = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_PASSWD);
    pjRole     = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_ROLE);
    pjStatus   = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_STATUS);
    if (LW_NULL == pjUserName ||
        LW_NULL == pjPassword ||
        LW_NULL == pjRole) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    }

    strncpy(loginHandle.cUsername, pjUserName->valuestring, sizeof(loginHandle.cUsername));
    strncpy(loginHandle.cPasswd,   pjPassword->valuestring, sizeof(loginHandle.cPasswd));
    loginHandle.iRole = pjRole->valueint;

    if (NULL != pjStatus) {
        loginHandle.iStatus = pjStatus->valueint;
    }

    iRet = userAdd(&loginHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        if (loginHandle.iRole == 1) {
            pcRole = ROLE_ADMIN;
        } else if (loginHandle.iRole == 2) {
            pcRole = ROLE_OPERATOR;
        } else if (loginHandle.iRole == 3) {
            pcRole = ROLE_VISITOR;
        }

        websAddUser(loginHandle.cUsername, loginHandle.cPasswd, pcRole);

        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __delUserAction
** 功能描述: 删除用户后台接口定义
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __delUserAction (Webs *wp)
{
    __LOGIN_HANDLE   loginHandle   = {{0}};
    cJSON           *pRecvjRes     = LW_NULL;
    cJSON           *pSendjRes     = LW_NULL;
    cJSON           *pjUserName    = LW_NULL;
    INT              iRet;

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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjUserName = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    if (LW_NULL == pjUserName) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    }

    strncpy(loginHandle.cUsername, pjUserName->valuestring, sizeof(loginHandle.cUsername));

    iRet = userDelete(&loginHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        websRemoveUser(loginHandle.cUsername);

        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __disableUserAction
** 功能描述: 禁用用户后台接口定义
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __disableUserAction (Webs *wp)
{
    __LOGIN_HANDLE   loginHandle   = {{0}};
    cJSON           *pRecvjRes     = LW_NULL;
    cJSON           *pSendjRes     = LW_NULL;
    cJSON           *pjUserName    = LW_NULL;
    cJSON           *pjStatus      = LW_NULL;
    INT              iRet;

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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjUserName = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    pjStatus   = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_STATUS);
    if (LW_NULL == pjUserName || LW_NULL == pjStatus) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    strncpy(loginHandle.cUsername, pjUserName->valuestring, sizeof(loginHandle.cUsername));
    loginHandle.iStatus = pjStatus->valueint;

    iRet = userDisable(&loginHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        websRemoveUser(loginHandle.cUsername);

        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __loginActionDefine
** 功能描述: login后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __loginActionDefine (VOID)
{
    websDefineAction("loginAction",          __loginAction);
    websDefineAction("logoutAction",         __logoutAction);
    websDefineAction("loginRecordQuery",     __loginRecordQuery);
    websDefineAction("changePasswordAction", __changePasswdAction);
    websDefineAction("queryUsersAction",     __queryUsersAction);
    websDefineAction("addUserAction",        __addUserAction);
    websDefineAction("delUserAction",        __delUserAction);
    websDefineAction("disableUserAction",    __disableUserAction);
}
/*********************************************************************************************************
** 函数名称: __loginActionDefine
** 功能描述: login后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  loginActionInit (VOID)
{
    /*
     *  将所有合法用户信息加载到 web server 中
     */
    __loadUser();

    __loginActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
