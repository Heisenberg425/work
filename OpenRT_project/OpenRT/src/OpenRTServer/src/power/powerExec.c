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
** 文   件   名: powerExec.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统电源设置操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "power_common.h"
#include "common.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static LW_HANDLE               _G_hPowerMutex;                          /*  操作该数据结构互斥信号量    */
static LW_HANDLE               _G_hPowerThreadId;                       /*  线程句柄                    */
/*********************************************************************************************************
  链表头全局变量
*********************************************************************************************************/
static LW_LIST_LINE_HEADER     _G_plinePwrEventHeader = LW_NULL;        /*  电源事件链表头              */
/*********************************************************************************************************
** 函数名称: __powerEventGetByEventId
** 功能描述: 根据事件 ID 查找电源事件节点
** 输  入  : uiEventId       电源事件 ID
** 输  出  : NONE
** 返  回  : 电源事件节点
*********************************************************************************************************/
static __PPOWER_NODE  __powerEventGetByEventId (UINT  uiEventId)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  遍历电源事件链表            */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);
        if (uiEventId == pPwrEvntNode->tNodeData.uiEventId) {
            break;                                                      /*  找到                        */
        }
    }

    if (plineTemp) {
        return  (pPwrEvntNode);
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** 函数名称: __powerEventNodeAdd
** 功能描述: 插入一个新的节点（后插法）
** 输  入  : pPowerInfo     电源事件指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __powerEventNodeAdd (__PPOWER_HANDLE  pPowerInfo)
{
    __PPOWER_NODE  pNode = NULL;

    if (NULL == pPowerInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "invalid arguments.\n");

        return  (PX_ERROR);
    }

    pNode = (__PPOWER_NODE)malloc(sizeof(__POWER_NODE));
    if (NULL == pNode) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc Memory Failed.\n");

        return  (PX_ERROR);
    }

    bzero(pNode, sizeof(__POWER_NODE));

    memcpy(&pNode->tNodeData, pPowerInfo, sizeof(__POWER_HANDLE));

    /*
     *  插入头节点后
     */
    _List_Line_Add_Tail(&pNode->powerLine, &_G_plinePwrEventHeader);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __powerEventNodeDel
** 功能描述: 根据事件 ID 删除节点
** 输  入  : uiEventId       电源事件 ID
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeDel (UINT  uiEventId)
{
    __PPOWER_NODE  pPowerNode = NULL;

    pPowerNode = __powerEventGetByEventId(uiEventId);
    if (NULL != pPowerNode) {
        _List_Line_Del(&pPowerNode->powerLine, &_G_plinePwrEventHeader);

        free(pPowerNode);
    }
}
/*********************************************************************************************************
** 函数名称: __powerEventNodeAllDel
** 功能描述: 删除所有链表节点
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeAllDel (VOID)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  遍历适配器表                */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);

        _List_Line_Del(&pPwrEvntNode->powerLine, &_G_plinePwrEventHeader);

        free(pPwrEvntNode);
    }

    Lw_SemaphoreM_Post(_G_hPowerMutex);
}
/*********************************************************************************************************
** 函数名称: __powerEventNodeUpdate
** 功能描述: 更新事件 ID 节点
** 输  入  : pPowerInfo      电源事件指针
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeUpdate (__PPOWER_HANDLE  pPowerInfo)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  遍历适配器表                */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);

        if (pPwrEvntNode->tNodeData.uiEventId == pPowerInfo->uiEventId) {
            memcpy(&pPwrEvntNode->tNodeData, pPowerInfo, sizeof(__POWER_HANDLE));

            break;
        }
    }
}
/*********************************************************************************************************
** 函数名称: __powerEventNodeStatSet
** 功能描述: 设置事件 ID 节点状态
** 输  入  : uiEventId     电源事件 ID
**           iStatus       电源事件状态
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeStatSet (UINT  uiEventId, INT  iStatus)
{
    __PPOWER_NODE  pPowerNode = NULL;

    pPowerNode = __powerEventGetByEventId(uiEventId);
    if (NULL != pPowerNode) {
        pPowerNode->tNodeData.iStatus = iStatus;
    }
}
/*********************************************************************************************************
** 函数名称: __powerEventListCreate
** 功能描述: 从数据库获取电源事件并加入到链表
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __powerEventListCreate (VOID)
{
    INT              iRet;
    sqlite3         *pDb             = NULL;
    CHAR             cSelectSql[200] = {0};
    CHAR           **cDbResult       = NULL;
    CHAR            *pcErrMsg        = NULL;
    INT              iRow            = 0;
    INT              iColumn         = 0;
    __POWER_HANDLE   powerInfo;
    INT              i;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", POWER_EVENT_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        goto Error_Handle;
    }

    Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

    for (i = 1; i <= iRow; i++) {                                       /*  按行获取(过滤第一行)        */
        bzero(&powerInfo, sizeof(__POWER_HANDLE));

        powerInfo.uiEventId = atoi(cDbResult[i * iColumn]);
        powerInfo.iStatus   = atoi(cDbResult[i * iColumn + 4]);

        strncpy(powerInfo.cEvent, cDbResult[i * iColumn + 1], sizeof(powerInfo.cEvent));
        strncpy(powerInfo.cDate,  cDbResult[i * iColumn + 2], sizeof(powerInfo.cDate));
        strncpy(powerInfo.cTime,  cDbResult[i * iColumn + 3], sizeof(powerInfo.cTime));
        strncpy(powerInfo.cDesc,  cDbResult[i * iColumn + 5], sizeof(powerInfo.cDesc));

        /*
         *  将电源事件节点加入到链表
         */
        __powerEventNodeAdd(&powerInfo);
    }

    Lw_SemaphoreM_Post(_G_hPowerMutex);

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    __powerEventNodeAllDel();

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: __powerEventPeriodDetect
** 功能描述: 周期检测电源事件是否发生
** 输  入  : cpcDate   系统当前日期
**           cpcTime   系统当前时间
** 输  出  : NONE
** 返  回  : 事件状态
*********************************************************************************************************/
static INT  __powerEventPeriodDetect (CPCHAR  cpcDate, CPCHAR  cpcTime)
{
    INT            iStatus = 0;
    PLW_LIST_LINE  plineTemp    = LW_NULL;
    __PPOWER_NODE  pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;                           /*  遍历电源事件链表            */
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);
        if (EVENT_ENABLE == pPwrEvntNode->tNodeData.iStatus) {
            if ((0 == strcmp(pPwrEvntNode->tNodeData.cDate, cpcDate)) &&
                (0 == strcmp(pPwrEvntNode->tNodeData.cTime, cpcTime))) {
                if (0 == strcmp(pPwrEvntNode->tNodeData.cEvent, SHUTDOWN)) {
                    iStatus |= SHUTDOWN_FLAG;
                } else if (0 == strcmp(pPwrEvntNode->tNodeData.cEvent, RESTART)) {
                    iStatus |= RESTART_FLAG;
                } else {
                    __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "invalid power event type.\n");

                    continue;
                }

                __powerEventNodeStatSet(pPwrEvntNode->tNodeData.uiEventId, EVENT_DISABLE);
                powerEventStatSetDB(&pPwrEvntNode->tNodeData, EVENT_DISABLE);
            }
        }
    }

    return  (iStatus);
}
/*********************************************************************************************************
** 函数名称: __powerEventHandleThread
** 功能描述: 电源事件处理线程
** 输  入  : pvArg     线程参数
** 输  出  : NONE
** 返  回  : NULL
*********************************************************************************************************/
PVOID  __powerEventHandleThread (PVOID  pvArg)
{
    time_t     time;
    struct tm  tmTime;
    INT        iStatus = 0;

    CHAR       cDate[12] = {0};
    CHAR       cTime[12] = {0};

    for (;;) {
        lib_time(&time);                                                /*  获得当前系统时间            */
        lib_localtime_r(&time, &tmTime);

        bzero(cDate, sizeof(cDate));
        bzero(cTime, sizeof(cTime));

        sprintf(cDate,"%02d%02d%02d", tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday);
        sprintf(cTime, "%02d:%02d", tmTime.tm_hour, tmTime.tm_min);

        /*
         *  周期检测是否有事件定时到
         */
        Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

        iStatus = __powerEventPeriodDetect(cDate, cTime);

        Lw_SemaphoreM_Post(_G_hPowerMutex);


        if (iStatus & RESTART_FLAG) {                                   /*  需要重启                    */
            sync();

            API_KernelReboot(LW_REBOOT_COLD);
        }

        if (iStatus & SHUTDOWN_FLAG) {                                  /*  需要关机                    */
            sync();

            API_KernelReboot(LW_REBOOT_SHUTDOWN);
        }

        sleep(5);
    }

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: powerEventInit
** 功能描述: 电源事件初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventInit (VOID)
{
    INT                  iRet;
    LW_CLASS_THREADATTR  threadAttr;

    _G_hPowerMutex = Lw_SemaphoreM_Create("event_lock",                 /*  创建事件链表操作互斥锁      */
                                          LW_PRIO_HIGH,
                                          LW_OPTION_WAIT_FIFO |
                                          LW_OPTION_OBJECT_LOCAL |
                                          LW_OPTION_INHERIT_PRIORITY |
                                          LW_OPTION_ERRORCHECK,
                                          LW_NULL);

    if (LW_OBJECT_HANDLE_INVALID == _G_hPowerMutex) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "mutex initialize failed.\n");

        goto  error;
    }

    /*
     *  生成电源事件链表
     */
    iRet = __powerEventListCreate();
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get power event information failed.\n");

        goto  error;
    }

    /*
     *  创建电源事件处理线程
     */
    Lw_ThreadAttr_Build(&threadAttr,
                        12 * LW_CFG_THREAD_DEFAULT_STK_SIZE,
                        LW_PRIO_NORMAL,
                        LW_OPTION_THREAD_STK_CHK,
                        NULL);

    _G_hPowerThreadId = Lw_Thread_Create("t_pwrevent",
                              __powerEventHandleThread,
                              &threadAttr,
                              LW_NULL);
    if (LW_OBJECT_HANDLE_INVALID == _G_hPowerThreadId) {
        fprintf(stderr, "Thread Create Failed.\n");

        goto  error;
    }

    return  (ERROR_NONE);

error:
    __powerEventNodeAllDel();

    if (LW_OBJECT_HANDLE_INVALID != _G_hPowerMutex) {
        Lw_SemaphoreM_Delete(&_G_hPowerMutex);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: powerDoOff
** 功能描述: 系统电源关闭
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  powerDoOff (__PPOWER_HANDLE  pPowerInfo)
{
    sync();

    API_KernelReboot(LW_REBOOT_SHUTDOWN);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: powerDoRestart
** 功能描述: 系统重启
** 输　入  : pPowerInfo  系统电源参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  powerDoRestart (__PPOWER_HANDLE  pPowerInfo)
{
    sync();

    API_KernelReboot(LW_REBOOT_COLD);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: powerEventDoConfig
** 功能描述: 配置电源事件
** 输　入  : pPowerInfo     power 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventDoConfig (__PPOWER_HANDLE  pPowerInfo)
{
    Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

    if (0 == strcmp(pPowerInfo->cOperType, EVENT_ADD)) {                /*  添加新事件                  */
        __powerEventNodeAdd(pPowerInfo);
        powerEventInsertIntoDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_DELETE)) {      /*  删除事件                    */
        __powerEventNodeDel(pPowerInfo->uiEventId);
        powerEventDeleteFromDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_UPDATE)) {      /*  更新当前事件                */
        __powerEventNodeUpdate(pPowerInfo);
        powerEventUpdateToDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_START)) {       /*  开始当前事件                */
        __powerEventNodeStatSet(pPowerInfo->uiEventId, EVENT_ENABLE);
        powerEventStatSetDB (pPowerInfo, EVENT_ENABLE);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_STOP)) {        /*  停止当前事件                */
        __powerEventNodeStatSet(pPowerInfo->uiEventId, EVENT_DISABLE);
        powerEventStatSetDB (pPowerInfo, EVENT_DISABLE);
    } else {                                                            /*  未知操作                    */
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "invalid operate type.\n");

        Lw_SemaphoreM_Post(_G_hPowerMutex);

        return  (PX_ERROR);
    }

    Lw_SemaphoreM_Post(_G_hPowerMutex);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
