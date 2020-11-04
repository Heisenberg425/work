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
** 文   件   名: monitorEntry.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 29 日
**
** 描        述: OpenRT 流控配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include <ifaddrs.h>
#include "netdev.h"
#include "lwip/netif.h"
#include "net/if.h"
#include "net/lwip/lwip_if.h"
#include "monitor_common.h"
/*********************************************************************************************************
** 函数名称: __ipAddrGetByIfName
** 功能描述: 根据网口名获取 IP 地址
** 输　入  : cpcIfName      网口信息
**           uiAddrLen      地址长度
** 输　出  : pcIpAddr       保存 IP 信息缓冲区
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  __ipAddrGetByIfName (CPCHAR  cpcIfName, PCHAR  pcIpAddr, UINT32  uiAddrLen)
{
    INT                 iRet;
    struct ifreq        ifr;
    struct sockaddr_in *pSockIn;
    int                 iSockFd;

    if (NULL == cpcIfName    ||
        '\0' == cpcIfName[0] ||
        NULL == pcIpAddr) {
        return  (PX_ERROR);
    }

    iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSockFd < 0) {
        return  (PX_ERROR);
    }

    strncpy(ifr.ifr_name, cpcIfName, sizeof(ifr.ifr_name));

    iRet = ioctl(iSockFd, SIOCGIFADDR, &ifr);
    if (iRet < 0) {
        printf("Get SIOCGIFADDR Failed!\n");

        close(iSockFd);

        return  (PX_ERROR);
    }

    close(iSockFd);

    pSockIn = (struct sockaddr_in *)&(ifr.ifr_addr);
    if (NULL == inet_ntoa(pSockIn->sin_addr)) {
        return  (PX_ERROR);
    }

    strncpy(pcIpAddr, inet_ntoa(pSockIn->sin_addr), uiAddrLen);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: monitorConfigDeal
** 功能描述: 配置监控处理函数
** 输　入  : pFlowctlHandle     flowctl参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  monitorConfigDeal (PTM_STATS  pMonitorHandle)
{
    replyConfigMsg(OPENRT_CFG_MONITOR_ACK_CMD, monitorDoConfig(pMonitorHandle));
}
/*********************************************************************************************************
** 函数名称: monitorShowDeal
** 功能描述: 查询监控处理函数
** 输　入  : pMonitorHandle     monitor参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  monitorShowDeal (PTM_STATS  pMonitorHandle)
{
    PTM_STATS  pMonitorHeader = LW_NULL;
    INT        iRecordNum     = 0;
    INT        i;

    pMonitorHeader = monitorDoShow(pMonitorHandle, &iRecordNum);

    if (TMT_TERM != pMonitorHandle->cType) {
        for (i = 0; i < iRecordNum; i++) {
            __ipAddrGetByIfName(pMonitorHeader[i].pIfname,
                                pMonitorHeader[i].pIpAddr,
                                sizeof(pMonitorHeader[i].pIpAddr));
        }
    }

    replyQueryMsg(OPENRT_GET_MONITOR_ACK_CMD, sizeof(TM_STATS), iRecordNum, pMonitorHeader);

    if (NULL != pMonitorHeader) {
        free(pMonitorHeader);
    }
}
/*********************************************************************************************************
** 函数名称: cpuMemShowDeal
** 功能描述: 查询cpu/内存/在线用户数处理函数
** 输　入  : pMonitorHandle     monitor参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  cpuMemShowDeal (PTM_STATS  pMonitorHandle)
{
	PTM_CPU_MEM  ptmCpuMem  = LW_NULL;
    INT          iRecordNum = 0;

    ptmCpuMem = cpuMemDoShow(pMonitorHandle, &iRecordNum);
    replyQueryMsg(OPENRT_GET_CPU_MEM_ACK_CMD, sizeof(TM_CPU_MEM), iRecordNum, ptmCpuMem);

    if (NULL != ptmCpuMem) {
        free(ptmCpuMem);
    }
}
/*********************************************************************************************************
** 函数名称: internetStatusHandle
** 功能描述: 互联网状态处理入口
** 输　入  : pArgBuf     monitor参数信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  internetStatusHandle (PVOID  pArgBuf)
{
    __INTERNET_STATUS  internetStatus = {0};

    internetStatusGet(&internetStatus);

    replyQueryMsg(OPENRT_GET_INTERNET_CMD, sizeof(__INTERNET_STATUS), 1, &internetStatus);
}
/*********************************************************************************************************
** 函数名称: monitorHandleEntry
** 功能描述: 监控配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID monitorHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_MONITOR_CMD:
        monitorConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_MONITOR_CMD:
        monitorShowDeal(pArgBuf);
        break;

    case OPENRT_GET_CPU_MEM_CMD:
    	cpuMemShowDeal(pArgBuf);
        break;

    case OPENRT_GET_INTERNET_CMD:
        internetStatusHandle(pArgBuf);
        break;

    default:
        break;
    }
}

