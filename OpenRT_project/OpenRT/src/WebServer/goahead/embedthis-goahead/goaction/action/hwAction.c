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
** ��   ��   ��: hwAction.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 2 ��
**
** ��        ��: OpenRT Ӳ����Ϣ��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "lwip/netif.h"
#include "netdev.h"
#include "sys/un.h"
#include "../webHandle.h"
#include "hw_info.h"
#include <net/lwip/event/lwip_netevent.h>
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: Ӳ����Ϣ
*********************************************************************************************************/
#define WEB_HW_CPU            "cpu"                                     /*  Json�ֶΣ�CPU               */
#define WEB_HW_MEM            "mem"                                     /*  Json�ֶΣ��ڴ�              */
#define WEB_HW_STO            "sto"                                     /*  Json�ֶΣ��洢              */
/*********************************************************************************************************
** ��������: __hwInfoGetAction
** ��������: ��ȡӲ����Ϣ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __hwInfoGetAction (Webs *wp)
{
    cJSON                *pSendjRes = LW_NULL;
    __HW_INFO             hwInfo;
    INT                   iRet;

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
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    bzero(&hwInfo, sizeof(hwInfo));

    iRet = libHardWareInfoGet(&hwInfo);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    cJSON_AddStringToObject(pSendjRes, WEB_HW_CPU, hwInfo.cCpuInfo);
    cJSON_AddStringToObject(pSendjRes, WEB_HW_MEM, hwInfo.cMemInfo);
    cJSON_AddStringToObject(pSendjRes, WEB_HW_STO, hwInfo.cStoInfo);

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/***************************************************s******************************************************
** ��������: __hwActionDefine
** ��������: LOG ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __hwActionDefine (VOID)
{
    websDefineAction("hwInfoGetAction", __hwInfoGetAction);
}
/*********************************************************************************************************
** ��������: hwActionInit
** ��������: LOG ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID hwActionInit (VOID)
{
    __hwActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
