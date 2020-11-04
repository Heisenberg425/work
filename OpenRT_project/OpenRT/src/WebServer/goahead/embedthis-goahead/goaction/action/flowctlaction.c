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
** 文   件   名: flowctlaction.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: flowctl配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "flowctl_info.h"
#include "../webHandle.h"
#include "interface_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                     "code"                             /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: flowctl配置及流控信息显示
*********************************************************************************************************/
#define WEB_FLOWCTL_TYPE             "fc_type"                          /* 1: 对协议进行流控
                                                                           2: 对网口进行流控            */
#define WEB_FLOWCTL_ENABLE           "fc_enable"                        /* 1: 使能 0: 去使能            */
#define WEB_FLOWCTL_PROTO            "fc_proto"                         /* 生效协议类型 tcp/udp/all     */
#define WEB_FLOWCTL_START            "fc_start"                         /* 流控起始 IP 地址             */
#define WEB_FLOWCTL_END              "fc_end"                           /* 流控结束 IP 地址             */
#define WEB_FLOWCTL_SPORT            "fc_sport"                         /* 流控起始端口                 */
#define WEB_FLOWCTL_EPORT            "fc_eport"                         /* 流控结束端口                 */
#define WEB_FLOWCTL_IFNAME           "fc_ifname"                        /* 网口                         */
#define WEB_FLOWCTL_UPRATE           "fc_uprate"                        /* 上行流量（KB/s）             */
#define WEB_FLOWCTL_DOWNRATE         "fc_downrate"                      /* 下行流量（KB/s）             */
#define WEB_FLOWCTL_BUFSIZE          "fc_bufsize"                       /* 缓冲区大小(16K ~ 128M Bytes) */
#define WEB_FLOWCTL_IP_FAMILY        "ip_family"                        /* ip 地址族，IPv4 或 IPv6      */
#define WEB_FLOWCTL_OPER_TYPE        "oper_type"                        /* 操作类型，增删改             */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                     "page"                             /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                    "total"                            /*  Json字段：总共的条目数      */
#define ARRAY_NAME                   "data"                             /*  Json字段：内网端口          */
/*********************************************************************************************************
** 函数名称: __flowctlInfoGetAction
** 功能描述: 获取流控信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __flowctlShowAction (Webs *wp)
{
    __PFLOWCTL_HANDLE     pFlowctlHeader   = LW_NULL;
    __PFLOWCTL_HANDLE     pTmpNode         = LW_NULL;
    __FLOWCTL_HANDLE      flowctlHandle    = {0};
    INT                   iPage            = 1;                         /*  待显示的页码                */
    INT                   iTotal           = 0;                         /*  总条目                      */
    cJSON*                pJsonflowctlArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;

    cJSON                *pjPage           = LW_NULL;
    cJSON                *pjFlowCtlType    = LW_NULL;
    cJSON                *pjOperType       = LW_NULL;

    INT                   iRecordTotal     = 0;
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

    pjSend           = cJSON_CreateObject();                            /*  创建返回 Json 对象          */
    pJsonflowctlArry = cJSON_CreateArray();                             /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || LW_NULL == pJsonflowctlArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjPage        = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    pjFlowCtlType = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_TYPE);
    pjOperType    = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_OPER_TYPE);
    if (NULL == pjPage        ||
        NULL == pjFlowCtlType ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    flowctlHandle.iType = pjFlowCtlType->valueint,
    strncpy(flowctlHandle.cOperType, pjOperType->valuestring, sizeof(flowctlHandle.cOperType));

    pFlowctlHeader = flowctlShow(&flowctlHandle, &iRecordTotal);        /*  发送查询流控请求            */
    if (LW_NULL != pFlowctlHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  遍历数组                    */
            pTmpNode = pFlowctlHeader + i;

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {

                cJSON*  pjFlowCtl = cJSON_CreateObject();               /*  创建一条映射 Json 对象      */
                if(LW_NULL == pjFlowCtl) {
                     break;
                }

                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_TYPE,     pTmpNode->iType);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_ENABLE,   pTmpNode->cEnable);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_PROTO,    pTmpNode->cProtocol);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_START,    pTmpNode->cStartIp);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_END,      pTmpNode->cEndIp);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_SPORT,    pTmpNode->iStartPort);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_EPORT,    pTmpNode->iEndPort);
                cJSON_AddStringToObject(pjFlowCtl, WEB_FLOWCTL_IFNAME,   pTmpNode->cIfname);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_UPRATE,   pTmpNode->ulUpRate);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_DOWNRATE, pTmpNode->ulDownRate);
                cJSON_AddNumberToObject(pjFlowCtl, WEB_FLOWCTL_BUFSIZE,  pTmpNode->ulBufSize);

                cJSON_AddItemToArray(pJsonflowctlArry, pjFlowCtl);      /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    flowctlInfoFree(pFlowctlHeader);

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonflowctlArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);
    cJSON_Delete(pJsonflowctlArry);
}
/*********************************************************************************************************
** 函数名称: __flowctlConfigAction
** 功能描述: 流控配置处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __flowctlConfigAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *pjType        = LW_NULL;
    cJSON                *pjUpRate      = LW_NULL;
    cJSON                *pjDownRate    = LW_NULL;
    cJSON                *pjBufSize     = LW_NULL;
    cJSON                *pjIfname      = LW_NULL;
    cJSON                *pjOperType    = LW_NULL;

    cJSON                *pjProtocol    = LW_NULL;
    cJSON                *pjStartPort   = LW_NULL;
    cJSON                *pjEndPort     = LW_NULL;
    cJSON                *pjStartIp     = LW_NULL;
    cJSON                *pjEndIp       = LW_NULL;

    __FLOWCTL_HANDLE      flowctlHandle = {0};
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
    pjType     = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_TYPE);
    pjUpRate   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_UPRATE);
    pjDownRate = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_DOWNRATE);
    pjBufSize  = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_BUFSIZE);
    pjIfname   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_IFNAME);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_OPER_TYPE);
    if (NULL == pjType     ||
        NULL == pjUpRate   ||
        NULL == pjDownRate ||
        NULL == pjBufSize  ||
        NULL == pjIfname   ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    flowctlHandle.iType      = pjType->valueint;
    flowctlHandle.ulUpRate   = (ULONG)pjUpRate->valueint;
    flowctlHandle.ulDownRate = (ULONG)pjDownRate->valueint;
    flowctlHandle.ulBufSize  = (ULONG)pjBufSize->valueint;

    strncpy(flowctlHandle.cIfname,   pjIfname->valuestring,   sizeof(flowctlHandle.cIfname));
    strncpy(flowctlHandle.cOperType, pjOperType->valuestring, sizeof(flowctlHandle.cOperType));

    if (FCT_IP == flowctlHandle.iType) {
        pjProtocol  = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_PROTO);
        pjStartIp   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_START);
        pjEndIp     = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_END);
        pjStartPort = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_SPORT);
        pjEndPort   = cJSON_GetObjectItem(pRecvjRes, WEB_FLOWCTL_EPORT);
        if (NULL == pjProtocol  ||
            NULL == pjStartIp   ||
            NULL == pjEndIp     ||
            NULL == pjStartPort ||
            NULL == pjEndPort) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __error;
        }

        strncpy(flowctlHandle.cProtocol, pjProtocol->valuestring, sizeof(flowctlHandle.cProtocol));
        strncpy(flowctlHandle.cStartIp,  pjStartIp->valuestring,  sizeof(flowctlHandle.cStartIp));
        strncpy(flowctlHandle.cEndIp,    pjEndIp->valuestring,    sizeof(flowctlHandle.cEndIp));

        flowctlHandle.iStartPort = pjStartPort->valueint;
        flowctlHandle.iEndPort   = pjEndPort->valueint;
    }

    iRet = flowctlConfig(&flowctlHandle);                               /*  发送配置流控请求            */
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __flowctlActionDefine
** 功能描述: 流控后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __flowctlActionDefine (VOID)
{
    websDefineAction("flowctlShowAction",   __flowctlShowAction);
    websDefineAction("flowctlConfigAction", __flowctlConfigAction);
}
/*********************************************************************************************************
** 函数名称: flowctlActionInit
** 功能描述: 流控后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  flowctlActionInit (VOID)
{
    __flowctlActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
