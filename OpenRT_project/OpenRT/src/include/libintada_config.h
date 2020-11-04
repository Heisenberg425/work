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
** 文   件   名: libintada_config.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: 网口定位信息配置头文件
*********************************************************************************************************/
#ifndef __LIBINTADA_CONFIG_H
#define __LIBINTADA_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  网口定位服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_ADA_GETS_CMD         (OPEN_RT_INTADA_CONFIG_BASE | 1)    /*  获取定位状态                */
#define OPENRT_ADA_GETS_ACK_CMD     (OPEN_RT_INTADA_CONFIG_BASE | 2)    /*  获取定位状态应答            */

#define OPENRT_NETIF_GET_CMD        (OPEN_RT_INTADA_CONFIG_BASE | 3)    /*  获取 netif                  */
#define OPENRT_NETIF_GET_ACK_CMD    (OPEN_RT_INTADA_CONFIG_BASE | 4)    /*  获取 netif 应答             */

#define OPENRT_ADA_SET_CMD          (OPEN_RT_INTADA_CONFIG_BASE | 5)    /*  进行定位操作                */
#define OPENRT_ADA_SET_ACK_CMD      (OPEN_RT_INTADA_CONFIG_BASE | 6)    /*  进行定位操作应答            */

#define OPENRT_ADA_CLEAR_CMD        (OPEN_RT_INTADA_CONFIG_BASE | 7)    /*  删除定位操作                */
#define OPENRT_ADA_CLEAR_ACK_CMD    (OPEN_RT_INTADA_CONFIG_BASE | 8)    /*  删除定位操作应答            */

#define OPENRT_WEBNETIF_GET_CMD     (OPEN_RT_INTADA_CONFIG_BASE | 9)    /*  获取 web netif              */
#define OPENRT_WEBNETIF_GET_ACK_CMD (OPEN_RT_INTADA_CONFIG_BASE | 10)   /*  获取 web netif 应答         */

/*********************************************************************************************************
  __INT_ADA_SET flag 字段宏
*********************************************************************************************************/
#define INT_ADA_FLAG_FIRST          (0)                                 /*  第一个网口                  */
#define INT_ADA_FLAG_LAST           (1)                                 /*  最后一个网口                */
#define INT_ADA_FLAG_MIDDLE         (2)                                 /*  中间网口                    */
/*********************************************************************************************************
  向配置工具发送的信息
*********************************************************************************************************/
struct int_ada_set {
    INT                iWebIndex;                                       /*  Web 对应的索引              */
    CHAR               cWebName[NETIF_NAMESIZE];                        /*  Web 设置的网口名称          */
    CHAR               cFlag;                                           /*  顺序标志                    */
};
typedef struct int_ada_set  __INT_ADA_SET;
typedef __INT_ADA_SET      *__PINT_ADA_SET;

#endif                                                                  /* __LIBINTADA_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
