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
** 文   件   名: arpConfAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述: OpenRT ARP 后台处理接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "arp_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
#include "situation_info.h"
#include "situationCommon.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */
/*********************************************************************************************************
  Jason key: arp 防御信息
*********************************************************************************************************/
#define WEB_ASD_EN                 "en"                                 /*  Json字段：是否开启防御      */
/*********************************************************************************************************
** 函数名称: __asdefenseEnConfAction
** 功能描述: arp 防御操作后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __asdefenseEnConfAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *jsEnFlag      = LW_NULL;
    UINT8                 ucEnFlag      = 0;

    __SITUATION_INFO      situationInfo;

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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    /*
     *  获取 Json 数据内容
     */
    jsEnFlag = cJSON_GetObjectItem(pRecvjRes, WEB_ASD_EN);
    if (LW_NULL == jsEnFlag) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto  __finish;
    }

    ucEnFlag = jsEnFlag->valueint;

    iRet = libAsDefenseHandle(ucEnFlag);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    }

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition, POS_CONFIG_COMPLETE, strlen(POS_CONFIG_COMPLETE));
    memcpy(situationInfo.cStatus,    STATUS_CONFIGING, strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = -1;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __arpConfigActionDefine
** 功能描述: arp后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpConfigActionDefine (VOID)
{
    websDefineAction("asdefenseEnConfAction", __asdefenseEnConfAction);
}

/*********************************************************************************************************
** 函数名称: arpConfigActionInit
** 功能描述: arp后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  arpConfigActionInit (VOID)
{
    __arpConfigActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
