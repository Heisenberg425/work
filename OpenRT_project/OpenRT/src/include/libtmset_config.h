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
** 文   件   名: libtmset_config.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: 时间设置头文件
*********************************************************************************************************/
#ifndef __LIBTMSET_CONFIG_H
#define __LIBTMSET_CONFIG_H

#include "libserver_type.h"
/*********************************************************************************************************
  系统时间相关命令宏定义 (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_SET_TIME_CMD         (OPEN_RT_TIMESET_CONFIG_BASE | 1)   /*  设置系统时间命令            */
#define OPENRT_SET_TIME_ACK_CMD     (OPEN_RT_TIMESET_CONFIG_BASE | 2)   /*  设置系统时间应答命令        */

#define OPENRT_GET_TIME_CMD         (OPEN_RT_TIMESET_CONFIG_BASE | 3)   /*  获取系统时间命令            */
#define OPENRT_GET_TIME_ACK_CMD     (OPEN_RT_TIMESET_CONFIG_BASE | 4)   /*  获取系统时间应答命令        */

/*********************************************************************************************************
 用于系统时间信息的结构
*********************************************************************************************************/
struct time_handle {
    CHAR  cDate[16];                                                    /*  日期                        */
    CHAR  cTime[16];                                                    /*  时间                        */
};

typedef struct time_handle  __TIME_HANDLE;
typedef struct time_handle  *__PTIME_HANDLE;

#endif                                                                  /*  __LIBTMSET_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
