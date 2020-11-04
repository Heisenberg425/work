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
** 文   件   名: dhcpAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: OpenRT DHCP 后台处理接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "dhcp_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
#define JSON_ARGERR_CODE      (100)                                     /*  参数错误返回Json code值     */
#define JSON_SUCCESS_CODE     (200)                                     /*  成功返回Json code值         */
#define JSON_FAILED_CODE      (500)                                     /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: DHCP 客户端处理
*********************************************************************************************************/
#define WEB_DHCPC_NETNAME     "netifname"                               /*  Json字段：网卡名称          */
#define WEB_DHCPC_HANDLE      "handle"                                  /*  Json字段：使能 / 禁能       */
/*********************************************************************************************************
** 函数名称: __dhcpClientAction
** 功能描述: DHCP 客户端后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpClientAction (Webs *wp)
{
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pRecvjRes        = LW_NULL;
    CHAR                 *pcNetifName      = LW_NULL;
    CHAR                 *pcHandle         = LW_NULL;
    cJSON                *pjNetifName      = LW_NULL;
    cJSON                *pjHandle         = LW_NULL;
    CHAR                  cDhcpHandle      = PX_ERROR;
    INT                   iRet             = PX_ERROR;

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
    pjNetifName = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPC_NETNAME);
    pjHandle    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPC_HANDLE);
    if (LW_NULL == pjNetifName || LW_NULL == pjHandle) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    pcNetifName  = pjNetifName->valuestring;
    pcHandle     = pjHandle->valuestring;

    if (!strcmp("enable", pcHandle)) {
        cDhcpHandle = DHCP_CLIENT_ENABLE;
    } else if (!strcmp("disable", pcHandle)) {
        cDhcpHandle = DHCP_CLIENT_DISABLE;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __error;
    }

    iRet = libDhcpClientHandle(pcNetifName, cDhcpHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __dhcpActionDefine
** 功能描述: DHCP 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpActionDefine (VOID)
{
    websDefineAction("dhcpClientAction", __dhcpClientAction);
}
/*********************************************************************************************************
** 函数名称: dhcpActionInit
** 功能描述: DHCP 后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpActionInit (VOID)
{
    __dhcpActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
