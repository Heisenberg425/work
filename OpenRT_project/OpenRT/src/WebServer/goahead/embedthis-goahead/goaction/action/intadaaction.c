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
** ��   ��   ��: iniadaaction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: OpenRT ��������Ӧ��λ��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "sys/un.h"
#include "intada_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  �����Զ�����غ궨��
*********************************************************************************************************/
#define WEB_INT_NAME       "name"                                       /*  Json�ֶΣ�������(�û������)*/
#define WEB_INT_INDEX      "index"                                      /*  Json�ֶΣ�ǰ̨����          */
#define WEB_INT_FLAG       "flag"                                       /*  Json�ֶΣ�����˳���־      */
#define WEB_INT_CODE       "code"                                       /*  Json�ֶΣ�����״̬          */

#define INT_FLAG_FIRST     "first"                                      /*  key :flag ��һ������        */
#define INT_FLAG_F         (0)                                          /*  flag = 0  ��һ������        */
#define INT_FLAG_MIDDLE    "middle"                                     /*  key :flag �м�����          */
#define INT_FLAG_M         (2)                                          /*  flag = 2  �м�����          */
#define INT_FLAG_LAST      "last"                                       /*  key :flag ���һ������      */
#define INT_FLAG_L         (1)                                          /*  flag = 1  ���һ������      */
/*********************************************************************************************************
** ��������: __intAdaHandleAction
** ��������: ��������Ӧ����
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __intAdaHandleAction (Webs *wp)
{
    INT                   iRet;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pSendjRes       = LW_NULL;
    CHAR                 *pcName          = LW_NULL;
    CHAR                 *pcFlag          = LW_NULL;
    cJSON                *pjName          = LW_NULL;
    cJSON                *pjFlag          = LW_NULL;
    cJSON                *pjIndex         = LW_NULL;

    __INT_ADA_SET         intAdaSetInfo;

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
        webBackHandle(wp, &_G_ErrorJRes);                               /*  Json �ṹ����ʧ��           */

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjName  = cJSON_GetObjectItem(pRecvjRes, WEB_INT_NAME);
    pjFlag  = cJSON_GetObjectItem(pRecvjRes, WEB_INT_FLAG);
    pjIndex = cJSON_GetObjectItem(pRecvjRes, WEB_INT_INDEX);
    if (LW_NULL == pjName ||
        LW_NULL == pjFlag ||
        LW_NULL == pjIndex) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 100);

        goto  __error;
    }

    pcName = pjName->valuestring;

    if ((NULL != lib_strstr(pcName, "VPN-")) ||
        (strlen(pcName) >= 10)) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 100);

        goto  __error;
    }

    bzero(&intAdaSetInfo, sizeof(intAdaSetInfo));
    strncpy(intAdaSetInfo.cWebName, pcName, sizeof(intAdaSetInfo.cWebName));

    intAdaSetInfo.iWebIndex = pjIndex->valueint;
    pcFlag                  = pjFlag->valuestring;
    intAdaSetInfo.cFlag     = ((0 == strcmp(pcFlag, INT_FLAG_FIRST)) ? INT_ADA_FLAG_FIRST :
                              ((0 == strcmp(pcFlag, INT_FLAG_LAST))  ? INT_ADA_FLAG_LAST  :
                               INT_ADA_FLAG_MIDDLE));

    iRet = libIntAdaSet(&intAdaSetInfo);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 200);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_INT_INDEX, intAdaSetInfo.iWebIndex);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** ��������: __intAdaCancelAction
** ��������: ��������Ӧȡ��(��ʱ�޲���)
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __intAdaCancelAction (Webs *wp)
{
    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    libIntAdaClear();

    /*
     *  ��¼������־
     */
    operLogInsert(wp, OPEN_RT_INTADA, "restart interface adaption\n", 0);

    webHeadHandle(wp, 200);
    webBackHandle(wp, NULL);
}

/*********************************************************************************************************
** ��������: __intAdaActionDefine
** ��������: ���ڶ�λ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __intAdaActionDefine (VOID)
{
    websDefineAction("intAdaHandleAction", __intAdaHandleAction);
    websDefineAction("intAdaCancelAction", __intAdaCancelAction);
}

/*********************************************************************************************************
** ��������: intAdaActionInit
** ��������: ���ڶ�λ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  intAdaActionInit (VOID)
{
    __intAdaActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
