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
** ��   ��   ��: userAdmin.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 8 �� 28 ��
**
** ��        ��: �û�����
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "goahead.h"
#include "userAdmin.h"
#include "webHandle.h"
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static ONLINE_USER  _G_OpenRT_Online_Users[MAX_ONLINE_USERS] = {{0}};   /*  OpenRT �����û���Ϣ         */
static INT          _G_OnlineUsers = 0;                                 /*  OpenRT �����û�����         */
/*********************************************************************************************************
  �ⲿ��������
*********************************************************************************************************/
extern BOOL  userAuthCheck (INT32  iModule, UINT8  ucOperaType);
/*********************************************************************************************************
** ��������: isUserLogin
** ��������: ����û��Ƿ��ѵ�½
** �䡡��  : wp                 Web ���
** �䡡��  : NONE
** ������  : LW_TRUE ��ͨ����Ȩ��LW_FALSE δͨ����Ȩ
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
** ��������: userLoginAuth
** ��������: �û���½��Ȩ�޼��
** �䡡��  : wp                 Web ���
**           iModule            ����ģ��(ö������ֵ)
**           ucOperaType        ��������(ACTION_OPERAT / ACTION_VIEW)
** �䡡��  : NONE
** ������  : LW_TRUE ��ͨ����Ȩ��LW_FALSE δͨ����Ȩ
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
     *  ����Ȩ����֤
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
        if (ACTION_OPERAT == ucOperaType) {                             /*  �۲��߲�����������ò���    */
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
** ��������: getUserOnlineSessionID
** ��������: ��ȡ�û����� session ID
** �䡡��  : pcUserName       �û���
** �䡡��  : NONE
** ������  : �ɹ����� session ID ָ�룬ʧ�ܷ��� NULL
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
** ��������: userOnline
** ��������: �û�����
** �䡡��  : pcSessionID          sessionID
**           pcUserName           �û���
** �䡡��  : NONE
** ������  : ��¼���
*********************************************************************************************************/
BOOL  userOnline (PCHAR  pcSessionID, PCHAR  pcUserName)
{
    INT  i;

    /*
     *  ͬʱ�����û����ﵽ���ֵ, �������ٵ�½
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
** ��������: userOffline
** ��������: ���û��޳�����
** �䡡��  : pcUserName    �û���
** �䡡��  : NONE
** ������  : �޳����
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

