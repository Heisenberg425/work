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
** ��   ��   ��: arpConfAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 15 ��
**
** ��        ��: OpenRT ARP ��̨����ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "arp_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
#include "situation_info.h"
#include "situationCommon.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define JSON_ARGERR_CODE           (100)                                /*  �������󷵻�Json codeֵ     */
/*********************************************************************************************************
  Jason key: arp ������Ϣ
*********************************************************************************************************/
#define WEB_ASD_EN                 "en"                                 /*  Json�ֶΣ��Ƿ�������      */
/*********************************************************************************************************
** ��������: __asdefenseEnConfAction
** ��������: arp ����������̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __asdefenseEnConfAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *jsEnFlag      = LW_NULL;
    UINT8                 ucEnFlag      = 0;

    __SITUATION_INFO      situationInfo;

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

        goto  __finish;
    }

    /*
     *  ��ȡ Json ��������
     */
    jsEnFlag = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_EN);
    if (LW_NULL == jsEnFlag) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    }

    ucEnFlag = jsEnFlag->valueint;

    iRet = libAsDefenseHandle(ucEnFlag);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    }

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition, POS_CONFIG_COMPLETE, strlen(POS_CONFIG_COMPLETE));
    memcpy(situationInfo.cStatus,    STATUS_CONFIGING, strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = -1;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __arpConfigActionDefine
** ��������: arp��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __arpConfigActionDefine (VOID)
{
    websDefineAction("asdefenseEnConfAction", __asdefenseEnConfAction);
}

/*********************************************************************************************************
** ��������: arpConfigActionInit
** ��������: arp��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpConfigActionInit (VOID)
{
    __arpConfigActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
