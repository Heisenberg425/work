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
** 文   件   名: qosaction.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 16 日
**
** 描        述: QoS配置后台接口
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
#include "qos_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */
/*********************************************************************************************************
  Jason key: QoS配置信息显示
*********************************************************************************************************/
#define WEB_QOS_NETIF              "netif"                              /*  Json字段：网卡接口          */
#define WEB_QOS_RULE               "rule"                               /*  Json字段：规则              */
#define WEB_QOS_IPS                "ipStart"                            /*  Json字段：起始ip            */
#define WEB_QOS_IPE                "ipEnd"                              /*  Json字段：终止ip            */
#define WEB_QOS_PORTS              "portStart"                          /*  Json字段：起始port          */
#define WEB_QOS_PORTE              "portEnd"                            /*  Json字段：终止port          */
#define WEB_QOS_CMPMETHOD          "cmpMethod"                          /*  Json字段：比较方法          */
#define WEB_QOS_PRIORITY           "priority"                           /*  Json字段：优先级            */
#define WEB_QOS_DONTDROP           "dontDrop"                           /*  Json字段：不丢包            */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                   "page"                               /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                  "total"                              /*  Json字段：总共的条目数      */
#define ARRAY_NAME                 "data"                               /*  Json字段：内网端口          */
/*********************************************************************************************************
** 函数名称: __qosShowAction
** 功能描述: QoS显示后台接口
** 输　入  : wp     服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __qosShowAction (Webs  *wp)
{
    __PQOS_HANDLE         pQosHeader               = LW_NULL;
    __PQOS_HANDLE         pTmpNode                 = LW_NULL;
    __QOS_HANDLE          qosHandle                = {0};
    cJSON*                pJsonQosArry             = LW_NULL;
    cJSON*                pjSend                   = LW_NULL;
    cJSON*                pRecvjRes                = LW_NULL;
    cJSON*                pjPage                   = LW_NULL;
    CHAR                  cWebName[NETIF_NAMESIZE] = {0};
    INT                   iPage                    = 1;                 /*  待显示的页码                */
    INT                   iTotal                   = 0;                 /*  总条目                      */
    INT                   iRecordTotal             = 0;
    INT                   i;

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

    pjSend       = cJSON_CreateObject();                                /*  创建返回 Json 对象          */
    pJsonQosArry = cJSON_CreateArray();                                 /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || LW_NULL == pJsonQosArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (LW_NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);                 /*  参数出错                    */

        goto __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);                 /*  参数出错                    */

        goto __error;
    }

    /*
     * 调用查询请求接口
     */
    pQosHeader = qosShow(&qosHandle, &iRecordTotal);
    if (LW_NULL != pQosHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            CHAR    cIpStart[20] = {0};
            CHAR    cIpEnd[20]   = {0};

            pTmpNode = pQosHeader + i;
            strncpy(cIpStart, inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpStart), sizeof(cIpStart));
            strncpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpEnd),   sizeof(cIpEnd));

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjQos = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
                if (LW_NULL == pjQos) {
                     break;
                }

                bzero(cWebName, sizeof(cWebName));
                libIntAdaGetWebNetif(pTmpNode->cNetif, cWebName, sizeof(cWebName));

                cJSON_AddStringToObject(pjQos, WEB_QOS_NETIF, cWebName);
                cJSON_AddStringToObject(pjQos, WEB_QOS_RULE, pTmpNode->cRule);
                cJSON_AddStringToObject(pjQos, WEB_QOS_IPS, cIpStart);
                cJSON_AddStringToObject(pjQos, WEB_QOS_IPE, cIpEnd);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PORTS, pTmpNode->uiPortStart);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PORTE, pTmpNode->uiPortEnd);
                cJSON_AddStringToObject(pjQos, WEB_QOS_CMPMETHOD, pTmpNode->cCmpMethod);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_PRIORITY, pTmpNode->uiPriority);
                cJSON_AddNumberToObject(pjQos, WEB_QOS_DONTDROP, pTmpNode->uiDontDrop);

                cJSON_AddItemToArray(pJsonQosArry, pjQos);              /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    if (pQosHeader) {
        free(pQosHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonQosArry);
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
    cJSON_Delete(pJsonQosArry);
}
/*********************************************************************************************************
** 函数名称: __qosConfigAction
** 功能描述: QoS设置后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __qosConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes      = LW_NULL;
    cJSON                *pSendjRes      = LW_NULL;
    cJSON                *pjIpStart      = LW_NULL;
    cJSON                *pjIpEnd        = LW_NULL;
    cJSON                *pjPortStart    = LW_NULL;
    cJSON                *pjPortEnd      = LW_NULL;
    cJSON                *pjPriority     = LW_NULL;
    cJSON                *pjDontDrop     = LW_NULL;
    cJSON                *pjNetif        = LW_NULL;
    cJSON                *pjRule         = LW_NULL;
    cJSON                *pjCmpMethod    = LW_NULL;
    __QOS_HANDLE          qosHandle;
    INT                   iRet;


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

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjIpStart    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPS);
    pjIpEnd      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPE);
    pjPortStart  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTS);
    pjPortEnd    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTE);
    pjPriority   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PRIORITY);
    pjDontDrop   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_DONTDROP);
    pjNetif      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_NETIF);
    pjRule       = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_RULE);
    pjCmpMethod  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_CMPMETHOD);
    if (pjIpStart    == LW_NULL  ||
        pjIpEnd      == LW_NULL  ||
        pjPortStart  == LW_NULL  ||
        pjPortEnd    == LW_NULL  ||
        pjPriority   == LW_NULL  ||
        pjDontDrop   == LW_NULL  ||
        pjNetif      == LW_NULL  ||
        pjRule       == LW_NULL  ||
        pjCmpMethod  == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto __error;
    }

    /*
     *  构造信息
     */
    bzero(&qosHandle, sizeof(qosHandle));
    qosHandle.uiIpStart   = inet_addr(pjIpStart->valuestring);
    qosHandle.uiIpEnd     = inet_addr(pjIpEnd->valuestring);
    qosHandle.uiPortStart = (UINT16)pjPortStart->valueint;
    qosHandle.uiPortEnd   = (UINT16)pjPortEnd->valueint;
    qosHandle.uiPriority  = (UINT8)pjPriority->valueint;
    qosHandle.uiDontDrop  = (UINT8)pjDontDrop->valueint;

    /*
     *  获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pjNetif->valuestring, qosHandle.cNetif, sizeof(qosHandle.cNetif));
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __error;
    }

    strncpy(qosHandle.cRule,      pjRule->valuestring,      sizeof(qosHandle.cRule));
    strncpy(qosHandle.cCmpMethod, pjCmpMethod->valuestring, sizeof(qosHandle.cCmpMethod));

    /*
     *  调用配置请求接口
     */
    iRet = qosConfig (&qosHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __qosDeleteAction
** 功能描述: QoS删除配置后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __qosDeleteAction (Webs  *wp)
{
    cJSON                *pRecvjRes      = LW_NULL;
    cJSON                *pSendjRes      = LW_NULL;
    cJSON                *pjIpStart      = LW_NULL;
    cJSON                *pjIpEnd        = LW_NULL;
    cJSON                *pjPortStart    = LW_NULL;
    cJSON                *pjPortEnd      = LW_NULL;
    cJSON                *pjPriority     = LW_NULL;
    cJSON                *pjDontDrop     = LW_NULL;
    cJSON                *pjNetif        = LW_NULL;
    cJSON                *pjRule         = LW_NULL;
    cJSON                *pjCmpMethod    = LW_NULL;
    __QOS_HANDLE          qosHandle;
    INT                   iRet;

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

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjIpStart    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPS);
    pjIpEnd      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_IPE);
    pjPortStart  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTS);
    pjPortEnd    = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PORTE);
    pjPriority   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_PRIORITY);
    pjDontDrop   = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_DONTDROP);
    pjNetif      = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_NETIF);
    pjRule       = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_RULE);
    pjCmpMethod  = cJSON_GetObjectItem(pRecvjRes, WEB_QOS_CMPMETHOD);
    if (pjIpStart    == LW_NULL  ||
        pjIpEnd      == LW_NULL  ||
        pjPortStart  == LW_NULL  ||
        pjPortEnd    == LW_NULL  ||
        pjPriority   == LW_NULL  ||
        pjDontDrop   == LW_NULL  ||
        pjNetif      == LW_NULL  ||
        pjRule       == LW_NULL  ||
        pjCmpMethod  == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, JSON_ARGERR_CODE); /*  参数出错                    */

        goto __error;
    }

    /*
     *  构造信息
     */
    bzero(&qosHandle, sizeof(qosHandle));
    qosHandle.uiIpStart     = inet_addr(pjIpStart->valuestring);
    qosHandle.uiIpEnd       = inet_addr(pjIpEnd->valuestring);
    qosHandle.uiPortStart   = (UINT16)pjPortStart->valueint;
    qosHandle.uiPortEnd     = (UINT16)pjPortEnd->valueint;
    qosHandle.uiPriority    = (UINT8)pjPriority->valueint;
    qosHandle.uiDontDrop    = (UINT8)pjDontDrop->valueint;

    /*
     *  获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pjNetif->valuestring, qosHandle.cNetif, sizeof(qosHandle.cNetif));
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __error;
    }

    strncpy(qosHandle.cRule,      pjRule->valuestring,      sizeof(qosHandle.cRule));
    strncpy(qosHandle.cCmpMethod, pjCmpMethod->valuestring, sizeof(qosHandle.cCmpMethod));

    /*
     *  调用配置请求接口
     */
    iRet = qosDelete(&qosHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __qosActionDefine
** 功能描述: QoS后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __qosActionDefine (VOID)
{
    websDefineAction("qosShowAction", __qosShowAction);
    websDefineAction("qosConfigAction", __qosConfigAction);
    websDefineAction("qosDeleteAction", __qosDeleteAction);
}
/*********************************************************************************************************
** 函数名称: qosActionInit
** 功能描述: QoS后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  qosActionInit (VOID)
{
    __qosActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
