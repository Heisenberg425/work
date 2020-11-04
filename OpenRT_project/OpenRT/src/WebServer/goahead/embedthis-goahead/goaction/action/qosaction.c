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
** ��   ��   ��: qosaction.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 16 ��
**
** ��        ��: QoS���ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "lwip/netif.h"
#include "intada_info.h"
#include "qos_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define JSON_ARGERR_CODE           (100)                                /*  �������󷵻�Json codeֵ     */
/*********************************************************************************************************
  Jason key: QoS������Ϣ��ʾ
*********************************************************************************************************/
#define WEB_QOS_NETIF              "netif"                              /*  Json�ֶΣ������ӿ�          */
#define WEB_QOS_RULE               "rule"                               /*  Json�ֶΣ�����              */
#define WEB_QOS_IPS                "ipStart"                            /*  Json�ֶΣ���ʼip            */
#define WEB_QOS_IPE                "ipEnd"                              /*  Json�ֶΣ���ֹip            */
#define WEB_QOS_PORTS              "portStart"                          /*  Json�ֶΣ���ʼport          */
#define WEB_QOS_PORTE              "portEnd"                            /*  Json�ֶΣ���ֹport          */
#define WEB_QOS_CMPMETHOD          "cmpMethod"                          /*  Json�ֶΣ��ȽϷ���          */
#define WEB_QOS_PRIORITY           "priority"                           /*  Json�ֶΣ����ȼ�            */
#define WEB_QOS_DONTDROP           "dontDrop"                           /*  Json�ֶΣ�������            */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                   "page"                               /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                  "total"                              /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                 "data"                               /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
** ��������: __qosShowAction
** ��������: QoS��ʾ��̨�ӿ�
** �䡡��  : wp     ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __qosShowAction (Webs  *wp)
{
    __PQOS_HANDLE         pQosHeader               = LW_NULL;
    __PQOS_HANDLE         pTmpNode                 = LW_NULL;
    __QOS_HANDLE          qosHandle                = {0};
    cJSON*                pJsonQosArry             = LW_NULL;
    cJSON*                pjSend                   = LW_NULL;
    cJSON*                pRecvjRes                = LW_NULL;
    cJSON*                pjPage                   = LW_NULL;
    CHAR                  cWebName[NETIF_NAMESIZE] = {0};
    INT                   iPage                    = 1;                 /*  ����ʾ��ҳ��                */
    INT                   iTotal                   = 0;                 /*  ����Ŀ                      */
    INT                   iRecordTotal             = 0;
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

    pjSend       = cJSON_CreateObject();                                /*  �������� Json ����          */
    pJsonQosArry = cJSON_CreateArray();                                 /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pjSend || LW_NULL == pJsonQosArry) {
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
    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (LW_NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);                 /*  ��������                    */

        goto __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);                 /*  ��������                    */

        goto __error;
    }

    /*
     * ���ò�ѯ����ӿ�
     */
    pQosHeader = qosShow(&qosHandle, &iRecordTotal);
    if (LW_NULL != pQosHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            CHAR    cIpStart[20] = {0};
            CHAR    cIpEnd[20]   = {0};

            pTmpNode = pQosHeader + i;
            strncpy(cIpStart, inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpStart), sizeof(cIpStart));
            strncpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpEnd),   sizeof(cIpEnd));

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjQos = cJSON_CreateObject();                   /*  ����һ��ӳ�� Json ����      */
                if (LW_NULL == pjQos) {
                     break;
                }

                bzero(cWebName, sizeof(cWebName));
                libIntAdaGetWebNetif(pTmpNode->cNetif, cWebName, sizeof(cWebName));

                cJSON_AddStringToObject(pjQos, WEB_QOS_NETIF, cWebName);
                cJSON_AddStringToObject(pjQos, WEB_QOS_RULE, pTmpNode->cRule);
                cJSON_AddStringToObject(pjQos, WEB_QOS_IPS, cIpStart);
                cJSON_AddStringToObject(pjQos, WEB_QOS_IPE, cIpEnd);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PORTS, pTmpNode->uiPortStart);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PORTE, pTmpNode->uiPortEnd);
                cJSON_AddStringToObject(pjQos, WEB_QOS_CMPMETHOD, pTmpNode->cCmpMethod);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PRIORITY, pTmpNode->uiPriority);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_DONTDROP, pTmpNode->uiDontDrop);

                cJSON_AddItemToArray(pJsonQosArry, pjQos);              /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    if (pQosHeader) {
        free(pQosHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonQosArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonQosArry);
}
/*********************************************************************************************************
** ��������: __qosConfigAction
** ��������: QoS���ú�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __qosConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes      = LW_NULL;
    cJSON                *pSendjRes      = LW_NULL;
    cJSON                *pjIpStart      = LW_NULL;
    cJSON                *pjIpEnd        = LW_NULL;
    cJSON                *pjPortStart    = LW_NULL;
    cJSON                *pjPortEnd      = LW_NULL;
    cJSON                *pjPriority     = LW_NULL;
    cJSON                *pjDontDrop     = LW_NULL;
    cJSON                *pjNetif        = LW_NULL;
    cJSON                *pjRule         = LW_NULL;
    cJSON                *pjCmpMethod    = LW_NULL;
    __QOS_HANDLE          qosHandle;
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
    pjIpStart    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPS);
    pjIpEnd      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPE);
    pjPortStart  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTS);
    pjPortEnd    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTE);
    pjPriority   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PRIORITY);
    pjDontDrop   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_DONTDROP);
    pjNetif      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_NETIF);
    pjRule       = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_RULE);
    pjCmpMethod  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_CMPMETHOD);
    if (pjIpStart    == LW_NULL  ||
        pjIpEnd      == LW_NULL  ||
        pjPortStart  == LW_NULL  ||
        pjPortEnd    == LW_NULL  ||
        pjPriority   == LW_NULL  ||
        pjDontDrop   == LW_NULL  ||
        pjNetif      == LW_NULL  ||
        pjRule       == LW_NULL  ||
        pjCmpMethod  == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto __error;
    }

    /*
     *  ������Ϣ
     */
    bzero(&qosHandle, sizeof(qosHandle));
    qosHandle.uiIpStart   = inet_addr(pjIpStart->valuestring);
    qosHandle.uiIpEnd     = inet_addr(pjIpEnd->valuestring);
    qosHandle.uiPortStart = (UINT16)pjPortStart->valueint;
    qosHandle.uiPortEnd   = (UINT16)pjPortEnd->valueint;
    qosHandle.uiPriority  = (UINT8)pjPriority->valueint;
    qosHandle.uiDontDrop  = (UINT8)pjDontDrop->valueint;

    /*
     *  ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pjNetif->valuestring, qosHandle.cNetif, sizeof(qosHandle.cNetif));
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __error;
    }

    strncpy(qosHandle.cRule,      pjRule->valuestring,      sizeof(qosHandle.cRule));
    strncpy(qosHandle.cCmpMethod, pjCmpMethod->valuestring, sizeof(qosHandle.cCmpMethod));

    /*
     *  ������������ӿ�
     */
    iRet = qosConfig (&qosHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __qosDeleteAction
** ��������: QoSɾ�����ú�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __qosDeleteAction (Webs  *wp)
{
    cJSON                *pRecvjRes      = LW_NULL;
    cJSON                *pSendjRes      = LW_NULL;
    cJSON                *pjIpStart      = LW_NULL;
    cJSON                *pjIpEnd        = LW_NULL;
    cJSON                *pjPortStart    = LW_NULL;
    cJSON                *pjPortEnd      = LW_NULL;
    cJSON                *pjPriority     = LW_NULL;
    cJSON                *pjDontDrop     = LW_NULL;
    cJSON                *pjNetif        = LW_NULL;
    cJSON                *pjRule         = LW_NULL;
    cJSON                *pjCmpMethod    = LW_NULL;
    __QOS_HANDLE          qosHandle;
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
    pjIpStart    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPS);
    pjIpEnd      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPE);
    pjPortStart  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTS);
    pjPortEnd    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTE);
    pjPriority   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PRIORITY);
    pjDontDrop   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_DONTDROP);
    pjNetif      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_NETIF);
    pjRule       = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_RULE);
    pjCmpMethod  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_CMPMETHOD);
    if (pjIpStart    == LW_NULL  ||
        pjIpEnd      == LW_NULL  ||
        pjPortStart  == LW_NULL  ||
        pjPortEnd    == LW_NULL  ||
        pjPriority   == LW_NULL  ||
        pjDontDrop   == LW_NULL  ||
        pjNetif      == LW_NULL  ||
        pjRule       == LW_NULL  ||
        pjCmpMethod  == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, JSON_ARGERR_CODE); /*  ��������                    */

        goto __error;
    }

    /*
     *  ������Ϣ
     */
    bzero(&qosHandle, sizeof(qosHandle));
    qosHandle.uiIpStart     = inet_addr(pjIpStart->valuestring);
    qosHandle.uiIpEnd       = inet_addr(pjIpEnd->valuestring);
    qosHandle.uiPortStart   = (UINT16)pjPortStart->valueint;
    qosHandle.uiPortEnd     = (UINT16)pjPortEnd->valueint;
    qosHandle.uiPriority    = (UINT8)pjPriority->valueint;
    qosHandle.uiDontDrop    = (UINT8)pjDontDrop->valueint;

    /*
     *  ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pjNetif->valuestring, qosHandle.cNetif, sizeof(qosHandle.cNetif));
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __error;
    }

    strncpy(qosHandle.cRule,      pjRule->valuestring,      sizeof(qosHandle.cRule));
    strncpy(qosHandle.cCmpMethod, pjCmpMethod->valuestring, sizeof(qosHandle.cCmpMethod));

    /*
     *  ������������ӿ�
     */
    iRet = qosDelete(&qosHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __qosActionDefine
** ��������: QoS��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __qosActionDefine (VOID)
{
    websDefineAction("qosShowAction", __qosShowAction);
    websDefineAction("qosConfigAction", __qosConfigAction);
    websDefineAction("qosDeleteAction", __qosDeleteAction);
}
/*********************************************************************************************************
** ��������: qosActionInit
** ��������: QoS��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  qosActionInit (VOID)
{
    __qosActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
