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
** ��   ��   ��: natAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��: OpenRT NAT ���ú�̨�ӿ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "goahead.h"
#include "nat_info.h"
#include "intada_info.h"
#include "lwip/netif.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: ��ȡ�˿�ʹ����Ϣ
*********************************************************************************************************/
#define WEB_NAT_ASSNODETOTAL "totalnode"                                /*  Json�ֶΣ��˿��ܸ���        */
#define WEB_NAT_ASSNODEUSE   "usenode"                                  /*  Json�ֶΣ��˿�ʹ�ø���      */
#define WEB_NAT_ASSNODEPRE   "percent"                                  /*  Json�ֶΣ��˿�ʹ�ðٷֱ�    */
/*********************************************************************************************************
  Jason key: ��Ƭ������
*********************************************************************************************************/
#define WEB_NAT_IPFRAG      "fragtype"                                    /*  Json�ֶΣ���Ƭ������      */
#define WEB_NAT_IPFRAG_TCP  "tcp"                                         /*  Json�ֶΣ�TCP ��Ƭ��      */
#define WEB_NAT_IPFRAG_UDP  "udp"                                         /*  Json�ֶΣ�UCP ��Ƭ��      */
#define WEB_NAT_IPFRAG_ICMP "icmp"                                        /*  Json�ֶΣ�ICMP ��Ƭ��     */
/*********************************************************************************************************
  Jason key: �˿�ӳ������
*********************************************************************************************************/
#define WEB_NAT_ASSPORT     "assport"                                    /*  Json�ֶΣ������˿�         */
#define WEB_NAT_LOCALPORT   "localport"                                  /*  Json�ֶΣ������˿�         */
#define WEB_NAT_IP          "ip"                                         /*  Json�ֶΣ�IP               */
#define WEB_NAT_IPCNT       "ipcnt"                                      /*  Json�ֶΣ���������         */
#define WEB_NAT_PROTO       "proto"                                      /*  Json�ֶΣ�Э��             */
#define WEB_NAT_HANDLE      "handle"                                     /*  Json�ֶΣ�����ʽ         */
/*********************************************************************************************************
  Jason key: �����������
*********************************************************************************************************/
#define WEB_NAT_ALIASIP     "aliasip"                                   /*  Json�ֶΣ��������          */
#define WEB_NAT_LANSTART    "lanstart"                                  /*  Json�ֶΣ�������ʼ IP       */
#define WEB_NAT_LANEND      "lanend"                                    /*  Json�ֶΣ��������� IP       */
/*********************************************************************************************************
  Jason key: �˿�ӳ���ȡ��Ϣ
*********************************************************************************************************/
#define WEB_NAT_PAGE        "page"                                       /*  Json�ֶΣ���ǰ����ʾ��ҳ�� */
#define WEB_NAT_PAGE_COUNT  "count"                                      /*  Json�ֶΣ�ÿҳ��ʾ�ĸ���   */
#define WEB_NAT_TOTAL       "total"                                      /*  Json�ֶΣ��ܹ�����Ŀ��     */
#define ARRAY_NAME          "data"                                       /*  Json�ֶΣ������˿�         */
/*********************************************************************************************************
  Jason key: ����WAN LAN ��
*********************************************************************************************************/
#define WEB_NAT_INDEX       "index"                                     /*  Json�ֶΣ����ں�            */
#define WEB_NAT_NETIF       "webnetif"                                  /*  Json�ֶΣ�ǰ��������        */
#define WEB_NAT_USE         "use"                                       /*  Json�ֶΣ�����              */
#define WEB_NAT_HANDLE      "handle"                                    /*  Json�ֶΣ�����ʽ          */
/*********************************************************************************************************
** ��������: __natAssNodeUseAction
** ��������: ��ȡ����˿�ʹ�������̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAssNodeUseAction (Webs *wp)
{
    cJSON*  pjSend           = LW_NULL;
    INT     iRet             = 0;
    float   fNatAssNodePre   = 0.0f;

    INT     iNatAssNodeTotal;
    INT     iNatAssNodeUse;

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

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iRet = libNatAssNodeGet(&iNatAssNodeTotal, &iNatAssNodeUse);
    if (PX_ERROR == iRet) {                                             /*  ��ȡʧ��                    */
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ���÷��ص� Json ����
     */
    fNatAssNodePre = (iNatAssNodeUse * 1.0) / iNatAssNodeTotal;

    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODETOTAL, iNatAssNodeTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODEUSE,   iNatAssNodeUse);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODEPRE,   fNatAssNodePre);

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}

/*********************************************************************************************************
** ��������: __natMapGetAction
** ��������: ��ȡ�˿�ӳ����Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natMapGetAction (Webs *wp)
{
    PCHAR                 pcNatMapBuf     = LW_NULL;
    PCHAR                 pcNatMapBufTmp  = LW_NULL;
    __PNATMAP_INFO        pNatMapInfo     = LW_NULL;

    INT                   iPage;                                        /*  ����ʾ��ҳ��                */
    INT                   iPageCount      = 0;
    INT                   iTotal          = 0;                          /*  ����Ŀ                      */
    cJSON*                pJsonNatMapArry = LW_NULL;
    cJSON*                pjSend          = LW_NULL;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pjiPages        = LW_NULL;
    cJSON                *pjPageCount     = LW_NULL;

    __NAT_INFO_GET        natInfoGet;

    INT                   iNum            = 0;
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

    pjSend          = cJSON_CreateObject();                             /*  �������� Json ����          */
    pJsonNatMapArry = cJSON_CreateArray();                              /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonNatMapArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE_COUNT);
    pjiPages    = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE);
    if (LW_NULL == pjiPages || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjiPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount = pjPageCount->valueint;

    natInfoGet.iStartIndex = (iPage - 1) * iPageCount + 1;
    natInfoGet.iEndIndex   = iPage * iPageCount;

    iTotal         = libNatMapNumGet();
    pcNatMapBuf    = libNatMapGet(&natInfoGet, &iNum);
    pcNatMapBufTmp = pcNatMapBuf;
    pNatMapInfo    = (__PNATMAP_INFO)pcNatMapBuf;

    if (0 != iNum && LW_NULL != pcNatMapBuf) {
        for(i = 0; i < iNum; i++) {

            cJSON  *pjNatMap = cJSON_CreateObject();                    /*  ����һ��ӳ�� Json ����      */
            if (LW_NULL == pjNatMap) {
                 break;
            }

            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_ASSPORT,   pNatMapInfo->iWanPort);
            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_LOCALPORT, pNatMapInfo->iLanPort);
            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_IPCNT,     pNatMapInfo->iIpCnt);
            cJSON_AddStringToObject(pjNatMap, WEB_NAT_IP,        pNatMapInfo->cIpString);
            cJSON_AddStringToObject(pjNatMap, WEB_NAT_PROTO,     pNatMapInfo->cProtoString);

            cJSON_AddItemToArray(pJsonNatMapArry, pjNatMap);        /*  ������Ŀ��������            */

            pcNatMapBuf += sizeof(__NATMAP_INFO);
            pNatMapInfo  = (__PNATMAP_INFO)(pcNatMapBuf);
        }
    }


    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonNatMapArry);

    cJSON_AddNumberToObject(pjSend, WEB_NAT_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,      200);

    if (NULL != pcNatMapBufTmp) {
        free(pcNatMapBufTmp);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pJsonNatMapArry);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __natMapHandleAction
** ��������: �˿�ӳ�䴦���̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natMapHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    PCHAR                 pcIpString    = LW_NULL;                      /*  Ip �ַ���                   */
    PCHAR                 pcProtoString = LW_NULL;                      /*  Э�� �ַ���                 */

    INT                   iRet;
    CHAR                  cLogBuff[128] = {0};

    INT                   iIpCnt;
    INT                   iWanPort;
    INT                   iLanPort;
    INT                   cProto;
    CHAR                  cNatMapHandle;

    cJSON                *pjIpCnt        = LW_NULL;;
    cJSON                *pjWanPort      = LW_NULL;
    cJSON                *pjLanPort      = LW_NULL;
    cJSON                *pjNatMapHandle = LW_NULL;
    cJSON                *pjIpString     = LW_NULL;
    cJSON                *pjProtoString  = LW_NULL;

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
    pjIpCnt        = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IPCNT);
    pjNatMapHandle = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    pjWanPort      = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_ASSPORT);
    pjLanPort      = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LOCALPORT);
    pjIpString     = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IP);
    pjProtoString  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PROTO);
    if (LW_NULL == pjIpCnt        ||
        LW_NULL == pjNatMapHandle ||
        LW_NULL == pjWanPort      ||
        LW_NULL == pjLanPort      ||
        LW_NULL == pjIpString     ||
        LW_NULL == pjProtoString) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    cNatMapHandle  = pjNatMapHandle->valueint;
    iWanPort       = atoi(pjWanPort->valuestring);
    iLanPort       = atoi(pjLanPort->valuestring);
    iIpCnt         = atoi(pjIpCnt->valuestring);
    pcIpString     = pjIpString->valuestring;
    pcProtoString  = pjProtoString->valuestring;

    if (!strcmp(pcProtoString, "tcp") || !strcmp(pcProtoString, "TCP")) {
        cProto = IPPROTO_TCP;
    } else if (!strcmp(pcProtoString, "udp") || !strcmp(pcProtoString, "UDP")) {
        cProto = IPPROTO_UDP;
    } else {
        cProto = -1;
    }

    iRet = libNatMapSet(pcIpString, iIpCnt, iWanPort, iLanPort, cProto, cNatMapHandle);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  ��¼������־
     */
    snprintf(cLogBuff, sizeof(cLogBuff),
             "%s nat map, wan port:%s lan port:%s ip:%s ipcnt:%d prto:%s\n",
             0 == cNatMapHandle ? "set" : "delete",
             pjWanPort->valuestring,
             pjLanPort->valuestring,
             pcIpString,
             iIpCnt,
             pcProtoString);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __natAliasGetAction
** ��������: ��ȡ���������Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAliasGetAction (Webs *wp)
{
    __PNATALIAS_INFO      pNatAliasInfo     = LW_NULL;
    PCHAR                 pcNatAliasBuf     = LW_NULL;
    PCHAR                 pcNatAliasBufTmp  = LW_NULL;
    INT                   iPage             = 0;                        /*  ����ʾ��ҳ��                */
    INT                   iPageCount        = 0;
    INT                   iTotal            = 0;                        /*  ����Ŀ                      */
    cJSON*                pJsonNatAliasArry = LW_NULL;
    cJSON*                pjSend            = LW_NULL;
    cJSON                *pRecvjRes         = LW_NULL;
    cJSON                *pjPages           = LW_NULL;
    cJSON                *pjPageCount       = LW_NULL;

    __NAT_INFO_GET        natInfoGet;

    INT                   iNum              = 0;
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

    pjSend            = cJSON_CreateObject();                           /*  �������� Json ����          */
    pJsonNatAliasArry = cJSON_CreateArray();                            /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonNatAliasArry) {
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
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE_COUNT);
    pjPages     = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE);
    if (LW_NULL == pjPages || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount = pjPageCount->valueint;

    natInfoGet.iStartIndex = (iPage - 1) * iPageCount + 1;
    natInfoGet.iEndIndex   = iPage * iPageCount;

    iTotal           = libNatAliasNumGet();
    pcNatAliasBuf    = libNatAliasGet(&natInfoGet, &iNum);              /*  ��ȡ��ǰ������Ϣ            */
    pcNatAliasBufTmp = pcNatAliasBuf;
    pNatAliasInfo    = (__PNATALIAS_INFO)pcNatAliasBuf;
    if (0 != iNum && LW_NULL != pcNatAliasBuf) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjNatAlias = cJSON_CreateObject();                  /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjNatAlias) {
                break;
            }

            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_ALIASIP,  pNatAliasInfo->cAliasString);
            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_LANSTART, pNatAliasInfo->cLoaclSString);
            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_LANEND,   pNatAliasInfo->cLoaclEString);

            cJSON_AddItemToArray(pJsonNatAliasArry, pjNatAlias);        /*  ������Ŀ��������            */

            pcNatAliasBuf += sizeof(__NATALIAS_INFO);
            pNatAliasInfo  = (__PNATALIAS_INFO)(pcNatAliasBuf);
        }
    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonNatAliasArry);

    cJSON_AddNumberToObject(pjSend, WEB_NAT_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,      200);

    if (NULL != pcNatAliasBufTmp) {
        free(pcNatAliasBufTmp);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonNatAliasArry);
}
/*********************************************************************************************************
** ��������: __natAliasHandleAction
** ��������: ������������̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natAliasHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    PCHAR                 pcAliasIP     = LW_NULL;                      /*  �������                    */
    PCHAR                 pcLanStart    = LW_NULL;                      /*  ������ʼ IP                 */
    PCHAR                 pcLanEnd      = LW_NULL;                      /*  ������� IP                 */
    cJSON                *pjAliasIP     = LW_NULL;
    cJSON                *pjLanStart    = LW_NULL;
    cJSON                *pjLanEnd      = LW_NULL;
    cJSON                *pjHandle      = LW_NULL;

    INT                   iLanStart;
    INT                   iLanEnd;
    CHAR                  cHandle;
    INT                   iRet;
    CHAR                  cLogBuff[128] = {0};

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
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjHandle   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    pjAliasIP  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_ALIASIP);
    pjLanStart = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LANSTART);
    pjLanEnd   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LANEND);
    if (LW_NULL == pjHandle   ||
        LW_NULL == pjAliasIP  ||
        LW_NULL == pjLanStart ||
        LW_NULL == pjLanEnd) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    cHandle    = pjHandle->valueint;
    pcAliasIP  = pjAliasIP->valuestring;
    pcLanStart = pjLanStart->valuestring;
    pcLanEnd   = pjLanEnd->valuestring;
    iLanStart  = ntohl(inet_addr(pcLanStart));
    iLanEnd    = ntohl(inet_addr(pcLanEnd));
    if (iLanEnd < iLanStart) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iRet = libNatAliasSet(pcAliasIP, pcLanStart, pcLanEnd, cHandle);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  ��¼������־
     */
    snprintf(cLogBuff, sizeof(cLogBuff),
             "%s nat alias, wan ip:%s lan start ip:%s lan end ip:%s\n",
             0 == cHandle ? "set" : "delete",
             pcAliasIP,
             pcLanStart,
             pcLanEnd);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

 __error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __natIpFragHandleAction
** ��������: ��Ƭ�����ú�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natIpFragHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes       = LW_NULL;
    PCHAR                 cIpFragType     = LW_NULL;                    /*  ��������                    */
    cJSON                *pSendjRes       = LW_NULL;

    INT                   iRet;

    INT                   iIpFragType;
    INT                   iIpFragHandle;
    cJSON                *pjIpFragType   = LW_NULL;
    cJSON                *pjIpFragHandle = LW_NULL;

    CHAR                  cLogBuff[50]   = {0};

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
    pjIpFragType   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IPFRAG);
    pjIpFragHandle = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    if (LW_NULL == pjIpFragType || LW_NULL == pjIpFragHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iIpFragHandle = pjIpFragHandle->valueint;
    cIpFragType   = pjIpFragType->valuestring;
    iIpFragType   = (0 == strcmp(cIpFragType, "tcp")) ? IPPROTO_TCP  :
                    (0 == strcmp(cIpFragType, "udp")  ? IPPROTO_UDP  :
                    (0 == strcmp(cIpFragType, "icmp") ? IPPROTO_ICMP : -1));

    iRet = libNatIpFragSet(iIpFragType, iIpFragHandle);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  ��¼������־
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s nat ipfragment, the prto is %s\n",
            iIpFragHandle == NAT_IPFRAG_ENABLE ? "enable" : "disable", cIpFragType);

    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __natIpFragGetAction
** ��������: ��ȡ��Ƭ����Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natIpFragGetAction (Webs *wp)
{
    cJSON*                pjSend          = LW_NULL;
    INT                   iNatIpFragInfo  = 0;
    INT                   iValue          = 0;

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

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iNatIpFragInfo = libNatIpFragGet();
    if (PX_ERROR == iNatIpFragInfo) {                                   /*  ��ȡʧ��                    */
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    } else {
        iValue = (iNatIpFragInfo & 0x01) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_TCP, iValue);

        iValue = (iNatIpFragInfo & 0x02) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_UDP, iValue);

        iValue = (iNatIpFragInfo & 0x04) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_ICMP, iValue);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __natWanLanAction
** ��������: WAN LAN������
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natWanLanAction (Webs *wp)
{
    cJSON                *pRecvjRes                  = LW_NULL;
    cJSON                *pSendjRes                  = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE] = { 0 };

    PCHAR                 pcUse                      = LW_NULL;
    PCHAR                 pcWebNetif                 = LW_NULL;
    INT                   iHandle                    = -1;
    INT                   iFunc                      = 0;

    cJSON                *pjUse                      = LW_NULL;
    cJSON                *pjWebNetif                 = LW_NULL;
    cJSON                *pjHandle                   = LW_NULL;
    CHAR                  cLogBuff[20]               = {0};
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
    pjUse       = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_USE);
    pjWebNetif  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_NETIF);
    pjHandle    = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    if (LW_NULL == pjUse      ||
        LW_NULL == pjWebNetif ||
        LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcUse      = pjUse->valuestring;
    pcWebNetif = pjWebNetif->valuestring;
    iHandle    = pjHandle->valueint;

    /*
     *  ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }


    /*
     *  ��¼������־
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s %s\n", NAT_WANLAN_SET == iHandle ? "set" : "delete", pcUse);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

    iFunc = strcmp(pcUse, "wan") ? (strcmp(pcUse, "lan") ? -1 : 1): 0;

    /*
     *  ���� WAN/LAN �ڲ���
     */
    iRet = libNatWanLanConfig(cNetIfName, iHandle, iFunc);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __natActionDefine
** ��������: NAT ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __natActionDefine (VOID)
{
    websDefineAction("natWanLanAction",       __natWanLanAction);
    websDefineAction("natIpFragHandleAction", __natIpFragHandleAction);
    websDefineAction("natIpFragGetAction",    __natIpFragGetAction);
    websDefineAction("natAssNodeUseAction",   __natAssNodeUseAction);
    websDefineAction("natMapHandleAction",    __natMapHandleAction);
    websDefineAction("natMapGetAction",       __natMapGetAction);
    websDefineAction("natAliasGetAction",     __natAliasGetAction);
    websDefineAction("natAliasHandleAction",  __natAliasHandleAction);
}

/*********************************************************************************************************
** ��������: natActionInit
** ��������: NAT ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  natActionInit (VOID)
{
    __natActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
