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
** 文   件   名: versionEntry.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 8 日
**
** 描        述: version处理入口
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
#include "version_common.h"

/*********************************************************************************************************
** 函数名称: versionShowDeal
** 功能描述: 查询version处理函数
** 输　入  : pVersionHandle     version信息
** 输　出  : none
** 返　回  : none
*********************************************************************************************************/
VOID  versionShowDeal (__PVERSION_HANDLE  pVersionHandle)
{
    __VERSION_HANDLE  versionHeader;

    memset(versionHeader.cVer, 0, sizeof(versionHeader.cVer));
    strncpy(versionHeader.cVer, OPENRT_VERSION, sizeof(versionHeader.cVer));

    replyQueryMsg(OPENRT_GET_VERSION_CMD, sizeof(__VERSION_HANDLE), 1 , &versionHeader);
}

/*********************************************************************************************************
** 函数名称: versionHandleEntry
** 功能描述: version 入口
** 输　入  : iCommand           服务命令
**           pvArg              处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID versionHandleEntry (INT  iCommand, PVOID  pvArg)
{
    switch (iCommand) {
    case OPENRT_GET_VERSION_CMD:
        versionShowDeal(pvArg);
        break;

    default:
        break;
    }
}

