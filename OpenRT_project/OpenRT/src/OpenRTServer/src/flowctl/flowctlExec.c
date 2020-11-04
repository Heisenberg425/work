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
** 文   件   名: flowctlExec.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: flowctl 操作
*********************************************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "flowctl_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "network/lwip/sockets.h"

/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define CMD_BUFF_LEN   (256)                                            /*  缓冲区长度                  */
/*********************************************************************************************************
** 函数名称: flowctlDoConfig
** 功能描述: 增加或修改或删除网络接口flowctl
** 输　入  : pRouteInfo      路由参数信息
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
** 函数名称: flowctlRecovery
** 功能描述: flowctl恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : INT 执行结果
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
