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
** 文   件   名: cpuMemStat.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 25 日
**
** 描        述: OpenRT CPU 内存性能监控
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <string.h>

/*********************************************************************************************************
** 函数名称: getVmmPhysicalSize
** 功能描述: 获得物理内存总大小和 VMM 管理已经使用的内存大小
** 输　入  : NONE
** 输　出  : pstPhyMemTotalSize      物理内存总大小 (KB)
**           pstVmmUsedSize          VMM 管理已经使用的内存大小 (KB)
** 返　回  : NONE
*********************************************************************************************************/
VOID  getVmmPhysicalSize (size_t  *pstPhyMemTotalSize, size_t  *pstVmmUsedSize)
{
    REGISTER INT                    i;
             addr_t                 ulPhysicalAddr;
             LW_MMU_PHYSICAL_DESC   phydescKernel[2];
             UINT                   uiAttr;
             ULONG                  ulFreePage;
             addr_t                 ulPgd;
             size_t                 stSize;

             size_t                 stTotalSize = 0;
             size_t                 stFreeSize  = 0;


    for (i = 0; i < LW_CFG_VMM_ZONE_NUM; i++) {
        if (API_VmmZoneStatus(i, &ulPhysicalAddr, &stSize, &ulPgd, &ulFreePage, &uiAttr)) {
            continue;
        }

        if (!stSize) {
            continue;
        }

        stTotalSize += stSize;
        stFreeSize  += (ulFreePage << LW_CFG_VMM_PAGE_SHIFT);
    }

    API_VmmPhysicalKernelDesc(&phydescKernel[0], &phydescKernel[1]);

    *pstPhyMemTotalSize = (phydescKernel[0].PHYD_stSize +
                           phydescKernel[1].PHYD_stSize + stTotalSize) / LW_CFG_KB_SIZE;
    *pstVmmUsedSize     = (stTotalSize - stFreeSize) / LW_CFG_KB_SIZE;
}

/*********************************************************************************************************
** 函数名称: getMemoryUsage
** 功能描述: 获得物理内存总大小和已经使用的内存大小
** 输　入  : NONE
** 输　出  : pstPhyMemTotalSize      物理内存总大小 (KB)
**           pstUsedSize             已经使用的内存大小 (KB)
** 返　回  : 操作结果
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT  getMemoryUsage (size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize)
{
    size_t      stByteSize;
    ULONG       ulSegmentCounter;
    size_t      stUsedByteSize;
    size_t      stUsedSize;
    size_t      stMaxUsedByteSize;
    size_t      stPhyMemTotalSize;
    size_t      stVmmUsedSize;

    API_KernelHeapInfo(LW_OPTION_HEAP_KERNEL,
                          &stByteSize,
                          &ulSegmentCounter,
                          &stUsedByteSize,
                          LW_NULL,
                          &stMaxUsedByteSize);

    stUsedSize = stUsedByteSize / LW_CFG_KB_SIZE;

    if (ERROR_NONE == API_KernelHeapInfo(LW_OPTION_HEAP_SYSTEM,
                                         &stByteSize,
                                         &ulSegmentCounter,
                                         &stUsedByteSize,
                                         LW_NULL,
                                         &stMaxUsedByteSize)) {
        stUsedSize += stUsedByteSize / LW_CFG_KB_SIZE;
    }

    getVmmPhysicalSize(&stPhyMemTotalSize, &stVmmUsedSize);

    stUsedSize          += stVmmUsedSize;
    *pstPhyMemTotalSize  = stPhyMemTotalSize;
    *pstUsedSize         = stUsedSize;

    return  (0);
}

/*********************************************************************************************************
** 函数名称: getCpuUsage
** 功能描述: 获得 CPU 已被占用的总数(千分率)
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT  getCpuUsage (VOID)
{
    REGISTER INT              i;
             INT              iThreadNum;
             LW_OBJECT_HANDLE ulId[LW_CFG_MAX_THREADS];
             UINT             uiThreadUsage[LW_CFG_MAX_THREADS];
             UINT             uiThreadKernel[LW_CFG_MAX_THREADS];
             CHAR             cName[LW_CFG_OBJECT_NAME_SIZE];
             UINT             iCpuUsedTotal = 0;

    if (LW_CPU_GET_CUR_NESTING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);

        return  (0);
    }


    iThreadNum = API_ThreadGetCPUUsageAll(ulId, uiThreadUsage, uiThreadKernel, LW_CFG_MAX_THREADS);
    if (iThreadNum <= 0) {
        return  (0);
    }

    for (i = iThreadNum - 1; i >= 0; i--) {
        if (ERROR_NONE == API_ThreadGetName(ulId[i], cName))  {
            if (0 != strncmp(cName, "t_idle", 6)) {
                iCpuUsedTotal += uiThreadUsage[i];
            }
        }
    }

    return  (iCpuUsedTotal);
}


