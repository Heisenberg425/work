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
** ��   ��   ��: powerExec.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ��Դ���ò���
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "power_common.h"
#include "common.h"
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static LW_HANDLE               _G_hPowerMutex;                          /*  ���������ݽṹ�����ź���    */
static LW_HANDLE               _G_hPowerThreadId;                       /*  �߳̾��                    */
/*********************************************************************************************************
  ����ͷȫ�ֱ���
*********************************************************************************************************/
static LW_LIST_LINE_HEADER     _G_plinePwrEventHeader = LW_NULL;        /*  ��Դ�¼�����ͷ              */
/*********************************************************************************************************
** ��������: __powerEventGetByEventId
** ��������: �����¼� ID ���ҵ�Դ�¼��ڵ�
** ��  ��  : uiEventId       ��Դ�¼� ID
** ��  ��  : NONE
** ��  ��  : ��Դ�¼��ڵ�
*********************************************************************************************************/
static __PPOWER_NODE  __powerEventGetByEventId (UINT  uiEventId)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ������Դ�¼�����            */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);
        if (uiEventId == pPwrEvntNode->tNodeData.uiEventId) {
            break;                                                      /*  �ҵ�                        */
        }
    }

    if (plineTemp) {
        return  (pPwrEvntNode);
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: __powerEventNodeAdd
** ��������: ����һ���µĽڵ㣨��巨��
** ��  ��  : pPowerInfo     ��Դ�¼�ָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
     *  ����ͷ�ڵ��
     */
    _List_Line_Add_Tail(&pNode->powerLine, &_G_plinePwrEventHeader);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __powerEventNodeDel
** ��������: �����¼� ID ɾ���ڵ�
** ��  ��  : uiEventId       ��Դ�¼� ID
** ��  ��  : NONE
** ��  ��  : NONE
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
** ��������: __powerEventNodeAllDel
** ��������: ɾ����������ڵ�
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeAllDel (VOID)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  ������������                */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);

        _List_Line_Del(&pPwrEvntNode->powerLine, &_G_plinePwrEventHeader);

        free(pPwrEvntNode);
    }

    Lw_SemaphoreM_Post(_G_hPowerMutex);
}
/*********************************************************************************************************
** ��������: __powerEventNodeUpdate
** ��������: �����¼� ID �ڵ�
** ��  ��  : pPowerInfo      ��Դ�¼�ָ��
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __powerEventNodeUpdate (__PPOWER_HANDLE  pPowerInfo)
{
    PLW_LIST_LINE          plineTemp    = LW_NULL;
    __PPOWER_NODE          pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;
         plineTemp != LW_NULL;
         plineTemp  = plineTemp->LINE_plistNext) {                      /*  ������������                */

        pPwrEvntNode = _LIST_ENTRY(plineTemp, __POWER_NODE, powerLine);

        if (pPwrEvntNode->tNodeData.uiEventId == pPowerInfo->uiEventId) {
            memcpy(&pPwrEvntNode->tNodeData, pPowerInfo, sizeof(__POWER_HANDLE));

            break;
        }
    }
}
/*********************************************************************************************************
** ��������: __powerEventNodeStatSet
** ��������: �����¼� ID �ڵ�״̬
** ��  ��  : uiEventId     ��Դ�¼� ID
**           iStatus       ��Դ�¼�״̬
** ��  ��  : NONE
** ��  ��  : NONE
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
** ��������: __powerEventListCreate
** ��������: �����ݿ��ȡ��Դ�¼������뵽����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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

    for (i = 1; i <= iRow; i++) {                                       /*  ���л�ȡ(���˵�һ��)        */
        bzero(&powerInfo, sizeof(__POWER_HANDLE));

        powerInfo.uiEventId = atoi(cDbResult[i * iColumn]);
        powerInfo.iStatus   = atoi(cDbResult[i * iColumn + 4]);

        strncpy(powerInfo.cEvent, cDbResult[i * iColumn + 1], sizeof(powerInfo.cEvent));
        strncpy(powerInfo.cDate,  cDbResult[i * iColumn + 2], sizeof(powerInfo.cDate));
        strncpy(powerInfo.cTime,  cDbResult[i * iColumn + 3], sizeof(powerInfo.cTime));
        strncpy(powerInfo.cDesc,  cDbResult[i * iColumn + 5], sizeof(powerInfo.cDesc));

        /*
         *  ����Դ�¼��ڵ���뵽����
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
** ��������: __powerEventPeriodDetect
** ��������: ���ڼ���Դ�¼��Ƿ���
** ��  ��  : cpcDate   ϵͳ��ǰ����
**           cpcTime   ϵͳ��ǰʱ��
** ��  ��  : NONE
** ��  ��  : �¼�״̬
*********************************************************************************************************/
static INT  __powerEventPeriodDetect (CPCHAR  cpcDate, CPCHAR  cpcTime)
{
    INT            iStatus = 0;
    PLW_LIST_LINE  plineTemp    = LW_NULL;
    __PPOWER_NODE  pPwrEvntNode = LW_NULL;

    for (plineTemp  = _G_plinePwrEventHeader;                           /*  ������Դ�¼�����            */
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
** ��������: __powerEventHandleThread
** ��������: ��Դ�¼������߳�
** ��  ��  : pvArg     �̲߳���
** ��  ��  : NONE
** ��  ��  : NULL
*********************************************************************************************************/
PVOID  __powerEventHandleThread (PVOID  pvArg)
{
    time_t     time;
    struct tm  tmTime;
    INT        iStatus = 0;

    CHAR       cDate[12] = {0};
    CHAR       cTime[12] = {0};

    for (;;) {
        lib_time(&time);                                                /*  ��õ�ǰϵͳʱ��            */
        lib_localtime_r(&time, &tmTime);

        bzero(cDate, sizeof(cDate));
        bzero(cTime, sizeof(cTime));

        sprintf(cDate,"%02d%02d%02d", tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday);
        sprintf(cTime, "%02d:%02d", tmTime.tm_hour, tmTime.tm_min);

        /*
         *  ���ڼ���Ƿ����¼���ʱ��
         */
        Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

        iStatus = __powerEventPeriodDetect(cDate, cTime);

        Lw_SemaphoreM_Post(_G_hPowerMutex);


        if (iStatus & RESTART_FLAG) {                                   /*  ��Ҫ����                    */
            sync();

            API_KernelReboot(LW_REBOOT_COLD);
        }

        if (iStatus & SHUTDOWN_FLAG) {                                  /*  ��Ҫ�ػ�                    */
            sync();

            API_KernelReboot(LW_REBOOT_SHUTDOWN);
        }

        sleep(5);
    }

    return  (NULL);
}
/*********************************************************************************************************
** ��������: powerEventInit
** ��������: ��Դ�¼���ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventInit (VOID)
{
    INT                  iRet;
    LW_CLASS_THREADATTR  threadAttr;

    _G_hPowerMutex = Lw_SemaphoreM_Create("event_lock",                 /*  �����¼��������������      */
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
     *  ���ɵ�Դ�¼�����
     */
    iRet = __powerEventListCreate();
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get power event information failed.\n");

        goto  error;
    }

    /*
     *  ������Դ�¼������߳�
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
** ��������: powerDoOff
** ��������: ϵͳ��Դ�ر�
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  powerDoOff (__PPOWER_HANDLE  pPowerInfo)
{
    sync();

    API_KernelReboot(LW_REBOOT_SHUTDOWN);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerDoRestart
** ��������: ϵͳ����
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  powerDoRestart (__PPOWER_HANDLE  pPowerInfo)
{
    sync();

    API_KernelReboot(LW_REBOOT_COLD);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: powerEventDoConfig
** ��������: ���õ�Դ�¼�
** �䡡��  : pPowerInfo     power ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  powerEventDoConfig (__PPOWER_HANDLE  pPowerInfo)
{
    Lw_SemaphoreM_Wait(_G_hPowerMutex, LW_OPTION_WAIT_INFINITE);

    if (0 == strcmp(pPowerInfo->cOperType, EVENT_ADD)) {                /*  ������¼�                  */
        __powerEventNodeAdd(pPowerInfo);
        powerEventInsertIntoDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_DELETE)) {      /*  ɾ���¼�                    */
        __powerEventNodeDel(pPowerInfo->uiEventId);
        powerEventDeleteFromDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_UPDATE)) {      /*  ���µ�ǰ�¼�                */
        __powerEventNodeUpdate(pPowerInfo);
        powerEventUpdateToDB(pPowerInfo);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_START)) {       /*  ��ʼ��ǰ�¼�                */
        __powerEventNodeStatSet(pPowerInfo->uiEventId, EVENT_ENABLE);
        powerEventStatSetDB (pPowerInfo, EVENT_ENABLE);
    } else if (0 == strcmp(pPowerInfo->cOperType, EVENT_STOP)) {        /*  ֹͣ��ǰ�¼�                */
        __powerEventNodeStatSet(pPowerInfo->uiEventId, EVENT_DISABLE);
        powerEventStatSetDB (pPowerInfo, EVENT_DISABLE);
    } else {                                                            /*  δ֪����                    */
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
