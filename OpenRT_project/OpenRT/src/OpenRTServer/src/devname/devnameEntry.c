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
** 文   件   名: devnameEntry.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 设备名称配置入口
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
#include "devname_common.h"
/*********************************************************************************************************
** 函数名称: devNameShowDeal
** 功能描述: 设备名称显示处理函数
** 输　入  : pdevNameInfo  设备名称参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  devNameShowDeal (__PDEVNAME_HANDLE  pdevNameInfo)
{
    INT                iNum;
    __PDEVNAME_HANDLE  pdevNameHeader = NULL;
    __DEVNAME_HANDLE   devNameInfo;

    pdevNameHeader = devNameGetFromDB(&iNum);
    if (NULL != pdevNameHeader) {
        if ('\0' == pdevNameHeader->cdevName[0]) {
            lib_memcpy(pdevNameHeader->cdevName, DEFAULT_DEV_NAME, strlen(DEFAULT_DEV_NAME) + 1);
        }

        replyQueryMsg(OPENRT_GET_DEV_NAME_ACK_CMD, sizeof(__DEVNAME_HANDLE), 1, pdevNameHeader);

        free(pdevNameHeader);
    } else {
        bzero(&devNameInfo, sizeof(__DEVNAME_HANDLE));
        lib_memcpy(devNameInfo.cdevName, DEFAULT_DEV_NAME, strlen(DEFAULT_DEV_NAME) + 1);

        replyQueryMsg(OPENRT_GET_DEV_NAME_ACK_CMD, sizeof(__DEVNAME_HANDLE), 1, &devNameInfo);
    }
}
/*********************************************************************************************************
** 函数名称: devNameConfigDeal
** 功能描述: 设备名称配置处理函数
** 输　入  : pdevNameInfo  设备名称参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  devNameConfigDeal (__PDEVNAME_HANDLE  pdevNameInfo)
{
    INT  iRet;

    iRet = devNameUpdateToDb(pdevNameInfo);

    replyConfigMsg(OPENRT_SET_DEV_NAME_ACK_CMD, iRet);
}
/*********************************************************************************************************
** 函数名称: devNameHandleEntry
** 功能描述: 设备名称配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID   devNameHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_SET_DEV_NAME_CMD:
        devNameConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_DEV_NAME_CMD:
        devNameShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
