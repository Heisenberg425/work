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
** 文   件   名: hwAction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 2 日
**
** 描        述: OpenRT 硬件信息后台接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "hw_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
#define JSON_SUCCESS_CODE     (200)                                     /*  成功返回Json code值         */
#define JSON_FAILED_CODE      (500)                                     /*  失败返回Json code值         */
/*********************************************************************************************************
  Jason key: 硬件信息
*********************************************************************************************************/
#define WEB_HW_CPU            "cpu"                                     /*  Json字段：CPU               */
#define WEB_HW_MEM            "mem"                                     /*  Json字段：内存              */
#define WEB_HW_STO            "sto"                                     /*  Json字段：存储              */
/*********************************************************************************************************
** 函数名称: __hwInfoGetAction
** 功能描述: 获取硬件信息后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __hwInfoGetAction (Webs *wp)
{
    cJSON                *pSendjRes = LW_NULL;
    __HW_INFO             hwInfo;
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

    bzero(&hwInfo, sizeof(hwInfo));

    iRet = libHardWareInfoGet(&hwInfo);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_HW_CPU, hwInfo.cCpuInfo);
    cJSON_AddStringToObject(pSendjRes, WEB_HW_MEM, hwInfo.cMemInfo);
    cJSON_AddStringToObject(pSendjRes, WEB_HW_STO, hwInfo.cStoInfo);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/***************************************************s******************************************************
** 函数名称: __hwActionDefine
** 功能描述: LOG 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __hwActionDefine (VOID)
{
    websDefineAction("hwInfoGetAction", __hwInfoGetAction);
}
/*********************************************************************************************************
** 函数名称: hwActionInit
** 功能描述: LOG 后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID hwActionInit (VOID)
{
    __hwActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
