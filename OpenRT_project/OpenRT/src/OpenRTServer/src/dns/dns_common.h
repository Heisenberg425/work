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
** 文   件   名: dns_common.h
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 30 日
**
** 描        述: DNS信息头文件
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
  DNS 数据表信息
*********************************************************************************************************/
#define DNS_TABLE_NAME            "dns"                                 /*  dns表单名                   */
#define DNS_TABLE_HEAD            "(PRIMARYDNS  TEXT, SECONDDNS  TEXT)" /*  dns表字段                   */
/*********************************************************************************************************
  函数声明
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
