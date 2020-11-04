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
** ��   ��   ��: commonAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 15 ��
**
** ��        ��: OpenRT ͨ�ú�̨����ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "../webHandle.h"
#include "situation_info.h"
#include "situationCommon.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define WEB_WEBSOCK_IP             "websockip"                          /*  Json�ֶΣ�Websocket ip      */
/*********************************************************************************************************
  OpenRT ��ѡ���ܺ궨��
*********************************************************************************************************/
#define WEB_KIDVPN_CLIENT         "kidvpn_client"                       /*  ��ѡ���ܣ�KidVPN �ͻ���     */
#define WEB_KIDVPN_SERVER         "kidvpn_server"                       /*  ��ѡ���ܣ�KidVPN �����     */
#define WEB_ARP_DEFENSE           "arp_defense"                         /*  ��ѡ���ܣ�ARP ����          */
#define FUNC_KIDVPN_CLIENT        (1 << 0)                              /*  ��ѡ���ܣ�KidVPN �ͻ���     */
#define FUNC_KIDVPN_SERVER        (1 << 1)                              /*  ��ѡ���ܣ�KidVPN �����     */
#define FUNC_ARP_DEFENSE          (1 << 2)                              /*  ��ѡ���ܣ�ARP ����          */
/*********************************************************************************************************
** ��������: __configureCompleteAction
** ��������: ������ɲ�����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __configCompleteAction (Webs *wp)
{
    cJSON                *pSendjRes     = LW_NULL;
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

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition, POS_CONFIG_COMPLETE, strlen(POS_CONFIG_COMPLETE));
    memcpy(situationInfo.cStatus,    STATUS_FINISHED, strlen(STATUS_FINISHED));
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
}
/*********************************************************************************************************
** ��������: __websocketIpGetAction
** ��������: ��ȡ websocket ip �����̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __websocketIpGetAction (Webs *wp)
{
    cJSON            *pSendjRes    = NULL;

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
    if(LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_WEBSOCK_IP, websGetIfaddr(wp));

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __situationStatusResetAction
** ��������: �龰ģʽ״̬��Ϣ��ʼ�������̨�ӿ�
** �䡡��  : wp      ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __situationStatusResetAction (Webs *wp)
{
    cJSON             *pSendjRes = NULL;

#if 0
    INT                iRet;
    __SITUATION_INFO   situationInfo;
#endif

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
    if(LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }
#if 0
    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition,      POS_STARTING_SET, strlen(POS_STARTING_SET));
    memcpy(situationInfo.cStatus,         STATUS_UNCONFIG,  strlen(STATUS_UNCONFIG));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT,  strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = 0;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __finish;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
#endif

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __optionalFuncCancelAction
** ��������: ������ѡ���ܺ�̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __optionalFuncCancelAction (Webs *wp)
{
    INT     iRet         = PX_ERROR;
    cJSON  *pRecvjRes    = LW_NULL;
    cJSON  *pSendjRes    = LW_NULL;

    INT     iFunc        = 0;
    INT     iKidVpnCli   = 0;
    INT     iKidVpnServ  = 0;
    INT     iArpDefense  = 0;

    cJSON  *pjKidVpnCli  = LW_NULL;
    cJSON  *pjKidVpnServ = LW_NULL;
    cJSON  *pjArpDefense = LW_NULL;


    if (!isUserLogin(wp)) {                                             /*  �û���Ȩ                    */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    webHeadHandle(wp, 200);                                             /*  ���� http ��ͷ              */

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    pjKidVpnCli = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);    /*  ��ȡ Json ��������          */
    if (LW_NULL == pjKidVpnCli) {
        iKidVpnCli = FUNC_KIDVPN_CLIENT;
    } else {
        iKidVpnCli = pjKidVpnCli->valueint;
    }

    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);   /*  ��ȡ Json ��������          */
    if (LW_NULL == pjKidVpnServ) {
        iKidVpnServ = FUNC_KIDVPN_SERVER;
    } else {
        iKidVpnServ = pjKidVpnServ->valueint;
    }

    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);     /*  ��ȡ Json ��������          */
    if (LW_NULL == pjArpDefense) {
        iArpDefense = FUNC_ARP_DEFENSE;
    } else {
        iArpDefense = pjArpDefense->valueint;
    }

    iFunc = iKidVpnCli | iKidVpnServ | iArpDefense;

    iRet = libOptionalFuncCancel(iFunc);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __configureCompleteActionDefine
** ��������: ������ɺ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __configureCompleteActionDefine (VOID)
{
    websDefineAction("configCompleteAction",     __configCompleteAction);
    websDefineAction("websocketIpGetAction",     __websocketIpGetAction);
    websDefineAction("situationStatResetAction", __situationStatusResetAction);
    websDefineAction("optionalFuncCancelAction", __optionalFuncCancelAction);
}

/*********************************************************************************************************
** ��������: configureCompletActionInit
** ��������: ������ɺ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  configureCompletActionInit (VOID)
{
    __configureCompleteActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
