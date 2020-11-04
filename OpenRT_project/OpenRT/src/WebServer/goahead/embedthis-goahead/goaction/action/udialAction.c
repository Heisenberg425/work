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
** ��   ��   ��: udialAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 15 ��
**
** ��        ��: OpenRT ���Ŵ����̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "udial_info.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "udial_info.h"
#include "intada_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: udial
*********************************************************************************************************/
#define WEB_UDIAL_INDEX       "webindex"                                /*  Json�ֶΣ����ں�            */
#define WEB_UDIAL_NETIF       "webnetif"                                /*  Json�ֶΣ�ǰ��������        */
#define WEB_UDIAL_ENNAME      "enname"                                  /*  Json�ֶΣ�������            */
#define WEB_UDIAL_ACCOUNT     "account"                                 /*  Json�ֶΣ��˺�              */
#define WEB_UDIAL_PASSWD      "passwd"                                  /*  Json�ֶΣ�����              */
#define WEB_UDIAL_HANDLE      "handle"                                  /*  Json�ֶΣ�����ʽ          */
#define WEB_UDIAL_STATUS      "status"                                  /*  Json�ֶΣ�״̬              */
/*********************************************************************************************************
** ��������: __udialHandleAction
** ��������: ���Ŵ����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};
    CHAR                  cUdialStatus[15]            = {0};

    PCHAR                 pcAccount                   = LW_NULL;
    PCHAR                 pcPassWd                    = LW_NULL;
    PCHAR                 pcWebNetif                  = LW_NULL;
    INT                   iHandle                     = -1;

    cJSON                *pjAccount                   = LW_NULL;
    cJSON                *pjPassWd                    = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;
    cJSON                *pjHandle                    = LW_NULL;

    INT                   iRet                        = PX_ERROR;
    CHAR                  cLogBuff[30]                = {0};

    __UDIAL_HANDLE        udialHandleInfo;

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
    pjAccount  = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ACCOUNT);
    pjPassWd   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_PASSWD);
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ENNAME);
    pjHandle   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_HANDLE);
    if (LW_NULL == pjAccount  ||
        LW_NULL == pjPassWd   ||
        LW_NULL == pjWebNetif ||
        LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcAccount  = pjAccount->valuestring;
    pcPassWd   = pjPassWd->valuestring;
    pcWebNetif = pjWebNetif->valuestring;
    iHandle    = pjHandle->valueint;

#if 0
    iRet = libIntAdaGetNetif(pcWebNetif,                                /*  ��ȡ��Ӧ��ϵͳ��������      */
                            cNetIfName,
                            sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }
#else
    /*
     *  ǰ�˸�����̫����������Ȼ���ʹ�� en ��ͨ��
     */
    strncpy(cNetIfName, pcWebNetif, sizeof(cNetIfName));
#endif
    /*
     *  ��¼������־
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s pppoe udial\n", 1 == iHandle ? "start" : "delete");
    operLogInsert(wp, OPEN_RT_UDAIL, cLogBuff, 0);

    if (1 == iHandle) {                                                 /*  ��������                    */
        iRet = libUdialStartHandle(pcAccount, pcPassWd, cNetIfName);
    } else if (0 == iHandle) {                                          /*  ɾ������                    */
        iRet = libUdialDelHandle(cNetIfName);
    } else if (2 == iHandle) {                                          /*  ��ȡ������Ϣ                */
        bzero(&udialHandleInfo, sizeof(udialHandleInfo));
        strncpy(udialHandleInfo.cNetifname, cNetIfName, sizeof(udialHandleInfo.cNetifname));
        iRet = libUdialInfoGet(&udialHandleInfo);

        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_ACCOUNT, udialHandleInfo.cUsrname);
        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_PASSWD,  udialHandleInfo.cPassword);

        switch (udialHandleInfo.iUdialStatus) {
        case NET_EVENT_PPP_DEAD:
            strncpy(cUdialStatus, "dead", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_INIT:
            strncpy(cUdialStatus, "init", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_AUTH:
            strncpy(cUdialStatus, "auth", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_RUN:
            strncpy(cUdialStatus, "run", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_DISCONN:
            strncpy(cUdialStatus, "disconnect", sizeof(cUdialStatus));
            break;

        default:
            strncpy(cUdialStatus, "error", sizeof(cUdialStatus));
            break;
        }

        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_STATUS, cUdialStatus);
    }

    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    } else if (1 == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** ��������: __udialActionDefine
** ��������: ���ź�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __udialActionDefine (VOID)
{
    websDefineAction("udialHandleAction", __udialHandleAction);
}

/*********************************************************************************************************
** ��������: udialActionInit
** ��������: ���ź�̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID udialActionInit (VOID)
{
    __udialActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
