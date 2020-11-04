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
** 文   件   名: libnpf_config.h
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 24 日
**
** 描        述: npf 配置头文件
*********************************************************************************************************/

#ifndef __LIBNPF_CONFIG_H
#define __LIBNPF_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  NPF 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_CFG_NPF_CMD              (OPEN_RT_NPF_CONFIG_BASE | 1)   /*  配置 NPF 规则               */
#define OPENRT_GET_NPF_CMD              (OPEN_RT_NPF_CONFIG_BASE | 2)   /*  获取 NPF 规则               */
/*********************************************************************************************************
  过滤规则
*********************************************************************************************************/
#define NPFRULE_MAC                     (0)                             /*  MAC 过滤规则                */
#define NPFRULE_IP                      (1)                             /*  IP 过滤规则                 */
#define NPFRULE_UDP                     (2)                             /*  TCP 过滤规则                */
#define NPFRULE_TCP                     (3)                             /*  UDP 过滤规则                */
#define NPFRULE_ATTACH                  (255)                           /*  开启过滤规则                */

#define NPFRULE_ATTACH_ENABLE           (1)                             /*  NPF 规则开启                */
/*********************************************************************************************************
  npf 配置信息数据结构
*********************************************************************************************************/
struct npf_handle {
    INT       iRuleSeq;                                                 /*  RULE 的索引号               */
    CHAR      cNetifName[NETIF_NAMESIZE];                               /*  网口名称                    */
    CHAR      cHandle[64];                                              /*  操作类型                    */
    UINT8     ucType;                                                   /*  NPF 规则类型                */
    CHAR      cHwAddr[18];                                              /*  过滤 MAC 地址               */
    CHAR      cIpAddrS[16];                                             /*  过滤起始 IP 地址            */
    CHAR      cIpAddrE[16];                                             /*  过滤终止 IP 地址            */
    UINT16    usPortS;                                                  /*  过滤起始 PORT               */
    UINT16    usPortE;                                                  /*  过滤终止 PORT               */
    CHAR      cDirect[16];                                              /*  数据方向 input/output       */
    CHAR      cMangle[16];                                              /*  数据处理 allow/deny         */
    BOOL      bEnable;                                                  /*  NPF 过滤功能打开            */
    INT       iTotal;                                                   /*  总目录条数                  */
    INT       iPage;                                                    /*  当前页码                    */
    INT       iPageSize;                                                /*  需要显示的条目数            */
    INT       iItemStart;                                               /*  数据库开始的条目            */
    CHAR      cWhereLimit[256];                                         /*  按页查询条件                */
};
typedef struct npf_handle    __NPF_HANDLE;
typedef struct npf_handle   *__PNPF_HANDLE;

#endif                                                                  /* __LIBNPF_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
