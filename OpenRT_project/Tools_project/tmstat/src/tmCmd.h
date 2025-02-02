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
** 文   件   名: tmCmd.h
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: 监控管理命令行头文件
*********************************************************************************************************/
#ifndef __TMCMD_H
#define __TMCMD_H

/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT   tmEnable (BOOL  bIsEnable);
VOID  tmInstallCmd (VOID);

#endif                                                                  /* __TMCMD_H                    */
