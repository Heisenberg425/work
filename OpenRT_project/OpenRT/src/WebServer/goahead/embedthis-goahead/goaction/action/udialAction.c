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
** 文   件   名: udialAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 15 日
**
** 描        述: OpenRT 拨号处理后台接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "udial_info.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "udial_info.h"
#include "intada_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
#define JSON_SUCCESS_CODE     (200)                                     /*  成功返回Json code值         */
#define JSON_FAILED_CODE      (500)                                     /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: udial
*********************************************************************************************************/
#define WEB_UDIAL_INDEX       "webindex"                                /*  Json字段：网口号            */
#define WEB_UDIAL_NETIF       "webnetif"                                /*  Json字段：前端网口名        */
#define WEB_UDIAL_ENNAME      "enname"                                  /*  Json字段：网口名            */
#define WEB_UDIAL_ACCOUNT     "account"                                 /*  Json字段：账号              */
#define WEB_UDIAL_PASSWD      "passwd"                                  /*  Json字段：密码              */
#define WEB_UDIAL_HANDLE      "handle"                                  /*  Json字段：处理方式          */
#define WEB_UDIAL_STATUS      "status"                                  /*  Json字段：状态              */
/*********************************************************************************************************
** 函数名称: __udialHandleAction
** 功能描述: 拨号处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialHandleAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};
    CHAR                  cUdialStatus[15]            = {0};

    PCHAR                 pcAccount                   = LW_NULL;
    PCHAR                 pcPassWd                    = LW_NULL;
    PCHAR                 pcWebNetif                  = LW_NULL;
    INT                   iHandle                     = -1;

    cJSON                *pjAccount                   = LW_NULL;
    cJSON                *pjPassWd                    = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;
    cJSON                *pjHandle                    = LW_NULL;

    INT                   iRet                        = PX_ERROR;
    CHAR                  cLogBuff[30]                = {0};

    __UDIAL_HANDLE        udialHandleInfo;

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
    pjAccount  = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ACCOUNT);
    pjPassWd   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_PASSWD);
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_ENNAME);
    pjHandle   = cJSON_GetObjectItem(pRecvjRes, WEB_UDIAL_HANDLE);
    if (LW_NULL == pjAccount  ||
        LW_NULL == pjPassWd   ||
        LW_NULL == pjWebNetif ||
        LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcAccount  = pjAccount->valuestring;
    pcPassWd   = pjPassWd->valuestring;
    pcWebNetif = pjWebNetif->valuestring;
    iHandle    = pjHandle->valueint;

#if 0
    iRet = libIntAdaGetNetif(pcWebNetif,                                /*  获取对应的系统网口名称      */
                            cNetIfName,
                            sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }
#else
    /*
     *  前端改起来太繁琐，因此先还是使用 en 来通信
     */
    strncpy(cNetIfName, pcWebNetif, sizeof(cNetIfName));
#endif
    /*
     *  记录操作日志
     */
    snprintf(cLogBuff, sizeof(cLogBuff), "%s pppoe udial\n", 1 == iHandle ? "start" : "delete");
    operLogInsert(wp, OPEN_RT_UDAIL, cLogBuff, 0);

    if (1 == iHandle) {                                                 /*  启动拨号                    */
        iRet = libUdialStartHandle(pcAccount, pcPassWd, cNetIfName);
    } else if (0 == iHandle) {                                          /*  删除拨号                    */
        iRet = libUdialDelHandle(cNetIfName);
    } else if (2 == iHandle) {                                          /*  获取拨号信息                */
        bzero(&udialHandleInfo, sizeof(udialHandleInfo));
        strncpy(udialHandleInfo.cNetifname, cNetIfName, sizeof(udialHandleInfo.cNetifname));
        iRet = libUdialInfoGet(&udialHandleInfo);

        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_ACCOUNT, udialHandleInfo.cUsrname);
        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_PASSWD,  udialHandleInfo.cPassword);

        switch (udialHandleInfo.iUdialStatus) {
        case NET_EVENT_PPP_DEAD:
            strncpy(cUdialStatus, "dead", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_INIT:
            strncpy(cUdialStatus, "init", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_AUTH:
            strncpy(cUdialStatus, "auth", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_RUN:
            strncpy(cUdialStatus, "run", sizeof(cUdialStatus));
            break;

        case NET_EVENT_PPP_DISCONN:
            strncpy(cUdialStatus, "disconnect", sizeof(cUdialStatus));
            break;

        default:
            strncpy(cUdialStatus, "error", sizeof(cUdialStatus));
            break;
        }

        cJSON_AddStringToObject(pSendjRes, WEB_UDIAL_STATUS, cUdialStatus);
    }

    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    } else if (1 == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 300);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** 函数名称: __udialActionDefine
** 功能描述: 拨号后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialActionDefine (VOID)
{
    websDefineAction("udialHandleAction", __udialHandleAction);
}

/*********************************************************************************************************
** 函数名称: udialActionInit
** 功能描述: 拨号后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID udialActionInit (VOID)
{
    __udialActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
