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
** 文   件   名: tmStat.h
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 11 日
**
** 描        述: OpenRT 流量监控管理统计头文件
*********************************************************************************************************/
#ifndef __TMSTAT_H
#define __TMSTAT_H

#include "common.h"

/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT       tmStatsStart (INT            iIpType,
                        INT            iHookType,
                        struct pbuf   *pBuf,
                        struct netif  *pinp,
                        struct netif  *poutp);
VOID      tmShowStats (INT  iType, ip4_addr_t* pip4Addr);
INT       tmStartStats (VOID);
INT       tmStatItemCount (PTM_STATS  pCondition, INT* piFilter);
PTM_STATS tmStatItemsCopy (PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
PTM_STATS tmStatItemsSumCopy (PTM_STATS*  ppTmStat, PTM_STATS pCondition);
VOID      tmWalkStats (INT    (*callback)(),
                       PVOID    pvArg0,
                       PVOID    pvArg1,
                       PVOID    pvArg2);
VOID      tmNotifyWanChange (PVOID  pvWan);
VOID      tmNotifyLanChange (PVOID  pvLan);

#endif                                                                  /* __TMSTAT_H                   */
