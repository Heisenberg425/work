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
** ��   ��   ��: loginEntry.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��: OpenRT ��¼���������
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
** ��������: loginRecordAdd
** ��������: �޸ĵ�¼��Ϣ
** �䡡��  : pLoginHandle           ǰ�˴���ĵ�¼��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginRecordAdd (__PLOGIN_RECORD  pLoginRecord)
{
    replyConfigMsg(OPENRT_LOGIN_RECORD_ADD_ACK_CMD, loginRecordInsert(pLoginRecord));
}
/*********************************************************************************************************
** ��������: loginRecordGet
** ��������: ��ȡ�û���Ϣ
** �䡡��  : NONE
** �䡡��  : pLoginRecord   ��½��Ϣ
** ������  : NONE
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
** ��������: loginInfoChange
** ��������: �޸ĵ�¼��Ϣ
** �䡡��  : pLoginHandle           ǰ�˴���ĵ�¼��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginInfoChange (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_CHANGE_PASSWD_ACK_CMD, changePasswd(pLoginHandle));
}
/*********************************************************************************************************
** ��������: loginInfoGet
** ��������: ��ȡ�û���Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
** ��������: loginUserAdd
** ��������: ���ӵ�¼�û�
** �䡡��  : pLoginHandle           ǰ�˴���ĵ�¼��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginUserAdd (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_ADD_ACK_CMD, loginInsert(pLoginHandle));
}
/*********************************************************************************************************
** ��������: loginUserDel
** ��������: ɾ��һ���û�
** �䡡��  : pLoginHandle           ǰ�˴�����û���Ϣ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginUserDel (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_DEL_ACK_CMD, loginDelete(pLoginHandle));
}
/*********************************************************************************************************
** ��������: loginInfoDoDisable
** ��������: ����һ���û�
** �䡡��  : pLoginHandle           ǰ�˴�����û���Ϣ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  loginUserDisable (__PLOGIN_HANDLE  pLoginHandle)
{
    replyConfigMsg(OPENRT_LOGIN_INFO_DISABLE_ACK_CMD, disableUserLogin(pLoginHandle));
}
/*********************************************************************************************************
** ��������: userAuthHanle
** ��������: �û�Ȩ����֤����
** �䡡��  : pArgBuf            ����
** �䡡��  : NONE
** ������  : NONE
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
** ��������: userAuthInfoGet
** ��������: �û�Ȩ����֤��ȡ
** �䡡��  : pArgBuf            ����
** �䡡��  : NONE
** ������  : NONE
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
** ��������: loginHandleEntry
** ��������: ��¼�������
** �䡡��  : pArgBuf            ����
** �䡡��  : NONE
** ������  : NONE
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
