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
** 文   件   名: kid_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: kidvpn 配置共用头文件
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
  链表操作宏
*********************************************************************************************************/
#ifndef _LIST_OFFSETOF
#define _LIST_OFFSETOF(type, member)                          \
        ((size_t)&((type *)0)->member)                                  /*  偏移量计算                  */
#define _LIST_CONTAINER_OF(ptr, type, member)                 \
        ((type *)((size_t)ptr - _LIST_OFFSETOF(type, member)))          /*  得到ptr的容器结构           */
#define _LIST_ENTRY(ptr, type, member)                        \
        _LIST_CONTAINER_OF(ptr, type, member)                           /*  获得链表母体指针结构        */
#define _list_line_get_next(pline)  ((pline)->LINE_plistNext)           /*  获取下一个                  */
#endif
/*********************************************************************************************************
  KidVPN 操作函数声明
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
  KidVPN 数据库操作函数声明
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
