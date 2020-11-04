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
** ��   ��   ��: powerAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ��Դ���ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "power_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define  CUR_MAX_ID(a, b)          (((a) > (b)) ? (a):(b))              /*  ��ȡ���ֵ                  */
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: power ���ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_POWER_NUMBER           "number"                             /*  Json�ֶΣ����              */
#define WEB_POWER_EVENT_ID         "event_id"                           /*  Json�ֶΣ�event id          */
#define WEB_POWER_MAX_EVENT_ID     "max_event_id"                       /*  Json�ֶΣ�max event id      */
#define WEB_POWER_EVENT            "event"                              /*  Json�ֶΣ�event             */
#define WEB_POWER_DATE             "date"                               /*  Json�ֶΣ�date              */
#define WEB_POWER_TIME             "time"                               /*  Json�ֶΣ�time              */
#define WEB_POWER_STATUS           "status"                             /*  Json�ֶΣ�״̬              */
#define WEB_POWER_OPER             "oper_type"                          /*  Json�ֶΣ���������          */
#define WEB_POWER_DESC             "desc"                               /*  Json�ֶΣ�˵����Ϣ          */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                   "page"                               /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                  "total"                              /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                 "data"                               /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
** ��������: __powerOffAction
** ��������: ��Դ�رմ����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __powerOffAction (Webs *wp)
{
    cJSON           *pSendjRes   = NULL;
    __POWER_HANDLE   powerHandle = {0};;

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
    if (NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);

    /*
     * ���õ�Դ�ر�����ӿ�
     */
    powerOff(&powerHandle);
}
/*********************************************************************************************************
** ��������: __powerRestartAction
** ��������: ��Դ���������̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __powerRestartAction (Webs *wp)
{
    cJSON           *pSendjRes   = NULL;
    __POWER_HANDLE   powerHandle = {0};

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
    if (NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);

    /*
     * ���õ�Դ��������ӿ�
     */
    powerRestart(&powerHandle);
}
/*********************************************************************************************************
** ��������: __powerShowAction
** ��������: power �¼���ʾ�����̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __powerEventShowAction (Webs *wp)
{
    __PPOWER_HANDLE   pPowerHeader   = NULL;
    __POWER_HANDLE    powerHandle    = {0};
    __PPOWER_HANDLE   pTmpNode       = NULL;
    cJSON            *pJsonPowerArry = NULL;
    cJSON            *pjSend         = NULL;
    cJSON            *pRecvjRes      = NULL;
    cJSON            *pjPage         = NULL;
    UINT              uiMaxEventId   = 0;
    INT               iPage          = 1;                               /*  ����ʾ��ҳ��                */
    INT               iTotal         = 0;                               /*  ����Ŀ                      */
    INT               iRecordTotal   = 0;
    INT               i;

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

    pjSend         = cJSON_CreateObject();                              /*  �������� Json ����          */
    pJsonPowerArry = cJSON_CreateArray();                               /*  ���� Json ӳ����Ϣ����      */
    if(NULL == pjSend || NULL == pJsonPowerArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     * ���ò�ѯ����ӿ�
     */
    pPowerHeader = powerEventShow(&powerHandle, &iRecordTotal);
    if (NULL != pPowerHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  ѭ�����÷��ص� Json ����    */
            pTmpNode = pPowerHeader + i;
            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {

                cJSON  *pjPower = cJSON_CreateObject();                 /*  ����һ��ӳ�� Json ����      */
                if(NULL == pjPower) {
                     break;
                }

                cJSON_AddNumberToObject(pjPower, WEB_POWER_NUMBER, i + 1);
                cJSON_AddNumberToObject(pjPower, WEB_POWER_EVENT_ID, pTmpNode->uiEventId);
                cJSON_AddStringToObject(pjPower, WEB_POWER_EVENT,    pTmpNode->cEvent);
                cJSON_AddStringToObject(pjPower, WEB_POWER_DATE,     pTmpNode->cDate);
                cJSON_AddStringToObject(pjPower, WEB_POWER_TIME,     pTmpNode->cTime);
                cJSON_AddNumberToObject(pjPower, WEB_POWER_STATUS,   pTmpNode->iStatus);
                cJSON_AddStringToObject(pjPower, WEB_POWER_DESC,     pTmpNode->cDesc);

                cJSON_AddItemToArray(pJsonPowerArry, pjPower);          /*  ������Ŀ��������            */
            }

            iTotal++;
            uiMaxEventId = CUR_MAX_ID(uiMaxEventId, pTmpNode->uiEventId);
        }
    }

    if (pPowerHeader) {
        free(pPowerHeader);
    }

    cJSON_AddItemToObject(pjSend,   ARRAY_NAME, pJsonPowerArry);
    cJSON_AddNumberToObject(pjSend, WEB_POWER_MAX_EVENT_ID, uiMaxEventId);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonPowerArry);
}
/*********************************************************************************************************
** ��������: __powerConfigAction
** ��������: power �¼����ô����̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __powerEventConfigAction (Webs *wp)
{
    INT              iRet;
    cJSON           *pRecvjRes   = NULL;
    cJSON           *pSendjRes   = NULL;

    cJSON           *pjStatus    = NULL;
    cJSON           *pjEventId   = NULL;
    cJSON           *pjEvent     = NULL;
    cJSON           *pjDate      = NULL;
    cJSON           *pjTime      = NULL;
    cJSON           *pjOper      = NULL;
    cJSON           *pjDesc      = NULL;

    __POWER_HANDLE   powerHandle = {0};

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
    if (NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjStatus  = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_STATUS);
    pjEventId = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_EVENT_ID);
    pjEvent   = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_EVENT);
    pjDate    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_DATE);
    pjTime    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_TIME);
    pjOper    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_OPER);
    pjDesc    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_DESC);
    if (NULL == pjStatus || NULL == pjEventId ||
        NULL == pjEvent  || NULL == pjDate ||
        NULL == pjTime   || NULL == pjOper || NULL == pjDesc) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    powerHandle.iStatus   = pjStatus->valueint;
    powerHandle.uiEventId = pjEventId->valueint;
    strncpy(powerHandle.cEvent,    pjEvent->valuestring, sizeof(powerHandle.cEvent));
    strncpy(powerHandle.cDate,     pjDate->valuestring,  sizeof(powerHandle.cDate));
    strncpy(powerHandle.cTime,     pjTime->valuestring,  sizeof(powerHandle.cTime));
    strncpy(powerHandle.cOperType, pjOper->valuestring,  sizeof(powerHandle.cOperType));
    strncpy(powerHandle.cDesc,     pjDesc->valuestring,  sizeof(powerHandle.cDesc));

    /*
     * ������������ӿ�
     */
    iRet = powerEventConfig(&powerHandle);
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
** ��������: __powerActionDefine
** ��������: power ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __powerActionDefine (VOID)
{
    websDefineAction("powerOffAction",         __powerOffAction);
    websDefineAction("powerRestartAction",     __powerRestartAction);
    websDefineAction("powerEventShowAction",   __powerEventShowAction);
    websDefineAction("powerEventConfigAction", __powerEventConfigAction);
}
/*********************************************************************************************************
** ��������: powerActionInit
** ��������: power action ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerActionInit (VOID)
{
    __powerActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
