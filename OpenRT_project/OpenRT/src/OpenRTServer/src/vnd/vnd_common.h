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
** 文   件   名: vnd_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: vnd 配置共用头文件
*********************************************************************************************************/
#ifndef __VND_COMMON_H
#define __VND_COMMON_H

/*********************************************************************************************************
  vnd 表信息
*********************************************************************************************************/
#define VNDINFO_TABLE              "vndinfo"                            /*  vnd 表单名                  */
#define VNDINFO_TABLE_HEAD         "(ID         INT,"                               \
                                   "IP          TEXT,"                              \
                                   "NETMASK     TEXT,"                              \
                                   "GATEWAY     TEXT,"                              \
                                   "MAC         TEXT,"                              \
                                   "MTU         INT,"                               \
                                   "NETIFINDEX  INT)"                  /*  vnd 表单接口                 */
/*********************************************************************************************************
  EXPORT 虚拟网卡函数
*********************************************************************************************************/
VOID vndHandleEntry (INT  iCommand, PVOID  *pArgBuf);
INT  vndTableCreate (VOID);
INT  vndRecovery (INT  iVndID);
INT  vndDel (INT   iVndID);

#endif                                                                  /*  __VND_COMMON_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
