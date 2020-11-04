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
** ��   ��   ��: updatefirmwareaction.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 2 ��
**
** ��        ��: update firmware ���ú�̨�ӿ�
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
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define DEFAULT_FAILED_CJSON       "{\n\t\"code\":\t500\n}"             /*  json�ṹ����ʧ��ʱʹ��      */
#define JSON_ARGERR_CODE           (100)                                /*  �������󷵻�Json codeֵ     */

/*********************************************************************************************************
** ��������: __updateFirmwareAction
** ��������: update firmware�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareAction (Webs  *wp)
{
    cJSON       *pjSend       = LW_NULL;
    WebsKey     *pKey         = LW_NULL;
    WebsUpload  *pUpLoad      = LW_NULL;
    INT          iFlag        = 0;
    INT          iRet         = PX_ERROR;

    /*
     * �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ����http��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
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

                goto __error;                                           /*  ��������                    */
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
** ��������: __updateFirmwareActionDefine
** ��������: update firmware��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareActionDefine (VOID)
{
    websDefineAction("updateFirmwareAction", __updateFirmwareAction);
}

/*********************************************************************************************************
** ��������: updateFirmwareActionInit
** ��������: update firmware��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  updateFirmwareActionInit (VOID)
{
    __updateFirmwareActionDefine();
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
