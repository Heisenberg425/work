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
** ��   ��   ��: kidvpnAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 27 ��
**
** ��        ��: OpenRT VPN ��̨����ӿ�
** 2018.08.16  �����°汾 KidVPN �޸Ĵ���.
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "kidvpn_info.h"
#include "vnd_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: kidvpn ����
*********************************************************************************************************/
#define WEB_VPN_VNDID         "vndid"                                   /*  Json�ֶΣ��������� id       */
#define WEB_VPN_IPADDR        "ipaddr"                                  /*  Json�ֶΣ�IP ��ַ           */
#define WEB_VPN_MTU           "mtu"                                     /*  Json�ֶΣ�mtu ֵ            */
#define WEB_VPN_PORT          "port"                                    /*  Json�ֶΣ��˿���Ϣ          */
#define WEB_VPN_HANDLE        "handle"                                  /*  Json�ֶΣ�����ʽ          */
#define WEB_VPN_STATUS        "status"                                  /*  Json�ֶΣ�vpn ����״̬      */
#define WEB_VPN_PASSWD        "passwd"                                  /*  Json�ֶΣ�vpn ������Կ      */
/*********************************************************************************************************
  Jason key: aes key ����
*********************************************************************************************************/
#define WEB_VPN_AES_BIT       "bit"                                     /*  Json�ֶΣ�bit               */
#define WEB_VPN_AES_KEY       "aeskey"                                  /*  Json�ֶΣ�key               */
/*********************************************************************************************************
  Jason key: ��ȡ��Ϣ
*********************************************************************************************************/
#define WEB_VPN_PAGE          "page"                                    /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_PAGECOUNT         "pagecount"                               /*  Json�ֶΣ�ÿҳ����          */
#define WEB_VPN_TOTAL         "total"                                   /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define WEB_VPN_SERIP         "serip"                                   /*  Json�ֶΣ������� IP ��ַ    */
#define WEB_VPN_POSITION      "position"                                /*  Json�ֶΣ�IP ��Ӧ�ĵ���λ�� */
#define WEB_VPN_LOCIP         "locip"                                   /*  Json�ֶΣ����� IP ��ַ      */
#define WEB_VPN_TYPE          "type"                                    /*  Json�ֶΣ�����              */
#define WEB_VPN_NETMASK       "netmask"                                 /*  Json�ֶΣ���������          */
#define WEB_VPN_GATEWAY       "gateway"                                 /*  Json�ֶΣ����ص�ַ          */
#define WEB_VPN_MAC           "mac"                                     /*  Json�ֶΣ�MAC ��ַ          */
#define WEB_VPN_LINK_IP       "cliip"                                   /*  Json�ֶΣ�IP ��ַ           */
#define WEB_VPN_LINK_MAC      "climac"                                  /*  Json�ֶΣ�MAC ��ַ          */
#define ARRAY_NAME            "data"                                    /*  Json�ֶΣ�������Ϣ          */
#define WEB_VPN_TYPE          "type"                                    /*  Json�ֶΣ�VPN ����          */
/*********************************************************************************************************
  Jason key: ����λ����Ϣ
*********************************************************************************************************/
#define WEB_VPN_SRC_IP         "cli_ip"                                 /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_VPN_SRC_POS        "cli_pos"                                /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define WEB_VPN_DST_IP         "ser_ip"                                 /*  Json�ֶΣ������� IP ��ַ    */
#define WEB_VPN_DST_POS        "ser_pos"                                /*  Json�ֶΣ����� IP ��ַ      */
#define KV_PORT_DEFAULT        (10088)                                  /*  kidvpn Ĭ�϶˿ں�           */
/*********************************************************************************************************
** ��������: __kidvpnAesKeyGenAction
** ��������: KidVPN ���� AES KEY ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeyGenAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcBitBuf         = LW_NULL;
    cJSON                *pjBitBuf         = LW_NULL;
    INT                   iRet             = PX_ERROR;
    CHAR                  cAesKey[70]      = {0};                       /*  ��� 64 �ֽڳ���            */

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjBitBuf = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_BIT);
    if (LW_NULL == pjBitBuf) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    pcBitBuf = pjBitBuf->valuestring;

    iRet = libKidVPNAesKeyGen(cAesKey, atoi(pcBitBuf), sizeof(cAesKey));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

    cJSON_AddStringToObject(pSendjRes, WEB_VPN_AES_KEY, cAesKey);       /*  ���� KEY                    */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnAesKeySaveAction
** ��������: KidVPN ���� AES KEY �����غ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeySaveAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;
    cJSON                *pjVndId          = LW_NULL;
    __AES_KEY_HANDLE      aesKeyHandle;
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjAesKey = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
    pjVndId  = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjAesKey || LW_NULL == pjVndId) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    bzero(&aesKeyHandle, sizeof(aesKeyHandle));
    pcAesKey             = pjAesKey->valuestring;
    aesKeyHandle.iVndID  = pjVndId->valueint;
    strncpy(aesKeyHandle.cAesKey, pcAesKey, sizeof(aesKeyHandle.cAesKey));

    iRet = libKidVPNAesKeySave(&aesKeyHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnAesKeyGetAction
** ��������: KidVPN ��ȡ AES KEY ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeyGetAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjVndId          = LW_NULL;
    INT                   iVndID           = 0;
    CHAR                  cAesKey[70]      = {0};
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);                               /*  Json �ṹ����ʧ��,ʹ���ַ���*/

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjVndId = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjVndId) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iVndID = pjVndId->valueint;

    iRet = libKidVPNAesKeyGet(cAesKey, iVndID, sizeof(cAesKey));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }
    cJSON_AddStringToObject(pSendjRes, WEB_VPN_AES_KEY, cAesKey);       /*  ���� KEY                    */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnInfoGetAction
** ��������: KidVPN ��ȡ��Ϣ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf     = LW_NULL;
    PCHAR                 pcKidVPNBufTmp  = LW_NULL;

    PCHAR                 pcVndBuf        = LW_NULL;
    PCHAR                 pcVndBufTmp     = LW_NULL;

    __PKIDVPN_DB_INFO     pcKidVPNInfo    = LW_NULL;
    __PVND_HANDLE         pcVndInfo       = LW_NULL;

    INT                   iVndNum          = 0;
    INT                   iKidVpnNum       = 0;

    CHAR                  cMacBuf[20]      = {0};
    PCHAR                 pcIpAddr         = LW_NULL;
    PCHAR                 pcHandleType     = LW_NULL;
    PCHAR                 pcStatus         = LW_NULL;

    INT                   iPage            = 1;                         /*  ����ʾ��ҳ��                */
    INT                   iTotal           = 0;                         /*  ����Ŀ                      */
    cJSON*                pJsonVpnInfoArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjPages          = LW_NULL;
    INT                   i;
    INT                   j;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend             = cJSON_CreateObject();                          /*  �������� Json ����          */
    pJsonVpnInfoArry   = cJSON_CreateArray();                           /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnInfoArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjPages = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PAGE);
    if (LW_NULL == pjPages) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     *  ��ȡ VPN ��VND ��Ϣ
     */
    pcVndBuf       = libVndGetInfo(&iVndNum);
    pcKidVPNBuf    = libKidVPNGetInfo(&iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    pcVndBufTmp    = pcVndBuf;

    pcKidVPNInfo = (__PKIDVPN_DB_INFO)pcKidVPNBuf;
    pcVndInfo    = (__PVND_HANDLE)pcVndBuf;

    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if (((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) ||
                 TYPE_ADD_VPN_S == pcKidVPNInfo->kidvpnHandle.cHandleType) {

                cJSON*  pjVpnInfo = cJSON_CreateObject();               /*  ����һ��ӳ�� Json ����      */
                if(LW_NULL == pjVpnInfo) {
                     break;
                }

                pcIpAddr     = inet_ntoa(*(struct in_addr *)&pcKidVPNInfo->kidvpnHandle.uiIpAddr);
                pcStatus     = pcKidVPNInfo->kidvpnHandle.cStatus ? "ON" : "OFF";
                pcHandleType = (TYPE_ADD_VPN_S == pcKidVPNInfo->kidvpnHandle.cHandleType) ?
                                                                        "servervpn" : "clientvpn";

                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SERIP,    pcIpAddr);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_POSITION, pcKidVPNInfo->kidvpnHandle.cPos);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_MTU,      pcKidVPNInfo->kidvpnHandle.iMtu);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_VNDID,    pcKidVPNInfo->kidvpnHandle.iVndID);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_PORT,     pcKidVPNInfo->kidvpnHandle.iPort);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_TYPE,     pcHandleType);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_STATUS,   pcStatus);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_PASSWD,   pcKidVPNInfo->kidvpnHandle.cPassWd);

                if (iVndNum && LW_NULL != pcVndBuf) {
                    for (j = 0; j < iVndNum; j++) {                     /*  Ѱ�Ҷ�Ӧ�� vnd ��Ϣ         */

                        /*
                         *  �ҵ���Ӧ�� VND ��Ϣ
                         */
                        if (pcVndInfo->iVndId == pcKidVPNInfo->kidvpnHandle.iVndID) {
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_LOCIP,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiIpAddr));
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_NETMASK,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiNetMask));
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_GATEWAY,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiGateWay));

                            sprintf(cMacBuf,
                                    "%02x:%02x:%02x:%02x:%02x:%02x",
                                    pcVndInfo->ucHwAddr[0],
                                    pcVndInfo->ucHwAddr[1],
                                    pcVndInfo->ucHwAddr[2],
                                    pcVndInfo->ucHwAddr[3],
                                    pcVndInfo->ucHwAddr[4],
                                    pcVndInfo->ucHwAddr[5]);

                            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_MAC, cMacBuf);
                        }

                        pcVndBuf += sizeof(__VND_HANDLE);
                        pcVndInfo = (__PVND_HANDLE)(pcVndBuf);
                    }
                }

                pcVndBuf  = pcVndBufTmp;
                pcVndInfo = (__PVND_HANDLE)pcVndBuf;

                cJSON_AddItemToArray(pJsonVpnInfoArry, pjVpnInfo);      /*  ������Ŀ��������            */
            }

            iTotal++;
            pcKidVPNBuf += sizeof(__KIDVPN_DB_INFO);
            pcKidVPNInfo = (__PKIDVPN_DB_INFO)(pcKidVPNBuf);
        }
    }

    if (NULL != pcKidVPNBufTmp) {
        free(pcKidVPNBufTmp);
    }

    if (NULL != pcVndBufTmp) {
        free(pcVndBufTmp);

    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnInfoArry);

    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_VPN_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVpnInfoArry);
}
/*********************************************************************************************************
** ��������: __kidvpnLinkInfoGetAction
** ��������: KidVPN ��ȡ������Ϣ��̨�ӿ�
** �䡡��  : wp     ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf      = LW_NULL;
    PCHAR                 pcKidVPNBufTmp   = LW_NULL;
    __PKIDVPN_LINK        pcKidVPNInfo     = LW_NULL;
    __VPN_INFO_HANDLE     vpnInfoHandle;
    INT                   iKidVpnNum       = 0;
    INT                   iPage            = 1;                         /*  ����ʾ��ҳ��                */
    INT                   iTotal           = -1;                        /*  ����Ŀ                      */
    INT                   iPageCount       = 0;

    cJSON*                pJsonVpnLinkArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjPages          = LW_NULL;
    cJSON                *pjPageCount      = LW_NULL;
    INT                   i;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend           = cJSON_CreateObject();                            /*  �������� Json ����          */
    pJsonVpnLinkArry = cJSON_CreateArray();                             /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnLinkArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjPages = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PAGE);
    if (LW_NULL == pjPages) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjPageCount) {
        iPageCount = 10;
    } else {
        iPageCount = pjPageCount->valueint;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    vpnInfoHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    vpnInfoHandle.llEndIndex   = iPage * iPageCount;

    /*
     *  ��ȡ VPN ������Ϣ��Ŀ
     */
    iTotal = libVpnLinkNumGet();
    if (-1 == iTotal) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ VPN ������Ϣ
     */
    pcKidVPNBuf    = libKidVPNLinkInfoGet(&vpnInfoHandle, &iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {
            cJSON*  pjVpnInfo = cJSON_CreateObject();                   /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjVpnInfo) {
                 break;
            }

            pcKidVPNInfo = (__PKIDVPN_LINK)(pcKidVPNBuf);

            cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_VNDID,    pcKidVPNInfo->iVndId);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_LINK_IP,  pcKidVPNInfo->cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_POSITION, pcKidVPNInfo->cPos);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_LINK_MAC, pcKidVPNInfo->cMac);

            cJSON_AddItemToArray(pJsonVpnLinkArry, pjVpnInfo);          /*  ������Ŀ��������            */

            pcKidVPNBuf += sizeof(__KIDVPN_LINK);
        }

        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnLinkArry);
    }

    free(pcKidVPNBufTmp);

    cJSON_AddNumberToObject(pjSend, WEB_VPN_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnPosInfoGetAction
** ��������: KidVPN ��ȡ���ӵ���λ����Ϣ��̨�ӿ�
** �䡡��  : wp     ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnPosInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf      = LW_NULL;
    PCHAR                 pcKidVPNBufTmp   = LW_NULL;
    __PKIDVPN_POS         pcKidVPNInfo     = LW_NULL;
    INT                   iKidVpnNum       = 0;
    INT                   iTotal           = 0;
    cJSON*                pJsonVpnLinkArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    INT                   i;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend             = cJSON_CreateObject();                          /*  �������� Json ����          */
    pJsonVpnLinkArry   = cJSON_CreateArray();                           /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnLinkArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  ��ȡ VPN ������Ϣ
     */
    pcKidVPNBuf    = libKidVPNPosInfoGet(&iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    pcKidVPNInfo   = (__PKIDVPN_POS)pcKidVPNBuf;
    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ
             */
            cJSON*  pjVpnInfo = cJSON_CreateObject();                   /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjVpnInfo) {
                break;
            }

            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SRC_IP,  pcKidVPNInfo->vpnSrc.cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SRC_POS, pcKidVPNInfo->vpnSrc.cPos);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_DST_IP,  pcKidVPNInfo->vpnDst.cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_DST_POS, pcKidVPNInfo->vpnDst.cPos);

            cJSON_AddItemToArray(pJsonVpnLinkArry, pjVpnInfo);          /*  ������Ŀ��������            */

            iTotal++;
            pcKidVPNBuf += sizeof(__KIDVPN_POS);
            pcKidVPNInfo = (__PKIDVPN_POS)(pcKidVPNBuf);
        }

        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnLinkArry);
    }

    if (NULL != pcKidVPNBufTmp) {
        free(pcKidVPNBufTmp);
    }

    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __kidvpnCreateAction
** ��������: ���� KidVPN ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnCreateAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;
    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcMtu            = LW_NULL;
    CHAR                 *pcHandle         = LW_NULL;
    CHAR                 *pcPassWd         = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjMtu            = LW_NULL;
    cJSON                *pjPort           = LW_NULL;
    cJSON                *pjHandle         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;
    cJSON                *pjVndID          = LW_NULL;
    cJSON                *pjPassWd         = LW_NULL;
    INT                   iVndID           = 0;
    INT                   iPort            = 0;

    __KIDVPN_HANDLE       kidvpnHandleInfo;
    __AES_KEY_HANDLE      aesKetInfo;
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }
    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjIpAddr = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_IPADDR);
    pjMtu    = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_MTU);
    pjPort   = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PORT);
    pjHandle = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_HANDLE);
    pjVndID  = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    pjPassWd = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PASSWD);
    pjAesKey = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
    if (LW_NULL == pjIpAddr ||
        LW_NULL == pjMtu    ||
        LW_NULL == pjHandle ||
        LW_NULL == pjVndID  ||
        LW_NULL == pjPassWd ||
        LW_NULL == pjAesKey) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    pcIpAddr = pjIpAddr->valuestring;
    pcMtu    = pjMtu->valuestring;
    pcHandle = pjHandle->valuestring;
    iVndID   = pjVndID->valueint;
    pcPassWd = pjPassWd->valuestring;
    if (LW_NULL == pjPort) {
        iPort = KV_PORT_DEFAULT;
    } else {
        iPort = pjPort->valueint;
    }

    pcAesKey = pjAesKey->valuestring;

    /*
     *  ������Ϣ
     */
    bzero(&kidvpnHandleInfo, sizeof(kidvpnHandleInfo));

    if (0 == strcmp(pcHandle, "addServer")) {
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_S;
    } else if (0 == strcmp(pcHandle, "addClient")) {
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_C;
    } else {
        libVndDel(iVndID);
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    kidvpnHandleInfo.iVndID = iVndID;
    kidvpnHandleInfo.iMtu   = atoi(pcMtu);
    kidvpnHandleInfo.iPort  = iPort;

    inet_aton(pcIpAddr, (struct in_addr *)&(kidvpnHandleInfo.uiIpAddr));
    strncpy(kidvpnHandleInfo.cPassWd, pcPassWd, sizeof(kidvpnHandleInfo.cPassWd));

    bzero(&aesKetInfo, sizeof(aesKetInfo));
    aesKetInfo.iVndID = iVndID;
    strncpy(aesKetInfo.cAesKey, pcAesKey, sizeof(aesKetInfo.cAesKey));

    libKidVPNAesKeySave(&aesKetInfo);
    iRet = libKidVPNCreate(&kidvpnHandleInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
    } else if (RET_HANDLE_OK == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnDelAction
** ��������: ɾ�� KidVPN ��̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnDelAction (Webs *wp)
{
    cJSON                *pRecvjRes = LW_NULL;
    cJSON                *pSendjRes = LW_NULL;
    cJSON                *pjVndID   = LW_NULL;
    INT                   iVndID    = 0;
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjVndID = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iVndID = pjVndID->valueint;

    iRet = libKidVPNDel(iVndID);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
    } else if (RET_HANDLE_OK == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnActionDefine
** ��������: VPN ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnActionDefine (VOID)
{
    websDefineAction("kidvpnCreateAction",    __kidvpnCreateAction);
    websDefineAction("kidvpnDelAction",       __kidvpnDelAction);
    websDefineAction("kidvpnAesKeyGenAction", __kidvpnAesKeyGenAction);
    websDefineAction("kidvpnAesKeyGetAction", __kidvpnAesKeyGetAction);
    websDefineAction("kidvpnInfoGetAction",   __kidvpnInfoGetAction);
    websDefineAction("kidvpnLinkInfoGetAction", __kidvpnLinkInfoGetAction);
    websDefineAction("kidvpnPosInfoGetAction",  __kidvpnPosInfoGetAction);
    websDefineAction("kidvpnAesKeySaveAction",  __kidvpnAesKeySaveAction);
}
/*********************************************************************************************************
** ��������: kidvpnActionInit
** ��������: VPN ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  kidvpnActionInit (VOID)
{
    __kidvpnActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
