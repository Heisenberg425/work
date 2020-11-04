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
** ��   ��   ��: timeaction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳʱ�����ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "time_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: time ���ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_TIME_DATE             "date"                                /*  Json�ֶΣ�date              */
#define WEB_TIME_TIME             "time"                                /*  Json�ֶΣ��ӿ�����          */
/*********************************************************************************************************
** ��������: __timeShowAction
** ��������: ʱ����ʾ��̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __timeShowAction (Webs *wp)
{
    __PTIME_HANDLE  pTimeHeader  = NULL;
    __TIME_HANDLE   timeHandle   = {{0}};
    cJSON          *pjSend       = NULL;
    INT             iRecordTotal = 0;

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
    if(NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     * ���ò�ѯ����ӿڲ����÷��ص� Json ����
     */
    pTimeHeader = timeShow(&timeHandle, &iRecordTotal);
    if (NULL != pTimeHeader) {
        cJSON_AddStringToObject(pjSend, WEB_TIME_DATE, pTimeHeader->cDate);
        cJSON_AddStringToObject(pjSend, WEB_TIME_TIME, pTimeHeader->cTime);
    }

    if (pTimeHeader) {
        free(pTimeHeader);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __timeConfigAction
** ��������: time ���ô����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __timeConfigAction (Webs *wp)
{
    cJSON          *pRecvjRes = NULL;
    cJSON          *pSendjRes = NULL;
    cJSON          *pjDate    = NULL;
    cJSON          *pjTime    = NULL;
    __TIME_HANDLE   timeHandle;
    INT             iRet;

    lib_bzero(&timeHandle, sizeof(__TIME_HANDLE));

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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡJson��������
     */
    pjDate = cJSON_GetObjectItem(pRecvjRes, WEB_TIME_DATE);
    pjTime = cJSON_GetObjectItem(pRecvjRes, WEB_TIME_TIME);
    if (NULL == pjDate || NULL == pjTime) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(timeHandle.cDate, pjDate->valuestring, sizeof(timeHandle.cDate));
    strncpy(timeHandle.cTime, pjTime->valuestring, sizeof(timeHandle.cTime));

    /*
     * ������������ӿ�
     */
    iRet = tmConfig(&timeHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
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
** ��������: __timeActionDefine
** ��������: time��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __timeActionDefine (VOID)
{
    websDefineAction("timeShowAction",   __timeShowAction);
    websDefineAction("timeConfigAction", __timeConfigAction);
}
/*********************************************************************************************************
** ��������: timeActionInit
** ��������: time action ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  timeActionInit (VOID)
{
    __timeActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
