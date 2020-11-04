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
** ��   ��   ��: loginHandle.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��:  ��¼������
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "liblogin_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: loginInfoQuery
** ��������: ��ѯ��¼��Ϣ
** �䡡��  : NONE
** �䡡��  : piRecordTotal     ��¼��Ϣ��ѯ�������
** ������  : ��¼��Ϣ����ָ��
*********************************************************************************************************/
PVOID  loginInfoQuery (INT  *piRecordTotal)
{
    return requestQueryMsg(OPENRT_LOGIN_INFO_GET_CMD, NULL, 0, piRecordTotal);
}

/*********************************************************************************************************
** ��������: loginChangePasswd
** ��������: �޸�����
** �䡡��  : pLoginHandle      ��¼�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  loginChangePasswd (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_CHANGE_PASSWD_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** ��������: userAdd
** ��������: ����һ���û�
** �䡡��  : pLoginHandle      �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  userAdd (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_ADD_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** ��������: userDelete
** ��������: ����һ���û�
** �䡡��  : pLoginHandle      �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  userDelete (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_DEL_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** ��������: userDisable
** ��������: ����һ���û�
** �䡡��  : pLoginHandle      �û���Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  userDisable (__PLOGIN_HANDLE  pLoginHandle)
{
   return  requestConfigMsg (OPENRT_LOGIN_INFO_DISABLE_CMD, pLoginHandle, sizeof(__LOGIN_HANDLE));
}

/*********************************************************************************************************
** ��������: loginRecordWrite
** ��������: �û���Ϊ��¼
** �䡡��  : pLoginRecord      �û���Ϊ�����Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  loginRecordWrite (__PLOGIN_RECORD  pLoginRecord)
{
   return  requestConfigMsg (OPENRT_LOGIN_RECORD_ADD_CMD, pLoginRecord, sizeof(__LOGIN_RECORD));
}

/*********************************************************************************************************
** ��������: loginRecordQuery
** ��������: ��ѯ��¼��Ϣ
** �䡡��  : pLoginRecord      �û���Ϊ��ѯ����
** �䡡��  : piRecordTotal     �û���Ϊ��ѯ�������
** ������  : �û���Ϊ����ָ��
*********************************************************************************************************/
PVOID  loginRecordQuery (__PLOGIN_RECORD  pLoginRecord, INT* piRecordTotal)
{
    return requestQueryMsg(OPENRT_LOGIN_RECORD_GET_CMD,
                           pLoginRecord,
                           sizeof(__LOGIN_RECORD),
                           piRecordTotal);
}

/*********************************************************************************************************
** ��������: libuserAuthHandle
** ��������: �û���֤��Ϣ����
** �䡡��  : userAuth           �û�������Ϣ
** �䡡��  : NONE
** ������  : ��֤��Ϣ�����Ƿ�ɹ�
*********************************************************************************************************/
INT32  libuserAuthHandle (__PUSER_AUTH_SET  pUserAuth)
{
    return  requestConfigMsg (OPENRT_USER_AUTH_SET_CMD, pUserAuth, sizeof(*pUserAuth));
}

/*********************************************************************************************************
** ��������: libuserAuthGet
** ��������: �û���֤Ȩ����Ϣ��ȡ(��Ҫ�û��ͷ��ڴ�)
** �䡡��  : pcRole             ��ɫ����
** �䡡��  : NONE
** ������  : Ȩ����Ϣ
*********************************************************************************************************/
PVOID  libuserAuthGet (PCHAR  pcRole)
{
    INT   iNum = 0;
    if (LW_NULL == pcRole) {
        return  (LW_NULL);
    }
    return requestQueryMsg(OPENRT_USER_AUTH_GET_CMD, pcRole, AUTH_BUFF_LEN, &iNum);
}

