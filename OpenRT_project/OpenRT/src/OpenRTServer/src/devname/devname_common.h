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
** 文   件   名: devname_common.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 设备名称信息头文件
*********************************************************************************************************/
#ifndef __DEV_NAME_COMMON_H
#define __DEV_NAME_COMMON_H

#include "libdevname_config.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define DEFAULT_DEV_NAME         "ACOINFO"                              /*  默认路由器设备名            */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PVOID  devNameGetFromDB(INT  *piNum);
INT    devNameUpdateToDb(__PDEVNAME_HANDLE  pDevName);
VOID   devNameHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    devNameTableCreate(VOID);

#endif                                                                  /* __DEV_NAME_COMMON_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
