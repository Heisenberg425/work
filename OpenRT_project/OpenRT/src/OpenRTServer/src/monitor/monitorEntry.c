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
** ��   ��   ��: monitorEntry.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 29 ��
**
** ��        ��: OpenRT �������ô������
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
** ��������: __ipAddrGetByIfName
** ��������: ������������ȡ IP ��ַ
** �䡡��  : cpcIfName      ������Ϣ
**           uiAddrLen      ��ַ����
** �䡡��  : pcIpAddr       ���� IP ��Ϣ������
** ������  : ERROR_CODE
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
** ��������: monitorConfigDeal
** ��������: ���ü�ش�����
** �䡡��  : pFlowctlHandle     flowctl������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  monitorConfigDeal (PTM_STATS  pMonitorHandle)
{
    replyConfigMsg(OPENRT_CFG_MONITOR_ACK_CMD, monitorDoConfig(pMonitorHandle));
}
/*********************************************************************************************************
** ��������: monitorShowDeal
** ��������: ��ѯ��ش�����
** �䡡��  : pMonitorHandle     monitor������Ϣ
** �䡡��  : NONE
** ������  : NONE
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
** ��������: cpuMemShowDeal
** ��������: ��ѯcpu/�ڴ�/�����û���������
** �䡡��  : pMonitorHandle     monitor������Ϣ
** �䡡��  : NONE
** ������  : NONE
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
** ��������: internetStatusHandle
** ��������: ������״̬�������
** �䡡��  : pArgBuf     monitor������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  internetStatusHandle (PVOID  pArgBuf)
{
    __INTERNET_STATUS  internetStatus = {0};

    internetStatusGet(&internetStatus);

    replyQueryMsg(OPENRT_GET_INTERNET_CMD, sizeof(__INTERNET_STATUS), 1, &internetStatus);
}
/*********************************************************************************************************
** ��������: monitorHandleEntry
** ��������: ����������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
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

