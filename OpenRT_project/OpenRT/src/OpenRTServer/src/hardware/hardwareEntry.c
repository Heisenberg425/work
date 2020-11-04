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
** 文   件   名: hardwareEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 2 日
**
** 描        述: OpenRT 硬件信息获取入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "msgcomm.h"
#include "hardware_common.h"
/*********************************************************************************************************
** 函数名称: __hwInfoGet
** 功能描述: 获取硬件信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __hwInfoGet (PVOID  pArgBuf)
{
    __HW_INFO   hwInfo;

    bzero(&hwInfo, sizeof(hwInfo));
    getHwInfo(&hwInfo);
    replyQueryMsg(OPENRT_HW_INFO_GET_ACK_CMD, sizeof(__HW_INFO), 1, &hwInfo);
}

/*********************************************************************************************************
** 函数名称: hwInfoHandleEntry
** 功能描述: 硬件信息获取入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID hwInfoHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_HW_INFO_GET_CMD:
        __hwInfoGet(pArgBuf);
        break;

    default:
        break;
    }
}
