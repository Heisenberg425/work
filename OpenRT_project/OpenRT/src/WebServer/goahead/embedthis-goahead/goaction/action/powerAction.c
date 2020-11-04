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
** 文   件   名: powerAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统电源配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "power_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define  CUR_MAX_ID(a, b)          (((a) > (b)) ? (a):(b))              /*  获取最大值                  */
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: power 配置及路由信息显示
*********************************************************************************************************/
#define WEB_POWER_NUMBER           "number"                             /*  Json字段：编号              */
#define WEB_POWER_EVENT_ID         "event_id"                           /*  Json字段：event id          */
#define WEB_POWER_MAX_EVENT_ID     "max_event_id"                       /*  Json字段：max event id      */
#define WEB_POWER_EVENT            "event"                              /*  Json字段：event             */
#define WEB_POWER_DATE             "date"                               /*  Json字段：date              */
#define WEB_POWER_TIME             "time"                               /*  Json字段：time              */
#define WEB_POWER_STATUS           "status"                             /*  Json字段：状态              */
#define WEB_POWER_OPER             "oper_type"                          /*  Json字段：操作类型          */
#define WEB_POWER_DESC             "desc"                               /*  Json字段：说明信息          */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                   "page"                               /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                  "total"                              /*  Json字段：总共的条目数      */
#define ARRAY_NAME                 "data"                               /*  Json字段：内网端口          */
/*********************************************************************************************************
** 函数名称: __powerOffAction
** 功能描述: 电源关闭处理后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __powerOffAction (Webs *wp)
{
    cJSON           *pSendjRes   = NULL;
    __POWER_HANDLE   powerHandle = {0};;

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

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);

    /*
     * 调用电源关闭请求接口
     */
    powerOff(&powerHandle);
}
/*********************************************************************************************************
** 函数名称: __powerRestartAction
** 功能描述: 电源重启处理后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __powerRestartAction (Webs *wp)
{
    cJSON           *pSendjRes   = NULL;
    __POWER_HANDLE   powerHandle = {0};

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

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);

    /*
     * 调用电源重启请求接口
     */
    powerRestart(&powerHandle);
}
/*********************************************************************************************************
** 函数名称: __powerShowAction
** 功能描述: power 事件显示处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __powerEventShowAction (Webs *wp)
{
    __PPOWER_HANDLE   pPowerHeader   = NULL;
    __POWER_HANDLE    powerHandle    = {0};
    __PPOWER_HANDLE   pTmpNode       = NULL;
    cJSON            *pJsonPowerArry = NULL;
    cJSON            *pjSend         = NULL;
    cJSON            *pRecvjRes      = NULL;
    cJSON            *pjPage         = NULL;
    UINT              uiMaxEventId   = 0;
    INT               iPage          = 1;                               /*  待显示的页码                */
    INT               iTotal         = 0;                               /*  总条目                      */
    INT               iRecordTotal   = 0;
    INT               i;

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

    pjSend         = cJSON_CreateObject();                              /*  创建返回 Json 对象          */
    pJsonPowerArry = cJSON_CreateArray();                               /*  创建 Json 映射信息数组      */
    if(NULL == pjSend || NULL == pJsonPowerArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     * 调用查询请求接口
     */
    pPowerHeader = powerEventShow(&powerHandle, &iRecordTotal);
    if (NULL != pPowerHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  循环设置返回的 Json 内容    */
            pTmpNode = pPowerHeader + i;
            /*
             *  把需要显示的内容狗造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {

                cJSON  *pjPower = cJSON_CreateObject();                 /*  创建一条映射 Json 对象      */
                if(NULL == pjPower) {
                     break;
                }

                cJSON_AddNumberToObject(pjPower, WEB_POWER_NUMBER, i + 1);
                cJSON_AddNumberToObject(pjPower, WEB_POWER_EVENT_ID, pTmpNode->uiEventId);
                cJSON_AddStringToObject(pjPower, WEB_POWER_EVENT,    pTmpNode->cEvent);
                cJSON_AddStringToObject(pjPower, WEB_POWER_DATE,     pTmpNode->cDate);
                cJSON_AddStringToObject(pjPower, WEB_POWER_TIME,     pTmpNode->cTime);
                cJSON_AddNumberToObject(pjPower, WEB_POWER_STATUS,   pTmpNode->iStatus);
                cJSON_AddStringToObject(pjPower, WEB_POWER_DESC,     pTmpNode->cDesc);

                cJSON_AddItemToArray(pJsonPowerArry, pjPower);          /*  将此条目加入数组            */
            }

            iTotal++;
            uiMaxEventId = CUR_MAX_ID(uiMaxEventId, pTmpNode->uiEventId);
        }
    }

    if (pPowerHeader) {
        free(pPowerHeader);
    }

    cJSON_AddItemToObject(pjSend,   ARRAY_NAME, pJsonPowerArry);
    cJSON_AddNumberToObject(pjSend, WEB_POWER_MAX_EVENT_ID, uiMaxEventId);
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
    cJSON_Delete(pJsonPowerArry);
}
/*********************************************************************************************************
** 函数名称: __powerConfigAction
** 功能描述: power 事件配置处理后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __powerEventConfigAction (Webs *wp)
{
    INT              iRet;
    cJSON           *pRecvjRes   = NULL;
    cJSON           *pSendjRes   = NULL;

    cJSON           *pjStatus    = NULL;
    cJSON           *pjEventId   = NULL;
    cJSON           *pjEvent     = NULL;
    cJSON           *pjDate      = NULL;
    cJSON           *pjTime      = NULL;
    cJSON           *pjOper      = NULL;
    cJSON           *pjDesc      = NULL;

    __POWER_HANDLE   powerHandle = {0};

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

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjStatus  = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_STATUS);
    pjEventId = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_EVENT_ID);
    pjEvent   = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_EVENT);
    pjDate    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_DATE);
    pjTime    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_TIME);
    pjOper    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_OPER);
    pjDesc    = cJSON_GetObjectItem(pRecvjRes, WEB_POWER_DESC);
    if (NULL == pjStatus || NULL == pjEventId ||
        NULL == pjEvent  || NULL == pjDate ||
        NULL == pjTime   || NULL == pjOper || NULL == pjDesc) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    powerHandle.iStatus   = pjStatus->valueint;
    powerHandle.uiEventId = pjEventId->valueint;
    strncpy(powerHandle.cEvent,    pjEvent->valuestring, sizeof(powerHandle.cEvent));
    strncpy(powerHandle.cDate,     pjDate->valuestring,  sizeof(powerHandle.cDate));
    strncpy(powerHandle.cTime,     pjTime->valuestring,  sizeof(powerHandle.cTime));
    strncpy(powerHandle.cOperType, pjOper->valuestring,  sizeof(powerHandle.cOperType));
    strncpy(powerHandle.cDesc,     pjDesc->valuestring,  sizeof(powerHandle.cDesc));

    /*
     * 调用配置请求接口
     */
    iRet = powerEventConfig(&powerHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __powerActionDefine
** 功能描述: power 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __powerActionDefine (VOID)
{
    websDefineAction("powerOffAction",         __powerOffAction);
    websDefineAction("powerRestartAction",     __powerRestartAction);
    websDefineAction("powerEventShowAction",   __powerEventShowAction);
    websDefineAction("powerEventConfigAction", __powerEventConfigAction);
}
/*********************************************************************************************************
** 函数名称: powerActionInit
** 功能描述: power action 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  powerActionInit (VOID)
{
    __powerActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
