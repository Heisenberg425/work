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
** 文   件   名: updatefirmwareaction.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 2 日
**
** 描        述: update firmware 配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "updatefirmware_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define DEFAULT_FAILED_CJSON       "{\n\t\"code\":\t500\n}"             /*  json结构创建失败时使用      */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */

/*********************************************************************************************************
** 函数名称: __updateFirmwareAction
** 功能描述: update firmware的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareAction (Webs  *wp)
{
    cJSON       *pjSend       = LW_NULL;
    WebsKey     *pKey         = LW_NULL;
    WebsUpload  *pUpLoad      = LW_NULL;
    INT          iFlag        = 0;
    INT          iRet         = PX_ERROR;

    /*
     * 用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置http表头
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        goto __error;
    }

    if (scaselessmatch(wp->method, "POST")) {
        for (pKey = hashFirst(wp->files); pKey; pKey = hashNext(wp->files, pKey)) {
            CHAR     cUpFile[100]        = {0};

            pUpLoad = pKey->content.value.symbol;
            if (!pUpLoad || !pUpLoad->clientFilename || strlen(pUpLoad->clientFilename) == 0) {
                cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

                goto __error;                                           /*  参数出错                    */
            }

            snprintf(cUpFile, sizeof(cUpFile), "%s/%s", UPDATE_FIRMWARE_LOCATION, UPDATE_FIRMWARE_TAR);
            rename(pUpLoad->filename, cUpFile);

            iFlag = 1;
        }
    }

    if (1 == iFlag) {
        iRet = updateFirmwareConfig();
    }

    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

        webBackHandle(wp, pjSend);
        cJSON_Delete(pjSend);

        system("reboot");
    }

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}

/*********************************************************************************************************
** 函数名称: __updateFirmwareActionDefine
** 功能描述: update firmware后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareActionDefine (VOID)
{
    websDefineAction("updateFirmwareAction", __updateFirmwareAction);
}

/*********************************************************************************************************
** 函数名称: updateFirmwareActionInit
** 功能描述: update firmware后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  updateFirmwareActionInit (VOID)
{
    __updateFirmwareActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
