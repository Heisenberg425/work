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
** ��   ��   ��: udialEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: OpenRT ���Ŵ������
*********************************************************************************************************/
#include "msgcomm.h"
#include "common.h"
#include "udial_common.h"
#include "udialExec.h"
#include "udialDB.h"
#include "libudial_config.h"
/*********************************************************************************************************
** ��������: __udialStart
** ��������: �����������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialStart (PVOID  pArgBuf)
{
    __PUDIAL_HANDLE  pUdialHandleInfo = LW_NULL;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pUdialHandleInfo = (__PUDIAL_HANDLE)pArgBuf;

    replyConfigMsg(OPENRT_UDIAL_DEL_ACK, udialStart(pUdialHandleInfo));
}

/*********************************************************************************************************
** ��������: __udialDel
** ��������: ɾ���������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialDel (PVOID  pArgBuf)
{
    PCHAR  pcNetifname = LW_NULL;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pcNetifname = (PCHAR)pArgBuf;

    replyConfigMsg(OPENRT_UDIAL_DEL_ACK, udialDel(pcNetifname));
}

/*********************************************************************************************************
** ��������: __udialGetInfo
** ��������: ��ȡ������Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialGetInfo (PVOID  pArgBuf)
{
    PCHAR           pcNetifname = LW_NULL;
    __UDIAL_HANDLE  udialInfo;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pcNetifname = (PCHAR)pArgBuf;

    bzero(&udialInfo, sizeof(udialInfo));
    udialGetSpecInfo(&udialInfo, "NETIF", pcNetifname, LW_NULL);        /*  ͨ�������������ƻ�ȡ��Ϣ    */

    accountInfoGetFromIniFile(pcNetifname, udialInfo.cUsrname, udialInfo.cPassword);

    replyQueryMsg(OPENRT_UDIAL_INFO_ACK, sizeof(udialInfo), 1, &udialInfo);
}

/*********************************************************************************************************
** ��������: udialHandleEntry
** ��������: �����������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID udialHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_UDIAL_START:
        __udialStart(pArgBuf);
        break;

    case OPENRT_UDIAL_DEL:
        __udialDel(pArgBuf);
        break;

    case OPENRT_UDIAL_INFO:
        __udialGetInfo(pArgBuf);
        break;

    default:
        break;
    }
}

