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
** ��   ��   ��: interfaceAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 4 ��
**
** ��        ��: OpenRT ������Ϣ��ȡ��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "../webHandle.h"
#include "lwip/netif.h"
#include "intada_info.h"
#include "interface_info.h"
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define JSON_ARGERR_CODE          (100)                                 /*  �������󷵻�Json codeֵ     */
#define JSON_SUCCESS_CODE         (200)                                 /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE          (500)                                 /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  __interfaceMonitorAction: json�ֶ�
*********************************************************************************************************/
#define ARRAY_NAME                "interfaces"                          /*  Json�ֶΣ�����������        */
#define WEB_INDEX                 "webindex"                            /*  Json�ֶΣ�ǰ����������      */
#define WEB_NAME                  "webname"                             /*  Json�ֶΣ��û��Զ�������    */
#define WAN_COUNT                 "wancount"                            /*  Json�ֶΣ�WAN �ڸ���        */
#define LAN_COUNT                 "lancount"                            /*  Json�ֶΣ�LAN �ڸ���        */
#define EN_NAME                   "enname"                              /*  Json�ֶΣ���������          */
#define DEV_NAME                  "devname"                             /*  Json�ֶΣ��豸����          */
#define LINK_STATUS               "linkstatus"                          /*  Json�ֶΣ�����״̬          */
#define FUNCTION                  "function"                            /*  Json�ֶΣ���������          */
#define TYPE                      "type"                                /*  Json�ֶΣ���������          */
#define IPADDR                    "ip"                                  /*  Json�ֶΣ�ip��ַ            */
#define NETMASK                   "netmask"                             /*  Json�ֶΣ������ַ          */
#define MACADDR                   "mac"                                 /*  Json�ֶΣ�mac��ַ           */
#define SPEED                     "speed"                               /*  Json�ֶΣ�����              */
#define LINK_ON                   "on"                                  /*  Jsonֵ�����ӳɹ�            */
#define LINK_OFF                  "off"                                 /*  Jsonֵ������ʧ��            */
/*********************************************************************************************************
  __interfaceInfoSetAction: json�ֶ�
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  Json�ֶΣ�code              */
#define WEB_IPADDR                "ipaddr"                              /*  Json�ֶΣ�IP ��ַ           */
#define WEB_MASK                  "netmask"                             /*  Json�ֶΣ������ַ          */
#define WEB_GATEWAY               "gateway"                             /*  Json�ֶΣ�����              */
/*********************************************************************************************************
** ��������: __interfaceInfoSetAction
** ��������: ����������Ϣ Action
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __interfaceInfoSetAction (Webs *wp)
{
    cJSON                *pRecvjRes   = LW_NULL;
    cJSON                *pSendjRes   = LW_NULL;

    INT                   iRet;

    PCHAR                 pcNetifName = LW_NULL;
    PCHAR                 pcIpAddr    = LW_NULL;
    PCHAR                 pcMask      = LW_NULL;
    PCHAR                 pcGateWay   = LW_NULL;

    cJSON                *pjIpAddr    = LW_NULL;
    cJSON                *pjMask      = LW_NULL;
    cJSON                *pjGateWay   = LW_NULL;
    cJSON                *pjNetifName = LW_NULL;

    __NETIF_SET           netifInfoSet;

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
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, JSON_FAILED_CODE);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjIpAddr    = cJSON_GetObjectItem(pRecvjRes, WEB_IPADDR);
    pjMask      = cJSON_GetObjectItem(pRecvjRes, WEB_MASK);
    pjGateWay   = cJSON_GetObjectItem(pRecvjRes, WEB_GATEWAY);
    pjNetifName = cJSON_GetObjectItem(pRecvjRes, EN_NAME);
    if (LW_NULL == pjIpAddr  ||
        LW_NULL == pjMask    ||
        LW_NULL == pjGateWay ||
        LW_NULL == pjNetifName) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcNetifName = pjNetifName->valuestring;
    pcIpAddr    = pjIpAddr->valuestring;
    pcMask      = pjMask->valuestring;
    pcGateWay   = pjGateWay->valuestring;

    bzero(&netifInfoSet, sizeof(netifInfoSet));
    strncpy(netifInfoSet.cNetifName, pcNetifName, sizeof(netifInfoSet.cNetifName));
    strncpy(netifInfoSet.cIpAddr,    pcIpAddr,    sizeof(netifInfoSet.cIpAddr));
    strncpy(netifInfoSet.cNetMask,   pcMask,      sizeof(netifInfoSet.cNetMask));
    strncpy(netifInfoSet.cGateWay,   pcGateWay,   sizeof(netifInfoSet.cGateWay));

    iRet = libInterFaceInfoSet(&netifInfoSet);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else if (RET_COLLISION == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __interfaceMonitorAction
** ��������: ��ȡ����״̬Action(��������������)
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __interfaceMonitorAction (Webs *wp)
{
    cJSON*                pJsonInterFaceArry  = LW_NULL;
    cJSON*                pjRes               = LW_NULL;

    PCHAR                 pNetInterfaceBuff   = LW_NULL;
    PCHAR                 pBuffTmp            = LW_NULL;
    __PNETIF_INFO         pNetInterfaceInfo   = LW_NULL;

    CHAR                  cIpAddr[16]         = "null";
    CHAR                  cNetMask[16]        = "null";
    CHAR                  cGateWay[16]        = "null";
    CHAR                  cMacAddr[20]        = "null";

    INT                   iIntAdaRet;
    INT                   iNetIfNum;
    int                   i;

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

    pjRes              = cJSON_CreateObject();                          /*  �������� Json ����          */
    pJsonInterFaceArry = cJSON_CreateArray();                           /*  ���� Json �����ӿ�����      */
    if(LW_NULL == pjRes || LW_NULL == pJsonInterFaceArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  �ж��Ƿ��������Ӧ��λ
     */
    iIntAdaRet = libIntAdaCheck();
    if (ERROR_NONE == iIntAdaRet) {
        cJSON_AddNumberToObject(pjRes, WEB_CODE, 200);                  /*  ��ɶ�λ����                */
    } else {
        cJSON_AddNumberToObject(pjRes, WEB_CODE, 500);                  /*  û��ɶ�λ����              */
    }

    /*
     *  WAN LAN �ھ���һ���ǿյ�
     */
    cJSON_AddNumberToObject(pjRes, WAN_COUNT, LW_CFG_NET_NAT_MAX_AP_IF - 1);
    cJSON_AddNumberToObject(pjRes, LAN_COUNT, LW_CFG_NET_NAT_MAX_LOCAL_IF - 1);

    /*
     *  ��ȡ�ӿ�״̬
     */
    pNetInterfaceBuff = libAllInterFaceInfoGet(&iNetIfNum);
    pBuffTmp          = pNetInterfaceBuff;
    for (i = 0; i < iNetIfNum; i++) {
        pNetInterfaceInfo = (__PNETIF_INFO)pNetInterfaceBuff;
        if (strstr(pNetInterfaceInfo->cNetifName, "vn")) {
            pNetInterfaceBuff += sizeof(__NETIF_INFO);
            iNetIfNum--;
            i--;
            continue;
        }

        cJSON*  pjInterFace = cJSON_CreateObject();                     /*  ������������Ԫ�� Json ����  */
        if(LW_NULL == pjInterFace) {
            cJSON_AddNumberToObject(pjRes, "code", 500);
             break;
        }

        sprintf(cMacAddr,
                "%02x:%02x:%02x:%02x:%02x:%02x",
                pNetInterfaceInfo->ucMac[0],
                pNetInterfaceInfo->ucMac[1],
                pNetInterfaceInfo->ucMac[2],
                pNetInterfaceInfo->ucMac[3],
                pNetInterfaceInfo->ucMac[4],
                pNetInterfaceInfo->ucMac[5]);

        /*
         *  ����ַתΪ�ַ�����ʽ
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiIp),      cIpAddr,  sizeof(cIpAddr));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiNetMask), cNetMask, sizeof(cNetMask));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiGateWay), cGateWay, sizeof(cGateWay));

        /*
         *  Ϊ���������� Json �ṹ
         */
        cJSON_AddStringToObject(pjInterFace, EN_NAME, pNetInterfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjInterFace, WEB_NAME, pNetInterfaceInfo->cWebName);
        cJSON_AddStringToObject(pjInterFace, DEV_NAME, pNetInterfaceInfo->cDevName);
        cJSON_AddStringToObject(pjInterFace,
                                LINK_STATUS,
                                (pNetInterfaceInfo->iLinkFlag & IFF_RUNNING) ? LINK_ON : LINK_OFF);

        /*
         *  �ж� NAT ����
         */
        switch(pNetInterfaceInfo->cNatFunction) {
        case NETIF_WAN:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "wan");
            break;

        case NETIF_LAN:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "lan");
            break;

        case NETIF_NORMAL:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "normal");
            break;

        default:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "error");
            break;
        }

        /*
         *  �ж���·����
         */
        switch(pNetInterfaceInfo->cNetType) {
        case NETIF_ADSL:
            cJSON_AddStringToObject(pjInterFace, TYPE, "adsl");
            break;

        case NETIF_DHCP:
            cJSON_AddStringToObject(pjInterFace, TYPE, "dhcp");
            break;

        case NETIF_STATIC:
            cJSON_AddStringToObject(pjInterFace, TYPE, "static");
            break;

        case NETIF_NORMAL:
            cJSON_AddStringToObject(pjInterFace, TYPE, "normal");
            break;

        default:
            cJSON_AddStringToObject(pjInterFace, TYPE, "error");
            break;
        }

        cJSON_AddStringToObject(pjInterFace, IPADDR,      cIpAddr);
        cJSON_AddStringToObject(pjInterFace, NETMASK,     cNetMask);
        cJSON_AddStringToObject(pjInterFace, WEB_GATEWAY, cGateWay);
        cJSON_AddStringToObject(pjInterFace, MACADDR,     cMacAddr);
        cJSON_AddNumberToObject(pjInterFace, SPEED,       pNetInterfaceInfo->uiSpeed);
        cJSON_AddNumberToObject(pjInterFace, WEB_INDEX,   pNetInterfaceInfo->iWebIndex);

        cJSON_AddItemToArray(pJsonInterFaceArry, pjInterFace);          /*  ����������������            */

        pNetInterfaceBuff += sizeof(__NETIF_INFO);
    }

    if (NULL != pBuffTmp) {
        free(pBuffTmp);
    }

    cJSON_AddItemToObject(pjRes, ARRAY_NAME, pJsonInterFaceArry);

    webBackHandle(wp, pjRes);
    cJSON_Delete(pjRes);
}
/*********************************************************************************************************
** ��������: __vndinterfaceGetAction
** ��������: ��ȡ����״̬Action(������������)
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __vndinterfaceGetAction (Webs *wp)
{
    cJSON*                pJsonInterFaceArry  = LW_NULL;
    cJSON*                pjRes               = LW_NULL;

    PCHAR                 pNetInterfaceBuff   = LW_NULL;
    PCHAR                 pBuffTmp            = LW_NULL;
    __PNETIF_INFO         pNetInterfaceInfo   = LW_NULL;

    CHAR                  cIpAddr[16]         = "null";
    CHAR                  cNetMask[16]        = "null";
    CHAR                  cGateWay[16]        = "null";
    CHAR                  cMacAddr[20]        = "null";

    INT                   iIntAdaRet;
    INT                   iNetIfNum;
    int                   i;

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

    pjRes              = cJSON_CreateObject();                          /*  �������� Json ����          */
    pJsonInterFaceArry = cJSON_CreateArray();                           /*  ���� Json �����ӿ�����      */
    if(LW_NULL == pjRes || LW_NULL == pJsonInterFaceArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  �ж��Ƿ��������Ӧ��λ
     */
    iIntAdaRet = libIntAdaCheck();
    if (ERROR_NONE == iIntAdaRet) {
        cJSON_AddNumberToObject(pjRes, "code", 200);                    /*  ��ɶ�λ����                */
    } else {
        cJSON_AddNumberToObject(pjRes, "code", 500);                    /*  û��ɶ�λ����              */
    }

    /*
     *  WAN LAN �ھ���һ���ǿյ�
     */
    cJSON_AddNumberToObject(pjRes, WAN_COUNT, LW_CFG_NET_NAT_MAX_AP_IF - 1);
    cJSON_AddNumberToObject(pjRes, LAN_COUNT, LW_CFG_NET_NAT_MAX_LOCAL_IF - 1);

    /*
     *  ��ȡ�ӿ�״̬
     */
    pNetInterfaceBuff = libAllInterFaceInfoGet(&iNetIfNum);
    pBuffTmp          = pNetInterfaceBuff;
    for (i = 0; i < iNetIfNum; i++) {
        pNetInterfaceInfo = (__PNETIF_INFO)pNetInterfaceBuff;

        cJSON*  pjInterFace = cJSON_CreateObject();                     /*  ������������Ԫ�� Json ����  */
        if(LW_NULL == pjInterFace) {
            cJSON_AddNumberToObject(pjRes, "code", 500);
             break;
        }

        sprintf(cMacAddr,
                "%02x:%02x:%02x:%02x:%02x:%02x",
                pNetInterfaceInfo->ucMac[0],
                pNetInterfaceInfo->ucMac[1],
                pNetInterfaceInfo->ucMac[2],
                pNetInterfaceInfo->ucMac[3],
                pNetInterfaceInfo->ucMac[4],
                pNetInterfaceInfo->ucMac[5]);

        /*
         *  ����ַתΪ�ַ�����ʽ
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiIp),      cIpAddr,  sizeof(cIpAddr));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiNetMask), cNetMask, sizeof(cNetMask));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiGateWay), cGateWay, sizeof(cGateWay));

        /*
         *  Ϊ���������� Json �ṹ
         */
        cJSON_AddStringToObject(pjInterFace, EN_NAME, pNetInterfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjInterFace, WEB_NAME, pNetInterfaceInfo->cWebName);
        cJSON_AddStringToObject(pjInterFace, DEV_NAME, pNetInterfaceInfo->cDevName);
        cJSON_AddStringToObject(pjInterFace,
                                LINK_STATUS,
                                (pNetInterfaceInfo->iLinkFlag & IFF_RUNNING) ? LINK_ON : LINK_OFF);

        /*
         *  �ж� NAT ����
         */
        switch(pNetInterfaceInfo->cNatFunction) {
        case NETIF_WAN:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "wan");
            break;

        case NETIF_LAN:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "lan");
            break;

        case NETIF_NORMAL:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "normal");
            break;

        default:
            cJSON_AddStringToObject(pjInterFace, FUNCTION, "error");
            break;
        }

        /*
         *  �ж���·����
         */
        switch(pNetInterfaceInfo->cNetType) {
        case NETIF_ADSL:
            cJSON_AddStringToObject(pjInterFace, TYPE, "adsl");
            break;

        case NETIF_DHCP:
            cJSON_AddStringToObject(pjInterFace, TYPE, "dhcp");
            break;

        case NETIF_STATIC:
            cJSON_AddStringToObject(pjInterFace, TYPE, "static");
            break;

        case NETIF_NORMAL:
            cJSON_AddStringToObject(pjInterFace, TYPE, "normal");
            break;

        default:
            cJSON_AddStringToObject(pjInterFace, TYPE, "error");
            break;
        }

        cJSON_AddStringToObject(pjInterFace, IPADDR,      cIpAddr);
        cJSON_AddStringToObject(pjInterFace, NETMASK,     cNetMask);
        cJSON_AddStringToObject(pjInterFace, WEB_GATEWAY, cGateWay);
        cJSON_AddStringToObject(pjInterFace, MACADDR,     cMacAddr);
        cJSON_AddNumberToObject(pjInterFace, SPEED,       pNetInterfaceInfo->uiSpeed);
        cJSON_AddNumberToObject(pjInterFace, WEB_INDEX,   pNetInterfaceInfo->iWebIndex);

        cJSON_AddItemToArray(pJsonInterFaceArry, pjInterFace);          /*  ����������������            */

        pNetInterfaceBuff += sizeof(__NETIF_INFO);
    }

    if (NULL != pBuffTmp) {
        free(pBuffTmp);
    }

    cJSON_AddItemToObject(pjRes, ARRAY_NAME, pJsonInterFaceArry);

    webBackHandle(wp, pjRes);
    cJSON_Delete(pjRes);
}

/*********************************************************************************************************
** ��������: interfaceActionDefine
** ��������: ���ڶ�λ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID __interfaceActionDefine (VOID)
{
    websDefineAction("interfaceMonitorAction", __interfaceMonitorAction);
    websDefineAction("vndinterfaceGetAction",  __vndinterfaceGetAction);
    websDefineAction("interfaceInfoSetAction", __interfaceInfoSetAction);
}

/*********************************************************************************************************
** ��������: interfaceActionDefine
** ��������: ���ڶ�λ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  interfaceActionInit (VOID)
{
    __interfaceActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
