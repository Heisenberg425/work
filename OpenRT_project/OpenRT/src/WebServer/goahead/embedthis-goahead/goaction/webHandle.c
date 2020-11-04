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
** ��   ��   ��: webHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 ��6 �� 9 ��
**
** ��        ��: action �ӿ� web��������
*********************************************************************************************************/
#include <stdio.h>

#include "webHandle.h"
/*********************************************************************************************************
** ��������: cJsonErrorReturnInit
** ��������: ����ʱ���� Json ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  cJsonErrorReturnInit (VOID)
{
    cJSON_AddNumberToObject(&_G_ErrorJRes, "code", 500);
}


/*********************************************************************************************************
** ��������: webHeadHandle
** ��������: action �յ�����ʱ��Web ͷ���Ĵ���
** �䡡��  : wp                 Web ���
**           iStates            http ͷcode����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  webHeadHandle (Webs *wp, INT  iStates)
{
    if (LW_NULL == wp) {
        return  (PX_ERROR);
    }

    websSetStatus(wp, iStates);                                         /*  http״̬                    */
    websWriteHeaders(wp, -1, 0);                                        /*  -1��ʾ�Զ��������ݳ���      */
    websWriteEndHeaders(wp);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: webBackHandle
** ��������: action �������ʱ������ʱ�Ĵ���
** �䡡��  : wp                 Web ���
**           cpJson             ���ص�cpJson����
** �䡡��  : NONE
** ������  : ERROR_CODE
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
