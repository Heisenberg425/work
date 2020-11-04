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
** ��   ��   ��: dnsAction.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 30 ��
**
** ��        ��: dns���ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "dns_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  ���ص�״̬��                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json �ṹ����ʧ��ʱʹ��     */
#define WEB_DNS_PRIMARY                 "primary"                       /*  ��ѡ DNS                    */
#define WEB_DNS_SECOND                  "second"                        /*  ���� DNS                    */
#define DNS_DEFAULT                     "8.8.8.8"                       /*  Ĭ�� DNS                    */
/*********************************************************************************************************
 �궨��
*********************************************************************************************************/
#define DNS_GETOBJSTRING(p, key, defVal) \
                (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                 defVal : cJSON_GetObjectItem(p, key)->valuestring      /*  ��ȡ Json ����������        */
#define DNS_GETOBJINTER(p, key, defVal) \
                (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                 defVal : cJSON_GetObjectItem(p, key)->valueint         /*  ��ȡ Json ����������        */

/*********************************************************************************************************
** ��������: __dnsShowAction
** ��������: ������ʾ DNS ����
** ��    ��: wp           ���������
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID  __dnsShowAction (Webs  *wp)
{
    __PDNS_HANDLE  pDnsHandle    = LW_NULL;
    cJSON         *pjSend        = LW_NULL;
    INT            iRecordTotal  = 0;

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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);

        return;
    }

    pDnsHandle = dnsShow(&iRecordTotal);
    if (LW_NULL == pDnsHandle) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto __error;
    }

    cJSON_AddStringToObject(pjSend, WEB_DNS_PRIMARY, pDnsHandle->primaryDNS);
    cJSON_AddStringToObject(pjSend, WEB_DNS_SECOND,  pDnsHandle->secondDNS);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    dnsInfoFree(pDnsHandle);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __dnsConfigAction
** ��������: DNS �����̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dnsConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;
    cJSON                *pjPrimDNS     = LW_NULL;
    cJSON                *pjSecDNS      = LW_NULL;
    __DNS_HANDLE          dnsHandle;

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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);
        operLogInsert(wp, OPEN_RT_DNS, "config dns init failed!\n", 0);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "config dns, parser request data failed!\n", 0);

        goto  __error;
    }

    bzero(&dnsHandle, sizeof(__DNS_HANDLE));

    pjPrimDNS = cJSON_GetObjectItem(pRecvjRes, WEB_DNS_PRIMARY);
    if (NULL == pjPrimDNS || '\0' == (pjPrimDNS->valuestring)[0]) {
        strncpy(dnsHandle.primaryDNS, DNS_DEFAULT, sizeof(dnsHandle.primaryDNS));
    } else {
        strncpy(dnsHandle.primaryDNS, pjPrimDNS->valuestring, sizeof(dnsHandle.primaryDNS));
    }

    pjSecDNS = cJSON_GetObjectItem(pRecvjRes, WEB_DNS_SECOND);
    if (NULL == pjSecDNS || '\0' == (pjSecDNS->valuestring)[0]) {
        strncpy(dnsHandle.secondDNS, DNS_DEFAULT, sizeof(dnsHandle.secondDNS));
    } else {
        strncpy(dnsHandle.secondDNS, pjSecDNS->valuestring, sizeof(dnsHandle.secondDNS));
    }

    if (ERROR_NONE != dnsConfig(&dnsHandle)) {                          /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "config dns failed!\n", 0);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
        operLogInsert(wp, OPEN_RT_DNS, "config dns success!\n", 0);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __dnsActionDefine
** ��������: DNS ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dnsActionDefine (VOID)
{
    websDefineAction("dnsShowAction",   __dnsShowAction);
    websDefineAction("dnsConfigAction", __dnsConfigAction);
}
/*********************************************************************************************************
** ��������: dnsActionInit
** ��������: DNS ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dnsActionInit (VOID)
{
    __dnsActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
