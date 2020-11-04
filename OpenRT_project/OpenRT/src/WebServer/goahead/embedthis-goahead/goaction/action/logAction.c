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
** ��   ��   ��: logAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 15 ��
**
** ��        ��: OpenRT��־��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "log_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE         (200)                                 /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE          (500)                                 /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: ��־ͨ��
*********************************************************************************************************/
#define WEB_LOG_STARTT            "starttime"                           /*  Json�ֶΣ���ʼʱ��          */
#define WEB_LOG_ENDT              "endtime"                             /*  Json�ֶΣ�����ʱ��          */
#define WEB_LOG_PAGE              "page"                                /*  Json�ֶΣ�ҳ��              */
#define WEB_PAGECOUNT             "pagecount"                           /*  Json�ֶΣ�ÿҳ����          */
#define WEB_LOG_TOTAL             "total"                               /*  Json�ֶΣ��ܸ���            */
#define WEB_LOG_TIME              "time"                                /*  Json�ֶΣ�ʱ��              */
#define ARRAY_NAME                "data"                                /*  Json�ֶΣ���������          */
/*********************************************************************************************************
  Jason key: ������־
*********************************************************************************************************/
#define WEB_UDIAL_LOG_ENNAME      "enname"                              /*  Json�ֶΣ���������          */
#define WEB_UDIAL_LOG_EVENT       "event"                               /*  Json�ֶΣ��¼�              */
/*********************************************************************************************************
  Jason key: ������־
*********************************************************************************************************/
#define WEB_VPN_LOG_VNDID         "vndid"                               /*  Json�ֶΣ���������id        */
#define WEB_VPN_LOG_LOGINFI       "loginfo"                             /*  Json�ֶΣ�log ��Ϣ          */

#define WEB_OPER_LOG_TIME         "oper_time"                           /*  Json�ֶΣ�����ʱ��          */
#define WEB_OPER_LOG_CLIENT_IP    "client_ip"                           /*  Json�ֶΣ��ͻ���IP          */
#define WEB_OPER_LOG_OPERATOR     "operator"                            /*  Json�ֶΣ�������            */
#define WEB_OPER_LOG_MODULE_NAME  "module_name"                         /*  Json�ֶΣ�ģ����            */
#define WEB_OPER_LOG_RESULT       "result"                              /*  Json�ֶΣ��������          */
#define WEB_OPER_LOG_BUF          "oper_log"                            /*  Json�ֶΣ�������־����      */
/*********************************************************************************************************
** ��������: __udialLogGetAction
** ��������: ������־��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;

    CHAR                  pcTimeBuf[30]               = {0};            /*  ������� 26 �ֽ�            */
    CHAR                 *pcStartTime                 = LW_NULL;
    CHAR                 *pcEndTime                   = LW_NULL;
    INT                   iPage                       = 0;
    INT                   iPageCount                  = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;

    cJSON                *pjStartTime                 = LW_NULL;
    cJSON                *pjEndTime                   = LW_NULL;
    cJSON                *pjPage                      = LW_NULL;
    cJSON                *pjPageCount                 = LW_NULL;

    INT32                iNum                         = 0;
    INT32                iTotal                       = 0;

    __UDIAL_LOG_HANDLE   udialLogHandle;
    __PUDIAL_LOG_INFO    pUdialInfoNode               = LW_NULL;
    PCHAR                pcUdialInfoBuff              = LW_NULL;
    PCHAR                pcBuffTmp                    = LW_NULL;

    cJSON*               pJsonUdialArry               = LW_NULL;

    INT32                i;

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

    pSendjRes      = cJSON_CreateObject();                              /*  �������� Json ����          */
    pJsonUdialArry = cJSON_CreateArray();                               /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonUdialArry) {
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
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjStartTime ||
        LW_NULL == pjEndTime   ||
        LW_NULL == pjPage      ||
        LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcStartTime = pjStartTime->valuestring;
    pcEndTime   = pjEndTime->valuestring;
    iPageCount  = pjPageCount->valueint;
    iPage       = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    bzero(&tmEndTime,   sizeof(tmEndTime));
    bzero(&tmStartTime, sizeof(tmStartTime));
    strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
    strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

    udialLogHandle.logStartTm   = mktime(&tmStartTime);
    udialLogHandle.logEndTm     = mktime(&tmEndTime);
    udialLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    udialLogHandle.llEndIndex   = iPage * iPageCount;

    iTotal = libUdialLogNumGet(&udialLogHandle);

    pcUdialInfoBuff = libUdialLogInfoGet(&udialLogHandle, &iNum);
    pcBuffTmp       = pcUdialInfoBuff;
    if (0 != iNum && LW_NULL != pcUdialInfoBuff) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjUdialNode = cJSON_CreateObject();                 /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjUdialNode) {
                 break;
            }

            pUdialInfoNode = (__PUDIAL_LOG_INFO)pcUdialInfoBuff;

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pUdialInfoNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjUdialNode, WEB_LOG_TIME,         pcTimeBuf);
            cJSON_AddStringToObject(pjUdialNode, WEB_UDIAL_LOG_ENNAME, pUdialInfoNode->cNetifName);
            cJSON_AddStringToObject(pjUdialNode, WEB_UDIAL_LOG_EVENT,  pUdialInfoNode->cEvent);

            cJSON_AddItemToArray(pJsonUdialArry, pjUdialNode);          /*  ������Ŀ��������            */

            pcUdialInfoBuff += sizeof(__UDIAL_LOG_INFO);
        }
    }

    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonUdialArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    if (NULL != pcBuffTmp) {
        free(pcBuffTmp);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonUdialArry);
}
/*********************************************************************************************************
** ��������: __vpnLogGetAction
** ��������: VPN ��־��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vpnLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;

    CHAR                  pcTimeBuf[30]               = {0};            /*  ������� 26 �ֽ�            */
    CHAR                 *pcStartTime                 = LW_NULL;
    CHAR                 *pcEndTime                   = LW_NULL;
    INT                   iPage                       = 0;
    INT                   iPageCount                  = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;
    INT32                 iVndID;

    cJSON                *pjStartTime                 = LW_NULL;
    cJSON                *pjEndTime                   = LW_NULL;
    cJSON                *pjVndID                     = LW_NULL;
    cJSON                *pjPage                      = LW_NULL;
    cJSON                *pjPageCount                 = LW_NULL;

    INT32                iNum                         = 0;
    INT32                iTotal                       = 0;

    __VPN_LOG_HANDLE     vpnLogHandle;
    __PKIDVPN_LOG_INFO   pVpnInfoNode                 = LW_NULL;
    PCHAR                pcVpnInfoBuff                = LW_NULL;
    PCHAR                pcBuffTmp                    = LW_NULL;

    cJSON*               pJsonVpnArry                 = LW_NULL;

    INT32                i;

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

    pSendjRes       = cJSON_CreateObject();                             /*  �������� Json ����          */
    pJsonVpnArry    = cJSON_CreateArray();                              /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonVpnArry) {
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
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjVndID     = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_LOG_VNDID);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjStartTime  ||
        LW_NULL == pjEndTime    ||
        LW_NULL == pjVndID      ||
        LW_NULL == pjPage       ||
        LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcStartTime = pjStartTime->valuestring;
    pcEndTime   = pjEndTime->valuestring;
    iVndID      = pjVndID->valueint;
    iPageCount  = pjPageCount->valueint;
    iPage       = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    bzero(&tmEndTime,   sizeof(tmEndTime));
    bzero(&tmStartTime, sizeof(tmStartTime));
    strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
    strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

    vpnLogHandle.logStartTm   = mktime(&tmStartTime);
    vpnLogHandle.logEndTm     = mktime(&tmEndTime);
    vpnLogHandle.iVndID       = iVndID;
    vpnLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    vpnLogHandle.llEndIndex   = iPage * iPageCount;

    iTotal = libVpnLogNumGet(&vpnLogHandle);
    if (!iTotal) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    pcVpnInfoBuff = libVpnLogInfoGet(&vpnLogHandle, &iNum);
    pcBuffTmp     = pcVpnInfoBuff;
    if (0 != iNum && LW_NULL != pcVpnInfoBuff) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjVpnNode = cJSON_CreateObject();                   /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjVpnNode) {
                 break;
            }

            pVpnInfoNode = (__PKIDVPN_LOG_INFO)pcVpnInfoBuff;

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pVpnInfoNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjVpnNode, WEB_LOG_TIME,        pcTimeBuf);
            cJSON_AddNumberToObject(pjVpnNode, WEB_VPN_LOG_VNDID,   pVpnInfoNode->iVndID);
            cJSON_AddStringToObject(pjVpnNode, WEB_UDIAL_LOG_EVENT, pVpnInfoNode->cLogInfo);

            cJSON_AddItemToArray(pJsonVpnArry, pjVpnNode);              /*  ������Ŀ��������            */

            pcVpnInfoBuff += sizeof(__KIDVPN_LOG_INFO);
        }
    }


    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonVpnArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    if (NULL != pcBuffTmp) {
        free(pcBuffTmp);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVpnArry);
}
/*********************************************************************************************************
** ��������: __operLogGetAction
** ��������: ������־��ȡ��̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __operLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    CHAR                  pcTimeBuf[30] = {0};                          /*  ������� 26 �ֽ�            */
    INT32                iNum           = 0;

    __OPER_LOG           operLogHandle  = {0};
    __POPER_LOG          pOperLogNode   = LW_NULL;
    __POPER_LOG          pOperLogHead   = LW_NULL;
    cJSON*               pJsonLogArry   = LW_NULL;

    INT32                i;
    CHAR                 *pcStartTime   = LW_NULL;
    CHAR                 *pcEndTime     = LW_NULL;
    INT                   iPage         = 0;
    INT                   iPageCount    = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;

    cJSON                *pjStartTime   = LW_NULL;
    cJSON                *pjEndTime     = LW_NULL;
    cJSON                *pjPage        = LW_NULL;
    cJSON                *pjPageCount   = LW_NULL;
    INT                   iTotalCount   = 0;

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

    pSendjRes    = cJSON_CreateObject();                                /*  �������� Json ����          */
    pJsonLogArry = cJSON_CreateArray();                                 /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonLogArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjPage || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount                 = pjPageCount->valueint;
    operLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    operLogHandle.llEndIndex   = iPage * iPageCount;

    if ((LW_NULL != pjStartTime) && (LW_NULL != pjEndTime)) {
        pcStartTime = pjStartTime->valuestring;
        pcEndTime   = pjEndTime->valuestring;

        bzero(&tmEndTime,   sizeof(tmEndTime));
        bzero(&tmStartTime, sizeof(tmStartTime));
        strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
        strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

        operLogHandle.logStartTm = mktime(&tmStartTime);
        operLogHandle.logEndTm   = mktime(&tmEndTime);
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_MODULE_NAME)) {
        strncpy(operLogHandle.cModuleName,
                cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_MODULE_NAME)->valuestring,
                sizeof(operLogHandle.cModuleName));
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_BUF)) {
        strncpy(operLogHandle.cLog,
                cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_BUF)->valuestring,
                sizeof(operLogHandle.cLog));
    }

    pOperLogHead = readOperLog(&operLogHandle, &iNum);
    if (0 != iNum && LW_NULL != pOperLogHead) {
    	iTotalCount = pOperLogHead->iTotal;
        for (i = 0; i < iNum; i++) {
            pOperLogNode = pOperLogHead + i;

            cJSON  *pjOperLogNode = cJSON_CreateObject();              /*  ����һ��ӳ�� Json ����      */
            if(LW_NULL == pjOperLogNode) {
                 break;
            }

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pOperLogNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_TIME,        pcTimeBuf);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_CLIENT_IP,   pOperLogNode->cClientIP);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_OPERATOR,    pOperLogNode->cOperator);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_MODULE_NAME, pOperLogNode->cModuleName);
            cJSON_AddNumberToObject(pjOperLogNode, WEB_OPER_LOG_RESULT,      pOperLogNode->iResult);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_BUF,         pOperLogNode->cLog);

            cJSON_AddItemToArray(pJsonLogArry, pjOperLogNode);          /*  ������Ŀ��������            */
        }
    }

    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonLogArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotalCount);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotalCount);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonLogArry);
}
/*********************************************************************************************************
** ��������: __udialLogDelAction
** ��������: ������־�����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialLogDelAction (Webs *wp)
{
    cJSON               *pSendjRes      = LW_NULL;
    __UDIAL_LOG_HANDLE   udialLogHandle = {0};

    INT                  iRet;

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

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    iRet = libUdialLogInfoDel(&udialLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __vpnLogDelAction
** ��������: VPN ��־�����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vpnLogDelAction (Webs *wp)
{
    cJSON               *pSendjRes    = LW_NULL;
    __VPN_LOG_HANDLE     vpnLogHandle = {0};

    INT                  iRet;

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

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    iRet = libVpnLogInfoDel(&vpnLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __operLogDelAction
** ��������: ������־�����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __operLogDelAction (Webs *wp)
{
    cJSON       *pSendjRes      = LW_NULL;
    __OPER_LOG   operLogHandle  = {0};

    INT          iRet;

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

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    iRet = libOperLogInfoDel(&operLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/***************************************************s******************************************************
** ��������: __logActionDefine
** ��������: LOG ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __logActionDefine (VOID)
{
    websDefineAction("udialLogGetAction", __udialLogGetAction);
    websDefineAction("vpnLogGetAction",   __vpnLogGetAction);
    websDefineAction("operLogGetAction",  __operLogGetAction);

    websDefineAction("udialLogDelAction", __udialLogDelAction);
    websDefineAction("vpnLogDelAction",   __vpnLogDelAction);
    websDefineAction("operLogDelAction",  __operLogDelAction);
}

/*********************************************************************************************************
** ��������: logActionInit
** ��������: LOG ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID logActionInit (VOID)
{
    __logActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
