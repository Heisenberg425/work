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
** 文   件   名: dhcpserver_common.h
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 18 日
**
** 描        述: DHCP Server信息头文件
**2018.09.20    DHCP SERVER 支持在多网口设备运行多个服务器
*********************************************************************************************************/
#ifndef __DHCPSERVER_COMMON_H
#define __DHCPSERVER_COMMON_H

#include "libdhcpserver_config.h"
/*********************************************************************************************************
  DHCP 相关宏定义
*********************************************************************************************************/
#define DHCPSERVER_INI_FILE        "/etc/dhcpdconfig.ini"               /* DHCP Server 配置文件         */
#define DHCPSERVER_SHOW            "dhcpserver"                         /* 查询DHCP Server信息          */
#define DHCPSERVER_SET             "set"                                /* 增加或修改DHCP Server信息    */
/*********************************************************************************************************
  DHCP Server 配置项
*********************************************************************************************************/
#define DHCPSERVER_CMD_START       "ipaddrpool:start"                   /* 地址池起始地址               */
#define DHCPSERVER_CMD_END         "ipaddrpool:end"                     /* 地址池结束地址               */
#define DHCPSERVER_CMD_LEASEFILE   "filepath:leasefile"                 /* 租约文件保存位置             */
#define DHCPSERVER_CMD_LOGFILE     "filepath:logfile"                   /* 日志文件保存位置             */
#define DHCPSERVER_CMD_INTERFACE   "network:interface"                  /* 服务使用网卡名称             */
#define DHCPSERVER_CMD_DNS1        "opt:dns1"                           /* 主dns                        */
#define DHCPSERVER_CMD_DNS2        "opt:dns2"                           /* 备用dns                      */
#define DHCPSERVER_CMD_MASK        "opt:subnet"                         /* 掩码                         */
#define DHCPSERVER_CMD_GATEWAY     "opt:router"                         /* 网关                         */
#define DHCPSERVER_CMD_LEASE       "opt:lease"                          /* 租约时间（单位100天）        */
#define DHCPSERVER_CMD_T1          "opt:t1"                             /* 租约时间50%                  */
#define DHCPSERVER_CMD_T2          "opt:t2"                             /* 租约时间87.5%                */
/*********************************************************************************************************
  DHCP Server 默认配置
*********************************************************************************************************/
#define DHCPSERVER_DEF_ENABLE      (0)                                  /* dhcp server默认关闭          */
#define DHCPSERVER_DEF_START       "192.168.1.100"                      /* 地址池起始地址默认值         */
#define DHCPSERVER_DEF_END         "192.168.1.200"                      /* 地址池结束地址默认值         */
#define DHCPSERVER_DEF_PATH        "/var/dhcplease"                     /* DHCP 默认路径                */
#define DHCPSERVER_DEF_LEASEFILE   "/var/dhcplease/dhcpd.leases"        /* 租约文件保存位置默认值       */
#define DHCPSERVER_DEF_LOGFILE     "/var/dhcplease/dhcpdlog"            /* 日志文件保存位置默认值       */
#define DHCPSERVER_DEF_INTERFACE   "en1"                                /* 服务使用网卡名称默认值       */
#define DHCPSERVER_DEF_DNS1        "8.8.8.8"                            /* 主dns默认值                  */
#define DHCPSERVER_DEF_DNS2        "8.8.8.8"                            /* 备用dns默认值                */
#define DHCPSERVER_DEF_MASK        "255.255.255.0"                      /* 掩码默认值                   */
#define DHCPSERVER_DEF_GATEWAY     "192.168.1.1"                        /* 网关默认值                   */
#define DHCPSERVER_DEF_LEASE       (86400)                              /* 租约时间（单位100天）默认值  */

#define DHCPSERVER_NAME            "dhcpserver "                        /* DHCP Server 后台进程名称     */
#define DHCPSERVER_PATH            "/apps/dhcpserver/dhcpserver"        /* DHCP Server 路径             */

#define DHCPSERVER_ENABLE          (1)                                  /* dhcp server 使能             */
#define DHCPSERVER_DISABLE         (0)                                  /* dhcp server 禁止             */
#define INVALID_PID                (-1)                                 /* 无效 PID                     */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PVOID  dhcpServerGetFromDB(INT  *piNum);
INT    dhcpServerPidGetFromDB(CPCHAR   cpcIfName, pid_t  *pPid);
INT    dhcpServerUpdatePidToDb(CPCHAR  cpcIfName, pid_t   pid);
INT    dhcpServerUpdateStatusToDb(CPCHAR  cpcIfName, INT  iStatus);
INT    dhcpServerAddItemToDb(__PDHCPSERVER_HANDLE  pDhcpServerInfo);
INT    dhcpServerDelItemFromDb (CPCHAR  cpcIfName);
INT    dhcpServerTableCreate (VOID);

VOID   dhcpServerHandleEntry(INT  iCommand, PVOID  pArgBuf);
PVOID  dhcpServerDoShow(INT  *piNum);
INT    dhcpServerDoConfig(__PDHCPSERVER_HANDLE  pDhcpServerInfo);
INT    dhcpServerDoDel(__PDHCPSERVER_HANDLE  pDhcpServerInfo);

INT    dhcpServerRecovery(VOID);

#endif                                                                  /* __DHCPSERVER_COMMON_H_       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
