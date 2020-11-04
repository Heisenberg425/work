/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: publicNetAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述: OpenRT 外网处理后台接口
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
  json code 编码值
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: udial
*********************************************************************************************************/
#define WEB_NET_WEBNAME           "webname"                             /*  Json字段：网口名            */
#define WEB_UDIAL_ACCOUNT         "account"                             /*  Json字段：账号              */
#define WEB_UDIAL_PASSWD          "passwd"                              /*  Json字段：密码              */
/*********************************************************************************************************
  Jason key: 静态 ip
*********************************************************************************************************/
#define WEB_IPADDR                "ipaddr"                              /*  Json字段：IP 地址           */
#define WEB_MASK                  "netmask"                             /*  Json字段：掩码地址          */
#define WEB_GATEWAY               "gateway"                             /*  Json字段：网关              */
/*********************************************************************************************************
  外网接入操作宏定义
*********************************************************************************************************/
#define WEB_NET_OPERTYPE          "oper_type"                           /*  Json字段：操作类型          */
#define OPER_TYPE_UDIAL           (1)                                   /*  操作类型：拨号接入          */
#define OPER_TYPE_DHCP            (2)                                   /*  操作类型：DHCP 接入         */
#define OPER_TYPE_STATIC          (3)                                   /*  操作类型：静态 IP 接入      */
/*********************************************************************************************************
  OpenRT 可选功能宏定义
*********************************************************************************************************/
#define WEB_KIDVPN_CLIENT         "kidvpn_client"                       /*  可选功能：KidVPN 客户端     */
#define WEB_KIDVPN_SERVER         "kidvpn_server"                       /*  可选功能：KidVPN 服务端     */
#define WEB_ARP_DEFENSE           "arp_defense"                         /*  可选功能：ARP 防御          */
#define FUNC_KIDVPN_CLIENT        (1 << 0)                              /*  可选功能：KidVPN 客户端     */
#define FUNC_KIDVPN_SERVER        (1 << 1)                              /*  可选功能：KidVPN 服务端     */
#define FUNC_ARP_DEFENSE          (1 << 2)                              /*  可选功能：ARP 防御          */
/*********************************************************************************************************
** 函数名称: __publicNetSetAction
** 功能描述: 外网设置处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    /*
     *  获取 Json 数据内容
     */
    pjWebNetif   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    pjKidVpnCli  = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);
    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);
    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);
    if (LW_NULL == pjOperType   ||                                      /*  参数错误                    */
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
     * 获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }
#else
    strncpy(cNetIfName, pcWebNetif, strlen(pcWebNetif));                /*  暂时仍使用 "en"             */
#endif

    snprintf(cRouteCmd, sizeof(cRouteCmd), "route add default dev %s", cNetIfName);
    bzero(&wanPageInfo,   sizeof(wanPageInfo));

    switch (iOperType) {
    case OPER_TYPE_UDIAL:                                               /*  拨号接入外网                */
        pjAccount  = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ACCOUNT);
        pjPassWd   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_PASSWD);
        if (LW_NULL == pjAccount || LW_NULL == pjPassWd) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        }

        iRet = libUdialStartHandle(pjAccount->valuestring, pjPassWd->valuestring, cNetIfName);
        if (1 == iRet) {                                                /*  拨号超过次数                */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);

            goto  __finish;
        } else if (PX_ERROR == iRet) {                                  /*  设置失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        wanPageInfo.ucMode = OPER_TYPE_UDIAL;
        memcpy(wanPageInfo.cNetifName, cNetIfName, strlen(cNetIfName));
        memcpy(wanPageInfo.cPppoeUsr, pjAccount->valuestring, strlen(pjAccount->valuestring));
        memcpy(wanPageInfo.cPppoePwd, pjPassWd->valuestring, strlen(pjPassWd->valuestring));

        break;

    case OPER_TYPE_DHCP:                                                /*  DHCP 接入外网               */
        iRet = libDhcpClientHandle(cNetIfName, DHCP_CLIENT_ENABLE);
        if (PX_ERROR == iRet) {                                         /*  操作失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        wanPageInfo.ucMode = OPER_TYPE_DHCP;
        memcpy(wanPageInfo.cNetifName, cNetIfName, strlen(cNetIfName));

        break;

    case OPER_TYPE_STATIC:                                              /*  静态 ip 接入外网            */
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
        if (PX_ERROR == iRet) {                                         /*  设置失败                    */
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
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __finish;
    }

    if (OPER_TYPE_DHCP == iOperType || OPER_TYPE_STATIC == iOperType) {

        /*
         *  进行 WAN LAN 口操作
         */
        iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_SET, NAT_FUNC_WAN);
        if (PX_ERROR == iRet) {                                         /*  设置失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }
    }

    /*
     *  设置该 WAN 口为默认路由
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
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

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
** 函数名称: __publicNetCancelAction
** 功能描述: 外网设置取消处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    /*
     *  获取 Json 数据内容
     */
    pjWebNetif   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType   = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    pjKidVpnCli  = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);
    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);
    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);
    if (LW_NULL == pjOperType   ||                                      /*  参数错误                    */
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
     * 获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }
#else
    strncpy(cNetIfName, pcWebNetif, strlen(pcWebNetif));                /*  暂时仍使用 "en"             */
#endif

    switch (iOperType) {
    case OPER_TYPE_UDIAL:                                               /*  拨号接入外网                */
        iRet = libUdialDelHandle(cNetIfName);
        if (PX_ERROR == iRet) {                                         /*  取消拨号失败                */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
            goto  __finish;
        }

        break;

    case OPER_TYPE_DHCP:                                                /*  DHCP 接入外网               */
        iRet = libDhcpClientHandle(cNetIfName, DHCP_CLIENT_DISABLE);
        if (PX_ERROR == iRet) {                                         /*  操作失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        break;

    case OPER_TYPE_STATIC:                                              /*  静态 ip 接入外网            */
        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __finish;
    }

    iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_DEL, NAT_FUNC_WAN);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
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
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __publicNetActionDefine
** 功能描述: 外网后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __publicNetActionDefine (VOID)
{
    websDefineAction("publicNetSetAction",    __publicNetSetAction);
    websDefineAction("publicNetCancelAction", __publicNetCancelAction);
}

/*********************************************************************************************************
** 函数名称: publicNetActionInit
** 功能描述: 外网后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  publicNetActionInit (VOID)
{
    __publicNetActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
