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
** 文   件   名: dnsHandle.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 30 日
**
** 描        述: DNS信息处理
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include "dns_info.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: dnsInfoFree
** 功能描述: 释放 DNS 信息空间
** 输　入  : pDnsHandle  DNS 句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dnsInfoFree (__PDNS_HANDLE  pDnsHandle)
{
    if (LW_NULL != pDnsHandle) {
    	free(pDnsHandle);
    }
}

/*********************************************************************************************************
** 函数名称: dnsShow
** 功能描述: 查询 DNS 信息
** 输　入  : piRecordTotal  DNS 记录查询结果总数
** 输　出  :
** 返　回  : dns查询结果指针
*********************************************************************************************************/
__PDNS_HANDLE  dnsShow (INT  *piRecordTotal)
{
	return  requestQueryMsg(OPENRT_GET_DNS_CMD, NULL, 0, piRecordTotal);
}

/*********************************************************************************************************
** 函数名称: dnsConfig
** 功能描述: 配置 DNS
** 输　入  : pDnsHandle  DNS 入参信息
** 输　出  : NONE
** 返　回  : 配置结果
*********************************************************************************************************/
INT  dnsConfig (__PDNS_HANDLE  pDnsHandle)
{
	return  requestConfigMsg(OPENRT_CFG_DNS_CMD, (PVOID)pDnsHandle, sizeof(__DNS_HANDLE));
}
