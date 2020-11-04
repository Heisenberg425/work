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
** ��   ��   ��: arpExec.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 13 ��
**
** ��        ��: arp ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "arp_common.h"
#include "common.h"
#include "fileparser/fileparser.h"

/*********************************************************************************************************
** ��������: __arpSetResultCheck
** ��������: ��ѯ ARP �Ƿ����óɹ�
** �䡡��  : pcIPData            IP ��ַ
** �䡡��  : pcMACData           mac ��ַ
** ������  : ERROR_NONE         ���óɹ�
**           PX_ERROR           ����ʧ��
*********************************************************************************************************/
static INT  __arpSetResultCheck (PCHAR  pcIPData, PCHAR  pcMACData)
{
    FILE   *pFp      = LW_NULL;
    CHAR   cBuf[100] = {0};
    PCHAR  pcIpTmp   = LW_NULL;
    PCHAR  pcMacTmp  = LW_NULL;
    INT    iIpLen    = 0;
    INT    iMacLen   = 0;

    if (LW_NULL == pcIPData || LW_NULL == pcMACData) {
       return  (PX_ERROR);
    }

    iIpLen  = strlen(pcIPData);
    iMacLen = strlen(pcMACData);

    pFp = popen("arp -a", "r");
    if (LW_NULL == pFp) {
        return  (PX_ERROR);
    }

    (VOID)fgets(cBuf, sizeof(cBuf), pFp);                               /* ��ȡ��һ�У�����             */

    bzero(cBuf, sizeof(cBuf));

    while (LW_NULL != fgets(cBuf, sizeof(cBuf), pFp)) {
        if (NULL == strstr(cBuf, "static")) {
            continue;
        }

        if ((NULL == strstr(cBuf, pcIPData)) || (NULL == strstr(cBuf, pcMACData))) {
            continue;
        }

        pcIpTmp  = strstr(cBuf, pcIPData);
        pcMacTmp = strstr(cBuf, pcMACData);
        if ((LW_NULL != pcIpTmp) && (LW_NULL != pcIpTmp)) {
            if ((' ' == pcIpTmp[iIpLen]) && (' ' == pcMacTmp[iMacLen])) {
                return  (ERROR_NONE);
            } else {
                continue;
            }
        }
    }

    return  (PX_ERROR);
}
#ifdef ARP_DEL_CHECK
/*********************************************************************************************************
** ��������: __arpShellGet
** ��������: shell����ͨ��ip��ȡmac
** �䡡��  : pIPData            IP ��ַ
** �䡡��  : cMACData           mac ��ַ
** ������  : ERROR_NONE         ��ȡ����Ӧ mac
**           PX_ERROR           δ�ҵ���Ӧ ip
*********************************************************************************************************/
static INT  __arpShellGet (PCHAR  pIPData, PCHAR  cMACData)
{
    FILE   *fp       = LW_NULL;
    CHAR   cBuf[100] = {0};

    if (LW_NULL == pIPData) {
       return  (PX_ERROR);
    }

    fp = popen("arp -a", "r");
    if (LW_NULL == fp) {
        return  (PX_ERROR);
    }

    (VOID)fgets(cBuf, sizeof(cBuf), fp);                                /* ��ȡ��һ�У�����             */

    while (memset(cBuf, 0, sizeof(cBuf)),
           fgets(cBuf, sizeof(cBuf), fp) != LW_NULL) {
        CHAR   *pTmp = cBuf;

        while (*(pTmp + 1) != '\0') {
            pTmp++;
        }

        while (*pTmp != ' ' && pTmp > cBuf) {
            pTmp--;                                                     /* ָ�����һ���ֶ�             */
        }

        if (strncmp(pTmp, " static", 7) != 0) {                         /* ֻ������̬arp                */
            continue;
        }

        pTmp = cBuf;
        while (*pTmp != ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;
        }

        while (*pTmp == ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;                                                   /* ָ��ڶ����ֶ�               */
        }

        if (strncmp(pTmp, pIPData, strlen(pIPData)) != 0) {             /* �ҵ���Ӧip                   */
            continue;
        }

        while (*pTmp != ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;
        }

        while (*pTmp == ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;                                                   /* ָ��������ֶ�               */
        }

        if (cMACData != 0) {
            memcpy(cMACData, pTmp, 17);                                 /* ��ȡmac��ַ�ֶ�              */
            cMACData[17] = '\0';
        }

        pclose(fp);

        return  (ERROR_NONE);
    }

    /*
     * δ�ҵ���Ӧ ip ����
     */
    pclose(fp);

    return  (PX_ERROR);
}
#endif
/*********************************************************************************************************
** ��������: __arpShellSet
** ��������: shell �������� arp
** �䡡��  : pcIPData          ip  ��Ϣ
**           pcMACData         mac ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpShellSet (PCHAR  pcIPData, PCHAR  pcMACData)
{
    INT     iRet         = ERROR_NONE;
    CHAR    cCmdTmp[200] = {0};

    if (LW_NULL == pcIPData || LW_NULL == pcMACData) {
        return  (PX_ERROR);
    }

    snprintf(cCmdTmp, sizeof(cCmdTmp), "arp -s %s %s", pcIPData, pcMACData);

    iRet = system(cCmdTmp);                                             /* ����arp                      */
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                               "error_code=%d.\n",cCmdTmp, iRet);
         return  (PX_ERROR);
    }

    /*
     * У���Ƿ����óɹ�
     */
    if (ERROR_NONE == __arpSetResultCheck(pcIPData, pcMACData)) {
        return  (ERROR_NONE);
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: __arpShellDel
** ��������: shell ����ɾ�� arp
** �䡡��  : pIPData           ip ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpShellDel (PCHAR  pIPData)
{
    CHAR    cCmdTmp[200] = {0};
    INT     iRet         = ERROR_NONE;

    if (LW_NULL == pIPData) {
        return  (PX_ERROR);
    }

    snprintf(cCmdTmp, sizeof(cCmdTmp), "arp -d %s", pIPData);

    iRet = system(cCmdTmp);                                             /* ɾ��arp                      */
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                               "error_code=%d.\n",cCmdTmp, iRet);
         return  (PX_ERROR);
    }

#ifdef ARP_DEL_CHECK
    /*
     * У���Ƿ�ɾ���ɹ�
     */
    if (__arpShellGet(pIPData, LW_NULL) != 0) {
        return  (ERROR_NONE);
    } else {
        return  (PX_ERROR);
    }
#else
    return  (ERROR_NONE);
#endif
}
/*********************************************************************************************************
** ��������: __arpHandleLinkInit
** ��������: ��ʼ�� arphandle ����
** �䡡��  : arpHandleHead    ����ͷ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __arpHandleLinkInit (__PARP_HANDLE_NODE  pArpHandleHead)
{
    if (LW_NULL == pArpHandleHead) {
        return;
    }

    pArpHandleHead->next = LW_NULL;
}
/*********************************************************************************************************
** ��������: __arpHandleLinkFree
** ��������: �ͷ� arphandle ����
** �䡡��  : arpHandleHead      ����ͷ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID __arpHandleLinkFree (__PARP_HANDLE_NODE  pArpHandleHead)
{
    __PARP_HANDLE_NODE pNode, pNext;

    if (LW_NULL == pArpHandleHead) {
        return;
    }

    pNode = pArpHandleHead;

    while (LW_NULL != pNode) {
        pNext = pNode->next;
        free(pNode);
        pNode = pNext;
    }
}
/*********************************************************************************************************
** ��������: __arpHandleLinkAdd
** ��������: ��� arphandle �ڵ�
** �䡡��  : arpHandleHead      ����ͷ
**           pNewHandle         �½ڵ�
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __arpHandleLinkAdd (__PARP_HANDLE_NODE  pArpHandleHead, __PARP_HANDLE_NODE  pNewHandle)
{
    __PARP_HANDLE_NODE  pNode = LW_NULL;

    if (LW_NULL == pArpHandleHead) {
        return;
    }

    pNode = pArpHandleHead;

    while (LW_NULL != pNode->next) {
        pNode = pNode->next;
    }

    pNode->next = pNewHandle;
}
/*********************************************************************************************************
** ��������: __arpHandleLinkDel
** ��������: ɾ�� arphandle �ڵ�
** �䡡��  : arpHandleHead      ����ͷ
** �䡡��  : pHandle            ����ɾ���ڵ�
** ������  : NONE
*********************************************************************************************************/
static VOID  __arpHandleLinkDel (__PARP_HANDLE_NODE  pArpHandleHead, __PARP_HANDLE_NODE  pHandle)
{
    __PARP_HANDLE_NODE  pNode = LW_NULL;

    if (LW_NULL == pArpHandleHead || LW_NULL == pHandle) {
        return;
    }

    pNode = pArpHandleHead->next;

    if (LW_NULL != pNode) {
        *pHandle = *pNode;
        pArpHandleHead->next = pNode->next;

        free(pNode);
    }
}
/*********************************************************************************************************
** ��������: arpTypeTransform
** ��������: ת��arp handle�ṹΪ�ַ���
** �䡡��  : pArpInfo           arp ������Ϣ
**           uiMACSize          mac ��ַ���泤��
** �䡡��  : pcMACData          mac ��ַ�ַ���
** ������  : ip ��ַ�ַ���
*********************************************************************************************************/
PCHAR  arpTypeTransform (__PARP_HANDLE  pArpInfo, PCHAR  pcMACData, UINT  uiMACSize )
{
    PCHAR  pIPData = LW_NULL;

    if (LW_NULL == pArpInfo || LW_NULL == pcMACData) {
        return  (LW_NULL);
    }

    pIPData = inet_ntoa(*(struct in_addr *)&pArpInfo->uiIpAddr);

    snprintf(pcMACData, uiMACSize,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            pArpInfo->ucHwAddr[0],
            pArpInfo->ucHwAddr[1],
            pArpInfo->ucHwAddr[2],
            pArpInfo->ucHwAddr[3],
            pArpInfo->ucHwAddr[4],
            pArpInfo->ucHwAddr[5]);

    return  (pIPData);
}
/*********************************************************************************************************
** ��������: arpShellCheckStatus
** ��������: shell����������״̬
** �䡡��  : pArpArrayHeader    arp ������Ϣ
**           IRecordNum         arp ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  arpShellCheckStatus (__PARP_HANDLE  pArpArrayHeader, INT  iRecordNum)
{
    INT  i;

    if (LW_NULL == pArpArrayHeader) {
       return;
    }

    for (i = 0; i < iRecordNum; i++) {
        PCHAR   pIPData           = LW_NULL;
        CHAR    cMACData[20]      = {0};

        pIPData = arpTypeTransform(&pArpArrayHeader[i], cMACData, sizeof(cMACData));

        if (ERROR_NONE == __arpSetResultCheck(pIPData, cMACData)) {     /*  ��ȡ����mac�����ݿ�һ��     */
            strcpy(pArpArrayHeader[i].cStatus, ARP_STATUS_CONFIGURED);  /*  ��������Ч                  */
        } else {
            strcpy(pArpArrayHeader[i].cStatus, ARP_STATUS_INVALID);     /*  ��������Ч                  */
        }
    }
}
/*********************************************************************************************************
** ��������: arpDynamicShellGet
** ��������: ͨ��shell�����ȡ��̬arp��
** �䡡��  : NONE
** �䡡��  : piRecordNum      �б����ָ��
** ������  : ��̬ arp ��
*********************************************************************************************************/
PVOID  arpDynamicShellGet (INT  *piRecordNum)
{
    FILE              *pFp               = LW_NULL;
    CHAR               cBuf[128]         = {0};
    __PARP_HANDLE      pArpArrayHeader   = LW_NULL;
    __ARP_HANDLE_NODE  arpHandleHead;
    INT                i;

    *piRecordNum = 0;

    if (LW_NULL == piRecordNum) {
       return  (LW_NULL);
    }

    __arpHandleLinkInit(&arpHandleHead);

    pFp = popen("arp -a", "r");
    if (LW_NULL == pFp) {
        return  (LW_NULL);
    }

    (VOID)fgets(cBuf, sizeof(cBuf), pFp);                               /* ��ȡ��һ�У�����             */

    while (memset(cBuf, 0, sizeof(cBuf)), fgets(cBuf, sizeof(cBuf), pFp) != LW_NULL) {
        CHAR               *pTmp;
        CHAR                cIPData[20]  = {0};
        CHAR                cMACData[20] = {0};
        __PARP_HANDLE_NODE  pnewHandle   = LW_NULL;

        pTmp = cBuf;
        while (*(pTmp + 1) != '\0') {
            pTmp++;
        }

        while (*pTmp != ' ' && pTmp > cBuf) {
            pTmp--;                                                     /* ָ�����һ���ֶ�             */
        }

        if (strncmp(pTmp, " dynamic", 8) != 0) {                        /* ֻ������̬arp                */
            continue;
        }

        pTmp = cBuf;
        while (*pTmp != ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;
        }

        while (*pTmp == ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;                                                   /* ָ��ڶ����ֶ�               */
        }

        i = 0;
        while (*pTmp != ' ' && i < sizeof(cIPData) - 1) {               /* ��ȡIP��ַ                   */
            cIPData[i] = *pTmp;
            i++;
            pTmp++;
        }

        cIPData[i] = '\0';

        while (*pTmp != ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;
        }

        while (*pTmp == ' ' && *pTmp != '\0') {
            pTmp++;
        }

        if (*pTmp == '\0') {
            continue;                                                   /* ָ��������ֶ�               */
        }

        memcpy(cMACData, pTmp, 17);                                     /* ��ȡmac��ַ�ֶ�              */
        cMACData[17] = '\0';

        /*
         * ���һ���ڵ�
         */
        pnewHandle = (__PARP_HANDLE_NODE)malloc(sizeof(__ARP_HANDLE_NODE));
        if (LW_NULL == pnewHandle) {
            goto __MEM_ERR;
        }

        pnewHandle->handle.uiIpAddr = inet_addr(cIPData);

        sscanf(cMACData,
                "%02x:%02x:%02x:%02x:%02x:%02x",
                (UINT *)&pnewHandle->handle.ucHwAddr[0],
                (UINT *)&pnewHandle->handle.ucHwAddr[1],
                (UINT *)&pnewHandle->handle.ucHwAddr[2],
                (UINT *)&pnewHandle->handle.ucHwAddr[3],
                (UINT *)&pnewHandle->handle.ucHwAddr[4],
                (UINT *)&pnewHandle->handle.ucHwAddr[5]);

        pnewHandle->next = LW_NULL;
        __arpHandleLinkAdd(&arpHandleHead, pnewHandle);

        (*piRecordNum)++;
    }

    /*
     * �����ռ�
     */
    pArpArrayHeader = (__PARP_HANDLE)malloc(sizeof(__ARP_HANDLE) * (*piRecordNum));
    if (LW_NULL == pArpArrayHeader) {
        goto  __MEM_ERR;
    }

    for (i = 0; i < *piRecordNum; i++) {
        __ARP_HANDLE_NODE  node;

        __arpHandleLinkDel(&arpHandleHead, &node);
        pArpArrayHeader[i] = node.handle;
    }

    pclose(pFp);

    return  (pArpArrayHeader);

__MEM_ERR:
    __arpHandleLinkFree(&arpHandleHead);

    pclose(pFp);

    __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "[OpenRTCfgTool] memery error!\n");

    return  (LW_NULL);
}

/*********************************************************************************************************
** ��������: arpDoConfig
** ��������: ���� arp
** �䡡��  : pArpInfo   arp ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  arpDoConfig (__PARP_HANDLE  pArpInfo)
{
    PCHAR   pIPData      = LW_NULL;
    CHAR    cMACData[20] = {0};
    INT     iRet         = ERROR_NONE;

    if (LW_NULL == pArpInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arp data is null.\n");

       return  (PX_ERROR);
    }

    pIPData = arpTypeTransform(pArpInfo, cMACData, sizeof(cMACData));
    if (NULL == pIPData) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(pArpInfo->cOperType, ARP_DELETE)){
        iRet = __arpShellDel(pIPData);
        if (ERROR_NONE != iRet) {
            return  (arpUpdateToDb(pArpInfo->cOperType, pArpInfo));
        }
    } else {
        iRet = __arpShellSet(pIPData, cMACData);
        if (ERROR_NONE != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to config arp ip:%s mac:%s, "
                                                    "error_code=%d.\n",pIPData, cMACData, iRet);
            return  (PX_ERROR);
        }
    }

    return  (arpUpdateToDb(pArpInfo->cOperType, pArpInfo));
}
/*********************************************************************************************************
** ��������: arpRecovery
** ��������: arp ���ûָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  arpRecovery (VOID)
{
    __PARP_HANDLE pArpHeader   = LW_NULL;
    __PARP_HANDLE pTmpNode     = LW_NULL;
    CHAR          cCmdTmp[128] = {0};
    INT           iRet         = 0;
    INT           iRecordNum   = 0;
    INT           i            = 0 ;

    pArpHeader = arpGetFromDB(&iRecordNum);
    if (LW_NULL == pArpHeader && 0 != iRecordNum) {
        return  (PX_ERROR);
    }

    sleep(1);                                                           /*  �ȴ�ϵͳ�ȶ�                */

    for (i = 0; i < iRecordNum; i++) {
        PCHAR   pIPData      = LW_NULL;
        CHAR    cMACData[20] = {0};

        pTmpNode = pArpHeader + i;
        pIPData  = arpTypeTransform(pTmpNode, cMACData, sizeof(cMACData));
        if (NULL == pIPData) {
            return  (PX_ERROR);
        }

        bzero(cCmdTmp, sizeof(cCmdTmp));
        snprintf(cCmdTmp, sizeof(cCmdTmp), "arp -s %s %s", pIPData, cMACData);

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet) {
             __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,
                             "Failed to execute recovery command = %s, "
                             "error_code=%d.\n", cCmdTmp, iRet);

             arpDeleteItemByIp(pTmpNode);

             continue;
        }
    }

    if (NULL != pArpHeader) {
        free(pArpHeader);
    }

    return  (ERROR_NONE);
}
