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
** 文   件   名: libpower_config.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 电源状态设置头文件
*********************************************************************************************************/
#ifndef __LIBPOWER_CONFIG_H
#define __LIBPOWER_CONFIG_H

#include "libserver_type.h"

/*********************************************************************************************************
  系统电源相关命令宏定义 (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_POWER_OFF_CMD          (OPEN_RT_POWER_CONFIG_BASE | 1)   /*  关闭系统命令                */
#define OPENRT_POWER_OFF_ACK_CMD      (OPEN_RT_POWER_CONFIG_BASE | 2)   /*  关闭系统应答命令            */

#define OPENRT_POWER_RESTART_CMD      (OPEN_RT_POWER_CONFIG_BASE | 3)   /*  重启系统命令                */
#define OPENRT_POWER_RESTART_ACK_CMD  (OPEN_RT_POWER_CONFIG_BASE | 4)   /*  重启系统应答命令            */

#define OPENRT_POWER_EVT_CFG_CMD      (OPEN_RT_POWER_CONFIG_BASE | 5)   /*  电源事件配置命令            */
#define OPENRT_POWER_EVT_CFG_ACK_CMD  (OPEN_RT_POWER_CONFIG_BASE | 6)   /*  电源事件配置应答命令        */

#define OPENRT_POWER_EVT_SHOW_CMD     (OPEN_RT_POWER_CONFIG_BASE | 7)   /*  电源事件显示命令            */
#define OPENRT_POWER_EVT_SHOW_ACK_CMD (OPEN_RT_POWER_CONFIG_BASE | 8)   /*  电源事件显示应答命令        */

/*********************************************************************************************************
 用于系统电源状态信息的结构
*********************************************************************************************************/
struct power_handle {
    UINT  uiEventId;                                                    /*  事件 ID                     */
    CHAR  cEvent[16];                                                   /*  事件                        */
    CHAR  cDate[12];                                                    /*  日期                        */
    CHAR  cTime[12];                                                    /*  时间                        */
    CHAR  cOperType[12];                                                /*  操作类型                    */
    CHAR  cDesc[128];                                                   /*  说明信息                    */
    INT   iStatus;                                                      /*  状态：是否启用              */
};
typedef struct power_handle  __POWER_HANDLE;
typedef struct power_handle  *__PPOWER_HANDLE;
/*********************************************************************************************************
  事件链表节点
*********************************************************************************************************/
typedef struct power_node{
    LW_LIST_LINE             powerLine;
    __POWER_HANDLE           tNodeData;                                 /*  节点数据                    */
} __POWER_NODE;
typedef __POWER_NODE       *__PPOWER_NODE;

#endif                                                                  /*  __LIBPOWER_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
