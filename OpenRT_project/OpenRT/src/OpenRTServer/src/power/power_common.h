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
** 文   件   名: power_common.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 系统电源状态头文件
*********************************************************************************************************/
#ifndef __POWER_COMMON_H
#define __POWER_COMMON_H

#include "libpower_config.h"
/*********************************************************************************************************
  操作类型宏定义
*********************************************************************************************************/
#define EVENT_ADD              "add"                                    /*  增加电源事件                */
#define EVENT_DELETE           "delete"                                 /*  删除电源事件                */
#define EVENT_START            "start"                                  /*  启用电源事件                */
#define EVENT_STOP             "stop"                                   /*  暂停电源事件                */
#define EVENT_UPDATE           "update"                                 /*  更新电源事件                */
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define EVENT_ENABLE           (1)                                      /*  启用电源事件                */
#define EVENT_DISABLE          (0)                                      /*  禁止电源事件                */
#define SHUTDOWN_FLAG          (1)                                      /*  关机标志                    */
#define RESTART_FLAG           (2)                                      /*  重启标志                    */
#define SHUTDOWN               "shutdown"                               /*  关机                        */
#define RESTART                "restart"                                /*  重启                        */
/*********************************************************************************************************
  数据库宏定义
*********************************************************************************************************/
#define POWER_EVENT_TABLE         "pwrevent"                             /*  power 事件表单名           */
#define POWER_EVENT_TABLE_HEAD    "(EVENT_ID INTEGER," \
                                  "EVENT     VARCHAR(32),"  \
                                  "DATE      VARCHAR(32),"  \
                                  "TIME      VARCHAR(32),"  \
                                  "STATUS    INTEGER,"  \
                                  "DESC      VARCHAR(32))"               /*  power 事件表单格式         */
/*********************************************************************************************************
  SylixOS list
*********************************************************************************************************/
extern VOID  _List_Line_Add_Tail( PLW_LIST_LINE  plineNew, LW_LIST_LINE_HEADER  *pplineHeader);
extern VOID  _List_Line_Del(PLW_LIST_LINE        plineDel, LW_LIST_LINE_HEADER  *pplineHeader);
/*********************************************************************************************************
  链表操作宏
*********************************************************************************************************/
#ifndef _LIST_OFFSETOF
#define _LIST_OFFSETOF(type, member)                          \
        ((size_t)&((type *)0)->member)                                  /*  偏移量计算                  */
#define _LIST_CONTAINER_OF(ptr, type, member)                 \
        ((type *)((size_t)ptr - _LIST_OFFSETOF(type, member)))          /*  得到ptr的容器结构           */
#define _LIST_ENTRY(ptr, type, member)                        \
        _LIST_CONTAINER_OF(ptr, type, member)                           /*  获得链表母体指针结构        */
#define _list_line_get_next(pline)  ((pline)->LINE_plistNext)           /*  获取下一个                  */
#endif
/*********************************************************************************************************
  函数声明
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
