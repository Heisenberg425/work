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
** 文   件   名: monitorExec.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 17 日
**
** 描        述: monitor操作
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
  函数声明
*********************************************************************************************************/
extern INT        tmStatItemCount(PTM_STATS  pCondition, INT* piFilter);
extern PTM_STATS  tmStatItemsCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
extern PTM_STATS  tmStatItemsSumCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition);
extern INT        getCpuUsage();
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
extern INT        tmGetOnlineUsers();
/*********************************************************************************************************
** 函数名称: monitorDoConfig
** 功能描述: 流量监控使能去使能统计项
** 输　入  : pMonitorHandle      统计项定位条件
** 输　出  : none
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
** 函数名称: cpuMemDoShow
** 功能描述: 查询 cpu/内存/在线用户数
** 输　入  : pMonitorHandle     统计项过滤条件
** 输　出  : piRecordNum        记录条数
** 返　回  : PTM_CPU_MEM对象的地址
**           如果操作失败，则返回LW_NULL
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
** 函数名称: __invalidMonitorInfoRemove
** 功能描述: 去除无效监控信息
** 输　入  : pSrcMonitorInfo     路由参数信息
**           iSrcRecordNum       监控信息个数
**           iNewRecordNum       有效监控信息个数指针
** 输　出  : NONE
** 返　回  : NONE
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
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  如果该硬件网口有效          */
                iNum++;
            } else {
                continue;
            }
        } else {                                                        /*  非硬件网口，如虚拟网口      */
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
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  如果该硬件网口有效          */
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
** 函数名称: monitorDoShow
** 功能描述: 根据过滤条件查询
** 输　入  : pMonitorHandle     统计项过滤条件
** 输　出  : piRecordNum        记录条数
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
** 函数名称: __internetStatusCheck
** 功能描述: 检测互联网连接状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           连接失败
**           ERROR_NONE         连接成功
*********************************************************************************************************/
static INT32  __internetStatusCheck (VOID)
{
    return  (system("ping 114.114.114.114 -n 1 -w 200 >/dev/null"));
}
/*********************************************************************************************************
** 函数名称: InternetStatusGet
** 功能描述: 互联网连接状态获取
** 输　入  : NONE
** 输　出  : pInternetStatus    连接状态信息
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT32  internetStatusGet (__PINTERNET_STATUS  pInternetStatus)
{
    static time_t    llInternetTime    = 0;                             /*  保存第一次连接外网的时间    */
    static INT32     iUnConnectedCount = 0;                             /*  断开连接次数                */
    struct timespec  tsMono;                                            /*  单调递增时间                */

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
