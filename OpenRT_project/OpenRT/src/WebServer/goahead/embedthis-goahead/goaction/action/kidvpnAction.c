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
** 文   件   名: kidvpnAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 27 日
**
** 描        述: OpenRT VPN 后台处理接口
** 2018.08.16  根据新版本 KidVPN 修改代码.
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "kidvpn_info.h"
#include "vnd_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
#define JSON_SUCCESS_CODE     (200)                                     /*  成功返回Json code值         */
#define JSON_FAILED_CODE      (500)                                     /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: kidvpn 处理
*********************************************************************************************************/
#define WEB_VPN_VNDID         "vndid"                                   /*  Json字段：虚拟网卡 id       */
#define WEB_VPN_IPADDR        "ipaddr"                                  /*  Json字段：IP 地址           */
#define WEB_VPN_MTU           "mtu"                                     /*  Json字段：mtu 值            */
#define WEB_VPN_PORT          "port"                                    /*  Json字段：端口信息          */
#define WEB_VPN_HANDLE        "handle"                                  /*  Json字段：处理方式          */
#define WEB_VPN_STATUS        "status"                                  /*  Json字段：vpn 连接状态      */
#define WEB_VPN_PASSWD        "passwd"                                  /*  Json字段：vpn 启动密钥      */
/*********************************************************************************************************
  Jason key: aes key 生成
*********************************************************************************************************/
#define WEB_VPN_AES_BIT       "bit"                                     /*  Json字段：bit               */
#define WEB_VPN_AES_KEY       "aeskey"                                  /*  Json字段：key               */
/*********************************************************************************************************
  Jason key: 获取信息
*********************************************************************************************************/
#define WEB_VPN_PAGE          "page"                                    /*  Json字段：当前待显示的页码  */
#define WEB_PAGECOUNT         "pagecount"                               /*  Json字段：每页个数          */
#define WEB_VPN_TOTAL         "total"                                   /*  Json字段：总共的条目数      */
#define WEB_VPN_SERIP         "serip"                                   /*  Json字段：服务器 IP 地址    */
#define WEB_VPN_POSITION      "position"                                /*  Json字段：IP 对应的地理位置 */
#define WEB_VPN_LOCIP         "locip"                                   /*  Json字段：本地 IP 地址      */
#define WEB_VPN_TYPE          "type"                                    /*  Json字段：类型              */
#define WEB_VPN_NETMASK       "netmask"                                 /*  Json字段：子网掩码          */
#define WEB_VPN_GATEWAY       "gateway"                                 /*  Json字段：网关地址          */
#define WEB_VPN_MAC           "mac"                                     /*  Json字段：MAC 地址          */
#define WEB_VPN_LINK_IP       "cliip"                                   /*  Json字段：IP 地址           */
#define WEB_VPN_LINK_MAC      "climac"                                  /*  Json字段：MAC 地址          */
#define ARRAY_NAME            "data"                                    /*  Json字段：数据信息          */
#define WEB_VPN_TYPE          "type"                                    /*  Json字段：VPN 类型          */
/*********************************************************************************************************
  Jason key: 地理位置信息
*********************************************************************************************************/
#define WEB_VPN_SRC_IP         "cli_ip"                                 /*  Json字段：当前待显示的页码  */
#define WEB_VPN_SRC_POS        "cli_pos"                                /*  Json字段：总共的条目数      */
#define WEB_VPN_DST_IP         "ser_ip"                                 /*  Json字段：服务器 IP 地址    */
#define WEB_VPN_DST_POS        "ser_pos"                                /*  Json字段：本地 IP 地址      */
#define KV_PORT_DEFAULT        (10088)                                  /*  kidvpn 默认端口号           */
/*********************************************************************************************************
** 函数名称: __kidvpnAesKeyGenAction
** 功能描述: KidVPN 生成 AES KEY 后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeyGenAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcBitBuf         = LW_NULL;
    cJSON                *pjBitBuf         = LW_NULL;
    INT                   iRet             = PX_ERROR;
    CHAR                  cAesKey[70]      = {0};                       /*  最大 64 字节长度            */

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
    pjBitBuf = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_BIT);
    if (LW_NULL == pjBitBuf) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    pcBitBuf = pjBitBuf->valuestring;

    iRet = libKidVPNAesKeyGen(cAesKey, atoi(pcBitBuf), sizeof(cAesKey));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

    cJSON_AddStringToObject(pSendjRes, WEB_VPN_AES_KEY, cAesKey);       /*  返回 KEY                    */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnAesKeySaveAction
** 功能描述: KidVPN 保存 AES KEY 到本地后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeySaveAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;
    cJSON                *pjVndId          = LW_NULL;
    __AES_KEY_HANDLE      aesKeyHandle;
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
    pjAesKey = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
    pjVndId  = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjAesKey || LW_NULL == pjVndId) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    bzero(&aesKeyHandle, sizeof(aesKeyHandle));
    pcAesKey             = pjAesKey->valuestring;
    aesKeyHandle.iVndID  = pjVndId->valueint;
    strncpy(aesKeyHandle.cAesKey, pcAesKey, sizeof(aesKeyHandle.cAesKey));

    iRet = libKidVPNAesKeySave(&aesKeyHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnAesKeyGetAction
** 功能描述: KidVPN 获取 AES KEY 后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAesKeyGetAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjVndId          = LW_NULL;
    INT                   iVndID           = 0;
    CHAR                  cAesKey[70]      = {0};
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
        webBackHandle(wp, &_G_ErrorJRes);                               /*  Json 结构创建失败,使用字符串*/

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
    pjVndId = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjVndId) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iVndID = pjVndId->valueint;

    iRet = libKidVPNAesKeyGet(cAesKey, iVndID, sizeof(cAesKey));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }
    cJSON_AddStringToObject(pSendjRes, WEB_VPN_AES_KEY, cAesKey);       /*  返回 KEY                    */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnInfoGetAction
** 功能描述: KidVPN 获取信息后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf     = LW_NULL;
    PCHAR                 pcKidVPNBufTmp  = LW_NULL;

    PCHAR                 pcVndBuf        = LW_NULL;
    PCHAR                 pcVndBufTmp     = LW_NULL;

    __PKIDVPN_DB_INFO     pcKidVPNInfo    = LW_NULL;
    __PVND_HANDLE         pcVndInfo       = LW_NULL;

    INT                   iVndNum          = 0;
    INT                   iKidVpnNum       = 0;

    CHAR                  cMacBuf[20]      = {0};
    PCHAR                 pcIpAddr         = LW_NULL;
    PCHAR                 pcHandleType     = LW_NULL;
    PCHAR                 pcStatus         = LW_NULL;

    INT                   iPage            = 1;                         /*  待显示的页码                */
    INT                   iTotal           = 0;                         /*  总条目                      */
    cJSON*                pJsonVpnInfoArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjPages          = LW_NULL;
    INT                   i;
    INT                   j;

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

    pjSend             = cJSON_CreateObject();                          /*  创建返回 Json 对象          */
    pJsonVpnInfoArry   = cJSON_CreateArray();                           /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnInfoArry) {
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
    pjPages = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PAGE);
    if (LW_NULL == pjPages) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     *  获取 VPN 、VND 信息
     */
    pcVndBuf       = libVndGetInfo(&iVndNum);
    pcKidVPNBuf    = libKidVPNGetInfo(&iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    pcVndBufTmp    = pcVndBuf;

    pcKidVPNInfo = (__PKIDVPN_DB_INFO)pcKidVPNBuf;
    pcVndInfo    = (__PVND_HANDLE)pcVndBuf;

    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if (((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) ||
                 TYPE_ADD_VPN_S == pcKidVPNInfo->kidvpnHandle.cHandleType) {

                cJSON*  pjVpnInfo = cJSON_CreateObject();               /*  创建一条映射 Json 对象      */
                if(LW_NULL == pjVpnInfo) {
                     break;
                }

                pcIpAddr     = inet_ntoa(*(struct in_addr *)&pcKidVPNInfo->kidvpnHandle.uiIpAddr);
                pcStatus     = pcKidVPNInfo->kidvpnHandle.cStatus ? "ON" : "OFF";
                pcHandleType = (TYPE_ADD_VPN_S == pcKidVPNInfo->kidvpnHandle.cHandleType) ?
                                                                        "servervpn" : "clientvpn";

                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SERIP,    pcIpAddr);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_POSITION, pcKidVPNInfo->kidvpnHandle.cPos);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_MTU,      pcKidVPNInfo->kidvpnHandle.iMtu);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_VNDID,    pcKidVPNInfo->kidvpnHandle.iVndID);
                cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_PORT,     pcKidVPNInfo->kidvpnHandle.iPort);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_TYPE,     pcHandleType);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_STATUS,   pcStatus);
                cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_PASSWD,   pcKidVPNInfo->kidvpnHandle.cPassWd);

                if (iVndNum && LW_NULL != pcVndBuf) {
                    for (j = 0; j < iVndNum; j++) {                     /*  寻找对应的 vnd 信息         */

                        /*
                         *  找到对应的 VND 信息
                         */
                        if (pcVndInfo->iVndId == pcKidVPNInfo->kidvpnHandle.iVndID) {
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_LOCIP,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiIpAddr));
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_NETMASK,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiNetMask));
                            cJSON_AddStringToObject(pjVpnInfo,
                                                    WEB_VPN_GATEWAY,
                                                    inet_ntoa(*(struct in_addr *)&pcVndInfo->uiGateWay));

                            sprintf(cMacBuf,
                                    "%02x:%02x:%02x:%02x:%02x:%02x",
                                    pcVndInfo->ucHwAddr[0],
                                    pcVndInfo->ucHwAddr[1],
                                    pcVndInfo->ucHwAddr[2],
                                    pcVndInfo->ucHwAddr[3],
                                    pcVndInfo->ucHwAddr[4],
                                    pcVndInfo->ucHwAddr[5]);

                            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_MAC, cMacBuf);
                        }

                        pcVndBuf += sizeof(__VND_HANDLE);
                        pcVndInfo = (__PVND_HANDLE)(pcVndBuf);
                    }
                }

                pcVndBuf  = pcVndBufTmp;
                pcVndInfo = (__PVND_HANDLE)pcVndBuf;

                cJSON_AddItemToArray(pJsonVpnInfoArry, pjVpnInfo);      /*  将此条目加入数组            */
            }

            iTotal++;
            pcKidVPNBuf += sizeof(__KIDVPN_DB_INFO);
            pcKidVPNInfo = (__PKIDVPN_DB_INFO)(pcKidVPNBuf);
        }
    }

    if (NULL != pcKidVPNBufTmp) {
        free(pcKidVPNBufTmp);
    }

    if (NULL != pcVndBufTmp) {
        free(pcVndBufTmp);

    }

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnInfoArry);

    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_VPN_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVpnInfoArry);
}
/*********************************************************************************************************
** 函数名称: __kidvpnLinkInfoGetAction
** 功能描述: KidVPN 获取连接信息后台接口
** 输　入  : wp     服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf      = LW_NULL;
    PCHAR                 pcKidVPNBufTmp   = LW_NULL;
    __PKIDVPN_LINK        pcKidVPNInfo     = LW_NULL;
    __VPN_INFO_HANDLE     vpnInfoHandle;
    INT                   iKidVpnNum       = 0;
    INT                   iPage            = 1;                         /*  待显示的页码                */
    INT                   iTotal           = -1;                        /*  总条目                      */
    INT                   iPageCount       = 0;

    cJSON*                pJsonVpnLinkArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pjPages          = LW_NULL;
    cJSON                *pjPageCount      = LW_NULL;
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
    pJsonVpnLinkArry = cJSON_CreateArray();                             /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnLinkArry) {
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
    pjPages = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PAGE);
    if (LW_NULL == pjPages) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    pjPageCount = cJSON_GetObjectItem(pRecvjRes, WEB_PAGECOUNT);
    if (LW_NULL == pjPageCount) {
        iPageCount = 10;
    } else {
        iPageCount = pjPageCount->valueint;
    }

    iPage = pjPages->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    vpnInfoHandle.llStartIndex = (iPage - 1) * iPageCount + 1;
    vpnInfoHandle.llEndIndex   = iPage * iPageCount;

    /*
     *  获取 VPN 连接信息数目
     */
    iTotal = libVpnLinkNumGet();
    if (-1 == iTotal) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 VPN 连接信息
     */
    pcKidVPNBuf    = libKidVPNLinkInfoGet(&vpnInfoHandle, &iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {
            cJSON*  pjVpnInfo = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjVpnInfo) {
                 break;
            }

            pcKidVPNInfo = (__PKIDVPN_LINK)(pcKidVPNBuf);

            cJSON_AddNumberToObject(pjVpnInfo, WEB_VPN_VNDID,    pcKidVPNInfo->iVndId);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_LINK_IP,  pcKidVPNInfo->cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_POSITION, pcKidVPNInfo->cPos);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_LINK_MAC, pcKidVPNInfo->cMac);

            cJSON_AddItemToArray(pJsonVpnLinkArry, pjVpnInfo);          /*  将此条目加入数组            */

            pcKidVPNBuf += sizeof(__KIDVPN_LINK);
        }

        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnLinkArry);
    }

    free(pcKidVPNBufTmp);

    cJSON_AddNumberToObject(pjSend, WEB_VPN_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnPosInfoGetAction
** 功能描述: KidVPN 获取连接地理位置信息后台接口
** 输　入  : wp     服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnPosInfoGetAction (Webs *wp)
{
    PCHAR                 pcKidVPNBuf      = LW_NULL;
    PCHAR                 pcKidVPNBufTmp   = LW_NULL;
    __PKIDVPN_POS         pcKidVPNInfo     = LW_NULL;
    INT                   iKidVpnNum       = 0;
    INT                   iTotal           = 0;
    cJSON*                pJsonVpnLinkArry = LW_NULL;
    cJSON*                pjSend           = LW_NULL;
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

    pjSend             = cJSON_CreateObject();                          /*  创建返回 Json 对象          */
    pJsonVpnLinkArry   = cJSON_CreateArray();                           /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVpnLinkArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     *  获取 VPN 连接信息
     */
    pcKidVPNBuf    = libKidVPNPosInfoGet(&iKidVpnNum);
    pcKidVPNBufTmp = pcKidVPNBuf;
    pcKidVPNInfo   = (__PKIDVPN_POS)pcKidVPNBuf;
    if (iKidVpnNum && LW_NULL != pcKidVPNBuf) {
        for (i = 0; i < iKidVpnNum; i++) {

            /*
             *  把需要显示的内容构造成 Json 格式
             */
            cJSON*  pjVpnInfo = cJSON_CreateObject();                   /*  创建一条映射 Json 对象      */
            if(LW_NULL == pjVpnInfo) {
                break;
            }

            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SRC_IP,  pcKidVPNInfo->vpnSrc.cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_SRC_POS, pcKidVPNInfo->vpnSrc.cPos);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_DST_IP,  pcKidVPNInfo->vpnDst.cIp);
            cJSON_AddStringToObject(pjVpnInfo, WEB_VPN_DST_POS, pcKidVPNInfo->vpnDst.cPos);

            cJSON_AddItemToArray(pJsonVpnLinkArry, pjVpnInfo);          /*  将此条目加入数组            */

            iTotal++;
            pcKidVPNBuf += sizeof(__KIDVPN_POS);
            pcKidVPNInfo = (__PKIDVPN_POS)(pcKidVPNBuf);
        }

        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonVpnLinkArry);
    }

    if (NULL != pcKidVPNBufTmp) {
        free(pcKidVPNBufTmp);
    }

    cJSON_AddNumberToObject(pjSend, WEB_VPN_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __kidvpnCreateAction
** 功能描述: 开启 KidVPN 后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnCreateAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;
    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcMtu            = LW_NULL;
    CHAR                 *pcHandle         = LW_NULL;
    CHAR                 *pcPassWd         = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjMtu            = LW_NULL;
    cJSON                *pjPort           = LW_NULL;
    cJSON                *pjHandle         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;
    cJSON                *pjVndID          = LW_NULL;
    cJSON                *pjPassWd         = LW_NULL;
    INT                   iVndID           = 0;
    INT                   iPort            = 0;

    __KIDVPN_HANDLE       kidvpnHandleInfo;
    __AES_KEY_HANDLE      aesKetInfo;
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
    pjIpAddr = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_IPADDR);
    pjMtu    = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_MTU);
    pjPort   = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PORT);
    pjHandle = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_HANDLE);
    pjVndID  = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    pjPassWd = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PASSWD);
    pjAesKey = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
    if (LW_NULL == pjIpAddr ||
        LW_NULL == pjMtu    ||
        LW_NULL == pjHandle ||
        LW_NULL == pjVndID  ||
        LW_NULL == pjPassWd ||
        LW_NULL == pjAesKey) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    pcIpAddr = pjIpAddr->valuestring;
    pcMtu    = pjMtu->valuestring;
    pcHandle = pjHandle->valuestring;
    iVndID   = pjVndID->valueint;
    pcPassWd = pjPassWd->valuestring;
    if (LW_NULL == pjPort) {
        iPort = KV_PORT_DEFAULT;
    } else {
        iPort = pjPort->valueint;
    }

    pcAesKey = pjAesKey->valuestring;

    /*
     *  构造信息
     */
    bzero(&kidvpnHandleInfo, sizeof(kidvpnHandleInfo));

    if (0 == strcmp(pcHandle, "addServer")) {
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_S;
    } else if (0 == strcmp(pcHandle, "addClient")) {
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_C;
    } else {
        libVndDel(iVndID);
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    kidvpnHandleInfo.iVndID = iVndID;
    kidvpnHandleInfo.iMtu   = atoi(pcMtu);
    kidvpnHandleInfo.iPort  = iPort;

    inet_aton(pcIpAddr, (struct in_addr *)&(kidvpnHandleInfo.uiIpAddr));
    strncpy(kidvpnHandleInfo.cPassWd, pcPassWd, sizeof(kidvpnHandleInfo.cPassWd));

    bzero(&aesKetInfo, sizeof(aesKetInfo));
    aesKetInfo.iVndID = iVndID;
    strncpy(aesKetInfo.cAesKey, pcAesKey, sizeof(aesKetInfo.cAesKey));

    libKidVPNAesKeySave(&aesKetInfo);
    iRet = libKidVPNCreate(&kidvpnHandleInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
    } else if (RET_HANDLE_OK == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnDelAction
** 功能描述: 删除 KidVPN 后台接口
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnDelAction (Webs *wp)
{
    cJSON                *pRecvjRes = LW_NULL;
    cJSON                *pSendjRes = LW_NULL;
    cJSON                *pjVndID   = LW_NULL;
    INT                   iVndID    = 0;
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
    pjVndID = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_VNDID);
    if (LW_NULL == pjVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iVndID = pjVndID->valueint;

    iRet = libKidVPNDel(iVndID);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
    } else if (RET_HANDLE_OK == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __kidvpnActionDefine
** 功能描述: VPN 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnActionDefine (VOID)
{
    websDefineAction("kidvpnCreateAction",    __kidvpnCreateAction);
    websDefineAction("kidvpnDelAction",       __kidvpnDelAction);
    websDefineAction("kidvpnAesKeyGenAction", __kidvpnAesKeyGenAction);
    websDefineAction("kidvpnAesKeyGetAction", __kidvpnAesKeyGetAction);
    websDefineAction("kidvpnInfoGetAction",   __kidvpnInfoGetAction);
    websDefineAction("kidvpnLinkInfoGetAction", __kidvpnLinkInfoGetAction);
    websDefineAction("kidvpnPosInfoGetAction",  __kidvpnPosInfoGetAction);
    websDefineAction("kidvpnAesKeySaveAction",  __kidvpnAesKeySaveAction);
}
/*********************************************************************************************************
** 函数名称: kidvpnActionInit
** 功能描述: VPN 后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  kidvpnActionInit (VOID)
{
    __kidvpnActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
