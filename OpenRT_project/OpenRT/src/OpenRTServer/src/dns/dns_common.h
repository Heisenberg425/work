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
** ��   ��   ��: dns_common.h
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 30 ��
**
** ��        ��: DNS��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __DNS_COMMON_H
#define __DNS_COMMON_H

#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "msgcomm.h"
#include "libdns_config.h"
#include "database/database_common.h"
/*********************************************************************************************************
  DNS ���ݱ���Ϣ
*********************************************************************************************************/
#define DNS_TABLE_NAME            "dns"                                 /*  dns����                   */
#define DNS_TABLE_HEAD            "(PRIMARYDNS  TEXT, SECONDDNS  TEXT)" /*  dns���ֶ�                   */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
VOID  dnsHandleEntry(INT iCommand, PVOID pArgBuf);
INT   dnsTableCreate (VOID);

INT   dnsInfoGetFromDb(__PDNS_HANDLE  pDnsHandle);
INT   dnsInfoAddItemToDb(__PDNS_HANDLE  pDnsHandle);

INT   dnsConfigDoSet(__PDNS_HANDLE pDnsHandle);
INT   dnsConfigDoDeal(__PDNS_HANDLE  pDnsHandle);
INT   dnsRecovery (VOID);

#endif                                                                  /*  __DNS_COMMON_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
