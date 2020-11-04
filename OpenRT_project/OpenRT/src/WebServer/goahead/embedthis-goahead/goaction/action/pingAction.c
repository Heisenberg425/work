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
** 文   件   名: pingAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统 PING 工具配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "ping_tool_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define PING_INFO_LEN              (1024)                               /*  PING 测试信息长度           */
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: PING 配置及路由信息显示
*********************************************************************************************************/
#define WEB_IP_DOMAIN             "ip/domain"                           /*  Json字段：IP或域名          */
#define WEB_SRC_INTERFACE         "interface"                           /*  Json字段：源接口名称        */
#define WEB_PING_TIMES            "times"                               /*  Json字段：PING 次数名称     */
#define WEB_PING_DATA_SIZE        "data_size"                           /*  Json字段：数据长度          */
#define WEB_PING_RESULT           "result"                              /*  Json字段：PING 结果名称     */
/*********************************************************************************************************
** 函数名称: __pingTestInfoGet
** 功能描述: 获取 PING 测试信息的部分结果
** 输　入  : cpcInputBuf       PING 测试结果缓冲区
**           pingHandle        PING 测试结构
** 输　出  : NONE
** 返　回  : 成功返回 PING 测试结果字符串地址，失败返回 NULL
*********************************************************************************************************/
static PVOID  __pingTestInfoGet (CPCHAR  cpcInputBuf, __PING_HANDLE  pingHandle)
{
    CHAR   cStr[PING_INFO_LEN] = {0};
    PCHAR  pcStr               = NULL;
    PCHAR  pcTmp               = NULL;
    PCHAR  pcPingInfo          = NULL;
    INT    iBufLen             = 0;
    INT    iOffset             = 0;

    pcPingInfo = (PCHAR)malloc(PING_INFO_LEN);
    if (NULL == pcPingInfo) {
        return  (NULL);
    }

    bzero(pcPingInfo, PING_INFO_LEN);

    sprintf(cStr, "Pinging %s", pingHandle.cDomain);

    /*
     *  提取 PING 测试信息
     */
    pcStr = lib_strstr(cpcInputBuf, cStr);
    if (NULL != pcStr) {                                                /*  检索Pinging+ip/domain 信息  */
        pcTmp = strchr(pcStr, '\n');
        iBufLen = pcTmp - pcStr + 1;

        lib_memcpy(pcPingInfo, pcStr, iBufLen);
        iOffset += iBufLen;

        if (lib_strstr(cpcInputBuf, "Request time out.")) {             /*  请求超时                    */
            pcStr = lib_strstr(cpcInputBuf, "Request time out.");
            lib_memcpy(pcPingInfo + iOffset, pcStr, strlen("Request time out.\n"));
        } else if (lib_strstr(cpcInputBuf, "error: Invalid argument or format.")) {

            /*
             *  PING 测试参数或格式无效
             */
            pcStr = lib_strstr(cpcInputBuf, "error: Invalid argument or format.");
            lib_memcpy(pcPingInfo + iOffset, pcStr, strlen("error: Invalid argument or format.\n"));
        } else if (lib_strstr(cpcInputBuf, "Reply from")) {             /*  PING 测试成功               */
            pcStr = lib_strstr(cpcInputBuf, "Reply from");
            pcTmp = strchr(pcStr, '\n');
            iBufLen = pcTmp - pcStr + 1;

            lib_memcpy(pcPingInfo + iOffset, pcStr, iBufLen);
        } else {                                                        /*  出错                        */
            lib_memcpy(pcPingInfo + iOffset, "unkown error\n", strlen("unkown error\n"));
        }
    } else if (lib_strstr(cpcInputBuf, "Pinging request could not find host")) {

        /*
         *  找不到主机
         */
        pcStr = lib_strstr(cpcInputBuf, "Pinging request could not find host");
        pcTmp = strchr(pcStr, '\n');
        iBufLen = pcTmp - pcStr + 1;

        lib_memcpy(pcPingInfo + iOffset, pcStr, iBufLen);
    } else {                                                            /*  参数无效                    */
        lib_memcpy(pcPingInfo + iOffset, "arguments error.\n", strlen("arguments error.\n"));
    }

    return  (pcPingInfo);
}
/*********************************************************************************************************
** 函数名称: __pingTestAction
** 功能描述: PING 测试信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __pingTestAction (Webs *wp)
{
    __PING_HANDLE   pingHandle   = {{0}};
    PCHAR           pcPingHeader = NULL;
    PCHAR           pcPingInfo   = NULL;
    cJSON          *pRecvjRes    = NULL;
    cJSON          *pSendjRes    = NULL;

    cJSON          *pjDomain     = NULL;
    cJSON          *pjNetifName  = NULL;
    cJSON          *pjDataSize   = NULL;

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

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if(LW_NULL == pSendjRes) {
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
    pjDomain    = cJSON_GetObjectItem(pRecvjRes, WEB_IP_DOMAIN);
    pjNetifName = cJSON_GetObjectItem(pRecvjRes, WEB_SRC_INTERFACE);
    pjDataSize  = cJSON_GetObjectItem(pRecvjRes, WEB_PING_DATA_SIZE);
    if (NULL == pjDomain || NULL == pjNetifName || NULL == pjDataSize) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    strncpy(pingHandle.cDomain,   pjDomain->valuestring,    sizeof(pingHandle.cDomain));
    strncpy(pingHandle.cEnetName, pjNetifName->valuestring, sizeof(pingHandle.cEnetName));

    pingHandle.iDataSize = pjDataSize->valueint;

    /*
     *  调用查询请求接口并设置返回的 Json 内容
     */
    pcPingHeader = pingTest(&pingHandle, &iRecordTotal);

    /*
     *  从 PING 测试结果字符串中获取前端需显示的结果
     */
    pcPingInfo = __pingTestInfoGet(pcPingHeader, pingHandle);
    if (NULL == pcPingInfo) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_PING_RESULT, pcPingInfo);
    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    if (pcPingHeader) {
        free(pcPingHeader);
    }

    if (pcPingInfo) {
        free(pcPingInfo);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __pingActionDefine
** 功能描述: ping 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __pingActionDefine (VOID)
{
    websDefineAction("pingTestAction",   __pingTestAction);
}
/*********************************************************************************************************
** 函数名称: pingActionInit
** 功能描述: ping action 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  pingActionInit (VOID)
{
    __pingActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
