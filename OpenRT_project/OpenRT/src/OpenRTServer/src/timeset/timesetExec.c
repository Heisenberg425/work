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
** 文   件   名: timesetExec.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统时间设置操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "timeset_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
/*********************************************************************************************************
** 函数名称: systemTimeGet
** 功能描述: 获取系统时间
** 输　入  : NONE
** 输　出  : pTmInfo  时间参数信息
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  systemTimeGet (__PTIME_HANDLE  pTmInfo)
{
    time_t              tim;
    struct tm           tmTime;

    lib_time(&tim);                                                     /*  获得当前系统时间            */
    lib_localtime_r(&tim, &tmTime);

    sprintf(pTmInfo->cDate,"%02d/%02d/%02d",
            tmTime.tm_year + 1900,
            tmTime.tm_mon  + 1,
            tmTime.tm_mday);

    sprintf(pTmInfo->cTime, "%02d:%02d:%02d",
            tmTime.tm_hour,
            tmTime.tm_min,
            tmTime.tm_sec);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: timeDoConfig
** 功能描述: 配置系统时间
** 输　入  : pTmInfo  时间参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  timeDoConfig (__PTIME_HANDLE  pTmInfo)
{
    INT   iRet;
    CHAR  cDateBuf[32] = {0};
    CHAR  cTimeBuf[32]  = {0};

    if (LW_NULL == pTmInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "time data is null.\n");

       return  (PX_ERROR);
    }

    sprintf(cDateBuf, "date -s %s", pTmInfo->cDate);
    sprintf(cTimeBuf, "date -s %s", pTmInfo->cTime);

    iRet = API_TShellExec(cDateBuf);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "set system date failed.\n");

       return  (PX_ERROR);
    }

    iRet = API_TShellExec(cTimeBuf);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "set system time failed.\n");

       return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
