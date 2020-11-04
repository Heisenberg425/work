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
** 描        述: OpenRT 流量监控管理 hash 表操作
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include "tmHash.h"
#include "common.h"

/*********************************************************************************************************
  hash 表相关全局变量
*********************************************************************************************************/
PTM_STAT_ENTRY  __G_tmStatCbtable[TM_STAT_CB_SIZE];                     /*  终端流量统计项控制块 hash 表*/
PTM_STAT_ENTRY  __G_tmLineStatCbtable[TM_STAT_CB_SIZE];                 /*  线路流量统计项控制块 hash 表*/
/*********************************************************************************************************
** 函数名称: tmStatCbTableInit
** 功能描述: 流量统计项控制块hash表初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  tmStatCbTableInit (VOID)
{
    bzero(__G_tmStatCbtable, sizeof(__G_tmStatCbtable));
}

/*********************************************************************************************************
** 函数名称: tmStatCbFindByIp
** 功能描述: 依据 IP 查询流量统计项控制块入口
** 输　入  : pip4Addr     hash 函数 key
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByIp (ip4_addr_t*  pip4Addr)
{
    return  __G_tmStatCbtable[TMCBHASH(pip4Addr)];
}

/*********************************************************************************************************
** 函数名称: tmStatCbFindByMac
** 功能描述: 依据 MAC 查询流量统计项控制块入口
** 输　入  : pucMac     hash 函数 key
** 输　出  : NONE
** 返　回  : 流量统计控制块指针
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByMac (UINT8  *pucMac)
{
    return  __G_tmLineStatCbtable[TMCB_MAC_HASH(*(pucMac + 5))];
}


/*********************************************************************************************************
** 函数名称: tmStatCbAdd
** 功能描述: 新增流量统计项控制块
** 输　入  : pip4Addr  IP 地址作为 hash key 值
**           ptmNode   待增加的控制块节点
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  tmStatCbAdd (ip4_addr_t  *pip4Addr, PTM_STAT_ENTRY  ptmNode)
{
    PLW_LIST_LINE   pline;
    PTM_STAT_ENTRY  pTmStatEntry = tmStatCbFindByIp(pip4Addr);

    if (NULL == pTmStatEntry){
        __G_tmStatCbtable[TMCBHASH(pip4Addr)] = ptmNode;
    } else {
        for (pline = (PLW_LIST_LINE)pTmStatEntry;
             pline != NULL;
             pline = _list_line_get_next(pline)) {
            ;
        }

        _List_Line_Add_Tail(&ptmNode->tmlist, (LW_LIST_LINE_HEADER  *)&pTmStatEntry);
    }
}

/*********************************************************************************************************
** 函数名称: tmStatCbAddByMac
** 功能描述: 新增流量统计项控制块
** 输　入  : pucMac      MAC 地址作为 hash key 值
**           ptmNode     待增加的控制块节点
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  tmStatCbAddByMac (UINT8  *pucMac, PTM_STAT_ENTRY  ptmNode)
{
    PLW_LIST_LINE   pline;
    PTM_STAT_ENTRY  pTmStatEntry = tmStatCbFindByMac(pucMac);

    if (NULL == pTmStatEntry){
        __G_tmLineStatCbtable[TMCB_MAC_HASH(*(pucMac + 5))] = ptmNode;
    } else {
        for (pline = (PLW_LIST_LINE)pTmStatEntry;
             pline != NULL;
             pline = _list_line_get_next(pline)) {
           ;
        }

        _List_Line_Add_Tail(&ptmNode->tmlist, (LW_LIST_LINE_HEADER  *)&pTmStatEntry);
    }
}
