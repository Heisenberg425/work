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
** ��   ��   ��: localNetAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 15 ��
**
** ��        ��: OpenRT ���������̨�ӿ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include "lwip/netif.h"
#include "netdev.h"
#include "sys/un.h"
#include <net/lwip/event/lwip_netevent.h>

#include "cJSON.h"
#include "goahead.h"
#include "nat_info.h"
#include "intada_info.h"
#include "interface_info.h"
#include "dhcpserver_info.h"
#include "situation_info.h"
#include "situationCommon.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code ����ֵ
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: ��̬ ip
*********************************************************************************************************/
#define ARRAY_NAME                "data"                                /*  Json�ֶΣ����� DHCP ��Ϣ    */
#define WEB_IPADDR                "ipaddr"                              /*  Json�ֶΣ�IP ��ַ           */
#define WEB_MASK                  "netmask"                             /*  Json�ֶΣ������ַ          */
#define WEB_GATEWAY               "gateway"                             /*  Json�ֶΣ�����              */
#define WEB_NAME                  "webname"                             /*  Json�ֶΣ��û��Զ�������    */
#define EN_NAME                   "enname"                              /*  Json�ֶΣ���������          */
/*********************************************************************************************************
  Jason key: dhcp server
*********************************************************************************************************/
#define WEB_DHCPSERVER_START      "start"                               /*  json�ֶΣ���ַ����ʼ��ַ    */
#define WEB_DHCPSERVER_END        "end"                                 /*  json�ֶΣ���ַ�ؽ�����ַ    */
#define DHCPSERVER_ENABLE         (1)                                   /*  ʹ�� DHCP SERVER            */
#define DHCP_IP_OFFSET_START      (10)                                  /*  DHCP IP ��ʼƫ��ֵ          */
#define DHCP_IP_OFFSET_END        (254)                                 /*  DHCP IP ����ƫ��ֵ          */
/*********************************************************************************************************
  ������������궨��
*********************************************************************************************************/
#define WEB_NET_WEBNAME           "webname"                             /*  Json�ֶΣ�������            */
#define WEB_NET_OPERTYPE          "oper_type"                           /*  Json�ֶΣ���������          */
#define OPER_TYPE_DHCP            (2)                                   /*  �������ͣ�DHCP ����         */
#define OPER_TYPE_STATIC          (3)                                   /*  �������ͣ���̬ IP ����      */
/*********************************************************************************************************
** ��������: __localDhcpIpValueGetAction
** ��������: DHCP server IP ȡֵ��Χ��ֵ��ȡ�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __localDhcpIpValueGetAction (Webs *wp)
{
    cJSON*                pJsDhcpArry         = LW_NULL;
    cJSON*                pjSendRes           = LW_NULL;

    PCHAR                 pIfaceBuf           = LW_NULL;
    PCHAR                 pBuffTmp            = LW_NULL;
    __PNETIF_INFO         pIfaceInfo          = LW_NULL;

    INT                   iNetId              = 0;
    INT                   iIpStart            = 0;
    INT                   iIpEnd              = 0;

    CHAR                  cIpStart[16]        = {0};
    CHAR                  cIpEnd[16]          = {0};

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

    pjSendRes   = cJSON_CreateObject();                                 /*  �������� Json ����          */
    pJsDhcpArry = cJSON_CreateArray();                                  /*  ���� Json DHCP ��Ϣ����     */
    if(LW_NULL == pjSendRes || LW_NULL == pJsDhcpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  ��ȡ�ӿ�״̬
     */
    pIfaceBuf = libAllInterFaceInfoGet(&iNetIfNum);
    pBuffTmp  = pIfaceBuf;
    for (i = 0; i < iNetIfNum; i++) {
        pIfaceInfo = (__PNETIF_INFO)pIfaceBuf;
        if (strstr(pIfaceInfo->cNetifName, "vn")) {
            pIfaceBuf += sizeof(__NETIF_INFO);

            continue;
        }

        cJSON  *pjDhcp = cJSON_CreateObject();                          /*  ������������Ԫ�� Json ����  */
        if(LW_NULL == pjDhcp) {
            cJSON_AddNumberToObject(pjSendRes, WEB_CODE, 500);

             break;
        }

        iNetId   = ntohl(pIfaceInfo->uiIp & pIfaceInfo->uiNetMask);
        iIpStart = iNetId + DHCP_IP_OFFSET_START;
        iIpEnd   = iNetId + DHCP_IP_OFFSET_END;

        iIpStart = htonl(iIpStart);
        iIpEnd   = htonl(iIpEnd);

        /*
         *  ����ַתΪ�ַ�����ʽ
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(iIpStart), cIpStart, sizeof(cIpStart));
        ip4addr_ntoa_r((const ip4_addr_t *)&(iIpEnd),   cIpEnd,   sizeof(cIpEnd));

        /*
         *  Ϊ���������� Json �ṹ
         */
        cJSON_AddStringToObject(pjDhcp, EN_NAME, pIfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjDhcp, WEB_NAME, pIfaceInfo->cWebName);

        cJSON_AddStringToObject(pjDhcp, WEB_DHCPSERVER_START,  cIpStart);
        cJSON_AddStringToObject(pjDhcp, WEB_DHCPSERVER_END,    cIpEnd);


        cJSON_AddItemToArray(pJsDhcpArry, pjDhcp);                      /*  �� DHCP ��Ϣ��������        */

        pIfaceBuf += sizeof(__NETIF_INFO);
    }

    if (NULL != pBuffTmp) {
        free(pBuffTmp);
    }

    cJSON_AddItemToObject(pjSendRes, ARRAY_NAME, pJsDhcpArry);

    cJSON_AddNumberToObject(pjSendRes, WEB_CODE, 200);

    webBackHandle(wp, pjSendRes);

    cJSON_Delete(pjSendRes);
}
/*********************************************************************************************************
** ��������: __localNetSetAction
** ��������: �������ô����̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __localNetSetAction (Webs *wp)
{
    INT                   iRet                        = PX_ERROR;

    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;

    INT                   iOperType                   = -1;
    cJSON                *pjOperType                  = LW_NULL;

    PCHAR                 pcWebNetif                  = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;

    cJSON                *pjIpAddr                    = LW_NULL;
    cJSON                *pjMask                      = LW_NULL;
    cJSON                *pjGateWay                   = LW_NULL;

    cJSON                *pjIpStart                   = LW_NULL;
    cJSON                *pjIpEnd                     = LW_NULL;

    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};
    __DHCPSERVER_HANDLE   dhcpServerHandle;
    __NETIF_SET           netifInfoSet;
    __SITUATION_INFO      situationInfo;

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

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����            */
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
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    if (LW_NULL == pjOperType || LW_NULL == pjWebNetif) {               /*  ��������                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType  = pjOperType->valueint;
    pcWebNetif = pjWebNetif->valuestring;

#if 1
    /*
     * ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }
#else
    strncpy(cNetIfName, pcWebNetif, strlen(pcWebNetif));                /*  ��ʱ��ʹ�� "en"             */
#endif

    switch (iOperType) {
    case OPER_TYPE_DHCP:                                                /*  DHCP                        */
        pjIpStart = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_START);
        pjIpEnd   = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_END);
        if (pjIpStart == LW_NULL || pjIpEnd == LW_NULL) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */

            goto __finish;
        }

        bzero(&dhcpServerHandle, sizeof(dhcpServerHandle));

        dhcpServerHandle.iEnable     = DHCPSERVER_ENABLE;
        dhcpServerHandle.uiIpStart   = inet_addr(pjIpStart->valuestring);
        dhcpServerHandle.uiIpEnd     = inet_addr(pjIpEnd->valuestring);
        dhcpServerHandle.uiIpDns1    = htonl(((ntohl(dhcpServerHandle.uiIpStart) & (0xffffff00)) | 1));
        dhcpServerHandle.uiIpDns2    = inet_addr("8.8.8.8");
        dhcpServerHandle.uiIpMask    = inet_addr("255.255.255.0");
        dhcpServerHandle.uiIpGateway = htonl(((ntohl(dhcpServerHandle.uiIpStart) & (0xffffff00)) | 1));
        dhcpServerHandle.uiLease     = 28800;

        memcpy(dhcpServerHandle.cInterface, cNetIfName, strlen(cNetIfName));

        bzero(&netifInfoSet, sizeof(netifInfoSet));

        strncpy(netifInfoSet.cNetifName,
                cNetIfName,
                sizeof(netifInfoSet.cNetifName));
        strncpy(netifInfoSet.cIpAddr,
                inet_ntoa(*(struct in_addr *)&dhcpServerHandle.uiIpGateway),
                sizeof(netifInfoSet.cIpAddr));
        strncpy(netifInfoSet.cNetMask,
                inet_ntoa(*(struct in_addr *)&dhcpServerHandle.uiIpMask),
                sizeof(netifInfoSet.cNetMask));
        strncpy(netifInfoSet.cGateWay,
                "0.0.0.0",
                sizeof(netifInfoSet.cGateWay));

        iRet = libInterFaceInfoSet(&netifInfoSet);
        if (PX_ERROR == iRet) {                                         /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        } else if (RET_COLLISION == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);

            goto  __finish;
        }

        /*
         * ������������ӿ�
         */
        iRet = dhcpServerConfig(&dhcpServerHandle);
        if (ERROR_NONE != iRet) {                                       /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        break;

    case OPER_TYPE_STATIC:                                              /*  ��̬ ip                     */
        pjIpAddr    = cJSON_GetObjectItem(pRecvjRes, WEB_IPADDR);
        pjMask      = cJSON_GetObjectItem(pRecvjRes, WEB_MASK);
        pjGateWay   = cJSON_GetObjectItem(pRecvjRes, WEB_GATEWAY);
        if (LW_NULL == pjIpAddr ||
            LW_NULL == pjMask   ||
            LW_NULL == pjGateWay) {                                     /*  ��������                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        }

        bzero(&netifInfoSet, sizeof(netifInfoSet));
        strncpy(netifInfoSet.cNetifName, cNetIfName,              sizeof(netifInfoSet.cNetifName));
        strncpy(netifInfoSet.cIpAddr,    pjIpAddr->valuestring,   sizeof(netifInfoSet.cIpAddr));
        strncpy(netifInfoSet.cNetMask,   pjMask->valuestring,     sizeof(netifInfoSet.cNetMask));
        strncpy(netifInfoSet.cGateWay,   pjGateWay->valuestring,  sizeof(netifInfoSet.cGateWay));

        iRet = libInterFaceInfoSet(&netifInfoSet);
        if (PX_ERROR == iRet) {                                         /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        } else if (RET_COLLISION == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);

            goto  __finish;
        }

        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    }

    /*
     *  ���� LAN ��
     */
    operLogInsert(wp, OPEN_RT_NAT, "set lan\n", 0);                     /*  ��¼������־                */

    iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_SET, NAT_FUNC_LAN);/*  ���� WAN LAN �ڲ���         */
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition,      POS_INTRANET_SET, strlen(POS_INTRANET_SET));
    memcpy(situationInfo.cStatus,         STATUS_CONFIGING, strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT,  strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = -1;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __error1;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error1;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    goto  __finish;

__error1:
    libNatWanLanConfig(cNetIfName, NAT_WANLAN_DEL, NAT_FUNC_LAN);

__error:
    if (OPER_TYPE_DHCP == iOperType) {
        dhcpServerDel(&dhcpServerHandle);
    }

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __localNetActionDefine
** ��������: ������̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __localNetActionDefine (VOID)
{
    websDefineAction("localNetSetAction",       __localNetSetAction);
    websDefineAction("localDhcpValueGetAction", __localDhcpIpValueGetAction);
}
/*********************************************************************************************************
** ��������: localNetActionInit
** ��������: ������̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  localNetActionInit (VOID)
{
    __localNetActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
