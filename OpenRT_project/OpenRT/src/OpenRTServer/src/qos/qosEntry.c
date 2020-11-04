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
** 文   件   名: qosEntry.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 15 日
**
** 描        述: QoS配置处理入口
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
#include "qos_common.h"

/*********************************************************************************************************
** 函数名称: qosConfigDeal
** 功能描述: 配置 QoS 处理函数
** 输　入  : pQosInfo     QoS 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  qosConfigDeal (__PQOS_HANDLE  pQosInfo)
{
    replyConfigMsg(OPENRT_CFG_QOS_CMD, qosDoConfig(pQosInfo));
}
/*********************************************************************************************************
** 函数名称: qosShowDeal
** 功能描述: 查询QoS处理函数
** 输　入  : pQosInfo   QoS 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  qosShowDeal (__PQOS_HANDLE  pQosInfo)
{
    __PQOS_HANDLE  pQosArrayHeader = LW_NULL;
    INT            iRecordNum      = 0;

    pQosArrayHeader = qosGetFromDB(&iRecordNum);                        /*  获取数据                    */

    replyQueryMsg(OPENRT_GET_QOS_CMD, sizeof(__QOS_HANDLE), iRecordNum , pQosArrayHeader);

    if (NULL != pQosArrayHeader) {
        free(pQosArrayHeader);
    }
}
/*********************************************************************************************************
** 函数名称: qosDelDeal
** 功能描述: 删除 QoS 处理函数
** 输　入  : pQosInfo     QoS 参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  qosDelDeal (__PQOS_HANDLE  pQosInfo)
{
    replyConfigMsg(OPENRT_DEL_QOS_CMD, qosDoDel(pQosInfo));
}
/*********************************************************************************************************
** 函数名称: qosHandleEntry
** 功能描述: QoS配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID qosHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_QOS_CMD:
        qosConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_QOS_CMD:
        qosShowDeal(pArgBuf);
        break;

    case OPENRT_DEL_QOS_CMD:
        qosDelDeal(pArgBuf);
        break;

    default:
        break;
    }
}
