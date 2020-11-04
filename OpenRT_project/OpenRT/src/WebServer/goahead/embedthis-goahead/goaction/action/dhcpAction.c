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
** ��   ��   ��: dhcpAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT DHCP ��̨����ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "dhcp_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_ARGERR_CODE      (100)                                     /*  �������󷵻�Json codeֵ     */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: DHCP �ͻ��˴���
*********************************************************************************************************/
#define WEB_DHCPC_NETNAME     "netifname"                               /*  Json�ֶΣ���������          */
#define WEB_DHCPC_HANDLE      "handle"                                  /*  Json�ֶΣ�ʹ�� / ����       */
/*********************************************************************************************************
** ��������: __dhcpClientAction
** ��������: DHCP �ͻ��˺�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpClientAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcNetifName      = LW_NULL;
    CHAR                 *pcHandle         = LW_NULL;
    cJSON                *pjNetifName      = LW_NULL;
    cJSON                *pjHandle         = LW_NULL;
    CHAR                  cDhcpHandle      = PX_ERROR;
    INT                   iRet             = PX_ERROR;

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
    pjNetifName = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPC_NETNAME);
    pjHandle    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPC_HANDLE);
    if (LW_NULL == pjNetifName || LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    pcNetifName  = pjNetifName->valuestring;
    pcHandle     = pjHandle->valuestring;

    if (!strcmp("enable", pcHandle)) {
        cDhcpHandle = DHCP_CLIENT_ENABLE;
    } else if (!strcmp("disable", pcHandle)) {
        cDhcpHandle = DHCP_CLIENT_DISABLE;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __error;
    }

    iRet = libDhcpClientHandle(pcNetifName, cDhcpHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** ��������: __dhcpActionDefine
** ��������: DHCP ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpActionDefine (VOID)
{
    websDefineAction("dhcpClientAction", __dhcpClientAction);
}
/*********************************************************************************************************
** ��������: dhcpActionInit
** ��������: DHCP ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpActionInit (VOID)
{
    __dhcpActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
