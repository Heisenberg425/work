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
** ��   ��   ��: kidvpnDB.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ��Ŀ¼���ݿ⹫��ͷ�ļ�
*********************************************************************************************************/

#ifndef __KIDVPN_DB_H
#define __KIDVPN_DB_H

#include "libvpn_config.h"
/*********************************************************************************************************
  vnd ����Ϣ
*********************************************************************************************************/
#define KIDVPN_TABLE               "kidvpn"                             /*  vpn ����                  */
#define KIDVPN_TABLE_HEAD          "(TYPE       INT,"                               \
                                   "VNDID       INT,"                               \
                                   "MTU         INT,"                               \
                                   "PORT        INT,"                               \
                                   "PID         INT,"                               \
                                   "IP          TEXT,"                              \
                                   "STATUS      TEXT,"                              \
                                   "PASSWD      TEXT)"                  /*  vpn ���ӿ�                */
/*********************************************************************************************************
  vpn ���ӱ���Ϣ
*********************************************************************************************************/
#define KIDVPN_LINK_TABLE          "kidvpnLink"                         /*  vpn ������Ϣ ����         */
#define KIDVPN_LINK_TABLE_HEAD     "(VNDID       INT,"   \
                                    "IP          TEXT,"  \
                                    "MAC         TEXT)"                 /*  vpn ������Ϣ���ӿ�        */
/*********************************************************************************************************
  KidVPN ���ݿ������������
*********************************************************************************************************/
INT         kidvpnDelDBItem (INT  iVndID);
INT         kidvpnAddItemToDb (__PKIDVPN_DB_INFO  pKidVpnDBInfo);
INT         kidvpnUpdateVpnStatToDb (INT  iVndId, PCHAR  pcStatus);
PCHAR       kidvpnGetLinkedCliInfoFromDB (INT  *piRow);
PVOID       kidvpnGetDBInfo (VOID);
INT         kidvpnGetDBRow (VOID);
INT         kidvpnServIpExistFlagGet (__PKIDVPN_HANDLE  pKidVpnInfo);
INT         kidvpnGetSpecInfo (__PKIDVPN_DB_INFO  pKidVpnDBInfo,
                               CPCHAR             cpcField,
                               PCHAR              pcArg,
                               INT               *piArg);

INT         kidvpnLinkDelDBItemById (INT  iVndID);
INT         kidvpnLinkDelDBItem (__PKIDVPN_LINK    pKidVpnLinkInfo);
INT         kidvpnLinkAddItemToDb (__PKIDVPN_LINK  pKidVpnLinkInfo);
PVOID       kidvpnLinkGetDBInfo (VOID);
INT         kidvpnLinkGetSpecInfo (INT64  llStartIndex,
                                   INT64  llEndIndex,
                                   PVOID  pvLinkBuff,
                                   INT32  iBuffLen);
INT         kidvpnLinkGetDBRow (VOID);

#endif                                                                  /* __KIDVPN_DB_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
