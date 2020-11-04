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
** ��   ��   ��: kidvpnEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: OpenRT KidVPN ���ô������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lwip/inet.h"
#include "common.h"
#include "msgcomm.h"
#include "libvpn_config.h"
#include "kidvpn_common.h"
#include "kidvpnDB.h"
#include "vnd/vnd_common.h"
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
            iItermCount  = iAllIterm - llStartIndex + 1;
        } else {
            iItermCount = llEndIndex - llStartIndex + 1;
        }
    }

    return  (iItermCount);
}
/*********************************************************************************************************
** ��������: __kidvpnAdd
** ��������: ��� VPN ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAdd (PVOID  pArgBuf)
{
    __PKIDVPN_HANDLE  pKidVpnHandle = LW_NULL;
    INT               iVpnRet       = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, RET_ARG_ERROR);

        return;
    }

    pKidVpnHandle = (__PKIDVPN_HANDLE)pArgBuf;

    /*
     *  ����Ǵ��������������жϷ����� ip ��ַ�� port �Ƿ��Ѵ���
     */
    if (TYPE_ADD_VPN_S == pKidVpnHandle->cHandleType) {
        iVpnRet = kidvpnServIpExistFlagGet(pKidVpnHandle);
        if (ERROR_NONE == iVpnRet) {                                    /*  ������ ip ��ַ�Ѵ���        */
            kidVpnAesKeyDel(pKidVpnHandle->iVndID);

            vndDel(pKidVpnHandle->iVndID);                              /*  ɾ�� vnd                    */

            replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, RET_ARG_ERROR);

            return;
        }
    }

    if (TYPE_ADD_VPN_C == pKidVpnHandle->cHandleType ||
        TYPE_ADD_VPN_S == pKidVpnHandle->cHandleType) {
        iVpnRet = kidVpnStart(pKidVpnHandle);
    }

    replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, iVpnRet);
}

/*********************************************************************************************************
** ��������: __kidvpnDel
** ��������: ɾ�� VPN ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnDel (PVOID  pArgBuf)
{
    INT               iVndID    = 0;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_DEL_VPN_ACK_CMD, PX_ERROR);
    }

    iVndID = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_DEL_VPN_ACK_CMD, kidVpnDel(iVndID));
}
/*********************************************************************************************************
** ��������: __kidvpnKeySave
** ��������: ���� KEY ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeySave (PVOID  pArgBuf)
{
    INT                iVpnRet       = PX_ERROR;
    __PAES_KEY_HANDLE  pKidVpnAesKey = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_KEY_SAVE_ACK_CMD, PX_ERROR);

        return;
    }

    pKidVpnAesKey = (__PAES_KEY_HANDLE)pArgBuf;
    iVpnRet = kidVpnAesKeySave(pKidVpnAesKey->cAesKey,
                               pKidVpnAesKey->iVndID,
                               strlen(pKidVpnAesKey->cAesKey));

    replyConfigMsg(OPENRT_KEY_SAVE_ACK_CMD, iVpnRet);
}

/*********************************************************************************************************
** ��������: __kidvpnKeyGen
** ��������: ���� KEY ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeyGen (PVOID  pArgBuf)
{
    INT      iKeyBit        = 0;
    CHAR     cKeyGenBuf[70] = {0};

    iKeyBit        = *(INT *)pArgBuf;

    kidVpnAesKeyGen(cKeyGenBuf, iKeyBit, sizeof(cKeyGenBuf));

    replyQueryMsg(OPENRT_KEY_GEN_ACK_CMD, sizeof(cKeyGenBuf), 1, cKeyGenBuf);
}

/*********************************************************************************************************
** ��������: __kidvpnKeyGet
** ��������: ��ȡ���� KEY ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeyGet (PVOID  pArgBuf)
{
    INT      iVndID         = 0;
    CHAR     cKeyGenBuf[70] = {0};

    iVndID  = *(INT *)pArgBuf;
    kidVpnAesKeyGet(cKeyGenBuf, iVndID, sizeof(cKeyGenBuf));
    replyQueryMsg(OPENRT_KEY_GET_ACK_CMD, sizeof(cKeyGenBuf), 1, cKeyGenBuf);
}
/*********************************************************************************************************
** ��������: __kidvpnInfoGet
** ��������: ��ȡ VPN ��Ϣ���
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnInfoGet (PVOID  pArgBuf)
{
    PCHAR     pcKidVPNAllInfo = LW_NULL;
    INT       iNum            = 0;

    iNum            = kidvpnGetDBRow();
    pcKidVPNAllInfo = kidvpnGetDBInfo();

    replyQueryMsg(OPENRT_INFO_GET_ACK_CMD, sizeof(__KIDVPN_DB_INFO), iNum, pcKidVPNAllInfo);

    free(pcKidVPNAllInfo);
}
/*********************************************************************************************************
** ��������: __kidvpnLinkInfoNumGet
** ��������: ��ȡ VPN ������Ϣ����Ŀ���
** �䡡��  : pArgBuf      ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_LINK_NUM_ACK_CMD, kidvpnLinkGetDBRow());
}
/*********************************************************************************************************
** ��������: __kidvpnLinkInfoGet
** ��������: ��ȡ VPN ������Ϣ���
** �䡡��  : pArgBuf      ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoGet (PVOID  pArgBuf)
{
    __PVPN_INFO_HANDLE   pVpnInfoHandle   = LW_NULL;
    INT32                iItermNum        = 0;
    INT32                iItermCount      = 0;
    INT32                iBufLen          = 0;
    INT                  iRet             = PX_ERROR;
    PCHAR                pcKidVPNLinkInfo = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    pVpnInfoHandle = (__PVPN_INFO_HANDLE)pArgBuf;

    iItermNum      =  kidvpnLinkGetDBRow();
    if (iItermNum <= 0) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    /*
     *  �����ڴ棬���ڱ������е���Ϣ
     */
    iItermCount = __indexChange(pVpnInfoHandle->llStartIndex,
                                pVpnInfoHandle->llEndIndex,
                                iItermNum,
                                &(pVpnInfoHandle->llEndIndex));
    if (!iItermCount) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    iBufLen    = iItermCount * sizeof(__KIDVPN_LINK);
    pcKidVPNLinkInfo = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcKidVPNLinkInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), 0, pcKidVPNLinkInfo);

        return;
    }

    bzero(pcKidVPNLinkInfo, iBufLen);

    iRet = kidvpnLinkGetSpecInfo(pVpnInfoHandle->llStartIndex,
                                 pVpnInfoHandle->llEndIndex,
                                 pcKidVPNLinkInfo,
                                 iBufLen);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get vpn link info error\n");

        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), 0, pcKidVPNLinkInfo);

        free(pcKidVPNLinkInfo);

        return;
    }

    replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

    free(pcKidVPNLinkInfo);
}
/*********************************************************************************************************
** ��������: __kidvpnLinkPositionGet
** ��������: ��ȡ VPN ���ӵ�����Ϣ���
** �䡡��  : pArgBuf      ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkPositionGet (PVOID  pArgBuf)
{
    INT                i;
    INT                iNum;
    INT                iRow;
    INT                iTotal;
    INT                iRet;
    static BOOL        bGetPubIp = FALSE;

    PCHAR              pcIp;
    static CHAR        cLocalIp[32]    = {0};
    CHAR               cLocalPos[128]  = {0};
    CHAR               cRemotePos[128] = {0};
    struct in_addr     inIpAddr;

    PCHAR              pcKidVPNLinkInfo = LW_NULL;
    __PKIDVPN_LINK     pcVpnLinkTmp     = LW_NULL;

    PCHAR              pKidVPNInfo      = LW_NULL;
    __PKIDVPN_DB_INFO  pKidVPNInfoTmp   = LW_NULL;

    PCHAR              pKidvpnPos       = LW_NULL;
    __PKIDVPN_POS      pKidvpnPosTmp    = LW_NULL;

    system("ifconfig dns 0 8.8.8.8");

    /*
     *  ��ȡ�������ڹ���IP
     */
    if (!bGetPubIp) {
        getPublicIp(cLocalIp);

        iRet = inet_aton(cLocalIp, &inIpAddr);
        if (1 == iRet) {                                                /*  �ɹ���ñ��ع���ip��ַ      */
            bGetPubIp = TRUE;
        } else {
            printf("Get Public ip failed.\n");
        }
    }

    /*
     *  ��ȡ�������ڵ���λ��
     */
    iRet = getPositionByIpAddr(cLocalIp, cLocalPos);
    if (ERROR_NONE != iRet) {
        strncpy(cLocalPos, "[\"invalid\", \"\", \"\", \"\", \"\"]", sizeof(cLocalPos));
    }

    /*
     *  ��ȡ�������ɹ����յ����ӵĿͻ��˸���
     */
    iNum = kidvpnLinkGetDBRow();

    /*
     *  ��ȡ���ؿͻ������ӵ�����������Ϣ
     */
    pKidVPNInfo = kidvpnGetLinkedCliInfoFromDB(&iRow);

    iTotal = iNum + iRow;                                               /*  �ܹ���������Ϣ              */

    /*
     *  Ϊ IP �� ����λ����Ϣ�����ڴ�
     */
    pKidvpnPos = (PCHAR)malloc(iTotal * sizeof(__KIDVPN_POS));
    if (NULL == pKidvpnPos) {
        free(pKidVPNInfo);

        return;
    }

    pKidVPNInfoTmp = (__PKIDVPN_DB_INFO)pKidVPNInfo;
    pKidvpnPosTmp  = (__PKIDVPN_POS)pKidvpnPos;

    bzero(pKidvpnPosTmp, iTotal * sizeof(__KIDVPN_POS));

    /*
     *  ��ȡ�ͻ���������Ϣ���� IP ת��Ϊ�����ַ
     */
    for (i = 0; i < iRow; i++) {

        /*
         *  �ͻ��˱�����Ϣ
         */
        strncpy(pKidvpnPosTmp->vpnSrc.cIp,  cLocalIp,  sizeof(pKidvpnPosTmp->vpnSrc.cIp));
        strncpy(pKidvpnPosTmp->vpnSrc.cPos, cLocalPos, sizeof(pKidvpnPosTmp->vpnSrc.cPos));

        /*
         *  �ͻ��������ӷ�������Ϣ
         */
        pcIp = inet_ntoa(*(struct in_addr *)&pKidVPNInfoTmp->kidvpnHandle.uiIpAddr);
        iRet = getPositionByIpAddr(pcIp, cRemotePos);
        if (iRet < 0) {
            strncpy(cRemotePos, "NONE", sizeof(cRemotePos));
        }

        strncpy(pKidvpnPosTmp->vpnDst.cIp,  pcIp,       sizeof(pKidvpnPosTmp->vpnDst.cIp));
        strncpy(pKidvpnPosTmp->vpnDst.cPos, cRemotePos, sizeof(pKidvpnPosTmp->vpnDst.cPos));

        pKidVPNInfoTmp += 1;
        pKidvpnPosTmp  += 1;
    }

    pcKidVPNLinkInfo = kidvpnLinkGetDBInfo();

    pcVpnLinkTmp = (__PKIDVPN_LINK)pcKidVPNLinkInfo;

    for (i = 0; i < iNum; i++) {

        /*
         *  �����������ӿͻ���ip������λ����Ϣ
         */
        iRet = getPositionByIpAddr(pcVpnLinkTmp->cIp, cRemotePos);
        if (iRet < 0) {
            strncpy(cRemotePos, "[\"invalid\", \"\", \"\", \"\", \"\"]", sizeof(cRemotePos));
        }

        strncpy(pKidvpnPosTmp->vpnSrc.cIp,  pcVpnLinkTmp->cIp, sizeof(pKidvpnPosTmp->vpnSrc.cIp));
        strncpy(pKidvpnPosTmp->vpnSrc.cPos, cRemotePos,        sizeof(pKidvpnPosTmp->vpnSrc.cPos));

        /*
         *  ���������� ip ������λ����Ϣ
         */
        strncpy(pKidvpnPosTmp->vpnDst.cIp,  cLocalIp,  sizeof(pKidvpnPosTmp->vpnDst.cIp));
        strncpy(pKidvpnPosTmp->vpnDst.cPos, cLocalPos, sizeof(pKidvpnPosTmp->vpnDst.cPos));

        pcVpnLinkTmp  += 1;
        pKidvpnPosTmp += 1;
    }

    replyQueryMsg(OPENRT_POSIT_GET_ACK_CMD, sizeof(__KIDVPN_POS), iTotal, pKidvpnPos);

    free(pKidVPNInfo);
    free(pcKidVPNLinkInfo);
    free(pKidvpnPos);
}
/*********************************************************************************************************
** ��������: kidvpnHandleEntry
** ��������: KidVPN ���ô������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID kidvpnHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_ADD_VPN_CMD:
        __kidvpnAdd(pArgBuf);
        break;

    case OPENRT_DEL_VPN_CMD:
        __kidvpnDel(pArgBuf);
        break;

    case OPENRT_KEY_SAVE_CMD:
        __kidvpnKeySave(pArgBuf);
        break;

    case OPENRT_KEY_GEN_CMD:
        __kidvpnKeyGen(pArgBuf);
        break;

    case OPENRT_KEY_GET_CMD:
        __kidvpnKeyGet(pArgBuf);
        break;

    case OPENRT_INFO_GET_CMD:
        __kidvpnInfoGet(pArgBuf);
        break;

    case OPENRT_LINK_NUM_CMD:
        __kidvpnLinkInfoNumGet(pArgBuf);
        break;

    case OPENRT_LINK_GET_CMD:
        __kidvpnLinkInfoGet(pArgBuf);
        break;

    case OPENRT_POSIT_GET_CMD:
        __kidvpnLinkPositionGet(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
