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
** ��   ��   ��: devnameAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ�豸�����ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "devname_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: time ���ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_DEV_NAME              "dev_name"                            /*  Json�ֶΣ��豸��            */
/*********************************************************************************************************
** ��������: __devNameGetAction
** ��������: ��ȡ�豸����Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __devNameGetAction (Webs *wp)
{
    __PDEVNAME_HANDLE  pDevNameHeader = LW_NULL;
    __DEVNAME_HANDLE   devNameHandle  = {{0}};
    cJSON*             pjSend         = LW_NULL;
    INT                iRecordTotal   = 0;

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

    /*
     *  ���ò�ѯ����ӿڲ����÷��ص� Json ����
     */
    pDevNameHeader = devNameShow(&devNameHandle, &iRecordTotal);
    if (LW_NULL != pDevNameHeader) {
        cJSON_AddStringToObject(pjSend, WEB_DEV_NAME, pDevNameHeader->cdevName);
    } else {
        cJSON_AddStringToObject(pjSend, WEB_DEV_NAME, "Acoinfo OpenRT");
    }

    if (pDevNameHeader) {
        free(pDevNameHeader);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __devNameConfigAction
** ��������: �����豸�������̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __devNameConfigAction (Webs *wp)
{
    INT                iRet;
    cJSON             *pRecvjRes     = LW_NULL;
    cJSON             *pSendjRes     = LW_NULL;
    cJSON             *pjDevName     = LW_NULL;
    __DEVNAME_HANDLE   devNameHandle = {{0}};

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
    pjDevName = cJSON_GetObjectItem(pRecvjRes, WEB_DEV_NAME);
    if (NULL == pjDevName) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(devNameHandle.cdevName, pjDevName->valuestring, sizeof(devNameHandle.cdevName));

    /*
     * ������������ӿ�
     */
    iRet = devNameConfig(&devNameHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __devNameActionDefine
** ��������: device name ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __devNameActionDefine (VOID)
{
    websDefineAction("devNameGetAction",    __devNameGetAction);
    websDefineAction("devNameConfigAction", __devNameConfigAction);
}
/*********************************************************************************************************
** ��������: devNameActionInit
** ��������: �豸�� action ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  devNameActionInit (VOID)
{
    __devNameActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
