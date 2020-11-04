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
** 文   件   名: qoshandle.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 16 日
**
** 描        述:  QoS管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "qos_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: qosShow
** 功能描述: 查询qos信息
** 输　入  : pQosHandle         QoS入参信息
** 输　出  : piRecordTotal      QoS记录查询结果总数
** 返　回  : QoS查询结果数组指针
*********************************************************************************************************/
PVOID  qosShow (__PQOS_HANDLE  pQosHandle, INT*  piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_QOS_CMD, pQosHandle, sizeof(__QOS_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: qosConfig
** 功能描述: 配置qos
** 输　入  : pQosHandle         QoS入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  qosConfig (__PQOS_HANDLE  pQosHandle)
{
    return  requestConfigMsg (OPENRT_CFG_QOS_CMD, pQosHandle, sizeof(__QOS_HANDLE));
}

/*********************************************************************************************************
** 函数名称: qosDelete
** 功能描述: 删除qos
** 输　入  : pQosHandle         QoS入参信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT  qosDelete (__PQOS_HANDLE  pQosHandle)
{
    return  requestConfigMsg (OPENRT_DEL_QOS_CMD, pQosHandle, sizeof(__QOS_HANDLE));
}
