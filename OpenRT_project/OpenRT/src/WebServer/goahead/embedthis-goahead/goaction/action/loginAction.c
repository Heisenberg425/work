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
** ��   ��   ��: loginAction.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 26 ��
**
** ��        ��: ��¼�����̨�ӿ�
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
#define WEB_CODE                      "code"                            /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: ��½��Ϣ
*********************************************************************************************************/
#define WEB_LOGIN_SESSION_ID          "id"                              /*  Json�ֶΣ�SessionID         */
#define WEB_LOGIN_USERNAME            "username"                        /*  Json�ֶΣ��û���            */
#define WEB_LOGIN_PASSWD              "passwd"                          /*  Json�ֶΣ�����              */
#define WEB_LOGIN_STATUS              "status"                          /*  Json�ֶΣ�״̬              */
#define WEB_LOGIN_ROLE                "role"                            /*  Json�ֶΣ���ɫ              */
#define WEB_LOGIN_NEW_PASSWD          "new_passwd"                      /*  Json�ֶΣ�������            */
#define WEB_LOGIN_CLIENT_IP           "clientIp"                        /*  Json�ֶΣ��ͻ���IP          */

#define WEB_LOGIN_TIME                "time"                            /*  Json�ֶΣ�����ʱ��          */
#define WEB_LOGIN_RESULT              "result"                          /*  Json�ֶΣ��������          */
#define WEB_LOGIN_TYPE                "type"                            /*  Json�ֶΣ���������          */
#define WEB_LOGIN_LOG_STARTT          "starttime"                       /*  Json�ֶΣ���ʼʱ��          */
#define WEB_LOGIN_LOG_ENDT            "endtime"                         /*  Json�ֶΣ�����ʱ��          */
#define WEB_LOGIN_LOG_PAGE            "page"                            /*  Json�ֶΣ�ҳ��              */
#define WEB_LOGIN_LOG_PAGECOUNT       "pagecount"                       /*  Json�ֶΣ�ÿҳ����          */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                      "page"                            /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                     "total"                           /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                    "data"                            /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static PCHAR  _G_pcInitPassWd = "21232f297a57a5a743894a0e4a801fc3";     /*  ��ʼ������                  */
/*********************************************************************************************************
** ��������: __changePasswdAction
** ��������: �޸�����
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
** ��������: __websLoginUser
** ��������: ��¼�û���֤��̨�ӿ�
** �䡡��  : wp            ���������
**           pcUserName    �û���
**           pcPassWd      ����
** �䡡��  : NONE
** ������  : �ɹ����� LW_TURE��ʧ�ܷ��� LW_FALSE
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
** ��������: __loginAction
** ��������: ��¼�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
         *  ʹ�ó�ʼ�����¼ʱ, code ���1000, �����û��޸�����
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
         *  �Ѿ���ͬһ���û��Ѿ���¼
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
     *  ��¼ web ������־
     */
    loginRecordWrite(&loginRecord);

 __error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __logoutAction
** ��������: ע����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __logoutAction (Webs *wp)
{
    cJSON                *pjSend        = LW_NULL;
    __LOGIN_RECORD        loginRecord   = {0};
    PCHAR                 pcSessionID   = LW_NULL;
    PCHAR                 pcUserName    = LW_NULL;

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
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
     *  ��¼ web ������־
     */
    loginRecordWrite(&loginRecord);

    webBackHandle(wp, pjSend);

    websSetSessionInValidate(pcSessionID);

    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __loadUser
** ��������: ���ر��غϷ��û�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
         *  �� buff �ͷŵ����ٴλ�ȡһ���µ���Ϣ
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

            websAddUser(pLogin->cUsername, pLogin->cPasswd, pcRole);    /*  ���û���Ϣ����web server    */
        }
    }
}
/*********************************************************************************************************
** ��������: __loginRecordQuery
** ��������: ��ѯ�û���¼��Ϣ��̨�ӿڶ���
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend        = cJSON_CreateObject();                               /*  �������� Json ����          */
    pJsonVlanArry = cJSON_CreateArray();                                /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVlanArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
     *  ���ò�ѯ����ӿ�
     */
    pLoginRecordHead = loginRecordQuery(&loginRecord, &iRecordTotal);

    /*
     *  ѭ�����÷��ص� Json ����
     */
    if (LW_NULL != pLoginRecordHead) {
        iCount = pLoginRecordHead->iTotal;

        for (i = 0; i < iRecordTotal; i++) {
            pTmpNode = pLoginRecordHead + i;

            cJSON  *pjLoginRec = cJSON_CreateObject();                  /*  ����һ��ӳ�� Json ����      */
            if (LW_NULL == pjLoginRec) {
                break;
            }

            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_USERNAME,  pTmpNode->cUsername);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_CLIENT_IP, pTmpNode->cClientIp);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_TIME,      pTmpNode->cTime);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_RESULT,    pTmpNode->iResult);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_TYPE,      pTmpNode->iType);

            cJSON_AddItemToArray(pJsonVlanArry, pjLoginRec);            /*  ������Ŀ��������            */
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
** ��������: __queryUsersAction
** ��������: ��ѯ�û���Ϣ��̨�ӿڶ���
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    if(LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGE)) {
        iPage = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_LOG_PAGE)->valueint;
    }

    /*
     *  ���÷��ص� Json ����
     */
    pUsersHead = loginInfoQuery(&iRecords);
    if (LW_NULL != pUsersHead) {
        pJsonUserArry = cJSON_CreateArray();                            /*  ���� Json ӳ����Ϣ����      */
        if (LW_NULL == pJsonUserArry) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto  __error;
        }

        for (i = 0; i < iRecords; i++) {
            pTmpNode = pUsersHead + i;

            cJSON  *pjLoginRec = cJSON_CreateObject();                  /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjLoginRec) {
                break;
            }

            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_USERNAME, pTmpNode->cUsername);
            cJSON_AddStringToObject(pjLoginRec, WEB_LOGIN_PASSWD,   pTmpNode->cPasswd);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_STATUS,   pTmpNode->iStatus);
            cJSON_AddNumberToObject(pjLoginRec, WEB_LOGIN_ROLE,     pTmpNode->iRole);

            cJSON_AddItemToArray(pJsonUserArry, pjLoginRec);            /*  ������Ŀ��������            */
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
** ��������: __addUserAction
** ��������: �����û���̨�ӿڶ���
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
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
** ��������: __delUserAction
** ��������: ɾ���û���̨�ӿڶ���
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __delUserAction (Webs *wp)
{
    __LOGIN_HANDLE   loginHandle   = {{0}};
    cJSON           *pRecvjRes     = LW_NULL;
    cJSON           *pSendjRes     = LW_NULL;
    cJSON           *pjUserName    = LW_NULL;
    INT              iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjUserName = cJSON_GetObjectItem(pRecvjRes, WEB_LOGIN_USERNAME);
    if (LW_NULL == pjUserName) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    }

    strncpy(loginHandle.cUsername, pjUserName->valuestring, sizeof(loginHandle.cUsername));

    iRet = userDelete(&loginHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
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
** ��������: __disableUserAction
** ��������: �����û���̨�ӿڶ���
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
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
** ��������: __loginActionDefine
** ��������: login��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
** ��������: __loginActionDefine
** ��������: login��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginActionInit (VOID)
{
    /*
     *  �����кϷ��û���Ϣ���ص� web server ��
     */
    __loadUser();

    __loginActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
