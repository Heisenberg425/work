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
** 文   件   名: commonAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述: OpenRT 通用后台处理接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "../webHandle.h"
#include "situation_info.h"
#include "situationCommon.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define WEB_WEBSOCK_IP             "websockip"                          /*  Json字段：Websocket ip      */
/*********************************************************************************************************
  OpenRT 可选功能宏定义
*********************************************************************************************************/
#define WEB_KIDVPN_CLIENT         "kidvpn_client"                       /*  可选功能：KidVPN 客户端     */
#define WEB_KIDVPN_SERVER         "kidvpn_server"                       /*  可选功能：KidVPN 服务端     */
#define WEB_ARP_DEFENSE           "arp_defense"                         /*  可选功能：ARP 防御          */
#define FUNC_KIDVPN_CLIENT        (1 << 0)                              /*  可选功能：KidVPN 客户端     */
#define FUNC_KIDVPN_SERVER        (1 << 1)                              /*  可选功能：KidVPN 服务端     */
#define FUNC_ARP_DEFENSE          (1 << 2)                              /*  可选功能：ARP 防御          */
/*********************************************************************************************************
** 函数名称: __configureCompleteAction
** 功能描述: 配置完成操作后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __configCompleteAction (Webs *wp)
{
    cJSON                *pSendjRes     = LW_NULL;
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

    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition, POS_CONFIG_COMPLETE, strlen(POS_CONFIG_COMPLETE));
    memcpy(situationInfo.cStatus,    STATUS_FINISHED, strlen(STATUS_FINISHED));
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
}
/*********************************************************************************************************
** 函数名称: __websocketIpGetAction
** 功能描述: 获取 websocket ip 处理后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __websocketIpGetAction (Webs *wp)
{
    cJSON            *pSendjRes    = NULL;

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
    if(LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_WEBSOCK_IP, websGetIfaddr(wp));

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __situationStatusResetAction
** 功能描述: 情景模式状态信息初始化处理后台接口
** 输　入  : wp      服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __situationStatusResetAction (Webs *wp)
{
    cJSON             *pSendjRes = NULL;

#if 0
    INT                iRet;
    __SITUATION_INFO   situationInfo;
#endif

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
    if(LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }
#if 0
    bzero(&situationInfo, sizeof(situationInfo));
    memcpy(situationInfo.ucPosition,      POS_STARTING_SET, strlen(POS_STARTING_SET));
    memcpy(situationInfo.cStatus,         STATUS_UNCONFIG,  strlen(STATUS_UNCONFIG));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT,  strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = 0;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __finish;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__finish:
#endif

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __optionalFuncCancelAction
** 功能描述: 撤销可选功能后台接口
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __optionalFuncCancelAction (Webs *wp)
{
    INT     iRet         = PX_ERROR;
    cJSON  *pRecvjRes    = LW_NULL;
    cJSON  *pSendjRes    = LW_NULL;

    INT     iFunc        = 0;
    INT     iKidVpnCli   = 0;
    INT     iKidVpnServ  = 0;
    INT     iArpDefense  = 0;

    cJSON  *pjKidVpnCli  = LW_NULL;
    cJSON  *pjKidVpnServ = LW_NULL;
    cJSON  *pjArpDefense = LW_NULL;


    if (!isUserLogin(wp)) {                                             /*  用户鉴权                    */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    webHeadHandle(wp, 200);                                             /*  设置 http 表头              */

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    pjKidVpnCli = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_CLIENT);    /*  获取 Json 数据内容          */
    if (LW_NULL == pjKidVpnCli) {
        iKidVpnCli = FUNC_KIDVPN_CLIENT;
    } else {
        iKidVpnCli = pjKidVpnCli->valueint;
    }

    pjKidVpnServ = cJSON_GetObjectItem(pRecvjRes, WEB_KIDVPN_SERVER);   /*  获取 Json 数据内容          */
    if (LW_NULL == pjKidVpnServ) {
        iKidVpnServ = FUNC_KIDVPN_SERVER;
    } else {
        iKidVpnServ = pjKidVpnServ->valueint;
    }

    pjArpDefense = cJSON_GetObjectItem(pRecvjRes, WEB_ARP_DEFENSE);     /*  获取 Json 数据内容          */
    if (LW_NULL == pjArpDefense) {
        iArpDefense = FUNC_ARP_DEFENSE;
    } else {
        iArpDefense = pjArpDefense->valueint;
    }

    iFunc = iKidVpnCli | iKidVpnServ | iArpDefense;

    iRet = libOptionalFuncCancel(iFunc);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作失败                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  参数成功                    */
    }

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __configureCompleteActionDefine
** 功能描述: 配置完成后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __configureCompleteActionDefine (VOID)
{
    websDefineAction("configCompleteAction",     __configCompleteAction);
    websDefineAction("websocketIpGetAction",     __websocketIpGetAction);
    websDefineAction("situationStatResetAction", __situationStatusResetAction);
    websDefineAction("optionalFuncCancelAction", __optionalFuncCancelAction);
}

/*********************************************************************************************************
** 函数名称: configureCompletActionInit
** 功能描述: 配置完成后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  configureCompletActionInit (VOID)
{
    __configureCompleteActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
