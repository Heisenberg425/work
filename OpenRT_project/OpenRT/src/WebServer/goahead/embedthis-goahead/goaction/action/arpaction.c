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
** 文   件   名: arpaction.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 11 日
**
** 描        述: arp配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "arp_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */

/*********************************************************************************************************
  Jason key: arp配置信息显示
*********************************************************************************************************/
#define WEB_ARP_IPADDR            "ipaddr"                              /*  Json字段：ip地址            */
#define WEB_ARP_HWADDR            "hwaddr"                              /*  Json字段：mac地址           */
#define WEB_ARP_OPER_TYPE         "oper_type"                           /*  Json字段：操作类型，增删改查*/
#define WEB_ARP_STATUS            "status"                              /*  Json字段：配置状态          */

/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                  "page"                                /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                 "total"                               /*  Json字段：总共的条目数      */
#define ARRAY_NAME                "data"                                /*  Json字段：内网端口          */
/*********************************************************************************************************
  Jason key: arp 防御信息
*********************************************************************************************************/
#define WEB_ASD_EN                 "en"                                 /*  Json字段：是否开启防御      */
#define WEB_ASD_TYPE               "type"                               /*  Json字段：类型              */
#define WEB_ASD_PAGE               "page"                               /*  Json字段：页码              */
#define WEB_ASD_PAGECOUNT          "pagecount"                          /*  Json字段：每页个数          */
#define WEB_ASD_IP                 "ip"                                 /*  Json字段：类型              */
#define WEB_ASD_STATUS             "status"                             /*  Json字段：开启状态          */
#define WEB_ASD_OLD_MAC            "oldmac"                             /*  Json字段：页码              */
#define WEB_ASD_NEW_MAC            "newmac"                             /*  Json字段：每页个数          */
#define WEB_ASD_FLAG               "flag"                               /*  Json字段：每页个数          */
/*********************************************************************************************************
** 函数名称: __arpDynamicShowAction
** 功能描述: 动态 arp 信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpDynamicShowAction (Webs  *wp)
{
    __PARP_HANDLE         pArpHeader       = LW_NULL;
    __PARP_HANDLE         pTmpNode         = LW_NULL;
    __ARP_HANDLE          ArpHandle        = {0};
    cJSON*                pJsonArpArry     = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON*                pRecvjRes        = LW_NULL;
    cJSON*                pjPage           = LW_NULL;
    INT                   iPage            = 1;                         /*  待显示的页码                */
    INT                   iTotal           = 0;                         /*  总条目                      */
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

    pjSend       = cJSON_CreateObject();                                /*  创建返回 Json 对象          */
    pJsonArpArry = cJSON_CreateArray();                                 /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonArpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
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
     *  调用查询请求接口
     */
    pArpHeader = arpDynamicShow(&ArpHandle, &iRecordTotal);
    if (LW_NULL != pArpHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            PCHAR  pIPData      = LW_NULL;
            CHAR   cMACData[20] = {0};

            pTmpNode = pArpHeader + i;

            pIPData = inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpAddr);
            snprintf(cMACData, sizeof(cMACData),
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    pTmpNode->ucHwAddr[0],
                    pTmpNode->ucHwAddr[1],
                    pTmpNode->ucHwAddr[2],
                    pTmpNode->ucHwAddr[3],
                    pTmpNode->ucHwAddr[4],
                    pTmpNode->ucHwAddr[5]);

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjArp = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
                if(LW_NULL == pjArp) {
                     break;
                }

                cJSON_AddStringToObject(pjArp, WEB_ARP_IPADDR, pIPData);
                cJSON_AddStringToObject(pjArp, WEB_ARP_HWADDR, cMACData);

                cJSON_AddItemToArray(pJsonArpArry, pjArp);              /*  将此条目加入数组            */
            }
            iTotal++;
        }
    }

    if (pArpHeader) {
        free(pArpHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonArpArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonArpArry);
}
/*********************************************************************************************************
** 函数名称: __arpShowAction
** 功能描述: arp信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpShowAction (Webs  *wp)
{
    __PARP_HANDLE         pArpHeader        = LW_NULL;
    __PARP_HANDLE         pTmpNode          = LW_NULL;
    __ARP_HANDLE          ArpHandle         = {0};
    cJSON*                pJsonArpArry      = LW_NULL;
    cJSON*                pjSend            = LW_NULL;
    cJSON*                pRecvjRes         = LW_NULL;
    cJSON*                pjPage            = LW_NULL;
    cJSON*                pjOperType        = LW_NULL;
    INT                   iPage             = 1;                        /*  待显示的页码                */
    INT                   iTotal            = 0;                        /*  总条目                      */
    INT                   iRecordTotal      = 0;
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
    pJsonArpArry = cJSON_CreateArray();                                 /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || LW_NULL == pJsonArpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjPage     = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_OPER_TYPE);
    if (LW_NULL == pjPage || LW_NULL == pjOperType) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, JSON_ARGERR_CODE);    /*  参数出错                    */

        goto __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);                 /*  参数出错                    */

        goto __error;
    }

    strncpy(ArpHandle.cOperType, pjOperType->valuestring, sizeof(ArpHandle.cOperType));

    /*
     * 调用查询请求接口
     */
    pArpHeader = arpShow(&ArpHandle, &iRecordTotal);
    if (LW_NULL != pArpHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            PCHAR  pIPData      = LW_NULL;
            CHAR   cMACData[20] = {0};

            pTmpNode = pArpHeader + i;

            pIPData = inet_ntoa(*(struct in_addr *)&pTmpNode->uiIpAddr);
            snprintf(cMACData, sizeof(cMACData),
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    pTmpNode->ucHwAddr[0],
                    pTmpNode->ucHwAddr[1],
                    pTmpNode->ucHwAddr[2],
                    pTmpNode->ucHwAddr[3],
                    pTmpNode->ucHwAddr[4],
                    pTmpNode->ucHwAddr[5]);

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjArp = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
                if (LW_NULL == pjArp) {
                     break;
                }

                cJSON_AddStringToObject(pjArp, WEB_ARP_IPADDR, pIPData);
                cJSON_AddStringToObject(pjArp, WEB_ARP_HWADDR, cMACData);
                cJSON_AddStringToObject(pjArp, WEB_ARP_STATUS, pTmpNode->cStatus);

                cJSON_AddItemToArray(pJsonArpArry, pjArp);              /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    if (pArpHeader) {
        free(pArpHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonArpArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonArpArry);
}
/*********************************************************************************************************
** 函数名称: __arpConfigAction
** 功能描述: arp 处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes      = LW_NULL;
    cJSON                *pSendjRes      = LW_NULL;
    cJSON                *pjIpAddr       = LW_NULL;
    cJSON                *pjMacAddr      = LW_NULL;
    cJSON                *pjOperType     = LW_NULL;
    __ARP_HANDLE          arpHandle;
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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjIpAddr   = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_IPADDR);
    pjMacAddr  = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_HWADDR);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_OPER_TYPE);
    if (LW_NULL == pjIpAddr   ||
        LW_NULL == pjMacAddr  ||
        LW_NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, JSON_ARGERR_CODE); /*  参数出错                    */

        goto  __error;
    }

    /*
     *  构造信息
     */
    inet_aton(pjIpAddr->valuestring, (struct in_addr *)&(arpHandle.uiIpAddr));
    sscanf(pjMacAddr->valuestring,
           "%x:%x:%x:%x:%x:%x",
           (UINT *)&arpHandle.ucHwAddr[0],
           (UINT *)&arpHandle.ucHwAddr[1],
           (UINT *)&arpHandle.ucHwAddr[2],
           (UINT *)&arpHandle.ucHwAddr[3],
           (UINT *)&arpHandle.ucHwAddr[4],
           (UINT *)&arpHandle.ucHwAddr[5]);
    strncpy(arpHandle.cOperType, pjOperType->valuestring, sizeof(arpHandle.cOperType));

    /*
     * 调用配置请求接口
     */
    iRet = arpConfig(&arpHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __asdefenseEnAction
** 功能描述: arp 防御操作后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asdefenseEnAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;
    cJSON                *jsEn          = LW_NULL;
    UINT8                 ucType        = 0;
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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    jsEn = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_EN);
    if (LW_NULL == jsEn) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    ucType = jsEn->valueint;
    if (0 != ucType && 1 != ucType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    iRet = libAsDefenseHandle(ucType);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __asdefenseInfoGetAction
** 功能描述: 获取 ARP 防御信息后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asdefenseInfoGetAction (Webs *wp)
{
    cJSON               *pRecvjRes                   = LW_NULL;
    cJSON               *pSendjRes                   = LW_NULL;
    UINT8                ucType                      = 0;
    INT                  iPage                       = 0;
    INT                  iPageCount                  = 0;
    cJSON               *pjType                      = LW_NULL;
    cJSON               *pjPage                      = LW_NULL;
    cJSON               *pjPageCount                 = LW_NULL;
    INT32                iStatus                     = 0;
    INT32                iNum                        = 0;
    INT32                iTotal                      = 0;
    __ASDEFENSE_GET      asDefenseGet;
    __PASDEFENSE_INFO    pAsDefenseInfoNode          = LW_NULL;
    PCHAR                pcAsDefenseInfoBuff         = LW_NULL;
    PCHAR                pcBuffTmp                   = LW_NULL;
    cJSON*               pJAsDefenseArry             = LW_NULL;

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
    pJAsDefenseArry = cJSON_CreateArray();                              /*  创建 Json 映射信息数组      */
    if (LW_NULL == pSendjRes || LW_NULL == pJAsDefenseArry) {
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
    pjType      = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_TYPE);
    pjPage      = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_PAGE);
    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_PAGECOUNT);
    if (LW_NULL == pjType ||
        LW_NULL == pjPage ||
        LW_NULL == pjPageCount) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    ucType      = pjType->valueint;
    iPageCount  = pjPageCount->valueint;
    iPage       = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);    /*  参数出错                    */

        goto __error;
    }

    iStatus = libAsDefenseStatusGet();
    if (ARP_SPOOF_DEFENSE_DISABLE == iStatus) {
        cJSON_AddStringToObject(pSendjRes, WEB_ASD_STATUS, "close");
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

        goto  __error;
    } else {
        cJSON_AddStringToObject(pSendjRes, WEB_ASD_STATUS, "open");
    }

    iTotal = libAsDefenseNumGet(ucType);
    if (!iTotal) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

        goto  __error;
    }

    asDefenseGet.ucType      = ucType;
    asDefenseGet.iStartIndex = (iPage - 1) * iPageCount + 1;
    asDefenseGet.iEndIndex   = iPage * iPageCount;

    pcAsDefenseInfoBuff = libAsDefenseInfoGet(&asDefenseGet, &iNum);
    pcBuffTmp           = pcAsDefenseInfoBuff;

    if (0 != iNum && LW_NULL != pcAsDefenseInfoBuff) {
        struct in_addr    uiIpAddr;
        CHAR              cMacBuff[20];

        for(i = 0; i < iNum; i++) {
            cJSON  *pjAsDefenseNode = cJSON_CreateObject();             /*  创建一条映射 Json 对象      */
            if (LW_NULL == pjAsDefenseNode) {
                 break;
            }

            pAsDefenseInfoNode = (__PASDEFENSE_INFO)pcAsDefenseInfoBuff;

            uiIpAddr.s_addr = htonl(pAsDefenseInfoNode->uiIpAddr.addr);
            cJSON_AddStringToObject(pjAsDefenseNode, WEB_ASD_IP, inet_ntoa(uiIpAddr));

            bzero(cMacBuff, sizeof(cMacBuff));
            sprintf(cMacBuff,
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    pAsDefenseInfoNode->ucTrueMacAddr[0],
                    pAsDefenseInfoNode->ucTrueMacAddr[1],
                    pAsDefenseInfoNode->ucTrueMacAddr[2],
                    pAsDefenseInfoNode->ucTrueMacAddr[3],
                    pAsDefenseInfoNode->ucTrueMacAddr[4],
                    pAsDefenseInfoNode->ucTrueMacAddr[5]);

            cJSON_AddStringToObject(pjAsDefenseNode, WEB_ASD_OLD_MAC, cMacBuff);

            bzero(cMacBuff, sizeof(cMacBuff));
            sprintf(cMacBuff,
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    pAsDefenseInfoNode->ucConMacAddr[0],
                    pAsDefenseInfoNode->ucConMacAddr[1],
                    pAsDefenseInfoNode->ucConMacAddr[2],
                    pAsDefenseInfoNode->ucConMacAddr[3],
                    pAsDefenseInfoNode->ucConMacAddr[4],
                    pAsDefenseInfoNode->ucConMacAddr[5]);

            cJSON_AddStringToObject(pjAsDefenseNode, WEB_ASD_NEW_MAC, cMacBuff);
            cJSON_AddStringToObject(pjAsDefenseNode, WEB_ASD_FLAG,
                                   (ARP_NORMAL_FLAG == pAsDefenseInfoNode->ucFlag) ?
                                    ARP_NORMAL_STR : ((ARP_CONFLICT_FLAG == pAsDefenseInfoNode->ucFlag) ?
                                    ARP_CONFLICT_STR : ARP_ATTACK_STR));

            cJSON_AddItemToArray(pJAsDefenseArry, pjAsDefenseNode);     /*  将此条目加入数组            */

            pcAsDefenseInfoBuff += sizeof(__ASDEFENSE_INFO);
        }

        cJSON_AddItemToObject(pSendjRes, ARRAY_NAME, pJAsDefenseArry);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

        goto  __error;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    if (NULL != pcBuffTmp) {
        free(pcBuffTmp);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    cJSON_AddNumberToObject(pSendjRes, WEB_TOTAL, iTotal);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJAsDefenseArry);
}
/*********************************************************************************************************
** 函数名称: __arpActionDefine
** 功能描述: arp 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpActionDefine (VOID)
{
    websDefineAction("arpDynamicShowAction",   __arpDynamicShowAction);
    websDefineAction("arpShowAction",          __arpShowAction);
    websDefineAction("arpConfigAction",        __arpConfigAction);
    websDefineAction("asdefenseEnAction",      __asdefenseEnAction);
    websDefineAction("asdefenseInfoGetAction", __asdefenseInfoGetAction);
}

/*********************************************************************************************************
** 函数名称: arpActionInit
** 功能描述: arp 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpActionInit (VOID)
{
    __arpActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
