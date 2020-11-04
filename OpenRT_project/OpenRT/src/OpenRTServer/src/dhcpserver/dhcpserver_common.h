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
** ��   ��   ��: dhcpserver_common.h
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 18 ��
**
** ��        ��: DHCP Server��Ϣͷ�ļ�
**2018.09.20    DHCP SERVER ֧���ڶ������豸���ж��������
*********************************************************************************************************/
#ifndef __DHCPSERVER_COMMON_H
#define __DHCPSERVER_COMMON_H

#include "libdhcpserver_config.h"
/*********************************************************************************************************
  DHCP ��غ궨��
*********************************************************************************************************/
#define DHCPSERVER_INI_FILE        "/etc/dhcpdconfig.ini"               /* DHCP Server �����ļ�         */
#define DHCPSERVER_SHOW            "dhcpserver"                         /* ��ѯDHCP Server��Ϣ          */
#define DHCPSERVER_SET             "set"                                /* ���ӻ��޸�DHCP Server��Ϣ    */
/*********************************************************************************************************
  DHCP Server ������
*********************************************************************************************************/
#define DHCPSERVER_CMD_START       "ipaddrpool:start"                   /* ��ַ����ʼ��ַ               */
#define DHCPSERVER_CMD_END         "ipaddrpool:end"                     /* ��ַ�ؽ�����ַ               */
#define DHCPSERVER_CMD_LEASEFILE   "filepath:leasefile"                 /* ��Լ�ļ�����λ��             */
#define DHCPSERVER_CMD_LOGFILE     "filepath:logfile"                   /* ��־�ļ�����λ��             */
#define DHCPSERVER_CMD_INTERFACE   "network:interface"                  /* ����ʹ����������             */
#define DHCPSERVER_CMD_DNS1        "opt:dns1"                           /* ��dns                        */
#define DHCPSERVER_CMD_DNS2        "opt:dns2"                           /* ����dns                      */
#define DHCPSERVER_CMD_MASK        "opt:subnet"                         /* ����                         */
#define DHCPSERVER_CMD_GATEWAY     "opt:router"                         /* ����                         */
#define DHCPSERVER_CMD_LEASE       "opt:lease"                          /* ��Լʱ�䣨��λ100�죩        */
#define DHCPSERVER_CMD_T1          "opt:t1"                             /* ��Լʱ��50%                  */
#define DHCPSERVER_CMD_T2          "opt:t2"                             /* ��Լʱ��87.5%                */
/*********************************************************************************************************
  DHCP Server Ĭ������
*********************************************************************************************************/
#define DHCPSERVER_DEF_ENABLE      (0)                                  /* dhcp serverĬ�Ϲر�          */
#define DHCPSERVER_DEF_START       "192.168.1.100"                      /* ��ַ����ʼ��ַĬ��ֵ         */
#define DHCPSERVER_DEF_END         "192.168.1.200"                      /* ��ַ�ؽ�����ַĬ��ֵ         */
#define DHCPSERVER_DEF_PATH        "/var/dhcplease"                     /* DHCP Ĭ��·��                */
#define DHCPSERVER_DEF_LEASEFILE   "/var/dhcplease/dhcpd.leases"        /* ��Լ�ļ�����λ��Ĭ��ֵ       */
#define DHCPSERVER_DEF_LOGFILE     "/var/dhcplease/dhcpdlog"            /* ��־�ļ�����λ��Ĭ��ֵ       */
#define DHCPSERVER_DEF_INTERFACE   "en1"                                /* ����ʹ����������Ĭ��ֵ       */
#define DHCPSERVER_DEF_DNS1        "8.8.8.8"                            /* ��dnsĬ��ֵ                  */
#define DHCPSERVER_DEF_DNS2        "8.8.8.8"                            /* ����dnsĬ��ֵ                */
#define DHCPSERVER_DEF_MASK        "255.255.255.0"                      /* ����Ĭ��ֵ                   */
#define DHCPSERVER_DEF_GATEWAY     "192.168.1.1"                        /* ����Ĭ��ֵ                   */
#define DHCPSERVER_DEF_LEASE       (86400)                              /* ��Լʱ�䣨��λ100�죩Ĭ��ֵ  */

#define DHCPSERVER_NAME            "dhcpserver "                        /* DHCP Server ��̨��������     */
#define DHCPSERVER_PATH            "/apps/dhcpserver/dhcpserver"        /* DHCP Server ·��             */

#define DHCPSERVER_ENABLE          (1)                                  /* dhcp server ʹ��             */
#define DHCPSERVER_DISABLE         (0)                                  /* dhcp server ��ֹ             */
#define INVALID_PID                (-1)                                 /* ��Ч PID                     */
/*********************************************************************************************************
  ��������
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
