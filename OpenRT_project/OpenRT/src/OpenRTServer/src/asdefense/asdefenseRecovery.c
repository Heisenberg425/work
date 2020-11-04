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
** 文   件   名: asdefenseRecovery.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 9 月 12 日
**
** 描        述: ARP 防御恢复
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
extern INT32  asDefenseGetFromDB (BOOL  *pbStatus);
/*********************************************************************************************************
** 函数名称: asDefenseRecovry
** 功能描述: 恢复 ARP 防御
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID asDefenseRecovry (VOID)
{
    BOOL    bStatus;
    INT     iRet;

    iRet = asDefenseGetFromDB(&bStatus);
    if (ERROR_NONE == iRet && LW_TRUE == bStatus) {
        system("asden 1");
    }
}
