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
** 文   件   名: dnsAction.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 30 日
**
** 描        述: dns配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "dns_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  返回的状态码                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json 结构创建失败时使用     */
#define WEB_DNS_PRIMARY                 "primary"                       /*  首选 DNS                    */
#define WEB_DNS_SECOND                  "second"                        /*  备用 DNS                    */
#define DNS_DEFAULT                     "8.8.8.8"                       /*  默认 DNS                    */
/*********************************************************************************************************
 宏定义
*********************************************************************************************************/
#define DNS_GETOBJSTRING(p, key, defVal) \
                (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                 defVal : cJSON_GetObjectItem(p, key)->valuestring      /*  获取 Json 对象中内容        */
#define DNS_GETOBJINTER(p, key, defVal) \
                (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                 defVal : cJSON_GetObjectItem(p, key)->valueint         /*  获取 Json 对象中内容        */

/*********************************************************************************************************
** 函数名称: __dnsShowAction
** 函数功能: 请求显示 DNS 规则
** 输    入: wp           服务器句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __dnsShowAction (Webs  *wp)
{
    __PDNS_HANDLE  pDnsHandle    = LW_NULL;
    cJSON         *pjSend        = LW_NULL;
    INT            iRecordTotal  = 0;

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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);

        return;
    }

    pDnsHandle = dnsShow(&iRecordTotal);
    if (LW_NULL == pDnsHandle) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto __error;
    }

    cJSON_AddStringToObject(pjSend, WEB_DNS_PRIMARY, pDnsHandle->primaryDNS);
    cJSON_AddStringToObject(pjSend, WEB_DNS_SECOND,  pDnsHandle->secondDNS);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    dnsInfoFree(pDnsHandle);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __dnsConfigAction
** 功能描述: DNS 处理后台接口
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dnsConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;
    cJSON                *pjPrimDNS     = LW_NULL;
    cJSON                *pjSecDNS      = LW_NULL;
    __DNS_HANDLE          dnsHandle;

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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);
        operLogInsert(wp, OPEN_RT_DNS, "config dns init failed!\n", 0);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "config dns, parser request data failed!\n", 0);

        goto  __error;
    }

    bzero(&dnsHandle, sizeof(__DNS_HANDLE));

    pjPrimDNS = cJSON_GetObjectItem(pRecvjRes, WEB_DNS_PRIMARY);
    if (NULL == pjPrimDNS || '\0' == (pjPrimDNS->valuestring)[0]) {
        strncpy(dnsHandle.primaryDNS, DNS_DEFAULT, sizeof(dnsHandle.primaryDNS));
    } else {
        strncpy(dnsHandle.primaryDNS, pjPrimDNS->valuestring, sizeof(dnsHandle.primaryDNS));
    }

    pjSecDNS = cJSON_GetObjectItem(pRecvjRes, WEB_DNS_SECOND);
    if (NULL == pjSecDNS || '\0' == (pjSecDNS->valuestring)[0]) {
        strncpy(dnsHandle.secondDNS, DNS_DEFAULT, sizeof(dnsHandle.secondDNS));
    } else {
        strncpy(dnsHandle.secondDNS, pjSecDNS->valuestring, sizeof(dnsHandle.secondDNS));
    }

    if (ERROR_NONE != dnsConfig(&dnsHandle)) {                          /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "config dns failed!\n", 0);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
        operLogInsert(wp, OPEN_RT_DNS, "config dns success!\n", 0);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __dnsActionDefine
** 功能描述: DNS 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dnsActionDefine (VOID)
{
    websDefineAction("dnsShowAction",   __dnsShowAction);
    websDefineAction("dnsConfigAction", __dnsConfigAction);
}
/*********************************************************************************************************
** 函数名称: dnsActionInit
** 功能描述: DNS 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dnsActionInit (VOID)
{
    __dnsActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
