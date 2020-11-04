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
** ��   ��   ��: usrScriptAction.c
**
** ��   ��   ��: Wu.pengcheng (������)
**
** �ļ���������: 2019 �� 5 �� 14 ��
**
** ��        ��: �û��Զ������� shell �ű�
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
#define WEB_CODE                        "code"                          /*  ���ص�״̬��                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json �ṹ����ʧ��ʱʹ��     */
#define WEB_USR_SCRIPT_TYPE             "type"                          /*  shell �ű�����              */
#define WEB_USR_SCRIPT_LEN              "len"                           /*  shell �ű�����              */
#define WEB_USR_SCRIPT_VAL              "script"                        /*  shell �ű�����              */
#define WEB_USR_SCRIPT_RUN_T            "run"                           /*  �������� shell ����ű�     */
#define WEB_USR_SCRIPT_FRO_T            "front"                         /*  ����ʱ���� OpenR T ǰ����   */
#define WEB_USR_SCRIPT_AFT_T            "after"                         /*  ����ʱ���� OpenR T ������   */
/*********************************************************************************************************
  ���ú�
*********************************************************************************************************/
#define FILE_MODE                       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define USR_RUN_SRIPT_FILE              "/etc/usrRunScript.sh"          /*  �û��������нű�            */
#define USR_FRO_SRIPT_FILE              "/etc/usrFroScript.sh"          /*  �� OpenR T ǰ���нű�       */
#define USR_AFT_SRIPT_FILE              "/etc/usrAftScript.sh"          /*  �� OpenR T �����нű�       */
#define USR_SCRIPT_MAX_LEN              (10 * 1024)                     /*  �ű���� 10 K               */
/*********************************************************************************************************
** ��������: __usrScriptShow
** ��������: �û��ű�������ʾ
** �䡡��  : pcusrScriptVal     �ű�����
**           pcusrScriptType    �ű�����
**           iusrScriptLen      �ű�����
** �䡡��  : NONE
** ������  : ������
*********************************************************************************************************/
static INT32  __usrScriptShow (CHAR  *pcusrScriptVal, CHAR  *pcusrScriptType, INT32  iusrScriptLen)
{
    INT32     iFd;
    INT32     iRet;
    CHAR      cScriptFile[50]   = {0};

    /*
     *  ���ļ������������µ�����
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
** ��������: __usrScriptHandle
** ��������: �û��ű�����
** �䡡��  : pcusrScriptVal     �ű�����
**           pcusrScriptType    �ű�����
**           iusrScriptLen      �ű�����
** �䡡��  : NONE
** ������  : ������
*********************************************************************************************************/
static INT32  __usrScriptHandle (CHAR  *pcusrScriptVal, CHAR  *pcusrScriptType, INT32  iusrScriptLen)
{
    INT32     iRet;
    INT32     iFd;
    UCHAR     uNeedRunNow       = 0;
    CHAR      cScriptRunCmd[60] = {0};
    CHAR      cScriptFile[50]   = {0};

    /*
     *  ���ļ������������µ�����
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
** ��������: __usrScriptShowAction
** ��������: ������ʾ�û�����ű�����
** ��    ��: wp           ���������
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID  __usrScriptShowAction (Webs  *wp)
{
    cJSON                *pjusrScriptType = LW_NULL;
    cJSON                *pRecvjRes       = LW_NULL;
    cJSON                *pSendjRes       = LW_NULL;
    CHAR                 *cScriptVal      = LW_NULL;

    INT32                 iRet;
    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if(LW_NULL == pSendjRes) {
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
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
** ��������: __usrScriptSetAction
** ��������: �û��ű����ú�̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, (cJSON *)DEFAULT_FAILED_CJSON);
        operLogInsert(wp, OPEN_RT_DNS, "set usr shell cmd failed!\n", 0);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
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
** ��������: __usrScriptActionDefine
** ��������: �û��ű� ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __usrScriptActionDefine (VOID)
{
    websDefineAction("usrScriptShowAction",   __usrScriptShowAction);
    websDefineAction("usrScriptSetAction", __usrScriptSetAction);
}
/*********************************************************************************************************
** ��������: dnsActionInit
** ��������: DNS ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  usrScriptActionInit (VOID)
{
    __usrScriptActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
