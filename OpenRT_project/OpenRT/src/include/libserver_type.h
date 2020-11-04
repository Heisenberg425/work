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
** 文   件   名: libserver_type.h
**
** 创   建   人: Li.xiaocheng (李孝成)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: OpenRT配置工具和前端服务基准类定义文件
*********************************************************************************************************/
#ifndef __LIBTYPE_H
#define __LIBTYPE_H
/*********************************************************************************************************
  获取服务TYPE值的宏
*********************************************************************************************************/
#define GET_OPENRT_SERVER_TYPE(x)                (x & 0xffff0000)       /*  服务类型                    */
/*********************************************************************************************************
  错误码
*********************************************************************************************************/
#define RET_HANDLE_OK                            (0)                    /*  操作成功                    */
#define RET_HANDLE_FAIL                          (-1)                   /*  操作失败                    */
#define RET_ARG_ERROR                            (-2)                   /*  参数出错                    */
/*********************************************************************************************************
  定义配置模块的基础BASE
*********************************************************************************************************/
#define OPEN_RT_VPN_CONFIG_BASE                  (0 << 16)              /*  VPN 类型                    */
#define OPEN_RT_NAT_CONFIG_BASE                  (1 << 16)              /*  NAT 类型                    */
#define OPEN_RT_ROUTE_CONFIG_BASE                (2 << 16)              /*  静态路由管理模块            */
#define OPEN_RT_UDAIL_CONFIG_BASE                (3 << 16)              /*  UDAIL 类型                  */
#define OPEN_RT_INTADA_CONFIG_BASE               (4 << 16)              /*  网口自适应定位类型          */
#define OPEN_RT_INTERFACE_CONFIG_BASE            (5 << 16)              /*  网口配置类型                */
#define OPEN_RT_VND_CONFIG_BASE                  (6 << 16)              /*  虚拟网卡配置类型            */
#define OPEN_RT_VLAN_CONFIG_BASE                 (7 << 16)              /*  vlan 管理模块               */
#define OPEN_RT_FLOWCTL_CONFIG_BASE              (8 << 16)              /*  流控管理模块                */
#define OPEN_RT_DHCP_CONFIG_BASE                 (9 << 16)              /*  DHCP                        */
#define OPEN_RT_LOG_CONFIG_BASE                  (10 << 16)             /*  LOG                         */
#define OPEN_RT_MONITOR_CONFIG_BASE              (11 << 16)             /*  监控管理模块                */
#define OPEN_RT_ARP_CONFIG_BASE                  (12 << 16)             /*  arp 管理模块                */
#define OPEN_RT_ARP_DEFENSE_BASE                 (13 << 16)             /*  arp 防御                    */
#define OPEN_RT_LOGIN_BASE                       (14 << 16)             /*  登录管理模块                */
#define OPEN_RT_POWER_CONFIG_BASE                (15 << 16)             /*  关机/重启管理模块           */
#define OPEN_RT_TIMESET_CONFIG_BASE              (16 << 16)             /*  系统时间设置模块            */
#define OPEN_RT_DEVNAME_CONFIG_BASE              (17 << 16)             /*  设备名称设置模块            */
#define OPEN_RT_PING_TEST_CONFIG_BASE            (18 << 16)             /*  系统 PING 测试模块          */
#define OPEN_RT_HWINFO_CONFIG_BASE               (19 << 16)             /*  获取系统硬件信息            */
#define OPEN_RT_DHCPSER_CONF_BASE                (20 << 16)             /*  DHCP Server 管理模块        */
#define OPEN_RT_NPF_CONFIG_BASE                  (21 << 16)             /*  npf 管理                    */
#define OPEN_RT_DNS_CONFIG_BASE                  (22 << 16)             /*  DNS 管理                    */
#define OPEN_RT_UPDATEFIRMWARE_CONFIG_BASE       (23 << 16)             /*  update firmware 管理模块    */
#define OPEN_RT_VERSION_CONFIG_BASE              (24 << 16)             /*  系统版本管理                */
#define OPEN_RT_QOS_CONFIG_BASE                  (25 << 16)             /*  QoS 管理                    */
#define OPEN_RT_SIT_CONFIG_BASE                  (27 << 16)             /*  情景模式管理模块            */

#endif                                                                  /*  __LIBTYPE_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
