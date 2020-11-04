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
** 文   件   名: common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述: 通用操作
*********************************************************************************************************/
#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <SylixOS.h>
#include "sqlite3.h"

/*********************************************************************************************************
  系统版本信息
*********************************************************************************************************/
#define OPENRT_VERSION             "0.8.13"                             /*  OpenRT 版本信息             */

/*********************************************************************************************************
  初始化返回值检测
*********************************************************************************************************/
#define INIT_CHECK(func)           do {                              \
                                       if (func) {                   \
                                           return  (PX_ERROR);       \
                                       }                             \
                                   } while (0);
/*********************************************************************************************************
  打印配置
*********************************************************************************************************/
#define OPENRT_LOG_LOG_EN          (1)                                  /*  调试信息使能                */
#define OPENRT_LOG_ERR_EN          (1)                                  /*  调试信息使能                */
/*********************************************************************************************************
  打印等级
*********************************************************************************************************/
#define OPENRT_LOG_LOG             "log"                                /*  调试信息                    */
#define OPENRT_LOG_LOG_LEVEL       (0x01)                               /*  输出调试信息                */
#define OPENRT_LOG_ERR             "error"                              /*  错误信息                    */
#define OPENRT_LOG_ERR_LEVEL       (0x02)                               /*  输出错误信息                */
/*********************************************************************************************************
  打印实现
*********************************************************************************************************/
#define __OPENRT_DEBUGF(level, format, ...)                                       \
    do {                                                                          \
        if (level == OPENRT_LOG_ERR_LEVEL && OPENRT_LOG_LOG_EN) {                 \
            fprintf(stderr,                                                       \
                    "[log_level: "OPENRT_LOG_ERR" | %s @ %s, %d] " format ,       \
                    __func__,                                                     \
                    __FILE__,                                                     \
                    __LINE__,                                                     \
                    ##__VA_ARGS__);                                               \
        } else if (level == OPENRT_LOG_LOG_LEVEL && OPENRT_LOG_LOG_EN) {          \
            fprintf(stderr,                                                       \
                    "[log_level: "OPENRT_LOG_LOG" | %s @ %s, %d] " format ,       \
                    __func__,                                                     \
                    __FILE__,                                                     \
                    __LINE__,                                                     \
                    ##__VA_ARGS__);                                               \
        }                                                                         \
    } while (0)

/*********************************************************************************************************
  数据库相关公用结构
*********************************************************************************************************/
/*********************************************************************************************************
  数据库配置
*********************************************************************************************************/
#define DATABASE_PATH        "/etc/OpenRT"                              /*  数据库路径                  */
#define CFG_DB_FILE          DATABASE_PATH"/OpenRTCFGDB"                /*  配置数据库名称              */
/*********************************************************************************************************
  数据库全局句柄
*********************************************************************************************************/
sqlite3  *_G_pOpenRTCfgDBHandle;
/*********************************************************************************************************
  获取数据库句柄
*********************************************************************************************************/
#define OPENRT_DB_HANDLE(pDb)    do {                              \
                                     pDb =  _G_pOpenRTCfgDBHandle; \
                                 } while(0);
/*********************************************************************************************************
  调用子进程工具时，子进程主动结束时，信号处理公用接口
*********************************************************************************************************/
INT  subRecliamHandleAdd (VOID  (*pvHandleFunc)(int));
INT  subProRecliamInit (VOID);

/*********************************************************************************************************
  内核模式下的函数
*********************************************************************************************************/
extern INT  if_list_lock (BOOL  bWrite);
extern INT  if_list_unlock (VOID);
#endif                                                                  /* SRC_COMMON_H_                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
