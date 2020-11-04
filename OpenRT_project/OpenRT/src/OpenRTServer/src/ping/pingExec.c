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
** 文   件   名: pingExec.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: ping 测试设置操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ping_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define DATA_SIZE_DEFAULT          (32)                                 /*  默认 PING 数据长度          */
#define PING_INFO_FILE             "/tmp/pingInfo"                      /*  存放 PING 结果信息文件位置  */
/*********************************************************************************************************
** 函数名称: pingTestDoDeal
** 功能描述: Ping 测试
** 输　入  : pPingInfo  PING 测试信息
** 输　出  : NONE
** 返　回  : PING 测试结果
*********************************************************************************************************/
PVOID  pingTestDoDeal (__PPING_HANDLE  pPingInfo)
{
    INT     iRet;
    INT     iFd;
    PCHAR   pcResult       = NULL;
    CHAR    cDomain[128]   = {0};
    CHAR    cEnetName[32]  = {0};
    CHAR    cCmdBuf[256]   = {0};
    INT     iDataSize      = 0;

    /*
     *  获取 PING 测试参数
     */
    iDataSize = pPingInfo->iDataSize;
    strncpy(cDomain, pPingInfo->cDomain,     sizeof(cDomain));
    strncpy(cEnetName, pPingInfo->cEnetName, sizeof(cEnetName));

    if (iDataSize <= 0) {
        iDataSize = DATA_SIZE_DEFAULT;
    }

    /*
     *  获取 PING 测试命令
     */
    sprintf(cCmdBuf, "ping %s -n %d -l %d &>%s", cDomain, 1, iDataSize, PING_INFO_FILE);

    lib_system(cCmdBuf);

    pcResult = (PCHAR)malloc(PING_TEST_RESULT_LEN);
    if (NULL == pcResult) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc memory failed.\n");

        return  (NULL);
    }

    bzero(pcResult, PING_TEST_RESULT_LEN);

    iFd = open(PING_INFO_FILE, O_RDONLY);
    if (iFd <= 2) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "open file failed.\n");

        goto  error;
    }

    /*
     *  读取 PING 测试结果
     */
    iRet = read(iFd, pcResult, PING_TEST_RESULT_LEN);
    if (iRet <= 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "read file failed.\n");

        goto  error;
    }

    close(iFd);

    return  (pcResult);

error:
    if (iFd > 2) {
        close(iFd);
    }

    if (NULL != pcResult) {
        free(pcResult);

        pcResult = NULL;
    }

    return  (NULL);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
