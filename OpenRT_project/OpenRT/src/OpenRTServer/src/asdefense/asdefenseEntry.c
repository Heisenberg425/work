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
** 文   件   名: asdefenseEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: OpenRT  ARP 防御操作入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "libasdefense_config.h"
#include "asdefense_common.h"
/*********************************************************************************************************
** 函数名称: __asDefenseHandle
** 功能描述: ARP 防御开启与关闭操作入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseHandle (PVOID  pArgBuf)
{
    INT     iRet;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_GET_ASD_EN_ACK_CMD, PX_ERROR);

        return;
    }

    if (*(BOOL *)pArgBuf) {
        iRet = (system("asden 1") == ERROR_NONE) ? asDefenseUpdateToDb(LW_TRUE) : (PX_ERROR);
    } else {
        iRet = (system("asden 0") == ERROR_NONE) ? asDefenseUpdateToDb(LW_FALSE) : (PX_ERROR);
    }

    replyConfigMsg(OPENRT_GET_ASD_EN_ACK_CMD, iRet);
}

/*********************************************************************************************************
** 函数名称: __asDefenseGetInfo
** 功能描述: 获取防御信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseGetInfo (PVOID  pArgBuf)
{
    PCHAR                 pcBuff     = LW_NULL;
    __PASDEFENSE_GET      pAsInfoGet = LW_NULL;
    INT                   iNum       = 0;
    INT                   iCount     = 0;
    INT                   iBuffLen   = 0;

    if (LW_NULL == pArgBuf) {
        replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD,
                      sizeof(__ASDEFENSE_INFO),
                      iNum,
                      pcBuff);

        return;
    }

    pAsInfoGet = (__PASDEFENSE_GET)pArgBuf;
    iNum = asDefenseGetSpecInfoNum(pAsInfoGet->ucType);                 /*  获取总条目个数              */
    if (0 == iNum) {
        replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD,
                      sizeof(__ASDEFENSE_INFO),
                      iNum,
                      pcBuff);
        return;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    if (pAsInfoGet->iStartIndex <= iNum) {
        if (pAsInfoGet->iEndIndex > iNum) {
            pAsInfoGet->iEndIndex = iNum;
            iCount = iNum - pAsInfoGet->iStartIndex + 1;
        } else {
            iCount = pAsInfoGet->iEndIndex - pAsInfoGet->iStartIndex + 1;
        }
    } else {
        /*
         *  这种情况一般不会出现
         */
        replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD,
                      sizeof(__ASDEFENSE_INFO),
                      iCount,
                      pcBuff);
        return;
    }

    iBuffLen = iCount * sizeof(__ASDEFENSE_INFO);
    pcBuff   = (PCHAR)malloc(iBuffLen);
    if (LW_NULL == pcBuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return;
    }

    bzero(pcBuff, iBuffLen);
    asDefenseGetSpecInfo(pAsInfoGet->iStartIndex,
                         pAsInfoGet->iEndIndex,
                         pAsInfoGet->ucType,
                         pcBuff,
                         iBuffLen);                                     /*  获取信息                    */
    replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD, sizeof(__ASDEFENSE_INFO), iCount, pcBuff);
    free(pcBuff);
}

/*********************************************************************************************************
** 函数名称: __asDefenseNumGet
** 功能描述: 获取指定信息个数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseNumGet (PVOID  pArgBuf)
{
    INT32                iItermNum       = 0;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_GET_ASD_NUM_ACK_CMD, 0);

        return;
    }

    iItermNum  = asDefenseGetSpecInfoNum(*(INT32 *)pArgBuf);
    if (iItermNum <= 0) {
        replyConfigMsg(OPENRT_GET_ASD_NUM_ACK_CMD, 0);

        return;
    }

    replyConfigMsg(OPENRT_GET_ASD_NUM_ACK_CMD, iItermNum);
}

/*********************************************************************************************************
** 函数名称: __asDefenseStatusGet
** 功能描述: 获取 ARP 防御开启状态入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asDefenseStatusGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_GET_ASD_STATUS_ACK_CMD, asDefenseStatus());
}

/*********************************************************************************************************
** 函数名称: dhcpHandleEntry
** 功能描述: DHCP 操作入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID asDefenseHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_GET_ASD_EN_CMD:
        __asDefenseHandle(pArgBuf);
        break;

    case OPENRT_GET_ASD_GET_CMD:
        __asDefenseGetInfo(pArgBuf);
        break;

    case OPENRT_GET_ASD_NUM_CMD:
        __asDefenseNumGet(pArgBuf);
        break;

    case OPENRT_GET_ASD_STATUS_CMD:
        __asDefenseStatusGet(pArgBuf);
        break;

    default:
        break;
    }
}
