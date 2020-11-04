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
** ��   ��   ��: monitorExec.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 17 ��
**
** ��        ��: monitor����
*********************************************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "monitor_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "network/lwip/sockets.h"
#include "intadaption/intada_common.h"

/*********************************************************************************************************
  ��������
*********************************************************************************************************/
extern INT        tmStatItemCount(PTM_STATS  pCondition, INT* piFilter);
extern PTM_STATS  tmStatItemsCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
extern PTM_STATS  tmStatItemsSumCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition);
extern INT        getCpuUsage();
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
extern INT        tmGetOnlineUsers();
/*********************************************************************************************************
** ��������: monitorDoConfig
** ��������: �������ʹ��ȥʹ��ͳ����
** �䡡��  : pMonitorHandle      ͳ���λ����
** �䡡��  : none
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT  monitorDoConfig (PTM_STATS  pMonitorHandle)
{
    if (LW_NULL == pMonitorHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "monitor data is null.\n");

       return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: cpuMemDoShow
** ��������: ��ѯ cpu/�ڴ�/�����û���
** �䡡��  : pMonitorHandle     ͳ�����������
** �䡡��  : piRecordNum        ��¼����
** ������  : PTM_CPU_MEM����ĵ�ַ
**           �������ʧ�ܣ��򷵻�LW_NULL
*********************************************************************************************************/
PTM_CPU_MEM  cpuMemDoShow (PTM_STATS  pMonitorHandle, INT  *piRecordNum)
{
    PTM_CPU_MEM  ptmCpuMem = LW_NULL;
    size_t       stCpuUsed = 0;
    size_t       stPhyMemTotalSize;
    size_t       stUsedSize;

    *piRecordNum = 0;

    if (LW_NULL == pMonitorHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "monitor data is null.\n");

       return  (LW_NULL);
    }

    ptmCpuMem = (PTM_CPU_MEM)malloc(sizeof(TM_CPU_MEM));
    if (NULL == ptmCpuMem) {
       __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc failed.\n");

       return  (LW_NULL);
    }

    bzero(ptmCpuMem, sizeof(TM_CPU_MEM));

    if (TMT_CPU_MEM_USER == pMonitorHandle->cType) {
        stCpuUsed = getCpuUsage();

        getMemoryUsage(&stPhyMemTotalSize, &stUsedSize);

        ptmCpuMem->dCpuUsage = (stCpuUsed/1000.0)*100;
        ptmCpuMem->dMemoryUsage = (stUsedSize*1.0/stPhyMemTotalSize)*100;
        ptmCpuMem->uiOnlineUsers = tmGetOnlineUsers();
    }

    *piRecordNum = 1;

    return  (ptmCpuMem);
}
/*********************************************************************************************************
** ��������: __invalidMonitorInfoRemove
** ��������: ȥ����Ч�����Ϣ
** �䡡��  : pSrcMonitorInfo     ·�ɲ�����Ϣ
**           iSrcRecordNum       �����Ϣ����
**           iNewRecordNum       ��Ч�����Ϣ����ָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
PVOID  __invalidMonitorInfoRemove (PTM_STATS  pSrcMonitorInfo,
                                   INT        iSrcRecordNum,
                                   INT       *piNewRecordNum)
{
    INT              i;
    INT              iRet;
    INT              iNum              = 0;
    INT              iStatus           = 0;
    PTM_STATS        pMonitorInfo      = LW_NULL;
    PTM_STATS        pValidMonitorInfo = LW_NULL;
    PTM_STATS        pValidMonitorTmp  = LW_NULL;

    *piNewRecordNum = 0;

    if (NULL == pSrcMonitorInfo) {
        return  (NULL);
    }

    for (i = 0; i < iSrcRecordNum; i++) {
        pMonitorInfo = pSrcMonitorInfo + i;

        if (strstr(pMonitorInfo->pIfname, "en")) {
            iRet = intadaStatusGetFromDbByName(pMonitorInfo->pIfname, &iStatus);
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  �����Ӳ��������Ч          */
                iNum++;
            } else {
                continue;
            }
        } else {                                                        /*  ��Ӳ�����ڣ�����������      */
            iNum++;
        }
    }

    pValidMonitorInfo = (PTM_STATS)malloc(iNum * sizeof(TM_STATS));
    if (NULL == pValidMonitorInfo) {
        return  (NULL);
    }

    pValidMonitorTmp = pValidMonitorInfo;

    for (i = 0; i < iSrcRecordNum; i++) {
        pMonitorInfo = pSrcMonitorInfo + i;

        if (strstr(pMonitorInfo->pIfname, "en")) {
            iRet = intadaStatusGetFromDbByName(pMonitorInfo->pIfname, &iStatus);
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  �����Ӳ��������Ч          */
                memcpy(pValidMonitorTmp, pMonitorInfo, sizeof(TM_STATS));

                pValidMonitorTmp += 1;
            } else {
                continue;
            }
        } else {
            memcpy(pValidMonitorTmp, pMonitorInfo, sizeof(TM_STATS));

            pValidMonitorTmp += 1;
        }
    }

    *piNewRecordNum = iNum;

    return  (pValidMonitorInfo);
}
/*********************************************************************************************************
** ��������: monitorDoShow
** ��������: ���ݹ���������ѯ
** �䡡��  : pMonitorHandle     ͳ�����������
** �䡡��  : piRecordNum        ��¼����
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
PTM_STATS  monitorDoShow (PTM_STATS  pMonitorHandle, INT  *piRecordNum)
{
    static UINT32  uiWanSendTotalLen;
    static UINT32  uiWanRecvTotalLen;

    INT            iCount            = 0;
    INT            iFilter           = 0;
    UINT32         uiSendTotalLenSum = 0;
    UINT32         uiRecvTotalLenSum = 0;
    PTM_STATS      pMonitorHeader    = NULL;
    PTM_STATS      pMonitorReturn    = NULL;
    PTM_STATS      pMonitorInfo      = NULL;
    INT            i;

    *piRecordNum = 0;

    if (LW_NULL == pMonitorHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "monitor data is null.\n");

       return  (LW_NULL);
    }

    if (TMT_TERM == pMonitorHandle->cType) {
        if (strlen(compact_strtrim(pMonitorHandle->pIpAddr)) > 0) {
            iFilter = TMF_IP;
        }
        else if (strlen(compact_strtrim(pMonitorHandle->pMac)) > 0) {
            iFilter = TMF_MAC;
        }
    }

    if (TMT_LINE_APP == pMonitorHandle->cType) {
        iCount = (APP_TYPE_MAX - 1);
    } else if (TMT_WAN_TOTAL == pMonitorHandle->cType) {
        iCount = 1;
    } else {
        iCount = tmStatItemCount(pMonitorHandle, &iFilter);
    }

    pMonitorHeader = (PTM_STATS)malloc(iCount * sizeof(TM_STATS));
    bzero(pMonitorHeader, iCount * sizeof(TM_STATS));

    if (TMT_LINE_APP == pMonitorHandle->cType || TMT_WAN_TOTAL == pMonitorHandle->cType) {
        pMonitorReturn = tmStatItemsSumCopy(&pMonitorHeader, pMonitorHandle);

        if (TMT_WAN_TOTAL == pMonitorHandle->cType) {
            uiWanSendTotalLen     = pMonitorReturn->iSendTotalLen;
            uiWanRecvTotalLen     = pMonitorReturn->iRecvTotalLen;
            pMonitorReturn->cType = TMT_WAN_TOTAL;
        } else {
             for (i = 0;i < (iCount - 1); i++) {
                 uiSendTotalLenSum += (pMonitorReturn + i)->iSendTotalLen;
                 uiRecvTotalLenSum += (pMonitorReturn + i)->iRecvTotalLen;
             }

             if (uiWanSendTotalLen > uiSendTotalLenSum) {
                 (pMonitorReturn + APP_TYPE_OTHERS - 1)->iSendTotalLen = uiWanSendTotalLen - uiSendTotalLenSum;
             }

             if (uiWanRecvTotalLen > uiRecvTotalLenSum){
                 (pMonitorReturn + APP_TYPE_OTHERS - 1)->iRecvTotalLen = uiWanRecvTotalLen - uiRecvTotalLenSum;
             }

             (pMonitorReturn + APP_TYPE_OTHERS - 1)->cProtocol = APP_TYPE_OTHERS;
        }
    } else {
        pMonitorReturn = tmStatItemsCopy(&pMonitorHeader, pMonitorHandle, &iFilter);
    }

    if (NULL != pMonitorReturn && iCount > 0) {
        pMonitorInfo = __invalidMonitorInfoRemove(pMonitorReturn, iCount, piRecordNum);
    } else {
        pMonitorInfo = NULL;
    }

    return  (pMonitorInfo);
}
/*********************************************************************************************************
** ��������: __internetStatusCheck
** ��������: ��⻥��������״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ����ʧ��
**           ERROR_NONE         ���ӳɹ�
*********************************************************************************************************/
static INT32  __internetStatusCheck (VOID)
{
    return  (system("ping 114.114.114.114 -n 1 -w 200 >/dev/null"));
}
/*********************************************************************************************************
** ��������: InternetStatusGet
** ��������: ����������״̬��ȡ
** �䡡��  : NONE
** �䡡��  : pInternetStatus    ����״̬��Ϣ
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT32  internetStatusGet (__PINTERNET_STATUS  pInternetStatus)
{
    static time_t    llInternetTime    = 0;                             /*  �����һ������������ʱ��    */
    static INT32     iUnConnectedCount = 0;                             /*  �Ͽ����Ӵ���                */
    struct timespec  tsMono;                                            /*  ��������ʱ��                */

    if (LW_NULL == pInternetStatus) {
        return  (PX_ERROR);
    }

    if (ERROR_NONE == __internetStatusCheck()) {
        if (!llInternetTime) {
            lib_clock_gettime(CLOCK_MONOTONIC, &tsMono);

            llInternetTime = tsMono.tv_sec;
        }

        iUnConnectedCount = 0;
    } else {
        iUnConnectedCount++;
    }

    if (!llInternetTime || 4 <= iUnConnectedCount) {
        llInternetTime            = 0;
        pInternetStatus->ucStatus = INTERNET_FAILED;
        pInternetStatus->llTime   = 0;
    } else {
        lib_clock_gettime(CLOCK_MONOTONIC, &tsMono);

        pInternetStatus->ucStatus = INTERNET_OK;
        pInternetStatus->llTime   = tsMono.tv_sec - llInternetTime;
    }

    return  (ERROR_NONE);
}
