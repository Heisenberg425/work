/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: arpExec.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 13 日
**
** 描        述: arp 操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "arp_common.h"
#include "common.h"
#include "fileparser/fileparser.h"

/*********************************************************************************************************
** 函数名称: __arpSetResultCheck
** 功能描述: 查询 ARP 是否设置成功
** 输　入  : pcIPData            IP 地址
** 输　出  : pcMACData           mac 地址
** 返　回  : ERROR_NONE         设置成功
**           PX_ERROR           设置失败
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

    (VOID)fgets(cBuf, sizeof(cBuf), pFp);                               /* 读取第一行，舍弃             */

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
** 函数名称: __arpShellGet
** 功能描述: shell命令通过ip获取mac
** 输　入  : pIPData            IP 地址
** 输　出  : cMACData           mac 地址
** 返　回  : ERROR_NONE         获取到对应 mac
**           PX_ERROR           未找到对应 ip
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

    (VOID)fgets(cBuf, sizeof(cBuf), fp);                                /* 读取第一行，舍弃             */

    while (memset(cBuf, 0, sizeof(cBuf)),
           fgets(cBuf, sizeof(cBuf), fp) != LW_NULL) {
        CHAR   *pTmp = cBuf;

        while (*(pTmp + 1) != '\0') {
            pTmp++;
        }

        while (*pTmp != ' ' && pTmp > cBuf) {
            pTmp--;                                                     /* 指向最后一个字段             */
        }

        if (strncmp(pTmp, " static", 7) != 0) {                         /* 只分析静态arp                */
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
            continue;                                                   /* 指向第二个字段               */
        }

        if (strncmp(pTmp, pIPData, strlen(pIPData)) != 0) {             /* 找到对应ip                   */
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
            continue;                                                   /* 指向第三个字段               */
        }

        if (cMACData != 0) {
            memcpy(cMACData, pTmp, 17);                                 /* 获取mac地址字段              */
            cMACData[17] = '\0';
        }

        pclose(fp);

        return  (ERROR_NONE);
    }

    /*
     * 未找到对应 ip 数据
     */
    pclose(fp);

    return  (PX_ERROR);
}
#endif
/*********************************************************************************************************
** 函数名称: __arpShellSet
** 功能描述: shell 命令配置 arp
** 输　入  : pcIPData          ip  信息
**           pcMACData         mac 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpShellSet (PCHAR  pcIPData, PCHAR  pcMACData)
{
    INT     iRet         = ERROR_NONE;
    CHAR    cCmdTmp[200] = {0};

    if (LW_NULL == pcIPData || LW_NULL == pcMACData) {
        return  (PX_ERROR);
    }

    snprintf(cCmdTmp, sizeof(cCmdTmp), "arp -s %s %s", pcIPData, pcMACData);

    iRet = system(cCmdTmp);                                             /* 配置arp                      */
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                               "error_code=%d.\n",cCmdTmp, iRet);
         return  (PX_ERROR);
    }

    /*
     * 校验是否配置成功
     */
    if (ERROR_NONE == __arpSetResultCheck(pcIPData, pcMACData)) {
        return  (ERROR_NONE);
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** 函数名称: __arpShellDel
** 功能描述: shell 命令删除 arp
** 输　入  : pIPData           ip 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpShellDel (PCHAR  pIPData)
{
    CHAR    cCmdTmp[200] = {0};
    INT     iRet         = ERROR_NONE;

    if (LW_NULL == pIPData) {
        return  (PX_ERROR);
    }

    snprintf(cCmdTmp, sizeof(cCmdTmp), "arp -d %s", pIPData);

    iRet = system(cCmdTmp);                                             /* 删除arp                      */
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                               "error_code=%d.\n",cCmdTmp, iRet);
         return  (PX_ERROR);
    }

#ifdef ARP_DEL_CHECK
    /*
     * 校验是否删除成功
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
** 函数名称: __arpHandleLinkInit
** 功能描述: 初始化 arphandle 链表
** 输　入  : arpHandleHead    链表头
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __arpHandleLinkInit (__PARP_HANDLE_NODE  pArpHandleHead)
{
    if (LW_NULL == pArpHandleHead) {
        return;
    }

    pArpHandleHead->next = LW_NULL;
}
/*********************************************************************************************************
** 函数名称: __arpHandleLinkFree
** 功能描述: 释放 arphandle 链表
** 输　入  : arpHandleHead      链表头
** 输　出  : NONE
** 返　回  : NONE
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
** 函数名称: __arpHandleLinkAdd
** 功能描述: 添加 arphandle 节点
** 输　入  : arpHandleHead      链表头
**           pNewHandle         新节点
** 输　出  : NONE
** 返　回  : NONE
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
** 函数名称: __arpHandleLinkDel
** 功能描述: 删除 arphandle 节点
** 输　入  : arpHandleHead      链表头
** 输　出  : pHandle            保存删除节点
** 返　回  : NONE
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
** 函数名称: arpTypeTransform
** 功能描述: 转化arp handle结构为字符串
** 输　入  : pArpInfo           arp 参数信息
**           uiMACSize          mac 地址缓存长度
** 输　出  : pcMACData          mac 地址字符串
** 返　回  : ip 地址字符串
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
** 函数名称: arpShellCheckStatus
** 功能描述: shell命令检查配置状态
** 输　入  : pArpArrayHeader    arp 参数信息
**           IRecordNum         arp 参数长度
** 输　出  : NONE
** 返　回  : NONE
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

        if (ERROR_NONE == __arpSetResultCheck(pIPData, cMACData)) {     /*  获取到的mac与数据库一致     */
            strcpy(pArpArrayHeader[i].cStatus, ARP_STATUS_CONFIGURED);  /*  此配置有效                  */
        } else {
            strcpy(pArpArrayHeader[i].cStatus, ARP_STATUS_INVALID);     /*  此配置无效                  */
        }
    }
}
/*********************************************************************************************************
** 函数名称: arpDynamicShellGet
** 功能描述: 通过shell命令获取动态arp表
** 输　入  : NONE
** 输　出  : piRecordNum      列表个数指针
** 返　回  : 动态 arp 表
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

    (VOID)fgets(cBuf, sizeof(cBuf), pFp);                               /* 读取第一行，舍弃             */

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
            pTmp--;                                                     /* 指向最后一个字段             */
        }

        if (strncmp(pTmp, " dynamic", 8) != 0) {                        /* 只分析动态arp                */
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
            continue;                                                   /* 指向第二个字段               */
        }

        i = 0;
        while (*pTmp != ' ' && i < sizeof(cIPData) - 1) {               /* 获取IP地址                   */
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
            continue;                                                   /* 指向第三个字段               */
        }

        memcpy(cMACData, pTmp, 17);                                     /* 获取mac地址字段              */
        cMACData[17] = '\0';

        /*
         * 添加一个节点
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
     * 创建空间
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
** 函数名称: arpDoConfig
** 功能描述: 配置 arp
** 输　入  : pArpInfo   arp 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: arpRecovery
** 功能描述: arp 配置恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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

    sleep(1);                                                           /*  等待系统稳定                */

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
