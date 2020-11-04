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
** ��   ��   ��: pingExec.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ping �������ò���
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ping_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define DATA_SIZE_DEFAULT          (32)                                 /*  Ĭ�� PING ���ݳ���          */
#define PING_INFO_FILE             "/tmp/pingInfo"                      /*  ��� PING �����Ϣ�ļ�λ��  */
/*********************************************************************************************************
** ��������: pingTestDoDeal
** ��������: Ping ����
** �䡡��  : pPingInfo  PING ������Ϣ
** �䡡��  : NONE
** ������  : PING ���Խ��
*********************************************************************************************************/
PVOID  pingTestDoDeal (__PPING_HANDLE  pPingInfo)
{
    INT     iRet;
    INT     iFd;
    PCHAR   pcResult       = NULL;
    CHAR    cDomain[128]   = {0};
    CHAR    cEnetName[32]  = {0};
    CHAR    cCmdBuf[256]   = {0};
    INT     iDataSize      = 0;

    /*
     *  ��ȡ PING ���Բ���
     */
    iDataSize = pPingInfo->iDataSize;
    strncpy(cDomain, pPingInfo->cDomain,     sizeof(cDomain));
    strncpy(cEnetName, pPingInfo->cEnetName, sizeof(cEnetName));

    if (iDataSize <= 0) {
        iDataSize = DATA_SIZE_DEFAULT;
    }

    /*
     *  ��ȡ PING ��������
     */
    sprintf(cCmdBuf, "ping %s -n %d -l %d &>%s", cDomain, 1, iDataSize, PING_INFO_FILE);

    lib_system(cCmdBuf);

    pcResult = (PCHAR)malloc(PING_TEST_RESULT_LEN);
    if (NULL == pcResult) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc memory failed.\n");

        return  (NULL);
    }

    bzero(pcResult, PING_TEST_RESULT_LEN);

    iFd = open(PING_INFO_FILE, O_RDONLY);
    if (iFd <= 2) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "open file failed.\n");

        goto  error;
    }

    /*
     *  ��ȡ PING ���Խ��
     */
    iRet = read(iFd, pcResult, PING_TEST_RESULT_LEN);
    if (iRet <= 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "read file failed.\n");

        goto  error;
    }

    close(iFd);

    return  (pcResult);

error:
    if (iFd > 2) {
        close(iFd);
    }

    if (NULL != pcResult) {
        free(pcResult);

        pcResult = NULL;
    }

    return  (NULL);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
