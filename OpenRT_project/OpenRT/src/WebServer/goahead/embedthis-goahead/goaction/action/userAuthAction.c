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
** ��   ��   ��: userAuthAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 9 �� 30 ��
**
** ��        ��: OpenRT �û�Ȩ�޹���
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "../webHandle.h"
#include "../operLog.h"
#include "liblogin_config.h"
/*********************************************************************************************************
  ȫ��Ȩ�ޱ�
*********************************************************************************************************/
static PCHAR  _G_pcOperatorAuthMap;                                     /*  ����ԱȨ��                  */
static PCHAR  _G_pcViewAuthMap;                                         /*  �۲�ԱȨ��                  */
extern const char *__G_pOpenRTModuleName[OPEN_RT_MAX_INDEX];            /*  OpenRT ģ����               */
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE               "code"                                   /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE      (200)                                    /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE       (500)                                    /*  ʧ�ܷ���Json codeֵ         */
#define OPERA_ARRAY_NAME       "operadata"                              /*  Json�ֶΣ�������Ȩ������    */
#define VIEW_ARRAY_NAME        "viewdata"                               /*  Json�ֶΣ��۲���Ȩ������    */
/*********************************************************************************************************
  Jason key
*********************************************************************************************************/
#define WEB_USER_AUTH_TYPE     "type"                                   /*  Json�ֶΣ�ģ������          */
#define WEB_USER_AUTH_ROLE     "role"                                   /*  Json�ֶΣ���ɫ              */
#define WEB_USER_AUTH_HANDLE   "handle"                                 /*  Json�ֶΣ���������          */
/*********************************************************************************************************
  handle ��
*********************************************************************************************************/
#define HANDLE_ON              "on"                                     /*  ����Ȩ��                    */
#define HANDLE_OFF             "off"                                    /*  ��ֹȨ��                    */
/*********************************************************************************************************
** ��������: authMapUpdate
** ��������: ������֤��Ϣ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __authMapUpdate (VOID)
{
    PVOID  pvInfo = LW_NULL;

    pvInfo = libuserAuthGet(OPERA_AUTH);                                /*  ��ȡ������Ȩ��              */
    if (NULL != pvInfo) {
        memcpy(_G_pcOperatorAuthMap, pvInfo, OPEN_RT_MAX_INDEX);

        free(pvInfo);

        pvInfo = NULL;
    }

    pvInfo = libuserAuthGet(VIEW_AUTH);                                 /*  ��ȡ�۲���Ȩ��              */
    if (NULL != pvInfo) {
        memcpy(_G_pcViewAuthMap, pvInfo, OPEN_RT_MAX_INDEX);

        free(pvInfo);

        pvInfo = NULL;
    }
}

/*********************************************************************************************************
** ��������: __userAuthHandleAction
** ��������: �û�Ȩ����ʾ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __userAuthShowAction (Webs *wp)
{
    cJSON            *pSendjRes         = LW_NULL;
    PCHAR             pcOperatorAuthMap = LW_NULL;
    PCHAR             pcViewAuthMap     = LW_NULL;
    INT32             i;

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

    pcOperatorAuthMap = libuserAuthGet(OPERA_AUTH);                     /*  ��ȡ������Ȩ��              */
    pcViewAuthMap     = libuserAuthGet(VIEW_AUTH);                      /*  ��ȡ�۲���Ȩ��              */
    if (LW_NULL == pcOperatorAuthMap || LW_NULL == pcViewAuthMap) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto __error;
    }

    cJSON  *pjOperatorAuth = cJSON_CreateObject();                      /*  ����һ��ӳ�� Json ����      */
    if(LW_NULL == pjOperatorAuth) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto __error;
    }

    cJSON  *pjViewAuth = cJSON_CreateObject();                          /*  ����һ��ӳ�� Json ����      */
    if(LW_NULL == pjViewAuth) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
        cJSON_Delete(pjOperatorAuth);

        goto __error;
    }

    for (i = 0; i < OPEN_RT_MAX_INDEX; i++) {
        cJSON_AddStringToObject(pjOperatorAuth,
                                __G_pOpenRTModuleName[i],
                                pcOperatorAuthMap[i] ? "on" : "off");
        cJSON_AddStringToObject(pjViewAuth,
                                __G_pOpenRTModuleName[i],
                                pcViewAuthMap[i] ? "on" : "off");
    }

    cJSON_AddItemToObject(pSendjRes, OPERA_ARRAY_NAME, pjOperatorAuth);
    cJSON_AddItemToObject(pSendjRes, VIEW_ARRAY_NAME, pjViewAuth);
    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);                  /*  �����ɹ�                    */

__error:
    free(pcOperatorAuthMap);
    free(pcViewAuthMap);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __userAuthHandleAction
** ��������: �û�Ȩ�����ú�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __userAuthHandleAction (Webs *wp)
{
    cJSON            *pRecvjRes                           = LW_NULL;
    cJSON            *pSendjRes                           = LW_NULL;
    cJSON            *pjType                              = LW_NULL;
    cJSON            *pjRole                              = LW_NULL;
    cJSON            *pjHandle                            = LW_NULL;
    CHAR             *pcType                              = LW_NULL;
    CHAR             *pcHandle                            = LW_NULL;
    CHAR             *pcRole                              = LW_NULL;

    __USER_AUTH_SET   userAuth;

    INT               iRet;

    /*
     * �û���Ȩ
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
    pjType   = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_TYPE);
    pjHandle = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_HANDLE);
    pjRole   = cJSON_GetObjectItem(pRecvjRes, WEB_USER_AUTH_ROLE);
    if (LW_NULL == pjType || LW_NULL == pjHandle || LW_NULL == pjRole) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcType   = pjType->valuestring;
    pcRole   = pjRole->valuestring;
    pcHandle = pjHandle->valuestring;

    bzero(&userAuth, sizeof(userAuth));
    userAuth.cHandle = (strcmp(pcHandle, HANDLE_OFF)) ? AUTH_HANDLE_ON : AUTH_HANDLE_OFF;
    strncpy(userAuth.cRole, pcRole, sizeof(userAuth.cRole));
    strncpy(userAuth.cTypeName, pcType, sizeof(userAuth.cTypeName));

    iRet = libuserAuthHandle(&userAuth);
    if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
    }

    __authMapUpdate();
__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** ��������: __userAuthActionDefine
** ��������: �û�Ȩ�޺�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __userAuthActionDefine (VOID)
{
    websDefineAction("userAuthHandleAction", __userAuthHandleAction);
    websDefineAction("userAuthShowAction", __userAuthShowAction);
}

/*********************************************************************************************************
** ��������: userAuthCheck
** ��������: �û�Ȩ����֤
** �䡡��  : iModule            ģ��
**           ucOperaType        ������ / �۲���
** �䡡��  : NONE
** ������  : �Ƿ�ͨ��
*********************************************************************************************************/
BOOL  userAuthCheck (INT32  iModule, UINT8  ucOperaType)
{
    if (ACTION_OPERAT == ucOperaType && _G_pcOperatorAuthMap[iModule]) {
        return  (LW_TRUE);
    } else if (ACTION_VIEW == ucOperaType && _G_pcViewAuthMap[iModule]) {
        return  (LW_TRUE);
    } else {
        return  (LW_FALSE);
    }
}

/*********************************************************************************************************
** ��������: userAuthActionInit
** ��������: �û�Ȩ�޺�̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  userAuthActionInit (VOID)
{
    _G_pcOperatorAuthMap = _G_pcViewAuthMap = LW_NULL;
    _G_pcOperatorAuthMap = libuserAuthGet(OPERA_AUTH);                  /*  ��ȡ������Ȩ��              */
    _G_pcViewAuthMap     = libuserAuthGet(VIEW_AUTH);                   /*  ��ȡ�۲���Ȩ��              */
    __userAuthActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
