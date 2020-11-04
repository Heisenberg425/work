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
** 文   件   名: tmHash.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: OpenRT 流量监控管理 hash 表头文件
*********************************************************************************************************/
#ifndef __TMHASH_H
#define __TMHASH_H

#include "common.h"

/*********************************************************************************************************
  hash 相关宏定义
*********************************************************************************************************/
#define TM_STAT_CB_SIZE           (256)                                 /*  hash 桶深度                 */

#if (TM_STAT_CB_SIZE & (TM_STAT_CB_SIZE-1)) == 0
#define TMCBHASHMOD(h)        ((h) & (TM_STAT_CB_SIZE - 1))             /*  求模                        */
#else
#define TMCBHASHMOD(h)        ((h) % TM_STAT_CB_SIZE)
#endif

#define TMCBHASH(ip)          TMCBHASHMOD(((ntohl(ip->addr)&0xFF) + \
                                           (ntohl(ip->addr)&0xFF)))     /*  根据 IP 计算 hash 值        */
#define TMCB_MAC_HASH(mac)    TMCBHASHMOD((mac) & 0xFF)                 /*  根据 MAC 计算 hash 值       */
/*********************************************************************************************************
  hash 全局变量声明
*********************************************************************************************************/
extern PTM_STAT_ENTRY __G_tmStatCbtable[TM_STAT_CB_SIZE];               /*  终端流量统计项控制块 hash 表*/
extern PTM_STAT_ENTRY __G_tmLineStatCbtable[TM_STAT_CB_SIZE];           /*  线路流量统计项控制块 hash 表*/
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByIp (ip4_addr_t  *pip4Addr);
PTM_STAT_ENTRY  tmStatCbFindByMac (UINT8  *pucMac);
VOID            tmStatCbAdd (ip4_addr_t  *pip4Addr,  PTM_STAT_ENTRY  ptmNode);
VOID            tmStatCbAddByMac (UINT8  *pucMac, PTM_STAT_ENTRY  ptmNode);
VOID            tmStatCbTableInit (VOID);

#endif                                                                  /* __TMHASH_H                   */
