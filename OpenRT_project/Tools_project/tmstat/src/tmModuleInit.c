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
** 文   件   名: tmModuleInit.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: OpenRT 流量监控模块初始化
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "tmCmd.h"
#include "tmHash.h"
#include "tmStat.h"
#include "tmStatItem.h"
#include "net/if_iphook.h"
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
VOID  tm_stat_module_func (VOID);
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
LW_OBJECT_HANDLE  _G_ulTmStatTimer           = LW_OBJECT_HANDLE_INVALID;/*  流量统计定时器              */
LW_OBJECT_HANDLE  _G_ulTmStatCountResetTimer = LW_OBJECT_HANDLE_INVALID;/*  统计项清除定时器            */
LW_OBJECT_HANDLE  _G_ulTmCpuStatMonitor      = LW_OBJECT_HANDLE_INVALID;/*  CPU 统计线程句柄            */
/*********************************************************************************************************
** 函数名称: module_init
** 功能描述: ko 模块初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  module_init (VOID)
{
    tm_stat_module_func();

    return  (0);
}

/*********************************************************************************************************
** 函数名称: module_exit
** 功能描述: ko模块卸载函数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  module_exit (VOID)
{
    API_TimerCancel(_G_ulTmStatTimer);
    API_TimerDelete(&_G_ulTmStatTimer);

    API_TimerCancel(_G_ulTmStatCountResetTimer);
    API_TimerDelete(&_G_ulTmStatCountResetTimer);

    API_ThreadDelete(&_G_ulTmCpuStatMonitor, LW_NULL);

    sleep(1);

    net_ip_hook_delete(tmStatsStart);

    sleep(1);
}

/*********************************************************************************************************
** 函数名称: tmTimerCreate
** 功能描述: 创建速率统计和统计清除定时器
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  tmTimerCreate (VOID)
{
    _G_ulTmStatTimer           = API_TimerCreate("tm_rate_timer",
                                                 LW_OPTION_ITIMER | LW_OPTION_OBJECT_GLOBAL,
                                                 LW_NULL);
    _G_ulTmStatCountResetTimer = API_TimerCreate("tm_stat_clean_timer",
                                                 LW_OPTION_ITIMER | LW_OPTION_OBJECT_GLOBAL,
                                                 LW_NULL);

    _BugHandle((_G_ulTmStatTimer == LW_OBJECT_HANDLE_INVALID),
                LW_TRUE,
                "can not create 'tm_rate_timer'\r\n");

    _BugHandle((_G_ulTmStatCountResetTimer == LW_OBJECT_HANDLE_INVALID),
                LW_TRUE,
                "can not create 'tm_stat_clean_timer'\r\n");
}

/*********************************************************************************************************
** 函数名称: t_cpuMonitorFunc
** 功能描述: CPU 统计
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  t_cpuMonitorFunc (VOID)
{
    while (1) {
        API_ThreadCPUUsageRefresh();                                    /*  刷新统计变量                */

        API_ThreadCPUUsageOn();

        API_TimeSleep((ULONG)1 * LW_TICK_HZ);

        API_ThreadCPUUsageOff();
    }
}

/*********************************************************************************************************
** 函数名称: tmCpuStatMonitor
** 功能描述: 创建 CPU 监控线程
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  tmCpuStatMonitor (VOID)
{
    LW_CLASS_THREADATTR  threakattr;

    /*
     *  创建 CPU 监控线程
     */
    threakattr            = API_ThreadAttrGetDefault();
    _G_ulTmCpuStatMonitor = API_ThreadCreate("t_cpuMonitor",
                                            (PTHREAD_START_ROUTINE)t_cpuMonitorFunc,
                                            &threakattr,
                                            LW_NULL);
}
/*********************************************************************************************************
** 函数名称: tm_stat_module_func
** 功能描述: 流量管理 ko 模块处理函数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tm_stat_module_func (VOID)
{
    tmInstallCmd();                                                     /*  安装命令行                  */
    tmTimerCreate();                                                    /*  创建定时器                  */
    tmStatCbTableInit();                                                /*  初始化 hash 表              */
    tmStatAllLinesInstall(NULL);                                        /*  安装设备上所有的线路监控    */
    tmCpuStatMonitor();                                                 /*  启动 CPU 监控               */
    tmStartStats();                                                     /*  开始统计                    */
}
