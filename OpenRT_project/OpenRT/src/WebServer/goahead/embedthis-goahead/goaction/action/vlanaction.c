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
** 文   件   名: vlanaction.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: VLAN配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "lwip/netif.h"
#include "vlan_info.h"
#include "log_info.h"
#include "intada_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: vlan 配置及路由信息显示
*********************************************************************************************************/
#define WEB_VLAN_VLANID           "vlan_id"                             /*  Json字段：vlanID            */
#define WEB_VLAN_IFACE            "iface"                               /*  Json字段：接口名称          */
#define WEB_VLAN_TAG              "tag"                                 /*  Json字段：tag               */
#define WEB_VLAN_PRIORITY         "priority"                            /*  Json字段：优先级            */
#define WEB_VLAN_OPER_TYPE        "oper_type"                           /*  Json字段：操作类型，增删改查*/
#define WEB_VLAN_WEBNAME          "webname"                             /*  Json字段：web 接口名        */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                  "page"                                /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                 "total"                               /*  Json字段：总共的条目数      */
#define ARRAY_NAME                "data"                                /*  Json字段：内网端口          */

/*********************************************************************************************************
** 函数名称: __vlanShowAction
** 功能描述: vlan信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vlanShowAction (Webs *wp)
{
    __PVLAN_HANDLE   pVlanHeader              = LW_NULL;
    __VLAN_HANDLE    vlanHandle               = {0};
    __PVLAN_HANDLE   pTmpNode                 = LW_NULL;
    INT              iPage                    = 1;                      /*  待显示的页码                */
    INT              iTotal                   = 0;                      /*  总条目                      */
    cJSON*           pJsonVlanArry            = LW_NULL;
    cJSON*           pjSend                   = LW_NULL;
    cJSON           *pRecvjRes                = LW_NULL;
    cJSON           *pjPage                   = LW_NULL;
    CHAR             cWebName[NETIF_NAMESIZE] = {0};
    INT              iRecordTotal             = 0;
    INT              i;

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

    pjSend        = cJSON_CreateObject();                               /*  创建返回 Json 对象          */
    pJsonVlanArry = cJSON_CreateArray();                                /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || LW_NULL == pJsonVlanArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
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
     *  调用查询请求接口
     */
    pVlanHeader = vlanShow(&vlanHandle, &iRecordTotal);
    if (LW_NULL != pVlanHeader) {
        for (i = 0; i < iRecordTotal; i++) {
            pTmpNode = pVlanHeader + i;

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * 5) && (iTotal < iPage * 5)) {
                cJSON*  pjVlan = cJSON_CreateObject();                  /*  创建一条映射 Json 对象      */
                if(LW_NULL == pjVlan) {
                    break;
                }

                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_VLANID,   pTmpNode->iVlanID);

                libIntAdaGetWebNetif(pTmpNode->cIface, cWebName, sizeof(cWebName));

                cJSON_AddStringToObject(pjVlan, WEB_VLAN_IFACE,    cWebName);
                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_TAG,      pTmpNode->iTag);
                cJSON_AddNumberToObject(pjVlan, WEB_VLAN_PRIORITY, pTmpNode->iPriority);

                cJSON_AddItemToArray(pJsonVlanArry, pjVlan);            /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    if (NULL != pVlanHeader) {
        free(pVlanHeader);
    }

    cJSON_AddItemToObject (pjSend, ARRAY_NAME, pJsonVlanArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonVlanArry);
}
/*********************************************************************************************************
** 函数名称: __vlanConfigAction
** 功能描述: vlan处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vlanConfigAction (Webs *wp)
{
    cJSON          *pRecvjRes       = LW_NULL;
    cJSON          *pSendjRes       = LW_NULL;

    cJSON          *pjVlanId        = LW_NULL;
    cJSON          *pjIface         = LW_NULL;
    cJSON          *pjOperType      = LW_NULL;
    __VLAN_HANDLE   vlanHandle      = {0};
    char            logContent[128] = {0};
    INT             iRet;

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
    pjVlanId   = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_VLANID);
    pjIface    = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_IFACE);
    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_VLAN_OPER_TYPE);
    if (NULL == pjVlanId ||
        NULL == pjIface  ||
        NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    vlanHandle.iVlanID = pjVlanId->valueint;
    strncpy(vlanHandle.cOperType, pjOperType->valuestring, sizeof(vlanHandle.cOperType));

    /*
     *  获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pjIface->valuestring, vlanHandle.cIface, sizeof(vlanHandle.cIface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     * 调用配置请求接口
     */
    iRet = vlanConfig(&vlanHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

    snprintf(logContent, sizeof(logContent), "Operation: vlan %s %s %d",
    		 vlanHandle.cOperType, vlanHandle.cIface, vlanHandle.iVlanID);
    operLogInsert(wp, OPEN_RT_VLAN, logContent, iRet);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}

/*********************************************************************************************************
** 函数名称: __vlanActionDefine
** 功能描述: vlan后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vlanActionDefine (VOID)
{
    websDefineAction("vlanShowAction", __vlanShowAction);
    websDefineAction("vlanConfigAction", __vlanConfigAction);
}

/*********************************************************************************************************
** 函数名称: vlanActionInit
** 功能描述: vlan后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vlanActionInit (VOID)
{
    __vlanActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
