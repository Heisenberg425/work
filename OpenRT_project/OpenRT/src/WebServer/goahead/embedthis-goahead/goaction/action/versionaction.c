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
** ��   ��   ��: versionaction.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 8 ��
**
** ��        ��: version��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "version_info.h"
#include "../webHandle.h"
#include "asdefense_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define JSON_ARGERR_CODE           (100)                                /*  �������󷵻�Json codeֵ     */
/*********************************************************************************************************
  Jason key: version��Ϣ��ʾ
*********************************************************************************************************/
#define WEB_VERSION_INFO          "version"                             /*  Json�ֶΣ�ϵͳ�汾          */
/*********************************************************************************************************
** ��������: __versionShowAction
** ��������: version��Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __versionShowAction (Webs  *wp)
{
    __PVERSION_HANDLE     pVersionHeader    = LW_NULL;
    __VERSION_HANDLE      VersionHandle;
    cJSON*                pjSend            = LW_NULL;
    INT                   iRecordTotal      = 0;

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

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    if (LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);
        return;
    }

    /*
     *  ���ò�ѯ����ӿ�
     */
    pVersionHeader = versionShow(&VersionHandle, &iRecordTotal);

    cJSON_AddStringToObject(pjSend, WEB_VERSION_INFO, pVersionHeader->cVer);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}

/*********************************************************************************************************
** ��������: __versionActionDefine
** ��������: version��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __versionActionDefine (VOID)
{
    websDefineAction("versionShowAction", __versionShowAction);
}

/*********************************************************************************************************
** ��������: versionActionInit
** ��������: version��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  versionActionInit (VOID)
{
    __versionActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
