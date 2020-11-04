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
** 文   件   名: logEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 17 日
**
** 描        述: OpenRT LOG 处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "log_common.h"
#include "liblog_config.h"
/*********************************************************************************************************
** 函数名称: __indexCheck
** 功能描述: 获取总的需要显示的条目数以及最后一个条目的index
** 输　入  : llStartIndex       开始的条目index
**           llEndIndex         计划的结束条目index
**           iAllIterm          总的条目数
** 输　出  : pllEndIndex        最终转换后结束的条目index
** 返　回  : iItermCount        总共显示的条目个数
*********************************************************************************************************/
static INT32  __indexChange (INT64  llStartIndex,
                             INT64  llEndIndex,
                             INT32  iAllIterm,
                             INT64  *pllEndIndex)
{
    INT32  iItermCount = 0;

    *pllEndIndex  = llEndIndex;

    /*
     *  申请内存，用于保存所有的信息
     */
    if (llStartIndex <= iAllIterm) {
        if (llEndIndex > iAllIterm) {
            *pllEndIndex = iAllIterm;
            iItermCount = iAllIterm - llStartIndex + 1;
        } else {
            iItermCount = llEndIndex - llStartIndex + 1;
        }
    }

    return  (iItermCount);
}
/*********************************************************************************************************
** 函数名称: __udialGetInfo
** 功能描述: 获取拨号日志信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialLogGet (PVOID  pArgBuf)
{
    __PUDIAL_LOG_HANDLE  pUdialLogHandle = LW_NULL;
    INT32                iItermNum       = 0;
    INT32                iItermCount     = 0;
    INT32                iBufLen         = 0;
    PCHAR                pcInfoBuff      = LW_NULL;
    INT                  iRet            = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pUdialLogHandle = (__PUDIAL_LOG_HANDLE)pArgBuf;
    iItermNum       = udialLogNumGet(pUdialLogHandle);
    if (iItermNum <= 0) {
        replyQueryMsg(OPENRT_UDIAL_LOG_GET_ACK, sizeof(__UDIAL_LOG_INFO), iItermCount, pcInfoBuff);

        return;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    iItermCount = __indexChange(pUdialLogHandle->llStartIndex,
                                pUdialLogHandle->llEndIndex,
                                iItermNum,
                                &(pUdialLogHandle->llEndIndex));
    if (!iItermCount) {
        replyQueryMsg(OPENRT_UDIAL_LOG_GET_ACK, sizeof(__UDIAL_LOG_INFO), iItermCount, pcInfoBuff);

        return;
    }

    iBufLen    = iItermCount * sizeof(__UDIAL_LOG_INFO);
    pcInfoBuff = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcInfoBuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_UDIAL_LOG_GET_ACK, sizeof(__UDIAL_LOG_INFO), 0, pcInfoBuff);

        return;
    }

    bzero(pcInfoBuff, iBufLen);
    iRet = udialLogGetSpecInfo(pUdialLogHandle->logStartTm,
                               pUdialLogHandle->logEndTm,
                               pUdialLogHandle->llStartIndex,
                               pUdialLogHandle->llEndIndex,
                               pcInfoBuff,
                               iBufLen);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "udialLogGetSpecInfo error\n");
        replyQueryMsg(OPENRT_UDIAL_LOG_GET_ACK, sizeof(__UDIAL_LOG_INFO), 0, pcInfoBuff);

        free(pcInfoBuff);
        return;
    }

    replyQueryMsg(OPENRT_UDIAL_LOG_GET_ACK, sizeof(__UDIAL_LOG_INFO), iItermCount, pcInfoBuff);

    free(pcInfoBuff);
}

/*********************************************************************************************************
** 函数名称: __udialLogNumGet
** 功能描述: 获取日志条目个数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialLogNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_UDIAL_LOG_NUM_ACK, udialLogNumGet(pArgBuf));
}

/*********************************************************************************************************
** 函数名称: __vpnLogGet
** 功能描述: 获取 VPN 日志信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vpnLogGet (PVOID  pArgBuf)
{
    __PVPN_LOG_HANDLE    pVpnLogHandle = LW_NULL;
    INT32                iItermNum       = 0;
    INT32                iItermCount     = 0;
    INT32                iBufLen         = 0;
    PCHAR                pcInfoBuff      = LW_NULL;
    INT                  iRet            = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pVpnLogHandle = (__PVPN_LOG_HANDLE)pArgBuf;
    iItermNum     =  vpnLogNumGet(pVpnLogHandle);
    if (iItermNum <= 0) {
        replyQueryMsg(OPENRT_VPN_LOG_GET_ACK, sizeof(__KIDVPN_LOG_INFO), iItermCount, pcInfoBuff);

        return;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    iItermCount = __indexChange(pVpnLogHandle->llStartIndex,
                                pVpnLogHandle->llEndIndex,
                                iItermNum,
                                &(pVpnLogHandle->llEndIndex));
    if (!iItermCount) {
        replyQueryMsg(OPENRT_VPN_LOG_GET_ACK, sizeof(__KIDVPN_LOG_INFO), iItermCount, pcInfoBuff);

        return;
    }

    iBufLen    = iItermCount * sizeof(__KIDVPN_LOG_INFO);
    pcInfoBuff = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcInfoBuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_VPN_LOG_GET_ACK, sizeof(__KIDVPN_LOG_INFO), 0, pcInfoBuff);

        return;
    }

    bzero(pcInfoBuff, iBufLen);
    iRet = vpnLogGetSpecInfo(pVpnLogHandle->logStartTm,
                             pVpnLogHandle->logEndTm,
                             pVpnLogHandle->iVndID,
                             pVpnLogHandle->llStartIndex,
                             pVpnLogHandle->llEndIndex,
                             pcInfoBuff,
                             iBufLen);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get vpn log info error\n");
        replyQueryMsg(OPENRT_VPN_LOG_GET_ACK, sizeof(__KIDVPN_LOG_INFO), 0, pcInfoBuff);

        free(pcInfoBuff);

        return;
    }

    replyQueryMsg(OPENRT_VPN_LOG_GET_ACK, sizeof(__KIDVPN_LOG_INFO), iItermCount, pcInfoBuff);

    free(pcInfoBuff);
}

/*********************************************************************************************************
** 函数名称: __vpnLogNumGet
** 功能描述: 获取 VPN 日志条目个数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vpnLogNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_VPN_LOG_NUM_ACK, vpnLogNumGet(pArgBuf));
}

/*********************************************************************************************************
** 函数名称: __operLogRead
** 功能描述: 获取操作日志信息
** 输　入  : pOperLog          操作日志信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __operLogRead (__POPER_LOG pOperLog)
{
	__POPER_LOG   pOperLogArrayHead = LW_NULL;
    INT           iRecordNum        = 0;

    pOperLogArrayHead = operLogGetFromDB(pOperLog, &iRecordNum);

    replyQueryMsg(OPENRT_OPER_LOG_GET_ACK, sizeof(__OPER_LOG), iRecordNum , pOperLogArrayHead);

    free(pOperLogArrayHead);
}
/*********************************************************************************************************
** 函数名称: __operLogWrite
** 功能描述: 写入操作日志信息
** 输　入  : pOperLog          操作日志信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __operLogWrite (__POPER_LOG  pOperLog)
{
    replyConfigMsg(OPENRT_OPER_LOG_PUT_ACK, operLog(pOperLog));
}
/*********************************************************************************************************
** 函数名称: __udialLogDel
** 功能描述: 清除拨号日志信息
** 输　入  : pArgBuf          拨号日志信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_UDIAL_LOG_DEL_ACK, udialLogDel());
}
/*********************************************************************************************************
** 函数名称: __operLogDel
** 功能描述: 清除操作日志信息
** 输　入  : pArgBuf          操作日志信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __operLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_OPER_LOG_DEL_ACK, operLogDel());
}
/*********************************************************************************************************
** 函数名称: __vpnLogDel
** 功能描述: 清除 VPN 日志信息
** 输　入  : pArgBuf          VPN 日志信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vpnLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_VPN_LOG_DEL_ACK, vpnLogDel());
}
/*********************************************************************************************************
** 函数名称: logHandleEntry
** 功能描述: LOG 入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  logHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_UDIAL_LOG_GET:
        __udialLogGet(pArgBuf);
        break;

    case OPENRT_UDIAL_LOG_NUM:
        __udialLogNumGet(pArgBuf);
        break;

    case OPENRT_VPN_LOG_GET:
        __vpnLogGet(pArgBuf);
        break;

    case OPENRT_VPN_LOG_NUM:
        __vpnLogNumGet(pArgBuf);
        break;

    case OPENRT_OPER_LOG_GET:
    	__operLogRead(pArgBuf);
        break;

    case OPENRT_OPER_LOG_PUT:
    	__operLogWrite(pArgBuf);
        break;

    case OPENRT_UDIAL_LOG_DEL:
        __udialLogDel(pArgBuf);
        break;

    case OPENRT_OPER_LOG_DEL:
        __operLogDel(pArgBuf);
        break;

    case OPENRT_VPN_LOG_DEL:
        __vpnLogDel(pArgBuf);
        break;

    default:
        break;
    }
}

