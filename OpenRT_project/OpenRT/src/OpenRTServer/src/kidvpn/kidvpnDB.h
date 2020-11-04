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
** 文   件   名: kidvpnDB.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录数据库公用头文件
*********************************************************************************************************/

#ifndef __KIDVPN_DB_H
#define __KIDVPN_DB_H

#include "libvpn_config.h"
/*********************************************************************************************************
  vnd 表信息
*********************************************************************************************************/
#define KIDVPN_TABLE               "kidvpn"                             /*  vpn 表单名                  */
#define KIDVPN_TABLE_HEAD          "(TYPE       INT,"                               \
                                   "VNDID       INT,"                               \
                                   "MTU         INT,"                               \
                                   "PORT        INT,"                               \
                                   "PID         INT,"                               \
                                   "IP          TEXT,"                              \
                                   "STATUS      TEXT,"                              \
                                   "PASSWD      TEXT)"                  /*  vpn 表单接口                */
/*********************************************************************************************************
  vpn 连接表信息
*********************************************************************************************************/
#define KIDVPN_LINK_TABLE          "kidvpnLink"                         /*  vpn 连接信息 表单名         */
#define KIDVPN_LINK_TABLE_HEAD     "(VNDID       INT,"   \
                                    "IP          TEXT,"  \
                                    "MAC         TEXT)"                 /*  vpn 连接信息表单接口        */
/*********************************************************************************************************
  KidVPN 数据库操作函数声明
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
