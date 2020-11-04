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
** 文   件   名: libudial_config.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: 拨号信息配置头文件
*********************************************************************************************************/
#ifndef __LIBUDIAL_CONFIG_H
#define __LIBUDIAL_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  网口定位服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_UDIAL_START         (OPEN_RT_UDAIL_CONFIG_BASE | 1)      /*  启动拨号                    */
#define OPENRT_UDIAL_START_ACK     (OPEN_RT_UDAIL_CONFIG_BASE | 2)      /*  启动拨号 应答               */

#define OPENRT_UDIAL_DEL           (OPEN_RT_UDAIL_CONFIG_BASE | 3)      /*  删除拨号                    */
#define OPENRT_UDIAL_DEL_ACK       (OPEN_RT_UDAIL_CONFIG_BASE | 4)      /*  删除拨号 应答               */

#define OPENRT_UDIAL_INFO          (OPEN_RT_UDAIL_CONFIG_BASE | 5)      /*  获取拨号信息                */
#define OPENRT_UDIAL_INFO_ACK      (OPEN_RT_UDAIL_CONFIG_BASE | 6)      /*  获取拨号信息 应答           */
/*********************************************************************************************************
 用于配置拨号信息的结构
*********************************************************************************************************/
struct udial_handle {
    CHAR               cUsrname[20];                                    /*  拨号账号                    */
    CHAR               cPassword[20];                                   /*  拨号密码                    */
    CHAR               cNetifname[NETIF_NAMESIZE];                      /*  拨号物理网卡名              */
    CHAR               cPppname[NETIF_NAMESIZE];                        /*  拨号 PPP 网卡名             */
    INT                iUdialStatus;                                    /*  拨号状态                    */
    UINT32             uiPid;                                           /*  拨号进程 PID                */
};
typedef struct udial_handle   __UDIAL_HANDLE;
typedef struct udial_handle  *__PUDIAL_HANDLE;

#endif                                                                  /*  __LIBUDIAL_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
