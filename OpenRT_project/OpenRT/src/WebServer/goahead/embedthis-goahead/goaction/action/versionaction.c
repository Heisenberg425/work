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
** 文   件   名: versionaction.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 8 日
**
** 描        述: version后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "version_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */
/*********************************************************************************************************
  Jason key: version信息显示
*********************************************************************************************************/
#define WEB_VERSION_INFO          "version"                             /*  Json字段：系统版本          */
/*********************************************************************************************************
** 函数名称: __versionShowAction
** 功能描述: version信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __versionShowAction (Webs  *wp)
{
    __PVERSION_HANDLE     pVersionHeader    = LW_NULL;
    __VERSION_HANDLE      VersionHandle;
    cJSON*                pjSend            = LW_NULL;
    INT                   iRecordTotal      = 0;

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
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);
        return;
    }

    /*
     *  调用查询请求接口
     */
    pVersionHeader = versionShow(&VersionHandle, &iRecordTotal);

    cJSON_AddStringToObject(pjSend, WEB_VERSION_INFO, pVersionHeader->cVer);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}

/*********************************************************************************************************
** 函数名称: __versionActionDefine
** 功能描述: version后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __versionActionDefine (VOID)
{
    websDefineAction("versionShowAction", __versionShowAction);
}

/*********************************************************************************************************
** 函数名称: versionActionInit
** 功能描述: version后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  versionActionInit (VOID)
{
    __versionActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
