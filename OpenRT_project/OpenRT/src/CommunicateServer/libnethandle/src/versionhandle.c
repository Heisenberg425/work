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
** 文   件   名: versionhandle.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 8 日
**
** 描        述:  系统version处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "version_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: versionShow
** 功能描述: 查询系统版本
** 输　入  : pVersionHandle     version入参信息
** 输　出  : none
** 返　回  : version查询结果数组指针
*********************************************************************************************************/
PVOID  versionShow (__PVERSION_HANDLE  pVersionHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_VERSION_CMD, pVersionHandle, sizeof(__VERSION_HANDLE), piRecordTotal);
}
