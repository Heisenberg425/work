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
** 文   件   名: vndAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 27 日
**
** 描        述: OpenRT 虚拟网卡后台处理接口
** 2018.08.16  修复了 sscanf 跨内存写覆盖产生的BUG.
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "sys/un.h"
#include "vnd_info.h"
#include "../webHandle.h"
#include "libinterface_config.h"
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
#define JSON_SUCCESS_CODE     (200)                                     /*  成功返回Json code值         */
#define JSON_FAILED_CODE      (500)                                     /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: kidvpn 处理
*********************************************************************************************************/
#define WEB_VND_VNDID       "vndid"                                     /*  Json字段：虚拟网卡 id       */
#define WEB_VND_IPADDR      "ipaddr"                                    /*  Json字段：IP 地址           */
#define WEB_VND_NETMASK     "netmask"                                   /*  Json字段：子网掩码          */
#define WEB_VND_GATEWAY     "gateway"                                   /*  Json字段：网关地址          */
#define WEB_VND_MTU         "mtu"                                       /*  Json字段：mtu 值            */
#define WEB_VND_MAC         "mac"                                       /*  Json字段：MAC 地址          */
/*********************************************************************************************************
** 函数名称: __vndHandleAction
** 功能描述: 虚拟网卡处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;

    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcMtu            = LW_NULL;
    CHAR                 *pcNetMask        = LW_NULL;
    CHAR                 *pcGateWay        = LW_NULL;
    CHAR                 *pcMac            = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjMtu            = LW_NULL;
    cJSON                *pjNetMask        = LW_NULL;
    cJSON                *pjGateWay        = LW_NULL;
    cJSON                *pjMac            = LW_NULL;

    __VND_HANDLE          vndHandleInfo;
    INT                   iVndID           = 0;
    UINT                  uiTmpHwAddr[6]   = {0};

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
    pjIpAddr  = cJSON_GetObjectItem(pRecvjRes, WEB_VND_IPADDR);
    pjNetMask = cJSON_GetObjectItem(pRecvjRes, WEB_VND_NETMASK);
    pjGateWay = cJSON_GetObjectItem(pRecvjRes, WEB_VND_GATEWAY);
    pjMtu     = cJSON_GetObjectItem(pRecvjRes, WEB_VND_MTU);
    pjMac     = cJSON_GetObjectItem(pRecvjRes, WEB_VND_MAC);
    if (LW_NULL == pjIpAddr  ||
        LW_NULL == pjMtu     ||
        LW_NULL == pjNetMask ||
        LW_NULL == pjGateWay ||
        LW_NULL == pjMac) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcIpAddr  = pjIpAddr->valuestring;
    pcNetMask = pjNetMask->valuestring;
    pcGateWay = pjGateWay->valuestring;
    pcMtu     = pjMtu->valuestring;
    pcMac     = pjMac->valuestring;

    /*
     *  构造信息
     */
    inet_aton(pcIpAddr, (struct in_addr *)&(vndHandleInfo.uiIpAddr));
    inet_aton(pcNetMask, (struct in_addr *)&(vndHandleInfo.uiNetMask));
    inet_aton(pcGateWay, (struct in_addr *)&(vndHandleInfo.uiGateWay));
    vndHandleInfo.iMtu = atoi(pcMtu);

    sscanf(pcMac, "%02x:%02x:%02x:%02x:%02x:%02x",
           &uiTmpHwAddr[0], &uiTmpHwAddr[1],
           &uiTmpHwAddr[2], &uiTmpHwAddr[3],
           &uiTmpHwAddr[4], &uiTmpHwAddr[5]);

    vndHandleInfo.ucHwAddr[0] = uiTmpHwAddr[0];
    vndHandleInfo.ucHwAddr[1] = uiTmpHwAddr[1];
    vndHandleInfo.ucHwAddr[2] = uiTmpHwAddr[2];
    vndHandleInfo.ucHwAddr[3] = uiTmpHwAddr[3];
    vndHandleInfo.ucHwAddr[4] = uiTmpHwAddr[4];
    vndHandleInfo.ucHwAddr[5] = uiTmpHwAddr[5];

    iVndID = libVndCreate(&vndHandleInfo);
    if (RET_ARG_ERROR == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */
    } else if (RET_HANDLE_FAIL == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
    } else {                                                            /*  返回的值是 vnd ID           */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_VND_VNDID, iVndID);          /*  返回 vnd ID                 */

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** 函数名称: __vndActionDefine
** 功能描述: 虚拟网卡后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndActionDefine (VOID)
{
    websDefineAction("vndHandleAction", __vndHandleAction);
}

/*********************************************************************************************************
** 函数名称: vndActionInit
** 功能描述: 虚拟网卡后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vndActionInit (VOID)
{
    __vndActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
