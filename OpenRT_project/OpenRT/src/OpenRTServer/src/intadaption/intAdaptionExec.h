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
** 文   件   名: intAdaptionExec.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录操作函数公用头文件
*********************************************************************************************************/

#ifndef __INTADAPTION_EXEC_H
#define __INTADAPTION_EXEC_H

#include "libintada_config.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  操作函数声明
*********************************************************************************************************/
INT  intAdaptionHandle (__PINT_ADA_SET  pcIntAdaptionInfo);
INT  intAdaptionIsExist (INT  iWebIndex);
INT  intAdaptionCheck (VOID);
VOID intAdaptionClear (VOID);
INT  intAdaptionGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName);
INT  intAdaptionGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName);
INT  intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex);

#endif                                                                  /* __INTADAPTION_EXEC_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
