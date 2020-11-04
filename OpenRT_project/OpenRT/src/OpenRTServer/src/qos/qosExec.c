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
** ��   ��   ��: qosExec.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 15 ��
**
** ��        ��: QoS ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "qos_common.h"
#include "common.h"
#include "fileparser/fileparser.h"

/*********************************************************************************************************
** ��������: __qosIsValid
** ��������: �ж������Ƿ���Ч
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : LW_TRUE            ������Ч
**           LW_FALSE           ������Ч
*********************************************************************************************************/
static INT  __qosIsValid (__PQOS_HANDLE  pQosInfo)
{
    if (LW_NULL == pQosInfo) {
        return (LW_FALSE);
    }

    /*
     * ���ȼ���Χ�ж�
     */
    if (pQosInfo->uiPriority > QOS_CFG_PRIORITY_MAX ||
        pQosInfo->uiPriority < QOS_CFG_PRIORITY_MIN) {
        return (LW_FALSE);
    }

    /*
     * Dont Drop��Χ�ж�
     */
    if (pQosInfo->uiDontDrop != 0 &&
        pQosInfo->uiDontDrop != 1) {
        return (LW_FALSE);
    }

    /*
     * Rule ��Χ�ж�
     */
    if (strcmp(pQosInfo->cRule, QOS_CFG_RULE_IP)   != 0 &&
        strcmp(pQosInfo->cRule, QOS_CFG_RULE_TCP)  != 0 &&
        strcmp(pQosInfo->cRule, QOS_CFG_RULE_UDP)  != 0) {
        return (LW_FALSE);
    }

    /*
     * CmpMethod ��Χ�ж�
     */
    if (strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_SRC)   != 0 &&
        strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_DEST)  != 0 &&
        strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_BOTH)  != 0) {
        return (LW_FALSE);
    }

    return (LW_TRUE);
}
/*********************************************************************************************************
** ��������: __qosCompare
** ��������: �Ƚ� QoS ����
** �䡡��  : pQosInfo1          QoS ������Ϣ1
**           pQosInfo2          QoS ������Ϣ2
** �䡡��  : NONE
** ������  : 0                  ���� QoS �������
**           ����               ���� QoS ���ò���
*********************************************************************************************************/
static INT  __qosCompare (__PQOS_HANDLE  pQosInfo1, __PQOS_HANDLE  pQosInfo2)
{
    INT    iResult = -1;

    if (NULL == pQosInfo1 || NULL == pQosInfo2) {
        return  (PX_ERROR);
    }

    iResult = (!(pQosInfo1->uiIpStart    == pQosInfo2->uiIpStart)     ||
               !(pQosInfo1->uiIpEnd      == pQosInfo2->uiIpEnd)       ||
               !(pQosInfo1->uiPriority   == pQosInfo2->uiPriority)    ||
               !(pQosInfo1->uiDontDrop   == pQosInfo2->uiDontDrop)    ||
                strcmp(pQosInfo1->cNetif, pQosInfo2->cNetif)          ||
                stricmp(pQosInfo1->cRule, pQosInfo2->cRule)           ||
                strcmp(pQosInfo1->cCmpMethod, pQosInfo2->cCmpMethod));

    if (0 != stricmp(pQosInfo1->cRule, "ip")) {
        iResult =  iResult                                              ||
                 !(pQosInfo1->uiPortStart  == pQosInfo2->uiPortStart)   ||
                 !(pQosInfo1->uiPortEnd    == pQosInfo2->uiPortEnd);
    }

    return  (iResult);
}
/*********************************************************************************************************
** ��������: __qosShellGetSeqNumOper
** ��������: Shell ��ȡ seqnum
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : seqnum С�� 0 ��ʾδ�ҵ�
*********************************************************************************************************/
static INT  __qosShellGetSeqNumOper (__PQOS_HANDLE  pQosInfo)
{
    FILE             *pFp            = LW_NULL;
    PCHAR             pToken         = LW_NULL;
    CHAR              cBuf[256]      = {0};
    __QOS_HANDLE      qosInfo;
    INT               iSeqNum        = -1;
    INT               i;
    INT               iFlag          = 0;

    if (LW_NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    pFp = popen("qoss", "r");
    if (LW_NULL == pFp) {
        return  (PX_ERROR);
    }

    (void)fgets(cBuf, sizeof(cBuf), pFp);                               /*  ��ȡ��һ�в�����            */
    while (memset(cBuf, 0, sizeof(cBuf)),
            fgets(cBuf, sizeof(cBuf), pFp) != LW_NULL) {

        if (strlen(cBuf) < 20) {
            continue;                                                   /*  ɾ�����Ȳ�����              */
        }

        pToken = strtok(cBuf, " ");                                     /*  ��ȡÿһ���ֶ�              */
        i = 0;

        while (LW_NULL != pToken) {
           switch (i) {
           case 0:                                                      /*  NETIF                       */
               strcpy(qosInfo.cNetif, pToken);
               break;

           case 1:                                                      /*  ATTACH                      */
               break;

           case 2:                                                      /*  SEQNUM                      */
               iSeqNum = atoi(pToken);
               break;

           case 3:                                                      /*  RULE                        */
               strcpy(qosInfo.cRule, pToken);
               break;

           case 4:                                                      /*  CMP_METHOD                  */
               strcpy(qosInfo.cCmpMethod, pToken);
               break;

           case 5:                                                      /*  PRIO                        */
               qosInfo.uiPriority = atoi(pToken);
               break;

           case 6:                                                      /*  DONT_DROP                   */
               if (0 == strcmp(pToken, "YES")) {
                   qosInfo.uiDontDrop = 1;
               } else {
                   qosInfo.uiDontDrop = 0;
               }

               break;

           case 7:                                                      /*  IPs                         */
               qosInfo.uiIpStart = inet_addr(pToken);
               break;

           case 8:                                                      /*  IPe                         */
               qosInfo.uiIpEnd = inet_addr(pToken);
               break;

           case 9:                                                      /*  PORTs                       */
               qosInfo.uiPortStart = atoi(pToken);
               break;

           case 10:                                                     /*  PORTe                       */
               qosInfo.uiPortEnd = atoi(pToken);
               break;

           default:
               break;
           }

           pToken = strtok(LW_NULL, " ");
           i++;
        }
        if (0 == __qosCompare(&qosInfo, pQosInfo)) {
            iFlag = 1;
            break;                                                      /*  �ҵ�SeqNum                  */
        }
    }

    pclose(pFp);

    return (iFlag ? iSeqNum : PX_ERROR);
}
/*********************************************************************************************************
** ��������: __qosShellAddOper
** ��������: Shell ���� QoS
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __qosShellAddOper (__PQOS_HANDLE  pQosInfo)
{
    CHAR    cCmdTmp[512]  = {0};
    CHAR    cIpStart[20]  = {0};
    CHAR    cIpEnd[20]    = {0};
    CHAR    cDontDrop[10] = {0};
    INT     iRet          = ERROR_NONE;

    if (LW_NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    if (PX_ERROR != __qosShellGetSeqNumOper(pQosInfo)) {
        return  (PX_ERROR);                                             /*  �Ѵ��ڴ��� QoS ����         */
    }

    strncpy(cIpStart, inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpStart), sizeof(cIpStart));
    strncpy(cIpEnd,   inet_ntoa(*(struct in_addr *)&pQosInfo->uiIpEnd),   sizeof(cIpEnd));

    if (0 == pQosInfo->uiDontDrop) {
        strcpy(cDontDrop, "no");
    } else {
        strcpy(cDontDrop, "yes");
    }

    if (0 == strcmp(pQosInfo->cRule, "ip")) {
        snprintf(cCmdTmp, sizeof(cCmdTmp), "%s %s %s %s %s %s %u %s", QOS_SHELL_CFG_ADD,
                                           pQosInfo->cNetif, pQosInfo->cRule,
                                           cIpStart, cIpEnd, pQosInfo->cCmpMethod,
                                           pQosInfo->uiPriority, cDontDrop);
        pQosInfo->uiPortStart   = 0;
        pQosInfo->uiPortEnd     = 0;
    } else {
        snprintf(cCmdTmp, sizeof(cCmdTmp), "%s %s %s %s %s %u %u %s %u %s", QOS_SHELL_CFG_ADD,
                                           pQosInfo->cNetif, pQosInfo->cRule,
                                           cIpStart, cIpEnd, pQosInfo->uiPortStart,
                                           pQosInfo->uiPortEnd, pQosInfo->cCmpMethod,
                                           pQosInfo->uiPriority, cDontDrop);
    }

    iRet = system(cCmdTmp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to config QoS:\n%s\n"
                                                "error_code=%d.\n", cCmdTmp, iRet);
    }

    return  (iRet);
}

/*********************************************************************************************************
** ��������: __qosShellDelOper
** ��������: Shell ɾ�� QoS ����
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __qosShellDelOper (__PQOS_HANDLE  pQosInfo)
{
    INT     iRet        = PX_ERROR;
    INT     iSeqNum     = -1;
    CHAR    cCmdTmp[64] = {0};

    if (LW_NULL == pQosInfo) {
        return  (PX_ERROR);
    }

    iSeqNum = __qosShellGetSeqNumOper (pQosInfo);
    if (iSeqNum >= 0) {
        snprintf(cCmdTmp, sizeof(cCmdTmp), "%s %s %d", QOS_SHELL_CFG_DEL, pQosInfo->cNetif, iSeqNum);

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to config qos:\n%s\n"
                                                    "error_code=%d.\n", cCmdTmp, iRet);
        }
    } else {
        iRet = iSeqNum;
    }

    return  (iRet);
}
/*********************************************************************************************************
** ��������: qosDoConfig
** ��������: ���� QoS
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  qosDoConfig (__PQOS_HANDLE  pQosInfo)
{
    INT  iRet = ERROR_NONE;

    if (LW_NULL == pQosInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is null.\n");

       return  (PX_ERROR);
    }

    /*
     * �ж������Ƿ���Ч
     */
    if (LW_FALSE == __qosIsValid(pQosInfo)) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is Invalid.\n");

        return  (PX_ERROR);
    }

    /*
     * �ж������Ƿ��Ѿ�����
     */
    iRet = qosIsExists(pQosInfo);
    if (LW_FALSE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is exists.\n");

        return  (PX_ERROR);
    }

    iRet = __qosShellAddOper(pQosInfo);
    if (ERROR_NONE != iRet) {
        return  (iRet);
    }

    return  qosConfigToDb(pQosInfo);
}
/*********************************************************************************************************
** ��������: qosDoDel
** ��������: ɾ�� QoS ����
** �䡡��  : pQosInfo           QoS ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  qosDoDel (__PQOS_HANDLE  pQosInfo)
{
    INT  iRet;

    if (LW_NULL == pQosInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is null.\n");

       return  (PX_ERROR);
    }

    iRet = __qosShellDelOper(pQosInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    return qosDeleteFromDb(pQosInfo);
}
/*********************************************************************************************************
** ��������: qosRecovery
** ��������: QoS ���ûָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  qosRecovery (VOID)
{
    __PQOS_HANDLE pQosHeader   = LW_NULL;
    __PQOS_HANDLE pTmpNode     = LW_NULL;
    INT           iRecordNum   = 0;
    INT           i            = 0 ;
    INT           iRet         = ERROR_NONE;

    pQosHeader = qosGetFromDB(&iRecordNum);
    if (LW_NULL == pQosHeader && 0 == iRecordNum) {
        return  (ERROR_NONE);
    }

    sleep(1);                                                           /*  �ȴ�ϵͳ�ȶ�                */

    for (i = 0; i < iRecordNum; i++) {
        pTmpNode = pQosHeader + i;
        if (ERROR_NONE != __qosShellAddOper(pTmpNode)) {
            iRet = PX_ERROR;
        }
    }

    if (NULL != pQosHeader) {
        free(pQosHeader);
    }

    return  (iRet);
}
