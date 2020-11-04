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
** 文   件   名: updatefirmwarehandle.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 3 日
**
** 描        述:  update firmware管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "updatefirmware_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: updateFirmwareConfig
** 功能描述: 更新系统固件
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  updateFirmwareConfig (void)
{
    return  requestConfigMsg (OPENRT_CFG_UPDATEFIRMWARE_CMD, LW_NULL, 0);
}
