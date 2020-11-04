/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: libserver_type.h
**
** ��   ��   ��: Li.xiaocheng (��Т��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: OpenRT���ù��ߺ�ǰ�˷����׼�ඨ���ļ�
*********************************************************************************************************/
#ifndef __LIBTYPE_H
#define __LIBTYPE_H
/*********************************************************************************************************
  ��ȡ����TYPEֵ�ĺ�
*********************************************************************************************************/
#define GET_OPENRT_SERVER_TYPE(x)                (x & 0xffff0000)       /*  ��������                    */
/*********************************************************************************************************
  ������
*********************************************************************************************************/
#define RET_HANDLE_OK                            (0)                    /*  �����ɹ�                    */
#define RET_HANDLE_FAIL                          (-1)                   /*  ����ʧ��                    */
#define RET_ARG_ERROR                            (-2)                   /*  ��������                    */
/*********************************************************************************************************
  ��������ģ��Ļ���BASE
*********************************************************************************************************/
#define OPEN_RT_VPN_CONFIG_BASE                  (0 << 16)              /*  VPN ����                    */
#define OPEN_RT_NAT_CONFIG_BASE                  (1 << 16)              /*  NAT ����                    */
#define OPEN_RT_ROUTE_CONFIG_BASE                (2 << 16)              /*  ��̬·�ɹ���ģ��            */
#define OPEN_RT_UDAIL_CONFIG_BASE                (3 << 16)              /*  UDAIL ����                  */
#define OPEN_RT_INTADA_CONFIG_BASE               (4 << 16)              /*  ��������Ӧ��λ����          */
#define OPEN_RT_INTERFACE_CONFIG_BASE            (5 << 16)              /*  ������������                */
#define OPEN_RT_VND_CONFIG_BASE                  (6 << 16)              /*  ����������������            */
#define OPEN_RT_VLAN_CONFIG_BASE                 (7 << 16)              /*  vlan ����ģ��               */
#define OPEN_RT_FLOWCTL_CONFIG_BASE              (8 << 16)              /*  ���ع���ģ��                */
#define OPEN_RT_DHCP_CONFIG_BASE                 (9 << 16)              /*  DHCP                        */
#define OPEN_RT_LOG_CONFIG_BASE                  (10 << 16)             /*  LOG                         */
#define OPEN_RT_MONITOR_CONFIG_BASE              (11 << 16)             /*  ��ع���ģ��                */
#define OPEN_RT_ARP_CONFIG_BASE                  (12 << 16)             /*  arp ����ģ��                */
#define OPEN_RT_ARP_DEFENSE_BASE                 (13 << 16)             /*  arp ����                    */
#define OPEN_RT_LOGIN_BASE                       (14 << 16)             /*  ��¼����ģ��                */
#define OPEN_RT_POWER_CONFIG_BASE                (15 << 16)             /*  �ػ�/��������ģ��           */
#define OPEN_RT_TIMESET_CONFIG_BASE              (16 << 16)             /*  ϵͳʱ������ģ��            */
#define OPEN_RT_DEVNAME_CONFIG_BASE              (17 << 16)             /*  �豸��������ģ��            */
#define OPEN_RT_PING_TEST_CONFIG_BASE            (18 << 16)             /*  ϵͳ PING ����ģ��          */
#define OPEN_RT_HWINFO_CONFIG_BASE               (19 << 16)             /*  ��ȡϵͳӲ����Ϣ            */
#define OPEN_RT_DHCPSER_CONF_BASE                (20 << 16)             /*  DHCP Server ����ģ��        */
#define OPEN_RT_NPF_CONFIG_BASE                  (21 << 16)             /*  npf ����                    */
#define OPEN_RT_DNS_CONFIG_BASE                  (22 << 16)             /*  DNS ����                    */
#define OPEN_RT_UPDATEFIRMWARE_CONFIG_BASE       (23 << 16)             /*  update firmware ����ģ��    */
#define OPEN_RT_VERSION_CONFIG_BASE              (24 << 16)             /*  ϵͳ�汾����                */
#define OPEN_RT_QOS_CONFIG_BASE                  (25 << 16)             /*  QoS ����                    */
#define OPEN_RT_SIT_CONFIG_BASE                  (27 << 16)             /*  �龰ģʽ����ģ��            */

#endif                                                                  /*  __LIBTYPE_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
