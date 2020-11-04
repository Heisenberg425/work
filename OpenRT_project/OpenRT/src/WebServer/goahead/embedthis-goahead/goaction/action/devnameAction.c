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
** 文   件   名: devnameAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统设备名配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "devname_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: time 配置及路由信息显示
*********************************************************************************************************/
#define WEB_DEV_NAME              "dev_name"                            /*  Json字段：设备名            */
/*********************************************************************************************************
** 函数名称: __devNameGetAction
** 功能描述: 获取设备名信息的后台接口
** 输　入  : wp        服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __devNameGetAction (Webs *wp)
{
    __PDEVNAME_HANDLE  pDevNameHeader = LW_NULL;
    __DEVNAME_HANDLE   devNameHandle  = {{0}};
    cJSON*             pjSend         = LW_NULL;
    INT                iRecordTotal   = 0;

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

    /*
     *  调用查询请求接口并设置返回的 Json 内容
     */
    pDevNameHeader = devNameShow(&devNameHandle, &iRecordTotal);
    if (LW_NULL != pDevNameHeader) {
        cJSON_AddStringToObject(pjSend, WEB_DEV_NAME, pDevNameHeader->cdevName);
    } else {
        cJSON_AddStringToObject(pjSend, WEB_DEV_NAME, "Acoinfo OpenRT");
    }

    if (pDevNameHeader) {
        free(pDevNameHeader);
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __devNameConfigAction
** 功能描述: 配置设备名处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __devNameConfigAction (Webs *wp)
{
    INT                iRet;
    cJSON             *pRecvjRes     = LW_NULL;
    cJSON             *pSendjRes     = LW_NULL;
    cJSON             *pjDevName     = LW_NULL;
    __DEVNAME_HANDLE   devNameHandle = {{0}};

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
    pjDevName = cJSON_GetObjectItem(pRecvjRes, WEB_DEV_NAME);
    if (NULL == pjDevName) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(devNameHandle.cdevName, pjDevName->valuestring, sizeof(devNameHandle.cdevName));

    /*
     * 调用配置请求接口
     */
    iRet = devNameConfig(&devNameHandle);
    if (ERROR_NONE != iRet) {
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
** 函数名称: __devNameActionDefine
** 功能描述: device name 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __devNameActionDefine (VOID)
{
    websDefineAction("devNameGetAction",    __devNameGetAction);
    websDefineAction("devNameConfigAction", __devNameConfigAction);
}
/*********************************************************************************************************
** 函数名称: devNameActionInit
** 功能描述: 设备名 action 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  devNameActionInit (VOID)
{
    __devNameActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
