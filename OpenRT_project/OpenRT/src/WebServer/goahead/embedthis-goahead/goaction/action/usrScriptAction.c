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
** 文   件   名: usrScriptAction.c
**
** 创   建   人: Wu.pengcheng (吴鹏程)
**
** 文件创建日期: 2019 年 5 月 14 日
**
** 描        述: 用户自定义启动 shell 脚本
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "dns_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  返回的状态码                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json 结构创建失败时使用     */
#define WEB_USR_SCRIPT_TYPE             "type"                          /*  shell 脚本类型              */
#define WEB_USR_SCRIPT_LEN              "len"                           /*  shell 脚本长度              */
#define WEB_USR_SCRIPT_VAL              "script"                        /*  shell 脚本内容              */
#define WEB_USR_SCRIPT_RUN_T            "run"                           /*  立即运行 shell 命令脚本     */
#define WEB_USR_SCRIPT_FRO_T            "front"                         /*  启动时，在 OpenR T 前运行   */
#define WEB_USR_SCRIPT_AFT_T            "after"                         /*  启动时，在 OpenR T 后运行   */
/*********************************************************************************************************
  配置宏
*********************************************************************************************************/
#define FILE_MODE                       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define USR_RUN_SRIPT_FILE              "/etc/usrRunScript.sh"          /*  用户立即运行脚本            */
#define USR_FRO_SRIPT_FILE              "/etc/usrFroScript.sh"          /*  在 OpenR T 前运行脚本       */
#define USR_AFT_SRIPT_FILE              "/etc/usrAftScript.sh"          /*  在 OpenR T 后运行脚本       */
#define USR_SCRIPT_MAX_LEN              (10 * 1024)                     /*  脚本最大 10 K               */
/*********************************************************************************************************
** 函数名称: __usrScriptShow
** 功能描述: 用户脚本内容显示
** 输　入  : pcusrScriptVal     脚本内容
**           pcusrScriptType    脚本类型
**           iusrScriptLen      脚本长度
** 输　出  : NONE
** 返　回  : 处理结果
*********************************************************************************************************/
static INT32  __usrScriptShow (CHAR  *pcusrScriptVal, CHAR  *pcusrScriptType, INT32  iusrScriptLen)
{
    INT32     iFd;
    INT32     iRet;
    CHAR      cScriptFile[50]   = {0};

    /*
     *  打开文件，并保存最新的内容
     */
    if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_RUN_T)) {
        strncpy(cScriptFile, USR_RUN_SRIPT_FILE, sizeof(cScriptFile));
    } else if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_FRO_T)) {
        strncpy(cScriptFile, USR_FRO_SRIPT_FILE, sizeof(cScriptFile));
    } else if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_AFT_T)) {
        strncpy(cScriptFile, USR_AFT_SRIPT_FILE, sizeof(cScriptFile));
    } else {
        return  (PX_ERROR);
    }

    iFd = open(cScriptFile, O_RDONLY | O_CREAT, FILE_MODE);
    if (iFd < 0) {
        return  (PX_ERROR);
    }

    iRet = read(iFd, pcusrScriptVal, iusrScriptLen);
    if (iRet < 0) {
        close(iFd);

        return  (PX_ERROR);
    }

    close(iFd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __usrScriptHandle
** 功能描述: 用户脚本处理
** 输　入  : pcusrScriptVal     脚本内容
**           pcusrScriptType    脚本类型
**           iusrScriptLen      脚本长度
** 输　出  : NONE
** 返　回  : 处理结果
*********************************************************************************************************/
static INT32  __usrScriptHandle (CHAR  *pcusrScriptVal, CHAR  *pcusrScriptType, INT32  iusrScriptLen)
{
    INT32     iRet;
    INT32     iFd;
    UCHAR     uNeedRunNow       = 0;
    CHAR      cScriptRunCmd[60] = {0};
    CHAR      cScriptFile[50]   = {0};

    /*
     *  打开文件，并保存最新的内容
     */
    if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_RUN_T)) {
        strncpy(cScriptFile, USR_RUN_SRIPT_FILE, sizeof(cScriptFile));
        snprintf(cScriptRunCmd, sizeof(cScriptRunCmd), "shfile %s", cScriptFile);
        uNeedRunNow = 1;
    } else if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_FRO_T)) {
        strncpy(cScriptFile, USR_FRO_SRIPT_FILE, sizeof(cScriptFile));
    } else if (!strcmp(pcusrScriptType, WEB_USR_SCRIPT_AFT_T)) {
        strncpy(cScriptFile, USR_AFT_SRIPT_FILE, sizeof(cScriptFile));
    } else {
        return  (PX_ERROR);
    }

    iFd = open(cScriptFile, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    if (iFd < 0) {
        return  (PX_ERROR);
    }

    iRet = write(iFd, pcusrScriptVal, iusrScriptLen);
    if (iRet < 0) {
        close(iFd);

        return  (PX_ERROR);
    }

    close(iFd);

    if (uNeedRunNow) {
        return  system(cScriptRunCmd);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** 函数名称: __usrScriptShowAction
** 函数功能: 请求显示用户定义脚本内容
** 输    入: wp           服务器句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __usrScriptShowAction (Webs  *wp)
{
    cJSON                *pjusrScriptType = LW_NULL;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pSendjRes       = LW_NULL;
    CHAR                 *cScriptVal      = LW_NULL;

    INT32                 iRet;
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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "get usr shell script, parser request data failed!\n", 0);

        goto  __error;
    }

    pjusrScriptType = cJSON_GetObjectItem(pRecvjRes, WEB_USR_SCRIPT_TYPE);
    if (NULL == pjusrScriptType || NULL == pjusrScriptType->valuestring) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_USR_SCRIPT, "get usr shell script, parser request data failed!\n", 0);

        goto  __error;
    }

    cScriptVal = (CHAR *)malloc(USR_SCRIPT_MAX_LEN);
    if (!cScriptVal) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);
        operLogInsert(wp, OPEN_RT_USR_SCRIPT, "shell script malloc mem failed!\n", 0);

        goto  __error;
    }

    bzero(cScriptVal, USR_SCRIPT_MAX_LEN);
    iRet = __usrScriptShow(cScriptVal, pjusrScriptType->valuestring, USR_SCRIPT_MAX_LEN);
    if (iRet == PX_ERROR) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);
        operLogInsert(wp, OPEN_RT_USR_SCRIPT, "get usr shell script  failed!\n", 0);

        goto  __error;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_USR_SCRIPT_VAL, cScriptVal);
    cJSON_AddNumberToObject(pSendjRes, WEB_USR_SCRIPT_LEN, iRet);
    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

    free(cScriptVal);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}


/*********************************************************************************************************
** 函数名称: __usrScriptSetAction
** 功能描述: 用户脚本设置后台接口
** 输　入  : wp          服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __usrScriptSetAction (Webs  *wp)
{
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pSendjRes       = LW_NULL;

    cJSON                *pjusrScriptVal  = LW_NULL;
    cJSON                *pjusrScriptType = LW_NULL;
    cJSON                *pjusrScriptLen  = LW_NULL;

    INT32                 iRet;

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
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);
        operLogInsert(wp, OPEN_RT_DNS, "set usr shell cmd failed!\n", 0);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_DNS, "set usr shell script, parser request data failed!\n", 0);

        goto  __error;
    }

    pjusrScriptType = cJSON_GetObjectItem(pRecvjRes, WEB_USR_SCRIPT_TYPE);
    pjusrScriptVal  = cJSON_GetObjectItem(pRecvjRes, WEB_USR_SCRIPT_VAL);
    pjusrScriptLen  = cJSON_GetObjectItem(pRecvjRes, WEB_USR_SCRIPT_LEN);
    if ((NULL == pjusrScriptType || NULL == pjusrScriptType->valuestring) ||
        (NULL == pjusrScriptVal  || NULL == pjusrScriptVal->valuestring)  ||
        (NULL == pjusrScriptLen  || 0    == pjusrScriptLen->valueint) ||
        USR_SCRIPT_MAX_LEN < pjusrScriptLen->valueint) {

        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_USR_SCRIPT, "set usr shell script, parser request data failed!\n", 0);

        goto  __error;
    }

    iRet = __usrScriptHandle(pjusrScriptVal->valuestring, pjusrScriptType->valuestring, pjusrScriptLen->valueint);
    if (iRet == PX_ERROR) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);
        operLogInsert(wp, OPEN_RT_USR_SCRIPT, "usr shell script handle failed!\n", 0);

        goto  __error;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __usrScriptActionDefine
** 功能描述: 用户脚本 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __usrScriptActionDefine (VOID)
{
    websDefineAction("usrScriptShowAction",   __usrScriptShowAction);
    websDefineAction("usrScriptSetAction", __usrScriptSetAction);
}
/*********************************************************************************************************
** 函数名称: dnsActionInit
** 功能描述: DNS 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  usrScriptActionInit (VOID)
{
    __usrScriptActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
