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
** ��   ��   ��: publicNetAction.c
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
#include "dhcp_info.h"
#include "udial_info.h"
#include "intada_info.h"
#include "interface_info.h"
#include "situation_info.h"
#include "situationCommon.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code ����ֵ
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: udial
*********************************************************************************************************/
#define WEB_NET_WEBNAME           "webname"                             /*  Json�ֶΣ�������            */
#define WEB_UDIAL_ACCOUNT         "account"                             /*  Json�ֶΣ��˺�              */
#define WEB_UDIAL_PASSWD          "passwd"                              /*  Json�ֶΣ�����              */
/*********************************************************************************************************
  Jason key: ��̬ ip
*********************************************************************************************************/
#define WEB_IPADDR                "ipaddr"                              /*  Json�ֶΣ�IP ��ַ           */
#define WEB_MASK                  "netmask"                             /*  Json�ֶΣ������ַ          */
#define WEB_GATEWAY               "gateway"                             /*  Json�ֶΣ�����              */
/*********************************************************************************************************
  ������������궨��
*********************************************************************************************************/
#define WEB_NET_OPERTYPE          "oper_type"                           /*  Json�ֶΣ���������          */
#define OPER_TYPE_UDIAL           (1)                                   /*  �������ͣ����Ž���          */
#define OPER_TYPE_DHCP            (2)                                   /*  �������ͣ�DHCP ����         */
#define OPER_TYPE_STATIC          (3)                                   /*  �������ͣ���̬ IP ����      */
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
** ��������: __publicNetSetAction
** ��������: �������ô����̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __publicNetSetAction (Webs *wp)
{
    INT                   iRet                        = PX_ERROR;

    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};

    INT                   iOperType                   = -1;
    cJSON                *pjOperType                  = LW_NULL;

    INT                   iKidVpnCli                  = 0;
    INT                   iKidVpnServ                 = 0;
    INT                   iArpDefense                 = 0;
    INT                   iFunc                       = 0;
    cJSON                *pjKidVpnCli                 = LW_NULL;
    cJSON                *pjKidVpnServ                = LW_NULL;
    cJSON                *pjArpDefense                = LW_NULL;

    PCHAR                 pcWebNetif                  = LW_NULL;
    cJSON                *pjAccount                   = LW_NULL;
    cJSON                *pjPassWd                    = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;

    cJSON                *pjIpAddr                    = LW_NULL;
    cJSON                *pjMask                      = LW_NULL;
    cJSON                *pjGateWay                   = LW_NULL;

    CHAR                  cRouteCmd[128]              = {0};

    __NETIF_SET           netifInfoSet;
    __SITUATION_INFO      situationInfo;
    __WANPAGE_INFO        wanPageInfo;

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
    pjWebNetif   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    pjKidVpnCli  = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);
    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);
    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);
    if (LW_NULL == pjOperType   ||                                      /*  ��������                    */
        LW_NULL == pjWebNetif   ||
        LW_NULL == pjKidVpnCli  ||
        LW_NULL == pjKidVpnServ ||
        LW_NULL == pjArpDefense) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType   = pjOperType->valueint;
    pcWebNetif  = pjWebNetif->valuestring;
    iKidVpnCli  = pjKidVpnCli->valueint;
    iKidVpnServ = pjKidVpnServ->valueint;
    iArpDefense = pjArpDefense->valueint;

    iFunc = iKidVpnCli | (iKidVpnServ << 1) | (iArpDefense << 2);

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

    snprintf(cRouteCmd, sizeof(cRouteCmd), "route add default dev %s", cNetIfName);
    bzero(&wanPageInfo,   sizeof(wanPageInfo));

    switch (iOperType) {
    case OPER_TYPE_UDIAL:                                               /*  ���Ž�������                */
        pjAccount  = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ACCOUNT);
        pjPassWd   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_PASSWD);
        if (LW_NULL == pjAccount || LW_NULL == pjPassWd) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        }

        iRet = libUdialStartHandle(pjAccount->valuestring, pjPassWd->valuestring, cNetIfName);
        if (1 == iRet) {                                                /*  ���ų�������                */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);

            goto  __finish;
        } else if (PX_ERROR == iRet) {                                  /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        wanPageInfo.ucMode = OPER_TYPE_UDIAL;
        memcpy(wanPageInfo.cNetifName, cNetIfName, strlen(cNetIfName));
        memcpy(wanPageInfo.cPppoeUsr, pjAccount->valuestring, strlen(pjAccount->valuestring));
        memcpy(wanPageInfo.cPppoePwd, pjPassWd->valuestring, strlen(pjPassWd->valuestring));

        break;

    case OPER_TYPE_DHCP:                                                /*  DHCP ��������               */
        iRet = libDhcpClientHandle(cNetIfName, DHCP_CLIENT_ENABLE);
        if (PX_ERROR == iRet) {                                         /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        wanPageInfo.ucMode = OPER_TYPE_DHCP;
        memcpy(wanPageInfo.cNetifName, cNetIfName, strlen(cNetIfName));

        break;

    case OPER_TYPE_STATIC:                                              /*  ��̬ ip ��������            */
        pjIpAddr    = cJSON_GetObjectItem(pRecvjRes, WEB_IPADDR);
        pjMask      = cJSON_GetObjectItem(pRecvjRes, WEB_MASK);
        pjGateWay   = cJSON_GetObjectItem(pRecvjRes, WEB_GATEWAY);
        if (LW_NULL == pjIpAddr || LW_NULL == pjMask || LW_NULL == pjGateWay) {
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

        wanPageInfo.ucMode = OPER_TYPE_STATIC;
        memcpy(wanPageInfo.cNetifName, cNetIfName, strlen(cNetIfName));
        memcpy(wanPageInfo.cIpAddr,  pjIpAddr->valuestring, strlen(pjIpAddr->valuestring));
        memcpy(wanPageInfo.cNetMask, pjMask->valuestring, strlen(pjMask->valuestring));
        memcpy(wanPageInfo.cGateWay, pjGateWay->valuestring, strlen(pjGateWay->valuestring));

        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    }

    if (OPER_TYPE_DHCP == iOperType || OPER_TYPE_STATIC == iOperType) {

        /*
         *  ���� WAN LAN �ڲ���
         */
        iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_SET, NAT_FUNC_WAN);
        if (PX_ERROR == iRet) {                                         /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }
    }

    /*
     *  ���ø� WAN ��ΪĬ��·��
     */
    system(cRouteCmd);

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition,      POS_CONNECT_INTERNET, strlen(POS_CONNECT_INTERNET));
    memcpy(situationInfo.cStatus,         STATUS_CONFIGING,     strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT,      strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = iFunc;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __finish;
    }

    iRet = libWanInfoSave(&wanPageInfo);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __publicNetCancelAction
** ��������: ��������ȡ�������̨�ӿ�
** �䡡��  : wp        ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __publicNetCancelAction (Webs *wp)
{
    INT                   iRet                        = PX_ERROR;

    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};

    INT                   iOperType                   = -1;
    cJSON                *pjOperType                  = LW_NULL;

    INT                   iKidVpnCli                  = 0;
    INT                   iKidVpnServ                 = 0;
    INT                   iArpDefense                 = 0;
    INT                   iFunc                       = 0;
    cJSON                *pjKidVpnCli                 = LW_NULL;
    cJSON                *pjKidVpnServ                = LW_NULL;
    cJSON                *pjArpDefense                = LW_NULL;

    PCHAR                 pcWebNetif                  = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;

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

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjWebNetif   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    pjKidVpnCli  = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);
    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);
    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);
    if (LW_NULL == pjOperType   ||                                      /*  ��������                    */
        LW_NULL == pjWebNetif   ||
        LW_NULL == pjKidVpnCli  ||
        LW_NULL == pjKidVpnServ ||
        LW_NULL == pjArpDefense) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType   = pjOperType->valueint;
    pcWebNetif  = pjWebNetif->valuestring;
    iKidVpnCli  = pjKidVpnCli->valueint;
    iKidVpnServ = pjKidVpnServ->valueint;
    iArpDefense = pjArpDefense->valueint;

    iFunc = iKidVpnCli | (iKidVpnServ << 1) | (iArpDefense << 2);

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
    case OPER_TYPE_UDIAL:                                               /*  ���Ž�������                */
        iRet = libUdialDelHandle(cNetIfName);
        if (PX_ERROR == iRet) {                                         /*  ȡ������ʧ��                */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
            goto  __finish;
        }

        break;

    case OPER_TYPE_DHCP:                                                /*  DHCP ��������               */
        iRet = libDhcpClientHandle(cNetIfName, DHCP_CLIENT_DISABLE);
        if (PX_ERROR == iRet) {                                         /*  ����ʧ��                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        break;

    case OPER_TYPE_STATIC:                                              /*  ��̬ ip ��������            */
        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    }

    iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_DEL, NAT_FUNC_WAN);
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition, POS_CONNECT_SUBMIT, strlen(POS_CONNECT_SUBMIT));
    memcpy(situationInfo.cStatus,    STATUS_CONFIGING, strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = iFunc;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __publicNetActionDefine
** ��������: ������̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __publicNetActionDefine (VOID)
{
    websDefineAction("publicNetSetAction",    __publicNetSetAction);
    websDefineAction("publicNetCancelAction", __publicNetCancelAction);
}

/*********************************************************************************************************
** ��������: publicNetActionInit
** ��������: ������̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  publicNetActionInit (VOID)
{
    __publicNetActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
