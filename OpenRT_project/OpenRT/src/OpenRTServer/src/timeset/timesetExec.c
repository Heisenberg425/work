/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: timesetExec.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳʱ�����ò���
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "timeset_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
/*********************************************************************************************************
** ��������: systemTimeGet
** ��������: ��ȡϵͳʱ��
** �䡡��  : NONE
** �䡡��  : pTmInfo  ʱ�������Ϣ
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  systemTimeGet (__PTIME_HANDLE  pTmInfo)
{
    time_t              tim;
    struct tm           tmTime;

    lib_time(&tim);                                                     /*  ��õ�ǰϵͳʱ��            */
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
** ��������: timeDoConfig
** ��������: ����ϵͳʱ��
** �䡡��  : pTmInfo  ʱ�������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
