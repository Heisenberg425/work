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
** ��   ��   ��: routeaction.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 22 ��
**
** ��        ��: ·�������ú�̨�ӿ�
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
#include "intada_info.h"
#include "vnd_info.h"
#include "route_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: ��̬·�����ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_ROUTE_DEST        "destination"                             /*  Json�ֶΣ�Ŀ�ĵ�ַ          */
#define WEB_ROUTE_GATEWAY     "gateway"                                 /*  Json�ֶΣ�����              */
#define WEB_ROUTE_GENMASK     "genmask"                                 /*  Json�ֶΣ�����              */
#define WEB_ROUTE_FLAGS       "flags"                                   /*  Json�ֶΣ�·��Flags         */
#define WEB_ROUTE_PREFIX      "prefix"                                  /*  Json�ֶΣ�ipv6 ǰ׺         */
#define WEB_ROUTE_METRIC      "metric"                                  /*  Json�ֶΣ�·�ɶ���ֵ        */
#define WEB_ROUTE_REF         "ref"                                     /*  Json�ֶΣ�����ֵ            */
#define WEB_ROUTE_USE         "use"                                     /*  Json�ֶΣ�ʹ��              */
#define WEB_ROUTE_IFACE       "iface"                                   /*  Json�ֶΣ��ӿ�����          */
#define WEB_ROUTE_IP_FAMILY   "ip_family"                               /*  Json�ֶΣ�ip��ַ��          */
#define WEB_ROUTE_OPER_TYPE   "oper_type"                               /*  Json�ֶΣ��������ͣ���ɾ�Ĳ�*/
#define WEB_ROUTE_ROUTE_TYPE  "route_type"                              /*  Json�ֶΣ�·�����ͣ�����\����*/
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_ROUTE_PAGE        "page"                                    /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_ROUTE_TOTAL       "total"                                   /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME            "data"                                    /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
** ��������: __routeInfoGetAction
** ��������: ��ȡ·����Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __routeInfoGetAction (Webs *wp)
{
    __PROUTE_HANDLE   pRouteInfo               = LW_NULL;
    __PROUTE_HANDLE   pRouteInfoTmp            = LW_NULL;
    __ROUTE_HANDLE    routeHandle              = {{0}};
    CHAR              cWebName[NETIF_NAMESIZE] = {0};
    INT               iNetIfIndex              = 0;
    INT               iVndid                   = 0;
    INT               iRet;

    INT               iPage                    = 1;                     /*  ����ʾ��ҳ��                */
    INT               iTotal                   = 0;                     /*  ����Ŀ                      */
    PCHAR             pcRouteType              = LW_NULL;
    cJSON*            pJsonRouteArry           = LW_NULL;
    cJSON*            pjSend                   = LW_NULL;
    cJSON            *pRecvjRes                = LW_NULL;
    cJSON            *pjPage                   = LW_NULL;
    cJSON            *pjFamily                 = LW_NULL;
    cJSON            *pjType                   = LW_NULL;
    INT               iRecordTotal             = 0;
    INT               i;

    if (!isUserLogin(wp)) {                                              /*  �û���Ȩ                   */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend         = cJSON_CreateObject();                              /*  �������� Json ����          */
    pJsonRouteArry = cJSON_CreateArray();                               /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pjSend || LW_NULL == pJsonRouteArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    bzero(&routeHandle, sizeof(__ROUTE_HANDLE));

    /*
     *  ��ȡ Json ��������
     */
    pjPage   = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_PAGE);
    pjFamily = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_IP_FAMILY);
    pjType   = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_ROUTE_TYPE);
    if (NULL == pjPage || NULL == pjFamily || NULL == pjType) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    if ('\0' == pjFamily->valuestring[0]) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(routeHandle.cIpFamily, pjFamily->valuestring, sizeof(routeHandle.cIpFamily));

    pcRouteType = pjType->valuestring;
    if ('\0' != pcRouteType[0]) {
        strncpy(routeHandle.cRouteType, pcRouteType, sizeof(routeHandle.cRouteType));
    }

    /*
     *  ����������Ͳ�ѯ·������
     */
    pRouteInfo = routeInfoQuery(&routeHandle, &iRecordTotal);
    if (LW_NULL != pRouteInfo) {
        for (i = 0; i < iRecordTotal; i++) {
            pRouteInfoTmp = pRouteInfo + i;

            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {  /*  ÿҳ��ʾ 5 ����Ϣ           */
                cJSON  *pjRoute = cJSON_CreateObject();                 /*  ����һ��ӳ�� Json ����      */
                if (LW_NULL == pjRoute) {
                     break;
                }

                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_DEST, pRouteInfoTmp->cDestination);
                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_GATEWAY, pRouteInfoTmp->cGateway);
                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_GENMASK, pRouteInfoTmp->cGenmask);
                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_FLAGS,  pRouteInfoTmp->cFlags);
                cJSON_AddNumberToObject(pjRoute, WEB_ROUTE_PREFIX, pRouteInfoTmp->iPrefix);
                cJSON_AddNumberToObject(pjRoute, WEB_ROUTE_METRIC, pRouteInfoTmp->iMetric);
                cJSON_AddNumberToObject(pjRoute, WEB_ROUTE_REF, pRouteInfoTmp->iRef);
                cJSON_AddNumberToObject(pjRoute, WEB_ROUTE_USE, pRouteInfoTmp->iUse);

                if (LW_NULL != lib_strstr(pRouteInfoTmp->cIface, "en")) {
                    bzero(cWebName, sizeof(cWebName));
                    iRet = libIntAdaGetWebNetif(pRouteInfoTmp->cIface, cWebName, sizeof(cWebName));
                    if (PX_ERROR == iRet) {
                        routeArrayFree(pRouteInfo);

                        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

                        goto  __error;
                    }

                    cJSON_AddStringToObject(pjRoute, WEB_ROUTE_IFACE, cWebName);
                } else if (LW_NULL != lib_strstr(pRouteInfoTmp->cIface, "vn")) {
                    bzero(cWebName, sizeof(cWebName));

                    iNetIfIndex = atoi(pRouteInfoTmp->cIface + 2);

                    iRet = libGetVndidByNetIfIndex(iNetIfIndex, &iVndid);
                    if (PX_ERROR == iRet) {
                        routeArrayFree(pRouteInfo);

                        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

                        goto  __error;
                    }

                    sprintf(cWebName, "VPN-%d", iVndid);

                    cJSON_AddStringToObject(pjRoute, WEB_ROUTE_IFACE, cWebName);
                } else {
                    cJSON_AddStringToObject(pjRoute, WEB_ROUTE_IFACE, pRouteInfoTmp->cIface);
                }

                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_IP_FAMILY, pRouteInfoTmp->cIpFamily);
                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_OPER_TYPE, pRouteInfoTmp->cOperType);
                cJSON_AddStringToObject(pjRoute, WEB_ROUTE_ROUTE_TYPE, pRouteInfoTmp->cRouteType);

                cJSON_AddItemToArray(pJsonRouteArry, pjRoute);          /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonRouteArry);
    cJSON_AddNumberToObject(pjSend, WEB_ROUTE_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_ROUTE_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    routeArrayFree(pRouteInfo);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
   webBackHandle(wp, pjSend);
   cJSON_Delete(pjSend);
   cJSON_Delete(pRecvjRes);
   cJSON_Delete(pJsonRouteArry);
}
/*********************************************************************************************************
** ��������: __routeConfigAction
** ��������: ��̬·�����ô����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __routeConfigAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *pjDest        = LW_NULL;
    cJSON                *pjGateway     = LW_NULL;
    cJSON                *pjMask        = LW_NULL;
    cJSON                *pjIface       = LW_NULL;
    cJSON                *pjMetric      = LW_NULL;
    cJSON                *pjFamily      = LW_NULL;
    cJSON                *pjOperType    = LW_NULL;
    cJSON                *pjRouteType   = LW_NULL;

    __ROUTE_HANDLE        routeHandle   = {{0}};

    INT                   iRet;

    if (!isUserLogin(wp)) {                                              /*  �û���Ȩ                   */
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

    pjDest      = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_DEST);
    pjGateway   = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_GATEWAY);
    pjMask      = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_GENMASK);
    pjIface     = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_IFACE);
    pjMetric    = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_METRIC);
    pjFamily    = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_IP_FAMILY);
    pjOperType  = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_OPER_TYPE);
    pjRouteType = cJSON_GetObjectItem(pRecvjRes, WEB_ROUTE_ROUTE_TYPE);
    if (NULL == pjDest     || NULL == pjGateway  ||
        NULL == pjMask     || NULL == pjIface    ||
        NULL == pjMetric   || NULL == pjFamily   ||
        NULL == pjOperType || NULL == pjRouteType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    strncpy(routeHandle.cDestination, pjDest->valuestring,    sizeof(routeHandle.cDestination));
    strncpy(routeHandle.cGateway,     pjGateway->valuestring, sizeof(routeHandle.cGateway));
    strncpy(routeHandle.cGenmask,     pjMask->valuestring,    sizeof(routeHandle.cGenmask));

    routeHandle.iMetric = pjMetric->valueint;

    strncpy(routeHandle.cIpFamily,  pjFamily->valuestring,    sizeof(routeHandle.cIpFamily));
    strncpy(routeHandle.cOperType,  pjOperType->valuestring,  sizeof(routeHandle.cOperType));
    strncpy(routeHandle.cRouteType, pjRouteType->valuestring, sizeof(routeHandle.cRouteType));

    strncpy(routeHandle.cIface, pjIface->valuestring, sizeof(routeHandle.cIface));

    iRet = routeConfig(&routeHandle);                                   /*  �����˷�������·������    */
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
** ��������: __routeActionDefine
** ��������: ·�ɺ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __routeActionDefine (VOID)
{
    websDefineAction("routeInfoGetAction", __routeInfoGetAction);
    websDefineAction("routeConfigAction", __routeConfigAction);
}
/*********************************************************************************************************
** ��������: routeActionInit
** ��������: ·�ɺ�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  routeActionInit (VOID)
{
    __routeActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
