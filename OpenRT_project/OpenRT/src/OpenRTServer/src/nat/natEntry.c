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
** ��   ��   ��: natEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: OpenRT NAT ���ô������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "libnat_config.h"
#include "natExec.h"
#include "natStatus.h"
/*********************************************************************************************************
** ��������: __natAliasNumGet
** ��������: ��ȡ��������ܸ������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAliasNumGet (PVOID  pArgBuf)
{
    INT32     iItermNum       = 0;

    iItermNum  = natAliasItemNumGet();
    if (iItermNum <= 0) {
        replyConfigMsg(OPENRT_NAT_ALI_NUM_GET_ACK, 0);

        return;
    }

    replyConfigMsg(OPENRT_NAT_ALI_NUM_GET_ACK, iItermNum);
}

/*********************************************************************************************************
** ��������: __natAliasGet
** ��������: ��ȡ����������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAliasGet (PVOID  pArgBuf)
{
    PCHAR                 pcNatAliasInfoNBuf = LW_NULL;
    __PNAT_INFO_GET       pNatInfoGetVar     = LW_NULL;
    INT                   iBuffLen           = 0;
    INT                   iCount             = 0;
    INT                   iNatAliasNum       = 0;

    if (LW_NULL == pArgBuf) {
        goto  __back;
    }

    pNatInfoGetVar = (__PNAT_INFO_GET)pArgBuf;
    iNatAliasNum = natAliasItemNumGet();                                /*  ��ȡ����Ŀ����              */
    if (0 == iNatAliasNum) {
        goto  __back;
    }

    /*
     *  �����ڴ棬���ڱ������е���Ϣ
     */
    if (pNatInfoGetVar->iStartIndex <= iNatAliasNum) {
        if (pNatInfoGetVar->iEndIndex > iNatAliasNum) {
            pNatInfoGetVar->iEndIndex = iNatAliasNum;
            iCount = iNatAliasNum - pNatInfoGetVar->iStartIndex + 1;
        } else {
            iCount = pNatInfoGetVar->iEndIndex - pNatInfoGetVar->iStartIndex + 1;
        }
    } else {
        /*
         *  �������һ�㲻�����
         */
        goto  __back;
    }

    iBuffLen = iCount * sizeof(__NATALIAS_INFO);
    pcNatAliasInfoNBuf   = (PCHAR)malloc(iBuffLen);
    if (LW_NULL == pcNatAliasInfoNBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return;
    }

    bzero(pcNatAliasInfoNBuf, iBuffLen);
    natAliasInfoGet(pcNatAliasInfoNBuf, pNatInfoGetVar->iStartIndex, pNatInfoGetVar->iEndIndex);

__back:
    replyQueryMsg(OPENRT_NAT_ALIAS_GET_ACK, sizeof(__NATALIAS_INFO), iCount, pcNatAliasInfoNBuf);
    free(pcNatAliasInfoNBuf);
}

/*********************************************************************************************************
** ��������: __natAliasSet
** ��������: ��������������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAliasSet (PVOID  pArgBuf)
{
    __PNATALIAS_HANDLE   pNatAliasHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_ALIAS_SET_ACK, PX_ERROR);

        return;
    }
    pNatAliasHandle = (__PNATALIAS_HANDLE)pArgBuf;

    replyConfigMsg(OPENRT_NAT_ALIAS_SET_ACK, natAliasCfg(pNatAliasHandle));
}

/*********************************************************************************************************
** ��������: __natMapNumGet
** ��������: ��ȡ NAT �˿�ӳ���ܸ������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natMapNumGet (PVOID  pArgBuf)
{
    INT32     iItermNum       = 0;

    iItermNum  = natMapItemNumGet();
    if (iItermNum <= 0) {
        replyConfigMsg(OPENRT_NAT_MAP_NUM_GET_ACK, 0);

        return;
    }

    replyConfigMsg(OPENRT_NAT_MAP_NUM_GET_ACK, iItermNum);
}
/*********************************************************************************************************
** ��������: __natMapGet
** ��������: ��ȡ NAT �˿�ӳ�����
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natMapGet (PVOID  pArgBuf)
{
    PCHAR                 pNatMapInfoNBuf  = LW_NULL;
    __PNAT_INFO_GET       pNatInfoGetVar   = LW_NULL;
    INT                   iNatMapNum       = 0;
    INT                   iBuffLen         = 0;
    INT                   iCount           = 0;

    if (LW_NULL == pArgBuf) {
        goto  __back;
    }

    pNatInfoGetVar = (__PNAT_INFO_GET)pArgBuf;
    iNatMapNum = natMapItemNumGet();                                    /*  ��ȡ����Ŀ����              */
    if (0 == iNatMapNum) {
        goto  __back;
    }

    /*
     *  �����ڴ棬���ڱ������е���Ϣ
     */
    if (pNatInfoGetVar->iStartIndex <= iNatMapNum) {
        if (pNatInfoGetVar->iEndIndex > iNatMapNum) {
            pNatInfoGetVar->iEndIndex = iNatMapNum;
            iCount = iNatMapNum - pNatInfoGetVar->iStartIndex + 1;
        } else {
            iCount = pNatInfoGetVar->iEndIndex - pNatInfoGetVar->iStartIndex + 1;
        }
    } else {
        /*
         *  �������һ�㲻�����
         */
        goto  __back;
    }

    iBuffLen = iCount * sizeof(__NATMAP_INFO);
    pNatMapInfoNBuf   = (PCHAR)malloc(iBuffLen);
    if (LW_NULL == pNatMapInfoNBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return;
    }

    bzero(pNatMapInfoNBuf, iBuffLen);
    natMapInfoGet(pNatMapInfoNBuf, pNatInfoGetVar->iStartIndex, pNatInfoGetVar->iEndIndex);

__back:
    replyQueryMsg(OPENRT_NAT_MAP_GET_ACK, sizeof(__NATMAP_INFO), iCount, pNatMapInfoNBuf);
    free(pNatMapInfoNBuf);
}

/*********************************************************************************************************
** ��������: __natMapSet
** ��������: ����NAT �˿�ӳ�����
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natMapSet (PVOID  pArgBuf)
{
    __PNAT_MAP_HANDLE    pNtMapHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_MAP_SET_ACK, PX_ERROR);

        return;
    }

    pNtMapHandle = (__PNAT_MAP_HANDLE)pArgBuf;

    /*
     *  �������Ƿ�ɹ����ߺ�̨�ӿ�
     */
    replyConfigMsg(OPENRT_NAT_MAP_SET_ACK, natMapCfg(pNtMapHandle));
}

/*********************************************************************************************************
** ��������: __natAssNodeGet
** ��������: ��ȡ NAT �˿�ʹ��������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAssNodeGet (PVOID  pArgBuf)
{
    __NAT_ASSNODE_INFO    natAssNodeUse;

    natAssNodeUseGet(&natAssNodeUse);
    replyQueryMsg(OPENRT_NAT_ASSNODE_GET_ACK, sizeof(__NAT_ASSNODE_INFO), 1, &natAssNodeUse);
}

/*********************************************************************************************************
** ��������: __natIpFragGet
** ��������: ��ȡ IP ��Ƭ��ʹ�����
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natIpFragGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_NAT_IPFRAG_GET_ACK, natIpFragGet());
}

/*********************************************************************************************************
** ��������: __natIpFragSet
** ��������: ���� IP ��Ƭ��ʹ�����
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natIpFragSet (PVOID  pArgBuf)
{
    __PNAT_IPFRAG_HANDLE   pNatIpFragHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_IPFRAG_SET_ACK, PX_ERROR);

        return;
    }

    pNatIpFragHandle = (__PNAT_IPFRAG_HANDLE)pArgBuf;

    /*
     *  �������Ƿ�ɹ����ߺ�̨�ӿ�
     */
    replyConfigMsg(OPENRT_NAT_IPFRAG_SET_ACK, natIpFragCfg(pNatIpFragHandle));
}

/*********************************************************************************************************
** ��������: __natWanLanSet
** ��������: ���� WAN LAN �����
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natWanLanSet (PVOID  pArgBuf)
{
    INT                     iWanLanRet       = PX_ERROR;
    __PNAT_WANLAN_HANDLE    pNatWanLanHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);

        return;
    }

    pNatWanLanHandle = (__PNAT_WANLAN_HANDLE)pArgBuf;

    if (0 == strcmp("NULL", pNatWanLanHandle->cpNetIfName)) {           /*  �������Ʋ���ֱ�ӷ���        */
        replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);
    }

    if (NAT_WANLAN_SET == pNatWanLanHandle->iHandle) {                  /*  ��Ҫ��� WAN LAN ��         */
        if (NAT_FUNC_WAN == pNatWanLanHandle->iFunc) {                  /*  �ж��ǽ���Wan�����û���Lan��*/
            iWanLanRet = natWanCfg(pNatWanLanHandle->cpNetIfName);
        } else {
            iWanLanRet = natLanCfg(pNatWanLanHandle->cpNetIfName);
        }
    } else if (NAT_WANLAN_DEL == pNatWanLanHandle->iHandle) {           /*  ��Ҫɾ�� WAN LAN ��         */
        iWanLanRet = natWanLanDelet(pNatWanLanHandle->cpNetIfName);
    }

    replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);
}

/*********************************************************************************************************
** ��������: natHandleEntry
** ��������: NAT �������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID natHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_NAT_WANLAN_SET:
        __natWanLanSet(pArgBuf);
        break;

    case OPENRT_NAT_IPFRAG_SET:
        __natIpFragSet(pArgBuf);
        break;

    case OPENRT_NAT_IPFRAG_GET:
        __natIpFragGet(pArgBuf);
        break;

    case OPENRT_NAT_ASSNODE_GET:
        __natAssNodeGet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_SET:
        __natMapSet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_GET:
        __natMapGet(pArgBuf);
        break;

    case OPENRT_NAT_ALIAS_SET:
        __natAliasSet(pArgBuf);
        break;

    case OPENRT_NAT_ALIAS_GET:
        __natAliasGet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_NUM_GET:
        __natMapNumGet(pArgBuf);
        break;

    case OPENRT_NAT_ALI_NUM_GET:
        __natAliasNumGet(pArgBuf);
        break;

    default:
        break;
    }
}
