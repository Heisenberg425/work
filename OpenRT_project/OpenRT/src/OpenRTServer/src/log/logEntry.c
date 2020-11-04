/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: logEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 17 ��
**
** ��        ��: OpenRT LOG �������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "log_common.h"
#include "liblog_config.h"
/*********************************************************************************************************
** ��������: __indexCheck
** ��������: ��ȡ�ܵ���Ҫ��ʾ����Ŀ���Լ����һ����Ŀ��index
** �䡡��  : llStartIndex       ��ʼ����Ŀindex
**           llEndIndex         �ƻ��Ľ�����Ŀindex
**           iAllIterm          �ܵ���Ŀ��
** �䡡��  : pllEndIndex        ����ת�����������Ŀindex
** ������  : iItermCount        �ܹ���ʾ����Ŀ����
*********************************************************************************************************/
static INT32  __indexChange (INT64  llStartIndex,
                             INT64  llEndIndex,
                             INT32  iAllIterm,
                             INT64  *pllEndIndex)
{
    INT32  iItermCount = 0;

    *pllEndIndex  = llEndIndex;

    /*
     *  �����ڴ棬���ڱ������е���Ϣ
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
** ��������: __udialGetInfo
** ��������: ��ȡ������־��Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
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
     *  �����ڴ棬���ڱ������е���Ϣ
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
** ��������: __udialLogNumGet
** ��������: ��ȡ��־��Ŀ�������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialLogNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_UDIAL_LOG_NUM_ACK, udialLogNumGet(pArgBuf));
}

/*********************************************************************************************************
** ��������: __vpnLogGet
** ��������: ��ȡ VPN ��־��Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
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
     *  �����ڴ棬���ڱ������е���Ϣ
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
** ��������: __vpnLogNumGet
** ��������: ��ȡ VPN ��־��Ŀ�������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vpnLogNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_VPN_LOG_NUM_ACK, vpnLogNumGet(pArgBuf));
}

/*********************************************************************************************************
** ��������: __operLogRead
** ��������: ��ȡ������־��Ϣ
** �䡡��  : pOperLog          ������־��Ϣ
** �䡡��  : NONE
** ������  : NONE
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
** ��������: __operLogWrite
** ��������: д�������־��Ϣ
** �䡡��  : pOperLog          ������־��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __operLogWrite (__POPER_LOG  pOperLog)
{
    replyConfigMsg(OPENRT_OPER_LOG_PUT_ACK, operLog(pOperLog));
}
/*********************************************************************************************************
** ��������: __udialLogDel
** ��������: ���������־��Ϣ
** �䡡��  : pArgBuf          ������־��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_UDIAL_LOG_DEL_ACK, udialLogDel());
}
/*********************************************************************************************************
** ��������: __operLogDel
** ��������: ���������־��Ϣ
** �䡡��  : pArgBuf          ������־��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __operLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_OPER_LOG_DEL_ACK, operLogDel());
}
/*********************************************************************************************************
** ��������: __vpnLogDel
** ��������: ��� VPN ��־��Ϣ
** �䡡��  : pArgBuf          VPN ��־��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vpnLogDel (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_VPN_LOG_DEL_ACK, vpnLogDel());
}
/*********************************************************************************************************
** ��������: logHandleEntry
** ��������: LOG ���
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
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

