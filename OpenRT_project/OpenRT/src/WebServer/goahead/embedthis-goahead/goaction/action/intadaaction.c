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
** 文   件   名: iniadaaction.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 16 日
**
** 描        述: OpenRT 网口自适应定位后台接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "sys/un.h"
#include "intada_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  网口自定义相关宏定义
*********************************************************************************************************/
#define WEB_INT_NAME       "name"                                       /*  Json字段：网口名(用户定义的)*/
#define WEB_INT_INDEX      "index"                                      /*  Json字段：前台索引          */
#define WEB_INT_FLAG       "flag"                                       /*  Json字段：网口顺序标志      */
#define WEB_INT_CODE       "code"                                       /*  Json字段：返回状态          */

#define INT_FLAG_FIRST     "first"                                      /*  key :flag 第一个网口        */
#define INT_FLAG_F         (0)                                          /*  flag = 0  第一个网口        */
#define INT_FLAG_MIDDLE    "middle"                                     /*  key :flag 中间网口          */
#define INT_FLAG_M         (2)                                          /*  flag = 2  中间网口          */
#define INT_FLAG_LAST      "last"                                       /*  key :flag 最后一个网口      */
#define INT_FLAG_L         (1)                                          /*  flag = 1  最后一个网口      */
/*********************************************************************************************************
** 函数名称: __intAdaHandleAction
** 功能描述: 网口自适应处理
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __intAdaHandleAction (Webs *wp)
{
    INT                   iRet;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pSendjRes       = LW_NULL;
    CHAR                 *pcName          = LW_NULL;
    CHAR                 *pcFlag          = LW_NULL;
    cJSON                *pjName          = LW_NULL;
    cJSON                *pjFlag          = LW_NULL;
    cJSON                *pjIndex         = LW_NULL;

    __INT_ADA_SET         intAdaSetInfo;

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
        webBackHandle(wp, &_G_ErrorJRes);                               /*  Json 结构创建失败           */

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjName  = cJSON_GetObjectItem(pRecvjRes, WEB_INT_NAME);
    pjFlag  = cJSON_GetObjectItem(pRecvjRes, WEB_INT_FLAG);
    pjIndex = cJSON_GetObjectItem(pRecvjRes, WEB_INT_INDEX);
    if (LW_NULL == pjName ||
        LW_NULL == pjFlag ||
        LW_NULL == pjIndex) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 100);

        goto  __error;
    }

    pcName = pjName->valuestring;

    if ((NULL != lib_strstr(pcName, "VPN-")) ||
        (strlen(pcName) >= 10)) {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 100);

        goto  __error;
    }

    bzero(&intAdaSetInfo, sizeof(intAdaSetInfo));
    strncpy(intAdaSetInfo.cWebName, pcName, sizeof(intAdaSetInfo.cWebName));

    intAdaSetInfo.iWebIndex = pjIndex->valueint;
    pcFlag                  = pjFlag->valuestring;
    intAdaSetInfo.cFlag     = ((0 == strcmp(pcFlag, INT_FLAG_FIRST)) ? INT_ADA_FLAG_FIRST :
                              ((0 == strcmp(pcFlag, INT_FLAG_LAST))  ? INT_ADA_FLAG_LAST  :
                               INT_ADA_FLAG_MIDDLE));

    iRet = libIntAdaSet(&intAdaSetInfo);
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_INT_CODE, 200);
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_INT_INDEX, intAdaSetInfo.iWebIndex);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** 函数名称: __intAdaCancelAction
** 功能描述: 网口自适应取消(暂时无操作)
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __intAdaCancelAction (Webs *wp)
{
    /*
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    libIntAdaClear();

    /*
     *  记录操作日志
     */
    operLogInsert(wp, OPEN_RT_INTADA, "restart interface adaption\n", 0);

    webHeadHandle(wp, 200);
    webBackHandle(wp, NULL);
}

/*********************************************************************************************************
** 函数名称: __intAdaActionDefine
** 功能描述: 网口定位后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __intAdaActionDefine (VOID)
{
    websDefineAction("intAdaHandleAction", __intAdaHandleAction);
    websDefineAction("intAdaCancelAction", __intAdaCancelAction);
}

/*********************************************************************************************************
** 函数名称: intAdaActionInit
** 功能描述: 网口定位后台接口初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  intAdaActionInit (VOID)
{
    __intAdaActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
