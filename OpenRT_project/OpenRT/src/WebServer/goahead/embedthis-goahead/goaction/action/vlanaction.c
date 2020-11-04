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
** ��   ��   ��: vlanaction.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: VLAN���ú�̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "lwip/netif.h"
#include "vlan_info.h"
#include "log_info.h"
#include "intada_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: vlan ���ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_VLAN_VLANID           "vlan_id"                             /*  Json�ֶΣ�vlanID            */
#define WEB_VLAN_IFACE            "iface"                               /*  Json�ֶΣ��ӿ�����          */
#define WEB_VLAN_TAG              "tag"                                 /*  Json�ֶΣ�tag               */
#define WEB_VLAN_PRIORITY         "priority"                            /*  Json�ֶΣ����ȼ�            */
#define WEB_VLAN_OPER_TYPE        "oper_type"                           /*  Json�ֶΣ��������ͣ���ɾ�Ĳ�*/
#define WEB_VLAN_WEBNAME          "webname"                             /*  Json�ֶΣ�web �ӿ���        */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                  "page"                                /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                 "total"                               /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                "data"                                /*  Json�ֶΣ������˿�          */

/*********************************************************************************************************
** ��������: __vlanShowAction
** ��������: vlan��Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vlanShowAction (Webs *wp)
{
    __PVLAN_HANDLE   pVlanHeader              = LW_NULL;
    __VLAN_HANDLE    vlanHandle               = {0};
    __PVLAN_HANDLE   pTmpNode                 = LW_NULL;
    INT              iPage                    = 1;                      /*  ����ʾ��ҳ��                */
    INT              iTotal                   = 0;                      /*  ����Ŀ                      */
    cJSON*           pJsonVlanArry            = LW_NULL;
    cJSON*           pjSend                   = LW_NULL;
    cJSON           *pRecvjRes                = LW_NULL;
    cJSON           *pjPage                   = LW_NULL;
    CHAR             cWebName[NETIF_NAMESIZE] = {0};
    INT              iRecordTotal             = 0;
    INT              i;

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

    pjSend        = cJSON_CreateObject();                               /*  �������� Json ����          */
    pJsonVlanArry = cJSON_CreateArray();                                /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVlanArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
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
     *  ���ò�ѯ����ӿ�
     */
    pVlanHeader = vlanShow(&vlanHandle, &iRecordTotal);
    if (LW_NULL != pVlanHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            pTmpNode = pVlanHeader + i;

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ5����
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjVlan = cJSON_CreateObject();                  /*  ����һ��ӳ�� Json ����      */
                if(LW_NULL == pjVlan) {
                    break;
                }

                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_VLANID,   pTmpNode->iVlanID);

                libIntAdaGetWebNetif(pTmpNode->cIface, cWebName, sizeof(cWebName));

                cJSON_AddStringToObject(pjVlan, WEB_VLAN_IFACE,    cWebName);
                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_TAG,      pTmpNode->iTag);
                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_PRIORITY, pTmpNode->iPriority);

                cJSON_AddItemToArray(pJsonVlanArry, pjVlan);            /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    if (NULL != pVlanHeader) {
        free(pVlanHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonVlanArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVlanArry);
}
/*********************************************************************************************************
** ��������: __vlanConfigAction
** ��������: vlan�����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vlanConfigAction (Webs *wp)
{
    cJSON          *pRecvjRes       = LW_NULL;
    cJSON          *pSendjRes       = LW_NULL;

    cJSON          *pjVlanId        = LW_NULL;
    cJSON          *pjIface         = LW_NULL;
    cJSON          *pjOperType      = LW_NULL;
    __VLAN_HANDLE   vlanHandle      = {0};
    char            logContent[128] = {0};
    INT             iRet;

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
    pjVlanId   = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_VLANID);
    pjIface    = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_IFACE);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_OPER_TYPE);
    if (NULL == pjVlanId ||
        NULL == pjIface  ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    vlanHandle.iVlanID = pjVlanId->valueint;
    strncpy(vlanHandle.cOperType, pjOperType->valuestring, sizeof(vlanHandle.cOperType));

    /*
     *  ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pjIface->valuestring, vlanHandle.cIface, sizeof(vlanHandle.cIface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     * ������������ӿ�
     */
    iRet = vlanConfig(&vlanHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    snprintf(logContent, sizeof(logContent), "Operation: vlan %s %s %d",
    		 vlanHandle.cOperType, vlanHandle.cIface, vlanHandle.iVlanID);
    operLogInsert(wp, OPEN_RT_VLAN, logContent, iRet);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** ��������: __vlanActionDefine
** ��������: vlan��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vlanActionDefine (VOID)
{
    websDefineAction("vlanShowAction", __vlanShowAction);
    websDefineAction("vlanConfigAction", __vlanConfigAction);
}

/*********************************************************************************************************
** ��������: vlanActionInit
** ��������: vlan��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vlanActionInit (VOID)
{
    __vlanActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
