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
** ��   ��   ��: hardwareExec.c
**
** ��   ��   ��: Wu.pengcheng (������)
**
** �ļ���������: 2018 �� 8 �� 2 ��
**
** ��        ��: ��ȡ OpenRT Ӳ���豸��Ϣ
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware_common.h"
#include "../fileparser/fileparser.h"
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
/*********************************************************************************************************
  ��Ϣɹȡ��
*********************************************************************************************************/
#define CPU_INFO_HEAD             "CPU         : "                      /*  CPU ��Ϣͷ������            */
#define STO_INFO_HEAD             "block product :"                     /*  �洢�豸��Ϣͷ������        */
/*********************************************************************************************************
** ��������: __getCpuInfo
** ��������: ��ȡ CPU Ӳ����Ϣ
** ��  ��  : pcCpuInfoBuff     ��� CPU ��Ϣ�� Buff
**           iBufferLen        Buff ����
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static INT32  __getCpuInfo (PCHAR  pcCpuInfoBuff, INT32  iBufferLen)
{
    FILE           *pCpuInfoFd          = LW_NULL;
    CHAR           *pcBuf               = LW_NULL;
    UINT            uilinenum           = 1;
    UINT            ulineOffset         = 0;
    PCHAR           pcCpuFile           = "/tmp/cpu";
    INT32           iRet;

    if (LW_NULL == pcCpuInfoBuff) {
        return  (PX_ERROR);
    }

    pCpuInfoFd = fopen(pcCpuFile, "w+");
    if(LW_NULL == pCpuInfoFd) {
        return  (PX_ERROR);
    }

    iRet = system("cat /proc/cpuinfo >/tmp/cpu");
    if(PX_ERROR == iRet) {
        fclose(pCpuInfoFd);

        return  (PX_ERROR);
    }

    fclose(pCpuInfoFd);

    iRet = specifiedStrFind(pcCpuFile, CPU_INFO_HEAD, &pcBuf, &uilinenum, &ulineOffset);
    if (PX_ERROR != iRet) {
        if (strlen(pcBuf) - strlen(CPU_INFO_HEAD) > iBufferLen) {
            free(pcBuf);

            return  (PX_ERROR);
        } else {
            strncpy(pcCpuInfoBuff,
                    (pcBuf + strlen(CPU_INFO_HEAD)),
                    strlen(pcBuf) - strlen(CPU_INFO_HEAD));
            compact_strtrim(pcCpuInfoBuff);
        }

        free(pcBuf);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __getStoDevInfo
** ��������: ��ȡ�洢�豸Ӳ����Ϣ
** ��  ��  : pcStoInfoBuff     ��Ŵ洢�豸��Ϣ�� Buff
**           iBufferLen        Buff ����
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static INT32  __getStoDevInfo (PCHAR  pcStoInfoBuff, INT32  iBufferLen)
{
    FILE           *pStoInfoFd          = LW_NULL;
    PCHAR           pcStoFile           = "/tmp/blk";
    CHAR           *pcBuf               = LW_NULL;
    UINT            uilinenum           = 1;
    UINT            ulineOffset         = 0;
    INT32           iRet;

    if (LW_NULL == pcStoInfoBuff) {
        return  (PX_ERROR);
    }

    pStoInfoFd = fopen(pcStoFile, "w+");
    if(LW_NULL == pStoInfoFd) {
        return  (PX_ERROR);
    }

    /*
     *  ��ȡ�洢�豸��Ϣ������ֻ��ȡ x86 ��ĵ�һ���洢�豸
     */
    iRet = system("fdisk /dev/blk/hdd-0 >/tmp/blk");
    if(PX_ERROR == iRet) {
        fclose(pStoInfoFd);

        return  (PX_ERROR);
    }

    fclose(pStoInfoFd);

    iRet = specifiedStrFind(pcStoFile, STO_INFO_HEAD, &pcBuf, &uilinenum, &ulineOffset);
    if (PX_ERROR != iRet) {
        if (strlen(pcBuf) - strlen(STO_INFO_HEAD) > iBufferLen) {
            free(pcBuf);

            return  (PX_ERROR);
        } else {
            strncpy(pcStoInfoBuff,
                    (pcBuf + strlen(STO_INFO_HEAD)),
                    strlen(pcBuf) - strlen(STO_INFO_HEAD));
            compact_strtrim(pcStoInfoBuff);
        }

        free(pcBuf);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __getAllMemInfo
** ��������: ��ȡ���ڴ���Ϣ
** ��  ��  : pcMemInfoBuff     ����ڴ���Ϣ�� Buff
**           iBufferLen        Buff ����
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static INT32  __getAllMemInfo (PCHAR  pcMemInfoBuff, INT32  iBufferLen)
{
    size_t       stPhyMemTotalSize;
    size_t       stUsedSize;

    getMemoryUsage(&stPhyMemTotalSize, &stUsedSize);

    snprintf(pcMemInfoBuff, iBufferLen, "%u MBytes", (UINT)stPhyMemTotalSize/1024);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: getHwInfo
** ��������: ��ȡӲ����Ϣ
** ��  ��  : pHwInfo     ��Ŵ洢�豸��Ϣ�� Buff
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
INT32  getHwInfo (__PHW_INFO pHwInfo)
{
    if (LW_NULL == pHwInfo) {
        return  (PX_ERROR);
    }

    __getCpuInfo(pHwInfo->cCpuInfo, sizeof(pHwInfo->cCpuInfo));
    __getStoDevInfo(pHwInfo->cStoInfo, sizeof(pHwInfo->cStoInfo));
    __getAllMemInfo(pHwInfo->cMemInfo, sizeof(pHwInfo->cMemInfo));

    return  (ERROR_NONE);
}
