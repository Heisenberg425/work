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
** ��   ��   ��: pingAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ PING �������ú�̨�ӿ�
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
  �궨��
*********************************************************************************************************/
#define PING_INFO_LEN              (1024)                               /*  PING ������Ϣ����           */
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key: PING ���ü�·����Ϣ��ʾ
*********************************************************************************************************/
#define WEB_IP_DOMAIN             "ip/domain"                           /*  Json�ֶΣ�IP������          */
#define WEB_SRC_INTERFACE         "interface"                           /*  Json�ֶΣ�Դ�ӿ�����        */
#define WEB_PING_TIMES            "times"                               /*  Json�ֶΣ�PING ��������     */
#define WEB_PING_DATA_SIZE        "data_size"                           /*  Json�ֶΣ����ݳ���          */
#define WEB_PING_RESULT           "result"                              /*  Json�ֶΣ�PING �������     */
/*********************************************************************************************************
** ��������: __pingTestInfoGet
** ��������: ��ȡ PING ������Ϣ�Ĳ��ֽ��
** �䡡��  : cpcInputBuf       PING ���Խ��������
**           pingHandle        PING ���Խṹ
** �䡡��  : NONE
** ������  : �ɹ����� PING ���Խ���ַ�����ַ��ʧ�ܷ��� NULL
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
     *  ��ȡ PING ������Ϣ
     */
    pcStr = lib_strstr(cpcInputBuf, cStr);
    if (NULL != pcStr) {                                                /*  ����Pinging+ip/domain ��Ϣ  */
        pcTmp = strchr(pcStr, '\n');
        iBufLen = pcTmp - pcStr + 1;

        lib_memcpy(pcPingInfo, pcStr, iBufLen);
        iOffset += iBufLen;

        if (lib_strstr(cpcInputBuf, "Request time out.")) {             /*  ����ʱ                    */
            pcStr = lib_strstr(cpcInputBuf, "Request time out.");
            lib_memcpy(pcPingInfo + iOffset, pcStr, strlen("Request time out.\n"));
        } else if (lib_strstr(cpcInputBuf, "error: Invalid argument or format.")) {

            /*
             *  PING ���Բ������ʽ��Ч
             */
            pcStr = lib_strstr(cpcInputBuf, "error: Invalid argument or format.");
            lib_memcpy(pcPingInfo + iOffset, pcStr, strlen("error: Invalid argument or format.\n"));
        } else if (lib_strstr(cpcInputBuf, "Reply from")) {             /*  PING ���Գɹ�               */
            pcStr = lib_strstr(cpcInputBuf, "Reply from");
            pcTmp = strchr(pcStr, '\n');
            iBufLen = pcTmp - pcStr + 1;

            lib_memcpy(pcPingInfo + iOffset, pcStr, iBufLen);
        } else {                                                        /*  ����                        */
            lib_memcpy(pcPingInfo + iOffset, "unkown error\n", strlen("unkown error\n"));
        }
    } else if (lib_strstr(cpcInputBuf, "Pinging request could not find host")) {

        /*
         *  �Ҳ�������
         */
        pcStr = lib_strstr(cpcInputBuf, "Pinging request could not find host");
        pcTmp = strchr(pcStr, '\n');
        iBufLen = pcTmp - pcStr + 1;

        lib_memcpy(pcPingInfo + iOffset, pcStr, iBufLen);
    } else {                                                            /*  ������Ч                    */
        lib_memcpy(pcPingInfo + iOffset, "arguments error.\n", strlen("arguments error.\n"));
    }

    return  (pcPingInfo);
}
/*********************************************************************************************************
** ��������: __pingTestAction
** ��������: PING ������Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
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
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
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
     *  ���ò�ѯ����ӿڲ����÷��ص� Json ����
     */
    pcPingHeader = pingTest(&pingHandle, &iRecordTotal);

    /*
     *  �� PING ���Խ���ַ����л�ȡǰ������ʾ�Ľ��
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
** ��������: __pingActionDefine
** ��������: ping ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __pingActionDefine (VOID)
{
    websDefineAction("pingTestAction",   __pingTestAction);
}
/*********************************************************************************************************
** ��������: pingActionInit
** ��������: ping action ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  pingActionInit (VOID)
{
    __pingActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
