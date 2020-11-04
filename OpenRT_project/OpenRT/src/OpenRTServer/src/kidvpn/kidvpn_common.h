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
** ��   ��   ��: kid_common.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: kidvpn ���ù���ͷ�ļ�
*********************************************************************************************************/
#ifndef __KIDVPN_COMMON_H
#define __KIDVPN_COMMON_H

#include "libvpn_config.h"

/*********************************************************************************************************
  SylixOS list
*********************************************************************************************************/
extern VOID  _List_Line_Add_Tail( PLW_LIST_LINE  plineNew, LW_LIST_LINE_HEADER  *pplineHeader);
extern VOID  _List_Line_Del(PLW_LIST_LINE        plineDel, LW_LIST_LINE_HEADER  *pplineHeader);
/*********************************************************************************************************
  ���������
*********************************************************************************************************/
#ifndef _LIST_OFFSETOF
#define _LIST_OFFSETOF(type, member)                          \
        ((size_t)&((type *)0)->member)                                  /*  ƫ��������                  */
#define _LIST_CONTAINER_OF(ptr, type, member)                 \
        ((type *)((size_t)ptr - _LIST_OFFSETOF(type, member)))          /*  �õ�ptr�������ṹ           */
#define _LIST_ENTRY(ptr, type, member)                        \
        _LIST_CONTAINER_OF(ptr, type, member)                           /*  �������ĸ��ָ��ṹ        */
#define _list_line_get_next(pline)  ((pline)->LINE_plistNext)           /*  ��ȡ��һ��                  */
#endif
/*********************************************************************************************************
  KidVPN ������������
*********************************************************************************************************/
INT    kidVpnInit(VOID);
INT    kidvpnRecovery(VOID);
VOID   kidvpnHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    kidvpnTableCreate(VOID);
INT    kidvpnLinkTableCreate(VOID);

INT    kidVpnStart(__PKIDVPN_HANDLE  pKidVPNHandle);
INT    kidVpnDel(INT  iVndID);

INT    kidVpnAesKeySave(CPCHAR  cpcKey, INT  iVndId, INT  iLen);
INT    kidVpnAesKeyGen(PCHAR    pcKey,  INT  iBit,   INT  iLen);
INT    kidVpnAesKeyGet(PCHAR    pcKey,  INT  iVndID, INT  iLen);
INT    kidVpnAesKeyDel(INT      iVndID);

INT    getPublicIp(PCHAR  pcIpBuf);
INT    getPositionByIpAddr(CPCHAR  cpcIp, PCHAR  pcResult);
/*********************************************************************************************************
  KidVPN ���ݿ������������
*********************************************************************************************************/
INT    kidvpnDelDBItem(INT  iVndID);
INT    kidvpnAddItemToDb(__PKIDVPN_DB_INFO  pKidVpnDBInfo);
INT    kidvpnUpdateVpnStatToDb(INT  iVndId, PCHAR  pcStatus);
PCHAR  kidvpnGetLinkedCliInfoFromDB(INT  *piRow);
PVOID  kidvpnGetDBInfo(VOID);
INT    kidvpnGetDBRow(VOID);
INT    kidvpnServIpExistFlagGet(__PKIDVPN_HANDLE  pKidVpnInfo);
INT    kidvpnGetSpecInfo(__PKIDVPN_DB_INFO        pKidVpnDBInfo,
                          CPCHAR                  cpcField,
                          PCHAR                   pcArg,
                          INT                    *piArg);
INT    kidvpnClientStatusGetFromDB(VOID);

INT    kidvpnLinkDelDBItemById(INT  iVndID);
INT    kidvpnLinkDelDBItem(__PKIDVPN_LINK    pKidVpnLinkInfo);
INT    kidvpnLinkAddItemToDb(__PKIDVPN_LINK  pKidVpnLinkInfo);
PVOID  kidvpnLinkGetDBInfo(VOID);
INT    kidvpnLinkGetSpecInfo(INT64  llStartIndex,
                             INT64  llEndIndex,
                             PVOID  pvLinkBuff,
                             INT32  iBuffLen);
INT    kidvpnLinkGetDBRow(VOID);

#endif                                                                  /*  __KIDVPN_COMMON_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
