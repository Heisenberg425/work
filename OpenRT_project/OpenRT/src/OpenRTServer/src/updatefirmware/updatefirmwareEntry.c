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
** 文   件   名: updatefirmwareEntry.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 3 日
**
** 描        述: update firmware 处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "updatefirmware_common.h"

/*********************************************************************************************************
** 函数名称: updateFirmwareConfigDeal
** 功能描述: 更新固件处理函数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  updateFirmwareConfigDeal (VOID)
{
    replyConfigMsg(OPENRT_CFG_UPDATEFIRMWARE_CMD, updateFirmwareDoConfig());
}
/*********************************************************************************************************
** 函数名称: updateFirmwareHandleEntry
** 功能描述: update firmware 配置入口
** 输　入  : iCommand           服务命令
**           pvArg              处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID updateFirmwareHandleEntry (INT  iCommand, PVOID  pvArg)
{
    switch (iCommand) {
    case OPENRT_CFG_UPDATEFIRMWARE_CMD:
        updateFirmwareConfigDeal();
        break;

    default:
        break;
    }
}

