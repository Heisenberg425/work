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
** 文   件   名: natAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述: OpenRT NAT 配置后台接口
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "cJSON.h"
#include "goahead.h"
#include "nat_info.h"
#include "intada_info.h"
#include "lwip/netif.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: 获取端口使用信息
*********************************************************************************************************/
#define WEB_NAT_ASSNODETOTAL "totalnode"                                /*  Json字段：端口总个数        */
#define WEB_NAT_ASSNODEUSE   "usenode"                                  /*  Json字段：端口使用个数      */
#define WEB_NAT_ASSNODEPRE   "percent"                                  /*  Json字段：端口使用百分比    */
/*********************************************************************************************************
  Jason key: 分片包设置
*********************************************************************************************************/
#define WEB_NAT_IPFRAG      "fragtype"                                    /*  Json字段：分片包类型      */
#define WEB_NAT_IPFRAG_TCP  "tcp"                                         /*  Json字段：TCP 分片包      */
#define WEB_NAT_IPFRAG_UDP  "udp"                                         /*  Json字段：UCP 分片包      */
#define WEB_NAT_IPFRAG_ICMP "icmp"                                        /*  Json字段：ICMP 分片包     */
/*********************************************************************************************************
  Jason key: 端口映射设置
*********************************************************************************************************/
#define WEB_NAT_ASSPORT     "assport"                                    /*  Json字段：外网端口         */
#define WEB_NAT_LOCALPORT   "localport"                                  /*  Json字段：内网端口         */
#define WEB_NAT_IP          "ip"                                         /*  Json字段：IP               */
#define WEB_NAT_IPCNT       "ipcnt"                                      /*  Json字段：内网个数         */
#define WEB_NAT_PROTO       "proto"                                      /*  Json字段：协议             */
#define WEB_NAT_HANDLE      "handle"                                     /*  Json字段：处理方式         */
/*********************************************************************************************************
  Jason key: 网络别名设置
*********************************************************************************************************/
#define WEB_NAT_ALIASIP     "aliasip"                                   /*  Json字段：网络别名          */
#define WEB_NAT_LANSTART    "lanstart"                                  /*  Json字段：内网起始 IP       */
#define WEB_NAT_LANEND      "lanend"                                    /*  Json字段：内网结束 IP       */
/*********************************************************************************************************
  Jason key: 端口映射获取信息
*********************************************************************************************************/
#define WEB_NAT_PAGE        "page"                                       /*  Json字段：当前待显示的页码 */
#define WEB_NAT_PAGE_COUNT  "count"                                      /*  Json字段：每页显示的个数   */
#define WEB_NAT_TOTAL       "total"                                      /*  Json字段：总共的条目数     */
#define ARRAY_NAME          "data"                                       /*  Json字段：内网端口         */
/*********************************************************************************************************
  Jason key: 配置WAN LAN 口
*********************************************************************************************************/
#define WEB_NAT_INDEX       "index"                                     /*  Json字段：网口号            */
#define WEB_NAT_NETIF       "webnetif"                                  /*  Json字段：前端网口名        */
#define WEB_NAT_USE         "use"                                       /*  Json字段：功能              */
#define WEB_NAT_HANDLE      "handle"                                    /*  Json字段：处理方式          */
/*********************************************************************************************************
** 函数名称: __natAssNodeUseAction
** 功能描述: 获取随机端口使用情况后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAssNodeUseAction (Webs *wp)
{
    cJSON*  pjSend           = LW_NULL;
    INT     iRet             = 0;
    float   fNatAssNodePre   = 0.0f;

    INT     iNatAssNodeTotal;
    INT     iNatAssNodeUse;

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

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iRet = libNatAssNodeGet(&iNatAssNodeTotal, &iNatAssNodeUse);
    if (PX_ERROR == iRet) {                                             /*  获取失败                    */
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  设置返回的 Json 内容
     */
    fNatAssNodePre = (iNatAssNodeUse * 1.0) / iNatAssNodeTotal;

    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODETOTAL, iNatAssNodeTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODEUSE,   iNatAssNodeUse);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_ASSNODEPRE,   fNatAssNodePre);

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}

/*********************************************************************************************************
** 函数名称: __natMapGetAction
** 功能描述: 获取端口映射信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natMapGetAction (Webs *wp)
{
    PCHAR                 pcNatMapBuf     = LW_NULL;
    PCHAR                 pcNatMapBufTmp  = LW_NULL;
    __PNATMAP_INFO        pNatMapInfo     = LW_NULL;

    INT                   iPage;                                        /*  待显示的页码                */
    INT                   iPageCount      = 0;
    INT                   iTotal          = 0;                          /*  总条目                      */
    cJSON*                pJsonNatMapArry = LW_NULL;
    cJSON*                pjSend          = LW_NULL;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pjiPages        = LW_NULL;
    cJSON                *pjPageCount     = LW_NULL;

    __NAT_INFO_GET        natInfoGet;

    INT                   iNum            = 0;
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

    pjSend          = cJSON_CreateObject();                             /*  创建返回 Json 对象          */
    pJsonNatMapArry = cJSON_CreateArray();                              /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonNatMapArry) {
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
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE_COUNT);
    pjiPages    = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE);
    if (LW_NULL == pjiPages || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjiPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount = pjPageCount->valueint;

    natInfoGet.iStartIndex = (iPage - 1) * iPageCount + 1;
    natInfoGet.iEndIndex   = iPage * iPageCount;

    iTotal         = libNatMapNumGet();
    pcNatMapBuf    = libNatMapGet(&natInfoGet, &iNum);
    pcNatMapBufTmp = pcNatMapBuf;
    pNatMapInfo    = (__PNATMAP_INFO)pcNatMapBuf;

    if (0 != iNum && LW_NULL != pcNatMapBuf) {
        for(i = 0; i < iNum; i++) {

            cJSON  *pjNatMap = cJSON_CreateObject();                    /*  创建一条映射 Json 对象      */
            if (LW_NULL == pjNatMap) {
                 break;
            }

            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_ASSPORT,   pNatMapInfo->iWanPort);
            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_LOCALPORT, pNatMapInfo->iLanPort);
            cJSON_AddNumberToObject(pjNatMap, WEB_NAT_IPCNT,     pNatMapInfo->iIpCnt);
            cJSON_AddStringToObject(pjNatMap, WEB_NAT_IP,        pNatMapInfo->cIpString);
            cJSON_AddStringToObject(pjNatMap, WEB_NAT_PROTO,     pNatMapInfo->cProtoString);

            cJSON_AddItemToArray(pJsonNatMapArry, pjNatMap);        /*  将此条目加入数组            */

            pcNatMapBuf += sizeof(__NATMAP_INFO);
            pNatMapInfo  = (__PNATMAP_INFO)(pcNatMapBuf);
        }
    }


    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonNatMapArry);

    cJSON_AddNumberToObject(pjSend, WEB_NAT_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,      200);

    if (NULL != pcNatMapBufTmp) {
        free(pcNatMapBufTmp);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pJsonNatMapArry);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __natMapHandleAction
** 功能描述: 端口映射处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natMapHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    PCHAR                 pcIpString    = LW_NULL;                      /*  Ip 字符串                   */
    PCHAR                 pcProtoString = LW_NULL;                      /*  协议 字符串                 */

    INT                   iRet;
    CHAR                  cLogBuff[128] = {0};

    INT                   iIpCnt;
    INT                   iWanPort;
    INT                   iLanPort;
    INT                   cProto;
    CHAR                  cNatMapHandle;

    cJSON                *pjIpCnt        = LW_NULL;;
    cJSON                *pjWanPort      = LW_NULL;
    cJSON                *pjLanPort      = LW_NULL;
    cJSON                *pjNatMapHandle = LW_NULL;
    cJSON                *pjIpString     = LW_NULL;
    cJSON                *pjProtoString  = LW_NULL;

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
    pjIpCnt        = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IPCNT);
    pjNatMapHandle = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    pjWanPort      = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_ASSPORT);
    pjLanPort      = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LOCALPORT);
    pjIpString     = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IP);
    pjProtoString  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PROTO);
    if (LW_NULL == pjIpCnt        ||
        LW_NULL == pjNatMapHandle ||
        LW_NULL == pjWanPort      ||
        LW_NULL == pjLanPort      ||
        LW_NULL == pjIpString     ||
        LW_NULL == pjProtoString) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    cNatMapHandle  = pjNatMapHandle->valueint;
    iWanPort       = atoi(pjWanPort->valuestring);
    iLanPort       = atoi(pjLanPort->valuestring);
    iIpCnt         = atoi(pjIpCnt->valuestring);
    pcIpString     = pjIpString->valuestring;
    pcProtoString  = pjProtoString->valuestring;

    if (!strcmp(pcProtoString, "tcp") || !strcmp(pcProtoString, "TCP")) {
        cProto = IPPROTO_TCP;
    } else if (!strcmp(pcProtoString, "udp") || !strcmp(pcProtoString, "UDP")) {
        cProto = IPPROTO_UDP;
    } else {
        cProto = -1;
    }

    iRet = libNatMapSet(pcIpString, iIpCnt, iWanPort, iLanPort, cProto, cNatMapHandle);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  记录操作日志
     */
    snprintf(cLogBuff, sizeof(cLogBuff),
             "%s nat map, wan port:%s lan port:%s ip:%s ipcnt:%d prto:%s\n",
             0 == cNatMapHandle ? "set" : "delete",
             pjWanPort->valuestring,
             pjLanPort->valuestring,
             pcIpString,
             iIpCnt,
             pcProtoString);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __natAliasGetAction
** 功能描述: 获取网络别名信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAliasGetAction (Webs *wp)
{
    __PNATALIAS_INFO      pNatAliasInfo     = LW_NULL;
    PCHAR                 pcNatAliasBuf     = LW_NULL;
    PCHAR                 pcNatAliasBufTmp  = LW_NULL;
    INT                   iPage             = 0;                        /*  待显示的页码                */
    INT                   iPageCount        = 0;
    INT                   iTotal            = 0;                        /*  总条目                      */
    cJSON*                pJsonNatAliasArry = LW_NULL;
    cJSON*                pjSend            = LW_NULL;
    cJSON                *pRecvjRes         = LW_NULL;
    cJSON                *pjPages           = LW_NULL;
    cJSON                *pjPageCount       = LW_NULL;

    __NAT_INFO_GET        natInfoGet;

    INT                   iNum              = 0;
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

    pjSend            = cJSON_CreateObject();                           /*  创建返回 Json 对象          */
    pJsonNatAliasArry = cJSON_CreateArray();                            /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonNatAliasArry) {
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
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE_COUNT);
    pjPages     = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_PAGE);
    if (LW_NULL == pjPages || LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageCount = pjPageCount->valueint;

    natInfoGet.iStartIndex = (iPage - 1) * iPageCount + 1;
    natInfoGet.iEndIndex   = iPage * iPageCount;

    iTotal           = libNatAliasNumGet();
    pcNatAliasBuf    = libNatAliasGet(&natInfoGet, &iNum);              /*  获取当前别名信息            */
    pcNatAliasBufTmp = pcNatAliasBuf;
    pNatAliasInfo    = (__PNATALIAS_INFO)pcNatAliasBuf;
    if (0 != iNum && LW_NULL != pcNatAliasBuf) {
        for(i = 0; i < iNum; i++) {
            cJSON  *pjNatAlias = cJSON_CreateObject();                  /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjNatAlias) {
                break;
            }

            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_ALIASIP,  pNatAliasInfo->cAliasString);
            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_LANSTART, pNatAliasInfo->cLoaclSString);
            cJSON_AddStringToObject(pjNatAlias, WEB_NAT_LANEND,   pNatAliasInfo->cLoaclEString);

            cJSON_AddItemToArray(pJsonNatAliasArry, pjNatAlias);        /*  将此条目加入数组            */

            pcNatAliasBuf += sizeof(__NATALIAS_INFO);
            pNatAliasInfo  = (__PNATALIAS_INFO)(pcNatAliasBuf);
        }
    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonNatAliasArry);

    cJSON_AddNumberToObject(pjSend, WEB_NAT_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_NAT_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,      200);

    if (NULL != pcNatAliasBufTmp) {
        free(pcNatAliasBufTmp);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonNatAliasArry);
}
/*********************************************************************************************************
** 函数名称: __natAliasHandleAction
** 功能描述: 网络别名处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAliasHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    PCHAR                 pcAliasIP     = LW_NULL;                      /*  网络别名                    */
    PCHAR                 pcLanStart    = LW_NULL;                      /*  网络起始 IP                 */
    PCHAR                 pcLanEnd      = LW_NULL;                      /*  网络结束 IP                 */
    cJSON                *pjAliasIP     = LW_NULL;
    cJSON                *pjLanStart    = LW_NULL;
    cJSON                *pjLanEnd      = LW_NULL;
    cJSON                *pjHandle      = LW_NULL;

    INT                   iLanStart;
    INT                   iLanEnd;
    CHAR                  cHandle;
    INT                   iRet;
    CHAR                  cLogBuff[128] = {0};

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
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjHandle   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    pjAliasIP  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_ALIASIP);
    pjLanStart = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LANSTART);
    pjLanEnd   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_LANEND);
    if (LW_NULL == pjHandle   ||
        LW_NULL == pjAliasIP  ||
        LW_NULL == pjLanStart ||
        LW_NULL == pjLanEnd) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    cHandle    = pjHandle->valueint;
    pcAliasIP  = pjAliasIP->valuestring;
    pcLanStart = pjLanStart->valuestring;
    pcLanEnd   = pjLanEnd->valuestring;
    iLanStart  = ntohl(inet_addr(pcLanStart));
    iLanEnd    = ntohl(inet_addr(pcLanEnd));
    if (iLanEnd < iLanStart) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iRet = libNatAliasSet(pcAliasIP, pcLanStart, pcLanEnd, cHandle);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  记录操作日志
     */
    snprintf(cLogBuff, sizeof(cLogBuff),
             "%s nat alias, wan ip:%s lan start ip:%s lan end ip:%s\n",
             0 == cHandle ? "set" : "delete",
             pcAliasIP,
             pcLanStart,
             pcLanEnd);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

 __error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __natIpFragHandleAction
** 功能描述: 分片包设置后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natIpFragHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes       = LW_NULL;
    PCHAR                 cIpFragType     = LW_NULL;                    /*  处理类型                    */
    cJSON                *pSendjRes       = LW_NULL;

    INT                   iRet;

    INT                   iIpFragType;
    INT                   iIpFragHandle;
    cJSON                *pjIpFragType   = LW_NULL;
    cJSON                *pjIpFragHandle = LW_NULL;

    CHAR                  cLogBuff[50]   = {0};

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
    pjIpFragType   = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_IPFRAG);
    pjIpFragHandle = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    if (LW_NULL == pjIpFragType || LW_NULL == pjIpFragHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iIpFragHandle = pjIpFragHandle->valueint;
    cIpFragType   = pjIpFragType->valuestring;
    iIpFragType   = (0 == strcmp(cIpFragType, "tcp")) ? IPPROTO_TCP  :
                    (0 == strcmp(cIpFragType, "udp")  ? IPPROTO_UDP  :
                    (0 == strcmp(cIpFragType, "icmp") ? IPPROTO_ICMP : -1));

    iRet = libNatIpFragSet(iIpFragType, iIpFragHandle);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    /*
     *  记录操作日志
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s nat ipfragment, the prto is %s\n",
            iIpFragHandle == NAT_IPFRAG_ENABLE ? "enable" : "disable", cIpFragType);

    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __natIpFragGetAction
** 功能描述: 获取分片包信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natIpFragGetAction (Webs *wp)
{
    cJSON*                pjSend          = LW_NULL;
    INT                   iNatIpFragInfo  = 0;
    INT                   iValue          = 0;

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

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iNatIpFragInfo = libNatIpFragGet();
    if (PX_ERROR == iNatIpFragInfo) {                                   /*  获取失败                    */
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    } else {
        iValue = (iNatIpFragInfo & 0x01) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_TCP, iValue);

        iValue = (iNatIpFragInfo & 0x02) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_UDP, iValue);

        iValue = (iNatIpFragInfo & 0x04) ? 1 : 0;
        cJSON_AddNumberToObject(pjSend, WEB_NAT_IPFRAG_ICMP, iValue);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __natWanLanAction
** 功能描述: WAN LAN口配置
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natWanLanAction (Webs *wp)
{
    cJSON                *pRecvjRes                  = LW_NULL;
    cJSON                *pSendjRes                  = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE] = { 0 };

    PCHAR                 pcUse                      = LW_NULL;
    PCHAR                 pcWebNetif                 = LW_NULL;
    INT                   iHandle                    = -1;
    INT                   iFunc                      = 0;

    cJSON                *pjUse                      = LW_NULL;
    cJSON                *pjWebNetif                 = LW_NULL;
    cJSON                *pjHandle                   = LW_NULL;
    CHAR                  cLogBuff[20]               = {0};
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
    pjUse       = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_USE);
    pjWebNetif  = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_NETIF);
    pjHandle    = cJSON_GetObjectItem(pRecvjRes, WEB_NAT_HANDLE);
    if (LW_NULL == pjUse      ||
        LW_NULL == pjWebNetif ||
        LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcUse      = pjUse->valuestring;
    pcWebNetif = pjWebNetif->valuestring;
    iHandle    = pjHandle->valueint;

    /*
     *  获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }


    /*
     *  记录操作日志
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s %s\n", NAT_WANLAN_SET == iHandle ? "set" : "delete", pcUse);
    operLogInsert(wp, OPEN_RT_NAT, cLogBuff, 0);

    iFunc = strcmp(pcUse, "wan") ? (strcmp(pcUse, "lan") ? -1 : 1): 0;

    /*
     *  进行 WAN/LAN 口操作
     */
    iRet = libNatWanLanConfig(cNetIfName, iHandle, iFunc);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
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
** 函数名称: __natActionDefine
** 功能描述: NAT 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natActionDefine (VOID)
{
    websDefineAction("natWanLanAction",       __natWanLanAction);
    websDefineAction("natIpFragHandleAction", __natIpFragHandleAction);
    websDefineAction("natIpFragGetAction",    __natIpFragGetAction);
    websDefineAction("natAssNodeUseAction",   __natAssNodeUseAction);
    websDefineAction("natMapHandleAction",    __natMapHandleAction);
    websDefineAction("natMapGetAction",       __natMapGetAction);
    websDefineAction("natAliasGetAction",     __natAliasGetAction);
    websDefineAction("natAliasHandleAction",  __natAliasHandleAction);
}

/*********************************************************************************************************
** 函数名称: natActionInit
** 功能描述: NAT 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  natActionInit (VOID)
{
    __natActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
