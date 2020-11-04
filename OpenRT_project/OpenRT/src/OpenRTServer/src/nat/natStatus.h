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
** 文   件   名: natStatus.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 本目录获取状态函数公用头文件
*********************************************************************************************************/

#ifndef __NAT_STATUS_H
#define __NAT_STATUS_H

#include "libnat_config.h"
#include "nat_common.h"

/*********************************************************************************************************
  NAT 获取信息函数声明
*********************************************************************************************************/
INT    natLanInfoGet (__PLAN_INFO   pLanInfo);
INT    natWanInfoGet (__PWAN_INFO   pWanInfo);
INT    natMapItemNumGet (VOID);
INT    natMapInfoGet (PVOID  pNatMapBuff, INT32  iStartIndex, INT32  iEndIndex);
INT    natIpFragGet (VOID);
INT    natAliasItemNumGet (VOID);
INT    natAliasInfoGet (PVOID  pNatAliasBuff, INT32  iStartIndex, INT32  iEndIndex);
INT    natAssNodeUseGet (__PNAT_ASSNODE_INFO  pAssNodeInfo);
#endif                                                                  /* __NAT_STATUS_H               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
