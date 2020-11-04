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
** ��   ��   ��: flowctlExec.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: flowctl ����
*********************************************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "flowctl_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "network/lwip/sockets.h"

/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define CMD_BUFF_LEN   (256)                                            /*  ����������                  */
/*********************************************************************************************************
** ��������: flowctlDoConfig
** ��������: ���ӻ��޸Ļ�ɾ������ӿ�flowctl
** �䡡��  : pRouteInfo      ·�ɲ�����Ϣ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT  flowctlDoConfig (__PFLOWCTL_HANDLE  pFlowctlInfo)
{
    INT           iRet;
    INT           iStrlen;
    INT           iRestLength;
    CHAR          cCmdTmp[CMD_BUFF_LEN] = {0};

    if (LW_NULL == pFlowctlInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "flowctl data is null.\n");

       return  (PX_ERROR);
    }

    iStrlen      = snprintf(cCmdTmp, sizeof(cCmdTmp), "flowctl %s ", pFlowctlInfo->cOperType);
    iRestLength  = CMD_BUFF_LEN - iStrlen;
    iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%s ",
                           (pFlowctlInfo->iType == FCT_IP)? "ip":"if");

    if (FCT_IP == pFlowctlInfo->iType) {
         iRestLength  = CMD_BUFF_LEN - iStrlen;
         iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%s %s ",
                                 pFlowctlInfo->cStartIp, pFlowctlInfo->cEndIp);

         if (0 == strcmp(pFlowctlInfo->cProtocol, FC_PROTO_IP)) {
             iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "all ");

             pFlowctlInfo->iStartPort = -1;
             pFlowctlInfo->iEndPort   = -1;
         } else {
             iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%s %d %d ",
                                     pFlowctlInfo->cProtocol,
                                     pFlowctlInfo->iStartPort,
                                     pFlowctlInfo->iEndPort);
         }

         if (0 != strcmp(pFlowctlInfo->cOperType, FLOWCTL_DELETE)) {
        	 iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s %lu %lu ",
                                     pFlowctlInfo->cIfname, pFlowctlInfo->ulUpRate,
                                     pFlowctlInfo->ulDownRate);

             if(0 != pFlowctlInfo->ulBufSize) {
            	 iRestLength  = CMD_BUFF_LEN - iStrlen;
                 iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%lu ",pFlowctlInfo->ulBufSize);
             }
         } else {
           	 iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s",pFlowctlInfo->cIfname);
         }

    } else {
         if (0 != strcmp(pFlowctlInfo->cOperType, FLOWCTL_DELETE)) {
           	 iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s %lu %lu ",
                                     pFlowctlInfo->cIfname,  pFlowctlInfo->ulUpRate,
                                     pFlowctlInfo->ulDownRate);

             if (0 != pFlowctlInfo->ulBufSize) {
               	 iRestLength = CMD_BUFF_LEN - iStrlen;
                 iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, "%lu ",pFlowctlInfo->ulBufSize);
             }
         } else {
        	 iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s",pFlowctlInfo->cIfname);
         }
    }

    iRet = system(cCmdTmp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "failed to execute command %s.\n",cCmdTmp);

        return  (iRet);
    }

    strncpy(pFlowctlInfo->cEnable, "En", sizeof(pFlowctlInfo->cEnable));
    iRet += flowctlUpdateToDb(pFlowctlInfo->cOperType, pFlowctlInfo);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: flowctlRecovery
** ��������: flowctl�ָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : INT ִ�н��
*********************************************************************************************************/
INT  flowctlRecovery (VOID)
{
    INT                 iRet           = ERROR_NONE;
    __PFLOWCTL_HANDLE   pTmpNode       = LW_NULL;
    __PFLOWCTL_HANDLE   pFlowctlHeader = LW_NULL;
    INT                 iStrlen;
    INT                 iRecordNum;
    INT                 iRestLength;
    CHAR                cCmdTmp[CMD_BUFF_LEN];
    INT                 i;

    pFlowctlHeader = flowctlGetFromDB(LW_NULL, &iRecordNum);
    if (LW_NULL == pFlowctlHeader) {
        if (-1 == iRecordNum) {
            return  (PX_ERROR);
        } else {
            return  (ERROR_NONE);
        }
    }

    for (i = 0; i < iRecordNum; i++) {
        pTmpNode = pFlowctlHeader + i;
        bzero(cCmdTmp, sizeof(cCmdTmp));

        iStrlen      = snprintf(cCmdTmp, sizeof(cCmdTmp), "flowctl add ");
        iRestLength  = CMD_BUFF_LEN - iStrlen;
        iStrlen     += snprintf(cCmdTmp + iStrlen,
                                iRestLength, "%s ",
                                (pTmpNode->iType == FCT_IP) ? "ip":"if");

        if (pTmpNode->iType == FCT_IP) {
             iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%s %s ",
                                     pTmpNode->cStartIp, pTmpNode->cEndIp);

             if (0 == strcmp(pTmpNode->cProtocol, FC_PROTO_IP)) {
                 iRestLength  = CMD_BUFF_LEN - iStrlen;
                 iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "all ");
             } else {
                 iRestLength  = CMD_BUFF_LEN - iStrlen;
                 iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%s %d %d ",
                                         pTmpNode->cProtocol,
                                         pTmpNode->iStartPort,
                                         pTmpNode->iEndPort);
             }

             iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s %lu %lu ",
                                     pTmpNode->cIfname,  pTmpNode->ulUpRate,pTmpNode->ulDownRate);

             if (0 != pTmpNode->ulBufSize) {
                 iRestLength  = CMD_BUFF_LEN - iStrlen;
                 iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%lu ",pTmpNode->ulBufSize);
             }
        } else {
             iRestLength  = CMD_BUFF_LEN - iStrlen;
             iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "dev %s %lu %lu ",
                                     pTmpNode->cIfname,  pTmpNode->ulUpRate,pTmpNode->ulDownRate);

             if (0 != pTmpNode->ulBufSize) {
                 iRestLength  = CMD_BUFF_LEN - iStrlen;
                 iStrlen     += snprintf(cCmdTmp + iStrlen, iRestLength, "%lu ",pTmpNode->ulBufSize);
             }
        }

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet){
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute recovery command = %s, "
                                                  "error_code=%d.\n",cCmdTmp, iRet);
        }
    }

    free(pFlowctlHeader);

    return  (iRet);
}
