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
** ��   ��   ��: tmModuleInit.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT �������ģ���ʼ��
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
  ��������
*********************************************************************************************************/
VOID  tm_stat_module_func (VOID);
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
LW_OBJECT_HANDLE  _G_ulTmStatTimer           = LW_OBJECT_HANDLE_INVALID;/*  ����ͳ�ƶ�ʱ��              */
LW_OBJECT_HANDLE  _G_ulTmStatCountResetTimer = LW_OBJECT_HANDLE_INVALID;/*  ͳ���������ʱ��            */
LW_OBJECT_HANDLE  _G_ulTmCpuStatMonitor      = LW_OBJECT_HANDLE_INVALID;/*  CPU ͳ���߳̾��            */
/*********************************************************************************************************
** ��������: module_init
** ��������: ko ģ���ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  module_init (VOID)
{
    tm_stat_module_func();

    return  (0);
}

/*********************************************************************************************************
** ��������: module_exit
** ��������: koģ��ж�غ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
** ��������: tmTimerCreate
** ��������: ��������ͳ�ƺ�ͳ�������ʱ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
** ��������: t_cpuMonitorFunc
** ��������: CPU ͳ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  t_cpuMonitorFunc (VOID)
{
    while (1) {
        API_ThreadCPUUsageRefresh();                                    /*  ˢ��ͳ�Ʊ���                */

        API_ThreadCPUUsageOn();

        API_TimeSleep((ULONG)1 * LW_TICK_HZ);

        API_ThreadCPUUsageOff();
    }
}

/*********************************************************************************************************
** ��������: tmCpuStatMonitor
** ��������: ���� CPU ����߳�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmCpuStatMonitor (VOID)
{
    LW_CLASS_THREADATTR  threakattr;

    /*
     *  ���� CPU ����߳�
     */
    threakattr            = API_ThreadAttrGetDefault();
    _G_ulTmCpuStatMonitor = API_ThreadCreate("t_cpuMonitor",
                                            (PTHREAD_START_ROUTINE)t_cpuMonitorFunc,
                                            &threakattr,
                                            LW_NULL);
}
/*********************************************************************************************************
** ��������: tm_stat_module_func
** ��������: �������� ko ģ�鴦����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
LW_SYMBOL_EXPORT VOID  tm_stat_module_func (VOID)
{
    tmInstallCmd();                                                     /*  ��װ������                  */
    tmTimerCreate();                                                    /*  ������ʱ��                  */
    tmStatCbTableInit();                                                /*  ��ʼ�� hash ��              */
    tmStatAllLinesInstall(NULL);                                        /*  ��װ�豸�����е���·���    */
    tmCpuStatMonitor();                                                 /*  ���� CPU ���               */
    tmStartStats();                                                     /*  ��ʼͳ��                    */
}
