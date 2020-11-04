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
** ��   ��   ��: vndAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 27 ��
**
** ��        ��: OpenRT ����������̨����ӿ�
** 2018.08.16  �޸��� sscanf ���ڴ�д���ǲ�����BUG.
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "sys/un.h"
#include "vnd_info.h"
#include "../webHandle.h"
#include "libinterface_config.h"
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: kidvpn ����
*********************************************************************************************************/
#define WEB_VND_VNDID       "vndid"                                     /*  Json�ֶΣ��������� id       */
#define WEB_VND_IPADDR      "ipaddr"                                    /*  Json�ֶΣ�IP ��ַ           */
#define WEB_VND_NETMASK     "netmask"                                   /*  Json�ֶΣ���������          */
#define WEB_VND_GATEWAY     "gateway"                                   /*  Json�ֶΣ����ص�ַ          */
#define WEB_VND_MTU         "mtu"                                       /*  Json�ֶΣ�mtu ֵ            */
#define WEB_VND_MAC         "mac"                                       /*  Json�ֶΣ�MAC ��ַ          */
/*********************************************************************************************************
** ��������: __vndHandleAction
** ��������: �������������̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;

    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcMtu            = LW_NULL;
    CHAR                 *pcNetMask        = LW_NULL;
    CHAR                 *pcGateWay        = LW_NULL;
    CHAR                 *pcMac            = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjMtu            = LW_NULL;
    cJSON                *pjNetMask        = LW_NULL;
    cJSON                *pjGateWay        = LW_NULL;
    cJSON                *pjMac            = LW_NULL;

    __VND_HANDLE          vndHandleInfo;
    INT                   iVndID           = 0;
    UINT                  uiTmpHwAddr[6]   = {0};

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
    pjIpAddr  = cJSON_GetObjectItem(pRecvjRes, WEB_VND_IPADDR);
    pjNetMask = cJSON_GetObjectItem(pRecvjRes, WEB_VND_NETMASK);
    pjGateWay = cJSON_GetObjectItem(pRecvjRes, WEB_VND_GATEWAY);
    pjMtu     = cJSON_GetObjectItem(pRecvjRes, WEB_VND_MTU);
    pjMac     = cJSON_GetObjectItem(pRecvjRes, WEB_VND_MAC);
    if (LW_NULL == pjIpAddr  ||
        LW_NULL == pjMtu     ||
        LW_NULL == pjNetMask ||
        LW_NULL == pjGateWay ||
        LW_NULL == pjMac) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcIpAddr  = pjIpAddr->valuestring;
    pcNetMask = pjNetMask->valuestring;
    pcGateWay = pjGateWay->valuestring;
    pcMtu     = pjMtu->valuestring;
    pcMac     = pjMac->valuestring;

    /*
     *  ������Ϣ
     */
    inet_aton(pcIpAddr, (struct in_addr *)&(vndHandleInfo.uiIpAddr));
    inet_aton(pcNetMask, (struct in_addr *)&(vndHandleInfo.uiNetMask));
    inet_aton(pcGateWay, (struct in_addr *)&(vndHandleInfo.uiGateWay));
    vndHandleInfo.iMtu = atoi(pcMtu);

    sscanf(pcMac, "%02x:%02x:%02x:%02x:%02x:%02x",
           &uiTmpHwAddr[0], &uiTmpHwAddr[1],
           &uiTmpHwAddr[2], &uiTmpHwAddr[3],
           &uiTmpHwAddr[4], &uiTmpHwAddr[5]);

    vndHandleInfo.ucHwAddr[0] = uiTmpHwAddr[0];
    vndHandleInfo.ucHwAddr[1] = uiTmpHwAddr[1];
    vndHandleInfo.ucHwAddr[2] = uiTmpHwAddr[2];
    vndHandleInfo.ucHwAddr[3] = uiTmpHwAddr[3];
    vndHandleInfo.ucHwAddr[4] = uiTmpHwAddr[4];
    vndHandleInfo.ucHwAddr[5] = uiTmpHwAddr[5];

    iVndID = libVndCreate(&vndHandleInfo);
    if (RET_ARG_ERROR == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
    } else if (RET_HANDLE_FAIL == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
    } else {                                                            /*  ���ص�ֵ�� vnd ID           */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_VND_VNDID, iVndID);          /*  ���� vnd ID                 */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** ��������: __vndActionDefine
** ��������: ����������̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndActionDefine (VOID)
{
    websDefineAction("vndHandleAction", __vndHandleAction);
}

/*********************************************************************************************************
** ��������: vndActionInit
** ��������: ����������̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vndActionInit (VOID)
{
    __vndActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
