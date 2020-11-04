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
** 文   件   名: localNetAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述: OpenRT 内网处理后台接口
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
  json code 编码值
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: 静态 ip
*********************************************************************************************************/
#define ARRAY_NAME                "data"                                /*  Json字段：网卡 DHCP 信息    */
#define WEB_IPADDR                "ipaddr"                              /*  Json字段：IP 地址           */
#define WEB_MASK                  "netmask"                             /*  Json字段：掩码地址          */
#define WEB_GATEWAY               "gateway"                             /*  Json字段：网关              */
#define WEB_NAME                  "webname"                             /*  Json字段：用户自定义名称    */
#define EN_NAME                   "enname"                              /*  Json字段：网卡名称          */
/*********************************************************************************************************
  Jason key: dhcp server
*********************************************************************************************************/
#define WEB_DHCPSERVER_START      "start"                               /*  json字段：地址池起始地址    */
#define WEB_DHCPSERVER_END        "end"                                 /*  json字段：地址池结束地址    */
#define DHCPSERVER_ENABLE         (1)                                   /*  使能 DHCP SERVER            */
#define DHCP_IP_OFFSET_START      (10)                                  /*  DHCP IP 起始偏移值          */
#define DHCP_IP_OFFSET_END        (254)                                 /*  DHCP IP 结束偏移值          */
/*********************************************************************************************************
  内网接入操作宏定义
*********************************************************************************************************/
#define WEB_NET_WEBNAME           "webname"                             /*  Json字段：网口名            */
#define WEB_NET_OPERTYPE          "oper_type"                           /*  Json字段：操作类型          */
#define OPER_TYPE_DHCP            (2)                                   /*  操作类型：DHCP 接入         */
#define OPER_TYPE_STATIC          (3)                                   /*  操作类型：静态 IP 接入      */
/*********************************************************************************************************
** 函数名称: __localDhcpIpValueGetAction
** 功能描述: DHCP server IP 取值范围数值获取接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    pjSendRes   = cJSON_CreateObject();                                 /*  创建返回 Json 对象          */
    pJsDhcpArry = cJSON_CreateArray();                                  /*  创建 Json DHCP 信息数组     */
    if(LW_NULL == pjSendRes || LW_NULL == pJsDhcpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  获取接口状态
     */
    pIfaceBuf = libAllInterFaceInfoGet(&iNetIfNum);
    pBuffTmp  = pIfaceBuf;
    for (i = 0; i < iNetIfNum; i++) {
        pIfaceInfo = (__PNETIF_INFO)pIfaceBuf;
        if (strstr(pIfaceInfo->cNetifName, "vn")) {
            pIfaceBuf += sizeof(__NETIF_INFO);

            continue;
        }

        cJSON  *pjDhcp = cJSON_CreateObject();                          /*  创建单个网卡元素 Json 对象  */
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
         *  将地址转为字符串格式
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(iIpStart), cIpStart, sizeof(cIpStart));
        ip4addr_ntoa_r((const ip4_addr_t *)&(iIpEnd),   cIpEnd,   sizeof(cIpEnd));

        /*
         *  为此网卡生成 Json 结构
         */
        cJSON_AddStringToObject(pjDhcp, EN_NAME, pIfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjDhcp, WEB_NAME, pIfaceInfo->cWebName);

        cJSON_AddStringToObject(pjDhcp, WEB_DHCPSERVER_START,  cIpStart);
        cJSON_AddStringToObject(pjDhcp, WEB_DHCPSERVER_END,    cIpEnd);


        cJSON_AddItemToArray(pJsDhcpArry, pjDhcp);                      /*  将 DHCP 信息加入数组        */

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
** 函数名称: __localNetSetAction
** 功能描述: 内网设置处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象            */
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
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_NET_WEBNAME);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_NET_OPERTYPE);
    if (LW_NULL == pjOperType || LW_NULL == pjWebNetif) {               /*  参数错误                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType  = pjOperType->valueint;
    pcWebNetif = pjWebNetif->valuestring;

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
    case OPER_TYPE_DHCP:                                                /*  DHCP                        */
        pjIpStart = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_START);
        pjIpEnd   = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_END);
        if (pjIpStart == LW_NULL || pjIpEnd == LW_NULL) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  参数出错                    */

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
        if (PX_ERROR == iRet) {                                         /*  设置失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        } else if (RET_COLLISION == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);

            goto  __finish;
        }

        /*
         * 调用配置请求接口
         */
        iRet = dhcpServerConfig(&dhcpServerHandle);
        if (ERROR_NONE != iRet) {                                       /*  设置失败                    */
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __finish;
        }

        break;

    case OPER_TYPE_STATIC:                                              /*  静态 ip                     */
        pjIpAddr    = cJSON_GetObjectItem(pRecvjRes, WEB_IPADDR);
        pjMask      = cJSON_GetObjectItem(pRecvjRes, WEB_MASK);
        pjGateWay   = cJSON_GetObjectItem(pRecvjRes, WEB_GATEWAY);
        if (LW_NULL == pjIpAddr ||
            LW_NULL == pjMask   ||
            LW_NULL == pjGateWay) {                                     /*  参数错误                    */
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

        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __finish;
    }

    /*
     *  设置 LAN 口
     */
    operLogInsert(wp, OPEN_RT_NAT, "set lan\n", 0);                     /*  记录操作日志                */

    iRet = libNatWanLanConfig(cNetIfName, NAT_WANLAN_SET, NAT_FUNC_LAN);/*  进行 WAN LAN 口操作         */
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
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
** 函数名称: __localNetActionDefine
** 功能描述: 内网后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __localNetActionDefine (VOID)
{
    websDefineAction("localNetSetAction",       __localNetSetAction);
    websDefineAction("localDhcpValueGetAction", __localDhcpIpValueGetAction);
}
/*********************************************************************************************************
** 函数名称: localNetActionInit
** 功能描述: 内网后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  localNetActionInit (VOID)
{
    __localNetActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
