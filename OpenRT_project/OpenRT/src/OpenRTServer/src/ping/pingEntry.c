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
** 文   件   名: pingEntry.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统 PING 配置入口
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
#include "ping_common.h"

/*********************************************************************************************************
** 函数名称: pingTestDeal
** 功能描述: PING 测试处理函数
** 输　入  : pPingInfo  ping 测试参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  pingTestDeal (__PPING_HANDLE  pPingInfo)
{
    PCHAR  pcPingResult = NULL;

    pcPingResult = pingTestDoDeal(pPingInfo);
    if (NULL != pcPingResult) {
        replyQueryMsg(OPENRT_PING_TEST_ACK_CMD, PING_TEST_RESULT_LEN, 1, pcPingResult);

        free(pcPingResult);

        pcPingResult = NULL;
    }

    return;
}
/*********************************************************************************************************
** 函数名称: pingHandleEntry
** 功能描述: PING 测试命令配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID pingHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_PING_TEST_CMD:
        pingTestDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
