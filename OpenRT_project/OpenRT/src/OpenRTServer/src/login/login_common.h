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
** ��   ��   ��: login_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��: loginͷ�ļ�
*********************************************************************************************************/

#ifndef __LOGIN_COMMON_H
#define __LOGIN_COMMON_H

#include "liblogin_config.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    loginInsert (__PLOGIN_HANDLE pLogin);
INT    loginDelete (__PLOGIN_HANDLE pLogin);
INT    disableUserLogin (__PLOGIN_HANDLE pLogin);
INT    changePasswd (__PLOGIN_HANDLE pLogin);
PVOID  loginGetFromDB (INT* piRecordNum);
INT    loginTableCreate (VOID);
VOID   loginHandleEntry (INT  iCommand, PVOID  pArgBuf);
INT    loginRecordInsert (__PLOGIN_RECORD pLoginRecord);
INT    authDBUpdate (__PUSER_AUTH_SET pAuthSet);
INT    authDBInfoGet (PCHAR pcRoleName, PCHAR  pcBuff, INT32  iBuffLen);

PVOID  loginRecordGetFromDB (__PLOGIN_RECORD pLoginRecord, INT* piRecordNum);
#endif                                                                  /*  __LOGIN_COMMON_H            */
