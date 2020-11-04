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
** 文   件   名: hardwareExec.c
**
** 创   建   人: Wu.pengcheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 2 日
**
** 描        述: 获取 OpenRT 硬件设备信息
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware_common.h"
#include "../fileparser/fileparser.h"
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
/*********************************************************************************************************
  信息晒取宏
*********************************************************************************************************/
#define CPU_INFO_HEAD             "CPU         : "                      /*  CPU 信息头部描述            */
#define STO_INFO_HEAD             "block product :"                     /*  存储设备信息头部描述        */
/*********************************************************************************************************
** 函数名称: __getCpuInfo
** 功能描述: 获取 CPU 硬件信息
** 输  入  : pcCpuInfoBuff     存放 CPU 信息的 Buff
**           iBufferLen        Buff 长度
** 输  出  : NONE
** 返  回  : NONE
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
** 函数名称: __getStoDevInfo
** 功能描述: 获取存储设备硬件信息
** 输  入  : pcStoInfoBuff     存放存储设备信息的 Buff
**           iBufferLen        Buff 长度
** 输  出  : NONE
** 返  回  : NONE
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
     *  读取存储设备信息，这里只读取 x86 里的第一个存储设备
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
** 函数名称: __getAllMemInfo
** 功能描述: 获取总内存信息
** 输  入  : pcMemInfoBuff     存放内存信息的 Buff
**           iBufferLen        Buff 长度
** 输  出  : NONE
** 返  回  : NONE
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
** 函数名称: getHwInfo
** 功能描述: 获取硬件信息
** 输  入  : pHwInfo     存放存储设备信息的 Buff
** 输  出  : NONE
** 返  回  : NONE
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
