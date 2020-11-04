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
** 文   件   名: timeaction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统时间配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "time_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: time 配置及路由信息显示
*********************************************************************************************************/
#define WEB_TIME_DATE             "date"                                /*  Json字段：date              */
#define WEB_TIME_TIME             "time"                                /*  Json字段：接口名称          */
/*********************************************************************************************************
** 函数名称: __timeShowAction
** 功能描述: 时间显示后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __timeShowAction (Webs *wp)
{
    __PTIME_HANDLE  pTimeHeader  = NULL;
    __TIME_HANDLE   timeHandle   = {{0}};
    cJSON          *pjSend       = NULL;
    INT             iRecordTotal = 0;

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
    if(NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    /*
     * 调用查询请求接口并设置返回的 Json 内容
     */
    pTimeHeader = timeShow(&timeHandle, &iRecordTotal);
    if (NULL != pTimeHeader) {
        cJSON_AddStringToObject(pjSend, WEB_TIME_DATE, pTimeHeader->cDate);
        cJSON_AddStringToObject(pjSend, WEB_TIME_TIME, pTimeHeader->cTime);
    }

    if (pTimeHeader) {
        free(pTimeHeader);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __timeConfigAction
** 功能描述: time 设置处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __timeConfigAction (Webs *wp)
{
    cJSON          *pRecvjRes = NULL;
    cJSON          *pSendjRes = NULL;
    cJSON          *pjDate    = NULL;
    cJSON          *pjTime    = NULL;
    __TIME_HANDLE   timeHandle;
    INT             iRet;

    lib_bzero(&timeHandle, sizeof(__TIME_HANDLE));

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
    if (NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取Json数据内容
     */
    pjDate = cJSON_GetObjectItem(pRecvjRes, WEB_TIME_DATE);
    pjTime = cJSON_GetObjectItem(pRecvjRes, WEB_TIME_TIME);
    if (NULL == pjDate || NULL == pjTime) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(timeHandle.cDate, pjDate->valuestring, sizeof(timeHandle.cDate));
    strncpy(timeHandle.cTime, pjTime->valuestring, sizeof(timeHandle.cTime));

    /*
     * 调用配置请求接口
     */
    iRet = tmConfig(&timeHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
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
** 函数名称: __timeActionDefine
** 功能描述: time后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __timeActionDefine (VOID)
{
    websDefineAction("timeShowAction",   __timeShowAction);
    websDefineAction("timeConfigAction", __timeConfigAction);
}
/*********************************************************************************************************
** 函数名称: timeActionInit
** 功能描述: time action 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  timeActionInit (VOID)
{
    __timeActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
