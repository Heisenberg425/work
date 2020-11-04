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
** ��   ��   ��: monitoraction.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 7 �� 16 ��
**
** ��        ��: monitor ���ú�̨�ӿ�
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
#include "monitor_info.h"
#include "../webHandle.h"
#include "intada_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: monitor ���ü�������Ϣ��ʾ
*********************************************************************************************************/
#define WEB_MONITOR_TYPE                "type"                          /*  ip (�ն�) ��APP(Ӧ��)��
                                                                            net interface(��·)��
                                                                            net interface(Ӧ��)         */
#define WEB_MONITOR_ENABLE              "enable"                        /*  1: ʹ�� 0: ȥʹ��           */
#define WEB_MONITOR_PROTO               "protocol"                      /*  Э������tcp/udp/all         */
#define WEB_MONITOR_PORT                "port"                          /*  app��֪���˿ں�             */
#define WEB_MONITOR_IPADDR              "ip"                            /*  ��ض���IP��ַ              */
#define WEB_MONITOR_MAC                 "mac"                           /*  ��ض���mac��ַ             */
#define WEB_MONITOR_ALIAS               "alias"                         /*  ��ض������                */
#define WEB_MONITOR_IFNAME              "ifname"                        /*  ��������                    */
#define WEB_MONITOR_WEBNAME             "webname"                       /*  ǰ����������                */
#define WEB_MONITOR_DURATION            "duration"                      /*  ����ʱ��                    */
#define WEB_MONITOR_SEND_RATE           "send_rate"                     /*  �������ʣ�KB/s��            */
#define WEB_MONITOR_RECV_RATE           "recv_rate"                     /*  �������ʣ�KB/s��            */
#define WEB_MONITOR_SEND_TOTAL_LEN      "send_total_length"             /*  ����������                  */
#define WEB_MONITOR_RECV_TOTAL_LEN      "recv_total_length"             /*  ����������                  */
#define WEB_MONITOR_SEND_PACKETS        "send_total_packets"            /*  �����ܱ�����                */
#define WEB_MONITOR_RECV_PACKETS        "recv_total_packets"            /*  �����ܱ�����                */
#define WEB_MONITOR_CPU_USAGE           "cpu_usage"                     /*  CPUռ����                   */
#define WEB_MONITOR_MEM_USAGE           "memory_usage"                  /*  �ڴ�ռ����                  */
#define WEB_MONITOR_ONLINE_USERS        "online_users"                  /*  �����û���                  */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                        "page"                          /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                       "total"                         /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                      "data"                          /*  Json�ֶΣ������˿�          */
#define WEB_PAGE_SIZE                   (10)                            /*  ��ҳÿҳ��ʾ����            */
/*********************************************************************************************************
  Jason key: ����������״̬��Ϣ
*********************************************************************************************************/
#define WEB_INTERNET_STATUS             "status"                        /*  Json�ֶΣ�����������״̬    */
#define WEB_INTERNET_TIMES              "time"                          /*  Json�ֶΣ�����������ʱ��    */
#define DAY_SEC                         (24 * 60 * 60)                  /*  һ��(����)                  */
#define HOUR_SEC                        (60 * 60)                       /*  һʱ(����)                  */
#define MIN_SEC                         (60)                            /*  һ��(����)                  */
#define SEC_SEC                         (1)                             /*  һ��(����)                  */
/*********************************************************************************************************
** ��������: __monitorShowAction
** ��������: ��ȡ�����Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __monitorShowAction (Webs *wp)
{
    PTM_STATS          pTmStatsHeader           = LW_NULL;
    PTM_STATS          pTmpNode                 = LW_NULL;
    TM_STATS           monitorHandle            = {0};
    INT                iPage                    = 1;                 /*  ����ʾ��ҳ��                   */
    INT                iTotal                   = 0;                 /*  ����Ŀ                         */
    cJSON*             pJsonMonitorArry         = LW_NULL;
    cJSON*             pjSend                   = LW_NULL;
    cJSON             *pRecvjRes                = LW_NULL;
    INT                iRecordTotal             = 0;
    INT                i;
    CHAR               cWebName[NETIF_NAMESIZE] = {0};

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
    pJsonMonitorArry = cJSON_CreateArray();                             /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pjSend || LW_NULL == pJsonMonitorArry) {
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
    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_PAGE)){
        iPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE)->valueint;
        if (iPage <= 0) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

            goto  __error;
        }
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE)) {
        monitorHandle.cType = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE)->valueint;
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR)) {
        strncpy(monitorHandle.pIpAddr,
                cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR)->valuestring,
                sizeof(monitorHandle.pIpAddr));
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_MAC) ) {
        strncpy(monitorHandle.pMac,
                cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_MAC)->valuestring,
                sizeof(monitorHandle.pMac));
    }

    /*
     *  �� server ���Ͳ�ѯ��������
     */
    if (TMT_CPU_MEM_USER == monitorHandle.cType) {
        PTM_CPU_MEM  ptmCpuMem = cpuMemShow(&monitorHandle, &iRecordTotal);
        if (NULL == ptmCpuMem) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto  __error;
        }

        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_CPU_USAGE,  ptmCpuMem->dCpuUsage);
        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_MEM_USAGE,  ptmCpuMem->dMemoryUsage);
        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_ONLINE_USERS,  ptmCpuMem->uiOnlineUsers);
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

        if (NULL != ptmCpuMem) {
            free(ptmCpuMem);
        }

        goto  __finish;
    }

    pTmStatsHeader = monitorShow(&monitorHandle, &iRecordTotal);
    if (LW_NULL != pTmStatsHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  ��������                    */
            pTmpNode = pTmStatsHeader + i;

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if ((iTotal >= (iPage - 1) * WEB_PAGE_SIZE) && (iTotal < iPage * WEB_PAGE_SIZE)) {
                cJSON*  pjMonitor = cJSON_CreateObject();               /*  ����һ��ӳ�� Json ����      */
                if(LW_NULL == pjMonitor) {
                    break;
                }

                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_TYPE,   pTmpNode->cType);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_ENABLE, pTmpNode->cEnable);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_PROTO,  pTmpNode->cProtocol);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_PORT,   pTmpNode->usPort);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_IPADDR, pTmpNode->pIpAddr);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_MAC,    pTmpNode->pMac);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_ALIAS,  pTmpNode->pAlias);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_IFNAME, pTmpNode->pIfname);

                if (strstr(pTmpNode->pIfname, "en")) {
                    libIntAdaGetWebNetif(pTmpNode->pIfname, cWebName, sizeof(cWebName));

                    cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_WEBNAME, cWebName);
                } else {
                    cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_WEBNAME, pTmpNode->pIfname);
                }

                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_DURATION,  pTmpNode->iDuration);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_RATE, pTmpNode->iSendRate);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_RATE, pTmpNode->iRecvRate);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_TOTAL_LEN, pTmpNode->iSendTotalLen);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_TOTAL_LEN, pTmpNode->iRecvTotalLen);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_PACKETS, pTmpNode->iSendTotalPackets);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_PACKETS, pTmpNode->iRecvTotalPackets);

                cJSON_AddItemToArray(pJsonMonitorArry, pjMonitor);      /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    monitorInfoFree(pTmStatsHeader);

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonMonitorArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__finish:
__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pJsonMonitorArry);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __monitorItemDisableAction
** ��������: �������ô����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __monitorItemDisableAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *pjType        = LW_NULL;
    cJSON                *pjEnable      = LW_NULL;
    cJSON                *pjIpAddr      = LW_NULL;
    cJSON                *pjProto       = LW_NULL;
    cJSON                *pjPort        = LW_NULL;

    TM_STATS              monitorHandle = {0};
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
    pjType   = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE);
    pjEnable = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_ENABLE);
    pjIpAddr = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR);
    if (NULL == pjType || NULL == pjEnable || NULL == pjIpAddr) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    monitorHandle.cType   = pjType->valueint;
    monitorHandle.cEnable = pjEnable->valueint;

    strncpy(monitorHandle.pIpAddr, pjIpAddr->valuestring, sizeof(monitorHandle.pIpAddr));

    if (monitorHandle.cType == TMT_APP) {
        pjProto = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_PROTO);
        pjPort  = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_PORT);
        if (NULL == pjProto || NULL == pjPort) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __error;
        }

        monitorHandle.cProtocol = pjProto->valueint;
        monitorHandle.usPort    = pjPort->valueint;
    }

    /*
     *  �� server ����������������
     */
    iRet = monitorConfig(&monitorHandle);
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
** ��������: __internetStatusAction
** ��������: ��ȡ����������״̬��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __internetStatusAction (Webs *wp)
{
    cJSON               *pSendjRes      = LW_NULL;
    INT                  iRet;
    __INTERNET_STATUS    internetStatus = {0};
    CHAR                 cTimeBuff[20]  = {0};
    INT32                iDay           = 0;
    INT32                iHour          = 0;
    INT32                iMin           = 0;
    INT32                iSec           = 0;

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

    iRet = libInternetStatusMonitor(&internetStatus);
    if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

        iDay  = internetStatus.llTime / DAY_SEC;
        iHour = internetStatus.llTime % DAY_SEC / HOUR_SEC;
        iMin  = internetStatus.llTime % HOUR_SEC / MIN_SEC;
        iSec  = internetStatus.llTime % MIN_SEC;

        snprintf(cTimeBuff, sizeof(cTimeBuff), "%d:%d:%d:%d", iDay, iHour, iMin, iSec);

        cJSON_AddStringToObject(pSendjRes, WEB_INTERNET_TIMES, cTimeBuff);
        cJSON_AddStringToObject(pSendjRes, WEB_INTERNET_STATUS,
                                (internetStatus.ucStatus == INTERNET_OK) ? "ok" : "failed");
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __monitorActionDefine
** ��������: ��غ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __monitorActionDefine (VOID)
{
    websDefineAction("monitorInfoGetAction",     __monitorShowAction);
    websDefineAction("monitorItemDisableAction", __monitorItemDisableAction);
    websDefineAction("internetStatusAction",     __internetStatusAction);
}

/*********************************************************************************************************
** ��������: monitorActionInit
** ��������: ��غ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  monitorActionInit (VOID)
{
    __monitorActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
