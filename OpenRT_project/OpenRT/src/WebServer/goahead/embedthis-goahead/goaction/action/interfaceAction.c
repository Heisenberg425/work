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
** 文   件   名: interfaceAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 4 日
**
** 描        述: OpenRT 网口信息获取后台接口
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
  json code编码值
*********************************************************************************************************/
#define JSON_ARGERR_CODE          (100)                                 /*  参数错误返回Json code值     */
#define JSON_SUCCESS_CODE         (200)                                 /*  成功返回Json code值         */
#define JSON_FAILED_CODE          (500)                                 /*  失败返回Json code值         */
/*********************************************************************************************************
  __interfaceMonitorAction: json字段
*********************************************************************************************************/
#define ARRAY_NAME                "interfaces"                          /*  Json字段：网卡数组名        */
#define WEB_INDEX                 "webindex"                            /*  Json字段：前端网口索引      */
#define WEB_NAME                  "webname"                             /*  Json字段：用户自定义名称    */
#define WAN_COUNT                 "wancount"                            /*  Json字段：WAN 口个数        */
#define LAN_COUNT                 "lancount"                            /*  Json字段：LAN 口个数        */
#define EN_NAME                   "enname"                              /*  Json字段：网卡名称          */
#define DEV_NAME                  "devname"                             /*  Json字段：设备名称          */
#define LINK_STATUS               "linkstatus"                          /*  Json字段：连接状态          */
#define FUNCTION                  "function"                            /*  Json字段：功能名称          */
#define TYPE                      "type"                                /*  Json字段：类型名称          */
#define IPADDR                    "ip"                                  /*  Json字段：ip地址            */
#define NETMASK                   "netmask"                             /*  Json字段：掩码地址          */
#define MACADDR                   "mac"                                 /*  Json字段：mac地址           */
#define SPEED                     "speed"                               /*  Json字段：网速              */
#define LINK_ON                   "on"                                  /*  Json值：连接成功            */
#define LINK_OFF                  "off"                                 /*  Json值：连接失败            */
/*********************************************************************************************************
  __interfaceInfoSetAction: json字段
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  Json字段：code              */
#define WEB_IPADDR                "ipaddr"                              /*  Json字段：IP 地址           */
#define WEB_MASK                  "netmask"                             /*  Json字段：掩码地址          */
#define WEB_GATEWAY               "gateway"                             /*  Json字段：网关              */
/*********************************************************************************************************
** 函数名称: __interfaceInfoSetAction
** 功能描述: 设置网卡信息 Action
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, JSON_FAILED_CODE);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
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
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
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
** 函数名称: __interfaceMonitorAction
** 功能描述: 获取网卡状态Action(不包括虚拟网卡)
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    pjRes              = cJSON_CreateObject();                          /*  创建返回 Json 对象          */
    pJsonInterFaceArry = cJSON_CreateArray();                           /*  创建 Json 网卡接口数组      */
    if(LW_NULL == pjRes || LW_NULL == pJsonInterFaceArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  判断是否完成自适应定位
     */
    iIntAdaRet = libIntAdaCheck();
    if (ERROR_NONE == iIntAdaRet) {
        cJSON_AddNumberToObject(pjRes, WEB_CODE, 200);                  /*  完成定位操作                */
    } else {
        cJSON_AddNumberToObject(pjRes, WEB_CODE, 500);                  /*  没完成定位操作              */
    }

    /*
     *  WAN LAN 口均有一个是空的
     */
    cJSON_AddNumberToObject(pjRes, WAN_COUNT, LW_CFG_NET_NAT_MAX_AP_IF - 1);
    cJSON_AddNumberToObject(pjRes, LAN_COUNT, LW_CFG_NET_NAT_MAX_LOCAL_IF - 1);

    /*
     *  获取接口状态
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

        cJSON*  pjInterFace = cJSON_CreateObject();                     /*  创建单个网卡元素 Json 对象  */
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
         *  将地址转为字符串格式
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiIp),      cIpAddr,  sizeof(cIpAddr));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiNetMask), cNetMask, sizeof(cNetMask));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiGateWay), cGateWay, sizeof(cGateWay));

        /*
         *  为此网卡生成 Json 结构
         */
        cJSON_AddStringToObject(pjInterFace, EN_NAME, pNetInterfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjInterFace, WEB_NAME, pNetInterfaceInfo->cWebName);
        cJSON_AddStringToObject(pjInterFace, DEV_NAME, pNetInterfaceInfo->cDevName);
        cJSON_AddStringToObject(pjInterFace,
                                LINK_STATUS,
                                (pNetInterfaceInfo->iLinkFlag & IFF_RUNNING) ? LINK_ON : LINK_OFF);

        /*
         *  判断 NAT 类型
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
         *  判断线路类型
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

        cJSON_AddItemToArray(pJsonInterFaceArry, pjInterFace);          /*  将单网卡加入数组            */

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
** 函数名称: __vndinterfaceGetAction
** 功能描述: 获取网卡状态Action(包括虚拟网卡)
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    pjRes              = cJSON_CreateObject();                          /*  创建返回 Json 对象          */
    pJsonInterFaceArry = cJSON_CreateArray();                           /*  创建 Json 网卡接口数组      */
    if(LW_NULL == pjRes || LW_NULL == pJsonInterFaceArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  判断是否完成自适应定位
     */
    iIntAdaRet = libIntAdaCheck();
    if (ERROR_NONE == iIntAdaRet) {
        cJSON_AddNumberToObject(pjRes, "code", 200);                    /*  完成定位操作                */
    } else {
        cJSON_AddNumberToObject(pjRes, "code", 500);                    /*  没完成定位操作              */
    }

    /*
     *  WAN LAN 口均有一个是空的
     */
    cJSON_AddNumberToObject(pjRes, WAN_COUNT, LW_CFG_NET_NAT_MAX_AP_IF - 1);
    cJSON_AddNumberToObject(pjRes, LAN_COUNT, LW_CFG_NET_NAT_MAX_LOCAL_IF - 1);

    /*
     *  获取接口状态
     */
    pNetInterfaceBuff = libAllInterFaceInfoGet(&iNetIfNum);
    pBuffTmp          = pNetInterfaceBuff;
    for (i = 0; i < iNetIfNum; i++) {
        pNetInterfaceInfo = (__PNETIF_INFO)pNetInterfaceBuff;

        cJSON*  pjInterFace = cJSON_CreateObject();                     /*  创建单个网卡元素 Json 对象  */
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
         *  将地址转为字符串格式
         */
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiIp),      cIpAddr,  sizeof(cIpAddr));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiNetMask), cNetMask, sizeof(cNetMask));
        ip4addr_ntoa_r((const ip4_addr_t *)&(pNetInterfaceInfo->uiGateWay), cGateWay, sizeof(cGateWay));

        /*
         *  为此网卡生成 Json 结构
         */
        cJSON_AddStringToObject(pjInterFace, EN_NAME, pNetInterfaceInfo->cNetifName);
        cJSON_AddStringToObject(pjInterFace, WEB_NAME, pNetInterfaceInfo->cWebName);
        cJSON_AddStringToObject(pjInterFace, DEV_NAME, pNetInterfaceInfo->cDevName);
        cJSON_AddStringToObject(pjInterFace,
                                LINK_STATUS,
                                (pNetInterfaceInfo->iLinkFlag & IFF_RUNNING) ? LINK_ON : LINK_OFF);

        /*
         *  判断 NAT 类型
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
         *  判断线路类型
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

        cJSON_AddItemToArray(pJsonInterFaceArry, pjInterFace);          /*  将单网卡加入数组            */

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
** 函数名称: interfaceActionDefine
** 功能描述: 网口定位后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID __interfaceActionDefine (VOID)
{
    websDefineAction("interfaceMonitorAction", __interfaceMonitorAction);
    websDefineAction("vndinterfaceGetAction",  __vndinterfaceGetAction);
    websDefineAction("interfaceInfoSetAction", __interfaceInfoSetAction);
}

/*********************************************************************************************************
** 函数名称: interfaceActionDefine
** 功能描述: 网口定位后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  interfaceActionInit (VOID)
{
    __interfaceActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
