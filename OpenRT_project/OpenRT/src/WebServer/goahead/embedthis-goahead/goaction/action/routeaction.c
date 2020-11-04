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
** 文   件   名: routeaction.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 22 日
**
** 描        述: 路由器配置后台接口
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
#define WEB_CODE              "code"                                    /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: 静态路由配置及路由信息显示
*********************************************************************************************************/
#define WEB_ROUTE_DEST        "destination"                             /*  Json字段：目的地址          */
#define WEB_ROUTE_GATEWAY     "gateway"                                 /*  Json字段：网关              */
#define WEB_ROUTE_GENMASK     "genmask"                                 /*  Json字段：掩码              */
#define WEB_ROUTE_FLAGS       "flags"                                   /*  Json字段：路由Flags         */
#define WEB_ROUTE_PREFIX      "prefix"                                  /*  Json字段：ipv6 前缀         */
#define WEB_ROUTE_METRIC      "metric"                                  /*  Json字段：路由度量值        */
#define WEB_ROUTE_REF         "ref"                                     /*  Json字段：引用值            */
#define WEB_ROUTE_USE         "use"                                     /*  Json字段：使用              */
#define WEB_ROUTE_IFACE       "iface"                                   /*  Json字段：接口名称          */
#define WEB_ROUTE_IP_FAMILY   "ip_family"                               /*  Json字段：ip地址族          */
#define WEB_ROUTE_OPER_TYPE   "oper_type"                               /*  Json字段：操作类型，增删改查*/
#define WEB_ROUTE_ROUTE_TYPE  "route_type"                              /*  Json字段：路由类型，主机\网络*/
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_ROUTE_PAGE        "page"                                    /*  Json字段：当前待显示的页码  */
#define WEB_ROUTE_TOTAL       "total"                                   /*  Json字段：总共的条目数      */
#define ARRAY_NAME            "data"                                    /*  Json字段：内网端口          */
/*********************************************************************************************************
** 函数名称: __routeInfoGetAction
** 功能描述: 获取路由信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    INT               iPage                    = 1;                     /*  待显示的页码                */
    INT               iTotal                   = 0;                     /*  总条目                      */
    PCHAR             pcRouteType              = LW_NULL;
    cJSON*            pJsonRouteArry           = LW_NULL;
    cJSON*            pjSend                   = LW_NULL;
    cJSON            *pRecvjRes                = LW_NULL;
    cJSON            *pjPage                   = LW_NULL;
    cJSON            *pjFamily                 = LW_NULL;
    cJSON            *pjType                   = LW_NULL;
    INT               iRecordTotal             = 0;
    INT               i;

    if (!isUserLogin(wp)) {                                              /*  用户鉴权                   */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pjSend         = cJSON_CreateObject();                              /*  创建返回 Json 对象          */
    pJsonRouteArry = cJSON_CreateArray();                               /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || LW_NULL == pJsonRouteArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    bzero(&routeHandle, sizeof(__ROUTE_HANDLE));

    /*
     *  获取 Json 数据内容
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
     *  向服务器发送查询路由请求
     */
    pRouteInfo = routeInfoQuery(&routeHandle, &iRecordTotal);
    if (LW_NULL != pRouteInfo) {
        for (i = 0; i < iRecordTotal; i++) {
            pRouteInfoTmp = pRouteInfo + i;

            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {  /*  每页显示 5 条信息           */
                cJSON  *pjRoute = cJSON_CreateObject();                 /*  创建一条映射 Json 对象      */
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

                cJSON_AddItemToArray(pJsonRouteArry, pjRoute);          /*  将此条目加入数组            */
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
** 函数名称: __routeConfigAction
** 功能描述: 静态路由配置处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
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

    if (!isUserLogin(wp)) {                                              /*  用户鉴权                   */
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
     *  获取 Json 数据内容
     */
    strncpy(routeHandle.cDestination, pjDest->valuestring,    sizeof(routeHandle.cDestination));
    strncpy(routeHandle.cGateway,     pjGateway->valuestring, sizeof(routeHandle.cGateway));
    strncpy(routeHandle.cGenmask,     pjMask->valuestring,    sizeof(routeHandle.cGenmask));

    routeHandle.iMetric = pjMetric->valueint;

    strncpy(routeHandle.cIpFamily,  pjFamily->valuestring,    sizeof(routeHandle.cIpFamily));
    strncpy(routeHandle.cOperType,  pjOperType->valuestring,  sizeof(routeHandle.cOperType));
    strncpy(routeHandle.cRouteType, pjRouteType->valuestring, sizeof(routeHandle.cRouteType));

    strncpy(routeHandle.cIface, pjIface->valuestring, sizeof(routeHandle.cIface));

    iRet = routeConfig(&routeHandle);                                   /*  向服务端发送配置路由请求    */
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
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
** 函数名称: __routeActionDefine
** 功能描述: 路由后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __routeActionDefine (VOID)
{
    websDefineAction("routeInfoGetAction", __routeInfoGetAction);
    websDefineAction("routeConfigAction", __routeConfigAction);
}
/*********************************************************************************************************
** 函数名称: routeActionInit
** 功能描述: 路由后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  routeActionInit (VOID)
{
    __routeActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
