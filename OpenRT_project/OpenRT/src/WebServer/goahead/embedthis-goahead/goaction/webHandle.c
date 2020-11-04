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
** 文   件   名: webHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年6 月 9 日
**
** 描        述: action 接口 web操作抽象
*********************************************************************************************************/
#include <stdio.h>

#include "webHandle.h"
/*********************************************************************************************************
** 函数名称: cJsonErrorReturnInit
** 功能描述: 出错时返回 Json 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  cJsonErrorReturnInit (VOID)
{
    cJSON_AddNumberToObject(&_G_ErrorJRes, "code", 500);
}


/*********************************************************************************************************
** 函数名称: webHeadHandle
** 功能描述: action 收到请求时，Web 头部的处理
** 输　入  : wp                 Web 句柄
**           iStates            http 头code编码
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  webHeadHandle (Webs *wp, INT  iStates)
{
    if (LW_NULL == wp) {
        return  (PX_ERROR);
    }

    websSetStatus(wp, iStates);                                         /*  http状态                    */
    websWriteHeaders(wp, -1, 0);                                        /*  -1表示自动设置内容长度      */
    websWriteEndHeaders(wp);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: webBackHandle
** 功能描述: action 完成请求时，返回时的处理
** 输　入  : wp                 Web 句柄
**           cpJson             返回的cpJson内容
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
VOID  webBackHandle (Webs  *wp, cJSON  *pcJson)
{
    PCHAR    pcJsonBuf  = LW_NULL;
    if (LW_NULL == wp) {
        return;
    }

    if (LW_NULL != pcJson) {
        pcJsonBuf = cJSON_Print(pcJson);
        websWriteBlock(wp, pcJsonBuf, strlen(pcJsonBuf));
        free(pcJsonBuf);
    }

    websDone(wp);
}
