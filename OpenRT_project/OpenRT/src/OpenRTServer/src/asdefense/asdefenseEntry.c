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
** ��   ��   ��: asdefenseEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT  ARP �����������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "libasdefense_config.h"
#include "asdefense_common.h"
/*********************************************************************************************************
** ��������: __asDefenseHandle
** ��������: ARP ����������رղ������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
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
** ��������: __asDefenseGetInfo
** ��������: ��ȡ������Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
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
    iNum = asDefenseGetSpecInfoNum(pAsInfoGet->ucType);                 /*  ��ȡ����Ŀ����              */
    if (0 == iNum) {
        replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD,
                      sizeof(__ASDEFENSE_INFO),
                      iNum,
                      pcBuff);
        return;
    }

    /*
     *  �����ڴ棬���ڱ������е���Ϣ
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
         *  �������һ�㲻�����
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
                         iBuffLen);                                     /*  ��ȡ��Ϣ                    */
    replyQueryMsg(OPENRT_GET_ASD_GET_ACK_CMD, sizeof(__ASDEFENSE_INFO), iCount, pcBuff);
    free(pcBuff);
}

/*********************************************************************************************************
** ��������: __asDefenseNumGet
** ��������: ��ȡָ����Ϣ�������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
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
** ��������: __asDefenseStatusGet
** ��������: ��ȡ ARP ��������״̬���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __asDefenseStatusGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_GET_ASD_STATUS_ACK_CMD, asDefenseStatus());
}

/*********************************************************************************************************
** ��������: dhcpHandleEntry
** ��������: DHCP �������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
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
