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
** 文   件   名: logAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 15 日
**
** 描        述: OpenRT日志后台接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "log_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE                  "code"                                /*  返回的状态码                */
#define JSON_SUCCESS_CODE         (200)                                 /*  成功返回Json code值         */
#define JSON_FAILED_CODE          (500)                                 /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: 日志通用
*********************************************************************************************************/
#define WEB_LOG_STARTT            "starttime"                           /*  Json字段：开始时间          */
#define WEB_LOG_ENDT              "endtime"                             /*  Json字段：结束时间          */
#define WEB_LOG_PAGE              "page"                                /*  Json字段：页码              */
#define WEB_PAGECOUNT             "pagecount"                           /*  Json字段：每页个数          */
#define WEB_LOG_TOTAL             "total"                               /*  Json字段：总个数            */
#define WEB_LOG_TIME              "time"                                /*  Json字段：时间              */
#define ARRAY_NAME                "data"                                /*  Json字段：数组名称          */
/*********************************************************************************************************
  Jason key: 拨号日志
*********************************************************************************************************/
#define WEB_UDIAL_LOG_ENNAME      "enname"                              /*  Json字段：网口名称          */
#define WEB_UDIAL_LOG_EVENT       "event"                               /*  Json字段：事件              */
/*********************************************************************************************************
  Jason key: 拨号日志
*********************************************************************************************************/
#define WEB_VPN_LOG_VNDID         "vndid"                               /*  Json字段：虚拟网卡id        */
#define WEB_VPN_LOG_LOGINFI       "loginfo"                             /*  Json字段：log 信息          */

#define WEB_OPER_LOG_TIME         "oper_time"                           /*  Json字段：操作时间          */
#define WEB_OPER_LOG_CLIENT_IP    "client_ip"                           /*  Json字段：客户端IP          */
#define WEB_OPER_LOG_OPERATOR     "operator"                            /*  Json字段：操作者            */
#define WEB_OPER_LOG_MODULE_NAME  "module_name"                         /*  Json字段：模块名            */
#define WEB_OPER_LOG_RESULT       "result"                              /*  Json字段：操作结果          */
#define WEB_OPER_LOG_BUF          "oper_log"                            /*  Json字段：操作日志内容      */
/*********************************************************************************************************
** 函数名称: __udialLogGetAction
** 功能描述: 拨号日志后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;

    CHAR                  pcTimeBuf[30]               = {0};            /*  必须大于 26 字节            */
    CHAR                 *pcStartTime                 = LW_NULL;
    CHAR                 *pcEndTime                   = LW_NULL;
    INT                   iPage                       = 0;
    INT                   iPageCount                  = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;

    cJSON                *pjStartTime                 = LW_NULL;
    cJSON                *pjEndTime                   = LW_NULL;
    cJSON                *pjPage                      = LW_NULL;
    cJSON                *pjPageCount                 = LW_NULL;

    INT32                iNum                         = 0;
    INT32                iTotal                       = 0;

    __UDIAL_LOG_HANDLE   udialLogHandle;
    __PUDIAL_LOG_INFO    pUdialInfoNode               = LW_NULL;
    PCHAR                pcUdialInfoBuff              = LW_NULL;
    PCHAR                pcBuffTmp                    = LW_NULL;

    cJSON*               pJsonUdialArry               = LW_NULL;

    INT32                i;

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

    pSendjRes      = cJSON_CreateObject();                              /*  创建返回 Json 对象          */
    pJsonUdialArry = cJSON_CreateArray();                               /*  创建 Json 映射信息数组      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonUdialArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjStartTime ||
        LW_NULL == pjEndTime   ||
        LW_NULL == pjPage      ||
        LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcStartTime = pjStartTime->valuestring;
    pcEndTime   = pjEndTime->valuestring;
    iPageCount  = pjPageCount->valueint;
    iPage       = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    bzero(&tmEndTime,   sizeof(tmEndTime));
    bzero(&tmStartTime, sizeof(tmStartTime));
    strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
    strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

    udialLogHandle.logStartTm   = mktime(&tmStartTime);
    udialLogHandle.logEndTm     = mktime(&tmEndTime);
    udialLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    udialLogHandle.llEndIndex   = iPage * iPageCount;

    iTotal = libUdialLogNumGet(&udialLogHandle);

    pcUdialInfoBuff = libUdialLogInfoGet(&udialLogHandle, &iNum);
    pcBuffTmp       = pcUdialInfoBuff;
    if (0 != iNum && LW_NULL != pcUdialInfoBuff) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjUdialNode = cJSON_CreateObject();                 /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjUdialNode) {
                 break;
            }

            pUdialInfoNode = (__PUDIAL_LOG_INFO)pcUdialInfoBuff;

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pUdialInfoNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjUdialNode, WEB_LOG_TIME,         pcTimeBuf);
            cJSON_AddStringToObject(pjUdialNode, WEB_UDIAL_LOG_ENNAME, pUdialInfoNode->cNetifName);
            cJSON_AddStringToObject(pjUdialNode, WEB_UDIAL_LOG_EVENT,  pUdialInfoNode->cEvent);

            cJSON_AddItemToArray(pJsonUdialArry, pjUdialNode);          /*  将此条目加入数组            */

            pcUdialInfoBuff += sizeof(__UDIAL_LOG_INFO);
        }
    }

    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonUdialArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    if (NULL != pcBuffTmp) {
        free(pcBuffTmp);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonUdialArry);
}
/*********************************************************************************************************
** 函数名称: __vpnLogGetAction
** 功能描述: VPN 日志后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vpnLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;

    CHAR                  pcTimeBuf[30]               = {0};            /*  必须大于 26 字节            */
    CHAR                 *pcStartTime                 = LW_NULL;
    CHAR                 *pcEndTime                   = LW_NULL;
    INT                   iPage                       = 0;
    INT                   iPageCount                  = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;
    INT32                 iVndID;

    cJSON                *pjStartTime                 = LW_NULL;
    cJSON                *pjEndTime                   = LW_NULL;
    cJSON                *pjVndID                     = LW_NULL;
    cJSON                *pjPage                      = LW_NULL;
    cJSON                *pjPageCount                 = LW_NULL;

    INT32                iNum                         = 0;
    INT32                iTotal                       = 0;

    __VPN_LOG_HANDLE     vpnLogHandle;
    __PKIDVPN_LOG_INFO   pVpnInfoNode                 = LW_NULL;
    PCHAR                pcVpnInfoBuff                = LW_NULL;
    PCHAR                pcBuffTmp                    = LW_NULL;

    cJSON*               pJsonVpnArry                 = LW_NULL;

    INT32                i;

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

    pSendjRes       = cJSON_CreateObject();                             /*  创建返回 Json 对象          */
    pJsonVpnArry    = cJSON_CreateArray();                              /*  创建 Json 映射信息数组      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonVpnArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjVndID     = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_LOG_VNDID);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjStartTime  ||
        LW_NULL == pjEndTime    ||
        LW_NULL == pjVndID      ||
        LW_NULL == pjPage       ||
        LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcStartTime = pjStartTime->valuestring;
    pcEndTime   = pjEndTime->valuestring;
    iVndID      = pjVndID->valueint;
    iPageCount  = pjPageCount->valueint;
    iPage       = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    bzero(&tmEndTime,   sizeof(tmEndTime));
    bzero(&tmStartTime, sizeof(tmStartTime));
    strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
    strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

    vpnLogHandle.logStartTm   = mktime(&tmStartTime);
    vpnLogHandle.logEndTm     = mktime(&tmEndTime);
    vpnLogHandle.iVndID       = iVndID;
    vpnLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    vpnLogHandle.llEndIndex   = iPage * iPageCount;

    iTotal = libVpnLogNumGet(&vpnLogHandle);
    if (!iTotal) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    pcVpnInfoBuff = libVpnLogInfoGet(&vpnLogHandle, &iNum);
    pcBuffTmp     = pcVpnInfoBuff;
    if (0 != iNum && LW_NULL != pcVpnInfoBuff) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjVpnNode = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjVpnNode) {
                 break;
            }

            pVpnInfoNode = (__PKIDVPN_LOG_INFO)pcVpnInfoBuff;

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pVpnInfoNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjVpnNode, WEB_LOG_TIME,        pcTimeBuf);
            cJSON_AddNumberToObject(pjVpnNode, WEB_VPN_LOG_VNDID,   pVpnInfoNode->iVndID);
            cJSON_AddStringToObject(pjVpnNode, WEB_UDIAL_LOG_EVENT, pVpnInfoNode->cLogInfo);

            cJSON_AddItemToArray(pJsonVpnArry, pjVpnNode);              /*  将此条目加入数组            */

            pcVpnInfoBuff += sizeof(__KIDVPN_LOG_INFO);
        }
    }


    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonVpnArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    if (NULL != pcBuffTmp) {
        free(pcBuffTmp);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotal);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVpnArry);
}
/*********************************************************************************************************
** 函数名称: __operLogGetAction
** 功能描述: 操作日志获取后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __operLogGetAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    CHAR                  pcTimeBuf[30] = {0};                          /*  必须大于 26 字节            */
    INT32                iNum           = 0;

    __OPER_LOG           operLogHandle  = {0};
    __POPER_LOG          pOperLogNode   = LW_NULL;
    __POPER_LOG          pOperLogHead   = LW_NULL;
    cJSON*               pJsonLogArry   = LW_NULL;

    INT32                i;
    CHAR                 *pcStartTime   = LW_NULL;
    CHAR                 *pcEndTime     = LW_NULL;
    INT                   iPage         = 0;
    INT                   iPageCount    = 0;

    struct tm             tmStartTime;
    struct tm             tmEndTime;

    cJSON                *pjStartTime   = LW_NULL;
    cJSON                *pjEndTime     = LW_NULL;
    cJSON                *pjPage        = LW_NULL;
    cJSON                *pjPageCount   = LW_NULL;
    INT                   iTotalCount   = 0;

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

    pSendjRes    = cJSON_CreateObject();                                /*  创建返回 Json 对象          */
    pJsonLogArry = cJSON_CreateArray();                                 /*  创建 Json 映射信息数组      */
    if (LW_NULL == pSendjRes || LW_NULL == pJsonLogArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjStartTime = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_STARTT);
    pjEndTime   = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_ENDT);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_LOG_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjPage || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount                 = pjPageCount->valueint;
    operLogHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    operLogHandle.llEndIndex   = iPage * iPageCount;

    if ((LW_NULL != pjStartTime) && (LW_NULL != pjEndTime)) {
        pcStartTime = pjStartTime->valuestring;
        pcEndTime   = pjEndTime->valuestring;

        bzero(&tmEndTime,   sizeof(tmEndTime));
        bzero(&tmStartTime, sizeof(tmStartTime));
        strptime(pcStartTime, "%a %b %d %Y %H:%M:%S", &tmStartTime);
        strptime(pcEndTime,   "%a %b %d %Y %H:%M:%S", &tmEndTime);

        operLogHandle.logStartTm = mktime(&tmStartTime);
        operLogHandle.logEndTm   = mktime(&tmEndTime);
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_MODULE_NAME)) {
        strncpy(operLogHandle.cModuleName,
                cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_MODULE_NAME)->valuestring,
                sizeof(operLogHandle.cModuleName));
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_BUF)) {
        strncpy(operLogHandle.cLog,
                cJSON_GetObjectItem(pRecvjRes, WEB_OPER_LOG_BUF)->valuestring,
                sizeof(operLogHandle.cLog));
    }

    pOperLogHead = readOperLog(&operLogHandle, &iNum);
    if (0 != iNum && LW_NULL != pOperLogHead) {
    	iTotalCount = pOperLogHead->iTotal;
        for (i = 0; i < iNum; i++) {
            pOperLogNode = pOperLogHead + i;

            cJSON  *pjOperLogNode = cJSON_CreateObject();              /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjOperLogNode) {
                 break;
            }

            bzero(pcTimeBuf, sizeof(pcTimeBuf));
            ctime_r(&pOperLogNode->llTime, pcTimeBuf);

            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_TIME,        pcTimeBuf);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_CLIENT_IP,   pOperLogNode->cClientIP);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_OPERATOR,    pOperLogNode->cOperator);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_MODULE_NAME, pOperLogNode->cModuleName);
            cJSON_AddNumberToObject(pjOperLogNode, WEB_OPER_LOG_RESULT,      pOperLogNode->iResult);
            cJSON_AddStringToObject(pjOperLogNode, WEB_OPER_LOG_BUF,         pOperLogNode->cLog);

            cJSON_AddItemToArray(pJsonLogArry, pjOperLogNode);          /*  将此条目加入数组            */
        }
    }

    cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJsonLogArry);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotalCount);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_LOG_TOTAL, iTotalCount);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonLogArry);
}
/*********************************************************************************************************
** 函数名称: __udialLogDelAction
** 功能描述: 拨号日志清除后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialLogDelAction (Webs *wp)
{
    cJSON               *pSendjRes      = LW_NULL;
    __UDIAL_LOG_HANDLE   udialLogHandle = {0};

    INT                  iRet;

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

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    iRet = libUdialLogInfoDel(&udialLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __vpnLogDelAction
** 功能描述: VPN 日志清除后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vpnLogDelAction (Webs *wp)
{
    cJSON               *pSendjRes    = LW_NULL;
    __VPN_LOG_HANDLE     vpnLogHandle = {0};

    INT                  iRet;

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

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    iRet = libVpnLogInfoDel(&vpnLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __operLogDelAction
** 功能描述: 操作日志清除后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __operLogDelAction (Webs *wp)
{
    cJSON       *pSendjRes      = LW_NULL;
    __OPER_LOG   operLogHandle  = {0};

    INT          iRet;

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

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    iRet = libOperLogInfoDel(&operLogHandle);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/***************************************************s******************************************************
** 函数名称: __logActionDefine
** 功能描述: LOG 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __logActionDefine (VOID)
{
    websDefineAction("udialLogGetAction", __udialLogGetAction);
    websDefineAction("vpnLogGetAction",   __vpnLogGetAction);
    websDefineAction("operLogGetAction",  __operLogGetAction);

    websDefineAction("udialLogDelAction", __udialLogDelAction);
    websDefineAction("vpnLogDelAction",   __vpnLogDelAction);
    websDefineAction("operLogDelAction",  __operLogDelAction);
}

/*********************************************************************************************************
** 函数名称: logActionInit
** 功能描述: LOG 后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID logActionInit (VOID)
{
    __logActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
