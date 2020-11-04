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
** 文   件   名: qosExec.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 15 日
**
** 描        述: QoS 操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "qos_common.h"
#include "common.h"
#include "fileparser/fileparser.h"

/*********************************************************************************************************
** 函数名称: __qosIsValid
** 功能描述: 判断数据是否有效
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : LW_TRUE            数据有效
**           LW_FALSE           数据无效
*********************************************************************************************************/
static INT  __qosIsValid (__PQOS_HANDLE  pQosInfo)
{
    if (LW_NULL == pQosInfo) {
        return (LW_FALSE);
    }

    /*
     * 优先级范围判断
     */
    if (pQosInfo->uiPriority > QOS_CFG_PRIORITY_MAX ||
        pQosInfo->uiPriority < QOS_CFG_PRIORITY_MIN) {
        return (LW_FALSE);
    }

    /*
     * Dont Drop范围判断
     */
    if (pQosInfo->uiDontDrop != 0 &&
        pQosInfo->uiDontDrop != 1) {
        return (LW_FALSE);
    }

    /*
     * Rule 范围判断
     */
    if (strcmp(pQosInfo->cRule, QOS_CFG_RULE_IP)   != 0 &&
        strcmp(pQosInfo->cRule, QOS_CFG_RULE_TCP)  != 0 &&
        strcmp(pQosInfo->cRule, QOS_CFG_RULE_UDP)  != 0) {
        return (LW_FALSE);
    }

    /*
     * CmpMethod 范围判断
     */
    if (strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_SRC)   != 0 &&
        strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_DEST)  != 0 &&
        strcmp(pQosInfo->cCmpMethod, QOS_CFG_CMPMETHOD_BOTH)  != 0) {
        return (LW_FALSE);
    }

    return (LW_TRUE);
}
/*********************************************************************************************************
** 函数名称: __qosCompare
** 功能描述: 比较 QoS 配置
** 输　入  : pQosInfo1          QoS 参数信息1
**           pQosInfo2          QoS 参数信息2
** 输　出  : NONE
** 返　回  : 0                  两个 QoS 配置相等
**           非零               两个 QoS 配置不等
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
** 函数名称: __qosShellGetSeqNumOper
** 功能描述: Shell 获取 seqnum
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : seqnum 小于 0 表示未找到
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

    (void)fgets(cBuf, sizeof(cBuf), pFp);                               /*  读取第一行并舍弃            */
    while (memset(cBuf, 0, sizeof(cBuf)),
            fgets(cBuf, sizeof(cBuf), pFp) != LW_NULL) {

        if (strlen(cBuf) < 20) {
            continue;                                                   /*  删除长度不足行              */
        }

        pToken = strtok(cBuf, " ");                                     /*  提取每一个字段              */
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
            break;                                                      /*  找到SeqNum                  */
        }
    }

    pclose(pFp);

    return (iFlag ? iSeqNum : PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: __qosShellAddOper
** 功能描述: Shell 配置 QoS
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        return  (PX_ERROR);                                             /*  已存在此条 QoS 配置         */
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
** 函数名称: __qosShellDelOper
** 功能描述: Shell 删除 QoS 配置
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: qosDoConfig
** 功能描述: 配置 QoS
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  qosDoConfig (__PQOS_HANDLE  pQosInfo)
{
    INT  iRet = ERROR_NONE;

    if (LW_NULL == pQosInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is null.\n");

       return  (PX_ERROR);
    }

    /*
     * 判断数据是否无效
     */
    if (LW_FALSE == __qosIsValid(pQosInfo)) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "QoS data is Invalid.\n");

        return  (PX_ERROR);
    }

    /*
     * 判断数据是否已经存在
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
** 函数名称: qosDoDel
** 功能描述: 删除 QoS 配置
** 输　入  : pQosInfo           QoS 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: qosRecovery
** 功能描述: QoS 配置恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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

    sleep(1);                                                           /*  等待系统稳定                */

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
