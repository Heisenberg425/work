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
** 文   件   名: login_common.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述: login头文件
*********************************************************************************************************/

#ifndef __LOGIN_COMMON_H
#define __LOGIN_COMMON_H

#include "liblogin_config.h"

/*********************************************************************************************************
  函数声明
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
