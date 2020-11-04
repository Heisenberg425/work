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
** 文   件   名: devnameHandle.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统设备名管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "devname_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: devNameShow
** 功能描述: 显示系统设备名
** 输　入  : pDevNameHandle      设备名入参信息
** 输　出  : piRecordTotal       结果总数
** 返　回  : 获取的系统设备名结构体指针
*********************************************************************************************************/
PVOID  devNameShow (__PDEVNAME_HANDLE  pDevNameHandle, INT  *piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_DEV_NAME_CMD,
                            pDevNameHandle,
                            sizeof(__DEVNAME_HANDLE),
                            piRecordTotal);
}
/*********************************************************************************************************
** 函数名称: devNameConfig
** 功能描述: 配置系统时间
** 输　入  : pDevNameHandle      设备名入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  devNameConfig (__PDEVNAME_HANDLE  pDevNameHandle)
{
    return  requestConfigMsg (OPENRT_SET_DEV_NAME_CMD, pDevNameHandle, sizeof(__DEVNAME_HANDLE));
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
