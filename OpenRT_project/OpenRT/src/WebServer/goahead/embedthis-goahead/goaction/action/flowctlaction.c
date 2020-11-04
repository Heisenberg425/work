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
** ��   ��   ��: flowctlaction.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: flowctl���ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "flowctl_info.h"
#include "../webHandle.h"
#include "interface_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                     "code"                             /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: flowctl���ü�������Ϣ��ʾ
*********************************************************************************************************/
#define WEB_FLOWCTL_TYPE             "fc_type"                          /* 1: ��Э���������
                                                                           2: �����ڽ�������            */
#define WEB_FLOWCTL_ENABLE           "fc_enable"                        /* 1: ʹ�� 0: ȥʹ��            */
#define WEB_FLOWCTL_PROTO            "fc_proto"                         /* ��ЧЭ������ tcp/udp/all     */
#define WEB_FLOWCTL_START            "fc_start"                         /* ������ʼ IP ��ַ             */
#define WEB_FLOWCTL_END              "fc_end"                           /* ���ؽ��� IP ��ַ             */
#define WEB_FLOWCTL_SPORT            "fc_sport"                         /* ������ʼ�˿�                 */
#define WEB_FLOWCTL_EPORT            "fc_eport"                         /* ���ؽ����˿�                 */
#define WEB_FLOWCTL_IFNAME           "fc_ifname"                        /* ����                         */
#define WEB_FLOWCTL_UPRATE           "fc_uprate"                        /* ����������KB/s��             */
#define WEB_FLOWCTL_DOWNRATE         "fc_downrate"                      /* ����������KB/s��             */
#define WEB_FLOWCTL_BUFSIZE          "fc_bufsize"                       /* ��������С(16K ~ 128M Bytes) */
#define WEB_FLOWCTL_IP_FAMILY        "ip_family"                        /* ip ��ַ�壬IPv4 �� IPv6      */
#define WEB_FLOWCTL_OPER_TYPE        "oper_type"                        /* �������ͣ���ɾ��             */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                     "page"                             /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                    "total"                            /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                   "data"                             /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
** ��������: __flowctlInfoGetAction
** ��������: ��ȡ������Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __flowctlShowAction (Webs *wp)
{
    __PFLOWCTL_HANDLE     pFlowctlHeader   = LW_NULL;
    __PFLOWCTL_HANDLE     pTmpNode         = LW_NULL;
    __FLOWCTL_HANDLE      flowctlHandle    = {0};
    INT                   iPage            = 1;                         /*  ����ʾ��ҳ��                */
    INT                   iTotal           = 0;                         /*  ����Ŀ                      */
    cJSON*                pJsonflowctlArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;

    cJSON                *pjPage           = LW_NULL;
    cJSON                *pjFlowCtlType    = LW_NULL;
    cJSON                *pjOperType       = LW_NULL;

    INT                   iRecordTotal     = 0;
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
    pJsonflowctlArry = cJSON_CreateArray();                             /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pjSend || LW_NULL == pJsonflowctlArry) {
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
    pjPage        = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    pjFlowCtlType = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_TYPE);
    pjOperType    = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_OPER_TYPE);
    if (NULL == pjPage        ||
        NULL == pjFlowCtlType ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    flowctlHandle.iType = pjFlowCtlType->valueint,
    strncpy(flowctlHandle.cOperType, pjOperType->valuestring, sizeof(flowctlHandle.cOperType));

    pFlowctlHeader = flowctlShow(&flowctlHandle, &iRecordTotal);        /*  ���Ͳ�ѯ��������            */
    if (LW_NULL != pFlowctlHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  ��������                    */
            pTmpNode = pFlowctlHeader + i;

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {

                cJSON*  pjFlowCtl = cJSON_CreateObject();               /*  ����һ��ӳ�� Json ����      */
                if(LW_NULL == pjFlowCtl) {
                     break;
                }

                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_TYPE,     pTmpNode->iType);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_ENABLE,   pTmpNode->cEnable);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_PROTO,    pTmpNode->cProtocol);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_START,    pTmpNode->cStartIp);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_END,      pTmpNode->cEndIp);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_SPORT,    pTmpNode->iStartPort);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_EPORT,    pTmpNode->iEndPort);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_IFNAME,   pTmpNode->cIfname);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_UPRATE,   pTmpNode->ulUpRate);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_DOWNRATE, pTmpNode->ulDownRate);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_BUFSIZE,  pTmpNode->ulBufSize);

                cJSON_AddItemToArray(pJsonflowctlArry, pjFlowCtl);      /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    flowctlInfoFree(pFlowctlHeader);

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonflowctlArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);
    cJSON_Delete(pJsonflowctlArry);
}
/*********************************************************************************************************
** ��������: __flowctlConfigAction
** ��������: �������ô����̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __flowctlConfigAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *pjType        = LW_NULL;
    cJSON                *pjUpRate      = LW_NULL;
    cJSON                *pjDownRate    = LW_NULL;
    cJSON                *pjBufSize     = LW_NULL;
    cJSON                *pjIfname      = LW_NULL;
    cJSON                *pjOperType    = LW_NULL;

    cJSON                *pjProtocol    = LW_NULL;
    cJSON                *pjStartPort   = LW_NULL;
    cJSON                *pjEndPort     = LW_NULL;
    cJSON                *pjStartIp     = LW_NULL;
    cJSON                *pjEndIp       = LW_NULL;

    __FLOWCTL_HANDLE      flowctlHandle = {0};
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
    pjType     = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_TYPE);
    pjUpRate   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_UPRATE);
    pjDownRate = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_DOWNRATE);
    pjBufSize  = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_BUFSIZE);
    pjIfname   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_IFNAME);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_OPER_TYPE);
    if (NULL == pjType     ||
        NULL == pjUpRate   ||
        NULL == pjDownRate ||
        NULL == pjBufSize  ||
        NULL == pjIfname   ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    flowctlHandle.iType      = pjType->valueint;
    flowctlHandle.ulUpRate   = (ULONG)pjUpRate->valueint;
    flowctlHandle.ulDownRate = (ULONG)pjDownRate->valueint;
    flowctlHandle.ulBufSize  = (ULONG)pjBufSize->valueint;

    strncpy(flowctlHandle.cIfname,   pjIfname->valuestring,   sizeof(flowctlHandle.cIfname));
    strncpy(flowctlHandle.cOperType, pjOperType->valuestring, sizeof(flowctlHandle.cOperType));

    if (FCT_IP == flowctlHandle.iType) {
        pjProtocol  = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_PROTO);
        pjStartIp   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_START);
        pjEndIp     = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_END);
        pjStartPort = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_SPORT);
        pjEndPort   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_EPORT);
        if (NULL == pjProtocol  ||
            NULL == pjStartIp   ||
            NULL == pjEndIp     ||
            NULL == pjStartPort ||
            NULL == pjEndPort) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __error;
        }

        strncpy(flowctlHandle.cProtocol, pjProtocol->valuestring, sizeof(flowctlHandle.cProtocol));
        strncpy(flowctlHandle.cStartIp,  pjStartIp->valuestring,  sizeof(flowctlHandle.cStartIp));
        strncpy(flowctlHandle.cEndIp,    pjEndIp->valuestring,    sizeof(flowctlHandle.cEndIp));

        flowctlHandle.iStartPort = pjStartPort->valueint;
        flowctlHandle.iEndPort   = pjEndPort->valueint;
    }

    iRet = flowctlConfig(&flowctlHandle);                               /*  ����������������            */
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __flowctlActionDefine
** ��������: ���غ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __flowctlActionDefine (VOID)
{
    websDefineAction("flowctlShowAction",   __flowctlShowAction);
    websDefineAction("flowctlConfigAction", __flowctlConfigAction);
}
/*********************************************************************************************************
** ��������: flowctlActionInit
** ��������: ���غ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  flowctlActionInit (VOID)
{
    __flowctlActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
