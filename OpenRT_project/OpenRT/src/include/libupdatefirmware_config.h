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
** 文   件   名: libupdatefirmware_config.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 3 日
**
** 描        述: update firmware 头文件
*********************************************************************************************************/
#ifndef __LIBUPDATE_FIRMWARE_CONFIG_H
#define __LIBUPDATE_FIRMWARE_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"

/*********************************************************************************************************
  update firmware 服务ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_UPDATEFIRMWARE_CMD     (OPEN_RT_UPDATEFIRMWARE_CONFIG_BASE | 1)
                                                                        /*  更新系统固件                */
#define OPENRT_CFG_UPDATEFIRMWARE_ACK_CMD (OPEN_RT_UPDATEFIRMWARE_CONFIG_BASE | 2)
                                                                        /*  更新系统固件应答            */
/*********************************************************************************************************
  固件更新相关宏定义
*********************************************************************************************************/
#define UPDATE_FIRMWARE_LOCATION             "/tmp"                     /*  系统更新包位置              */
#define UPDATE_FIRMWARE_TAR                  "update.tar.gz"            /*  系统更新包名称              */
#define UPDATE_FIRMWARE_DIRECTORY            "update"                   /*  系统更新包目录名称          */
#define UPDATE_FIRMWARE_SHELL                "update.sh"                /*  系统更新脚本                */
#define UPDATE_FIRMWARE_DATING_FLAG          "update.flg"               /*  正在更新标志文件            */

#endif                                                                  /* __LIBUPDATE_FIRMWARE_CONFIG_H*/
/*********************************************************************************************************
  END
*********************************************************************************************************/
