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
** ��   ��   ��: vndEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: OpenRT �������� ���ô������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "libvnd_config.h"
#include "vndExec.h"
#include "vndDB.h"
#include "msgcomm.h"
#include "interface/interface_common.h"
/*********************************************************************************************************
** ��������: __vndAdd
** ��������: ��������������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndAdd (PVOID  pArgBuf)
{
    __PVND_HANDLE     pVndHandle = LW_NULL;
    __IFPARAM_INFO    ifparamInfo;
    INT               iVndRet;
    INT               iVndID     = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pVndHandle = (__PVND_HANDLE)pArgBuf;
    /*
     *  todo: 1������ ifparam.ini ��Ϣ ,�ո�ϵͳ��ʶ��
     *        2�������ṹת�����
     */
    bzero(&ifparamInfo, sizeof(ifparamInfo));
    ifparamInfo.cDefault = 0;
    ifparamInfo.cEnable  = 1;
    snprintf(ifparamInfo.cDevName, sizeof(ifparamInfo.cDevName), "vnd-%d", vndGetNextId());
    ifparamInfo.uiIpAddr.addr  = pVndHandle->uiIpAddr.addr;
    ifparamInfo.uiNetMask.addr = pVndHandle->uiNetMask.addr;
    ifparamInfo.uiGateWay.addr = pVndHandle->uiGateWay.addr;
    strncpy((PCHAR)ifparamInfo.ucMac, (const CHAR *)pVndHandle->ucHwAddr, sizeof(ifparamInfo.ucMac));
    ifparamInfo.uiUpdateFlag = INI_UPDATE_FLAG;
    ifparamInfo.cHandle      = INI_UPDATE_SEC;

    ifParamIniFileHandle(&ifparamInfo);                                 /*  ����Ϣ���� ifparam.ini �ļ� */

    /*
     *  ���һ����������
     */
    iVndRet = vndStart(&iVndID, pVndHandle);
    if (PX_ERROR == iVndRet) {
        ifparamInfo.cHandle = INI_DEL_SEC;
        ifParamIniFileHandle(&ifparamInfo);                             /*  ɾ�� ifparam.ini �ļ�����Ϣ */
    }

    replyConfigMsg(OPENRT_ADD_VND_ACK_CMD, iVndID);
}

/*********************************************************************************************************
** ��������: __vndDel
** ��������: ɾ�������������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndDel (PVOID  pArgBuf)
{
    INT               iVndID  = 0;

    if (LW_NULL == pArgBuf) {
        return;
    }

    iVndID = *(INT *)pArgBuf;
    replyConfigMsg(OPENRT_DEL_VND_ACK_CMD, vndDel(iVndID));
}

/*********************************************************************************************************
** ��������: __vndGetInfo
** ��������: ��ȡ����������Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndGetInfo (PVOID  pArgBuf)
{
    PCHAR     pcVndAllInfo    = LW_NULL;
    INT       iNum            = 0;

    iNum            = vndGetDBRow();
    pcVndAllInfo    = vndGetDBInfo();
    replyQueryMsg(OPENRT_GET_VND_ACK_CMD, sizeof(__VND_HANDLE), iNum, pcVndAllInfo);

    free(pcVndAllInfo);
}
/*********************************************************************************************************
** ��������: __vndGetNetIfIndex
** ��������: ��ȡ�������� NetIfIndex
** �䡡��  : pArgBuf      ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndGetNetIfIndex (PVOID  pArgBuf)
{
    INT  iVndid = *(INT *)pArgBuf;
    INT  iNetIfIndex;
    INT  iRet;

    iRet = netIfIndexGetByVndId(iVndid, &iNetIfIndex);
    if (PX_ERROR == iRet) {
        replyQueryMsg(OPENRT_GET_IFINDEX_ACK_CMD, sizeof(INT), 0, NULL);

        return;
    }

    replyQueryMsg(OPENRT_GET_IFINDEX_ACK_CMD, sizeof(INT), 1, &iNetIfIndex);
}
/*********************************************************************************************************
** ��������: __vndGetVndid
** ��������: ��ȡ�������� VNDID
** �䡡��  : pArgBuf      ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndGetVndid (PVOID  pArgBuf)
{
    INT  iNetIfIndex = *(INT *)pArgBuf;
    INT  iVndid;
    INT  iRet;

    iRet = vndIdGetByNetIfIndex(iNetIfIndex, &iVndid);
    if (PX_ERROR == iRet) {
        replyQueryMsg(OPENRT_GET_VNDID_ACK_CMD, sizeof(INT), 0, NULL);

        return;
    }

    replyQueryMsg(OPENRT_GET_VNDID_ACK_CMD, sizeof(INT), 1, &iVndid);
}
/*********************************************************************************************************
** ��������: vndHandleEntry
** ��������: ���������������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID vndHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_ADD_VND_CMD:
        __vndAdd(pArgBuf);
        break;

    case OPENRT_DEL_VND_CMD:
        __vndDel(pArgBuf);
        break;

    case OPENRT_GET_VND_CMD:
        __vndGetInfo(pArgBuf);
        break;

    case OPENRT_GET_IFINDEX_CMD:
        __vndGetNetIfIndex(pArgBuf);
        break;

    case OPENRT_GET_VNDID_CMD:
        __vndGetVndid(pArgBuf);
        break;

    default:
        break;
    }
}
