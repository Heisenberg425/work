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
** ��   ��   ��: power_common.h
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ��Դ״̬ͷ�ļ�
*********************************************************************************************************/
#ifndef __POWER_COMMON_H
#define __POWER_COMMON_H

#include "libpower_config.h"
/*********************************************************************************************************
  �������ͺ궨��
*********************************************************************************************************/
#define EVENT_ADD              "add"                                    /*  ���ӵ�Դ�¼�                */
#define EVENT_DELETE           "delete"                                 /*  ɾ����Դ�¼�                */
#define EVENT_START            "start"                                  /*  ���õ�Դ�¼�                */
#define EVENT_STOP             "stop"                                   /*  ��ͣ��Դ�¼�                */
#define EVENT_UPDATE           "update"                                 /*  ���µ�Դ�¼�                */
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define EVENT_ENABLE           (1)                                      /*  ���õ�Դ�¼�                */
#define EVENT_DISABLE          (0)                                      /*  ��ֹ��Դ�¼�                */
#define SHUTDOWN_FLAG          (1)                                      /*  �ػ���־                    */
#define RESTART_FLAG           (2)                                      /*  ������־                    */
#define SHUTDOWN               "shutdown"                               /*  �ػ�                        */
#define RESTART                "restart"                                /*  ����                        */
/*********************************************************************************************************
  ���ݿ�궨��
*********************************************************************************************************/
#define POWER_EVENT_TABLE         "pwrevent"                             /*  power �¼�����           */
#define POWER_EVENT_TABLE_HEAD    "(EVENT_ID INTEGER," \
                                  "EVENT     VARCHAR(32),"  \
                                  "DATE      VARCHAR(32),"  \
                                  "TIME      VARCHAR(32),"  \
                                  "STATUS    INTEGER,"  \
                                  "DESC      VARCHAR(32))"               /*  power �¼�����ʽ         */
/*********************************************************************************************************
  SylixOS list
*********************************************************************************************************/
extern VOID  _List_Line_Add_Tail( PLW_LIST_LINE  plineNew, LW_LIST_LINE_HEADER  *pplineHeader);
extern VOID  _List_Line_Del(PLW_LIST_LINE        plineDel, LW_LIST_LINE_HEADER  *pplineHeader);
/*********************************************************************************************************
  ���������
*********************************************************************************************************/
#ifndef _LIST_OFFSETOF
#define _LIST_OFFSETOF(type, member)                          \
        ((size_t)&((type *)0)->member)                                  /*  ƫ��������                  */
#define _LIST_CONTAINER_OF(ptr, type, member)                 \
        ((type *)((size_t)ptr - _LIST_OFFSETOF(type, member)))          /*  �õ�ptr�������ṹ           */
#define _LIST_ENTRY(ptr, type, member)                        \
        _LIST_CONTAINER_OF(ptr, type, member)                           /*  �������ĸ��ָ��ṹ        */
#define _list_line_get_next(pline)  ((pline)->LINE_plistNext)           /*  ��ȡ��һ��                  */
#endif
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    powerEventInit(VOID);
INT    powerDoOff(__PPOWER_HANDLE  pPowerInfo);
INT    powerDoRestart(__PPOWER_HANDLE  pPowerInfo);
INT    powerEventDoConfig(__PPOWER_HANDLE  pPowerInfo);
PVOID  powerEventGetFromDB(INT*  piRecordNum);
VOID   powerHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    powerEventInsertIntoDB(__PPOWER_HANDLE  pPowerInfo);
INT    powerEventDeleteFromDB(__PPOWER_HANDLE  pPowerInfo);
INT    powerEventStatSetDB(__PPOWER_HANDLE     pPowerInfo, INT  iStatus);
INT    powerEventUpdateToDB(__PPOWER_HANDLE    pPowerInfo);
INT    powerEventTableCreate(VOID);

#endif                                                                  /* __POWER_COMMON_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
