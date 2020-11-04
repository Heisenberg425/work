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
** 文   件   名: dnsEntry.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 30 日
**
** 描        述: dns操作入口
*********************************************************************************************************/
#include <SylixOS.h>
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "dns_common.h"
/*********************************************************************************************************
** 函数名称: __dnsConfigDeal
** 函数功能: DNS 配置操作
** 输    入: pDnsHandle  前端传入的参数句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID __dnsConfigDeal (__PDNS_HANDLE  pDnsHandle)
{
    if (LW_NULL == pDnsHandle) {
        replyConfigMsg(OPENRT_CFG_DNS_CMD, PX_ERROR);

        return;
    }

    replyConfigMsg(OPENRT_CFG_DNS_CMD, dnsConfigDoDeal(pDnsHandle));
}
/*********************************************************************************************************
** 函数名称: __dnsInfoGetDeal
** 函数功能: 获取 DNS 配置
** 输    入: pDnsHandle  DNS 句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID __dnsInfoGetDeal (__PDNS_HANDLE  pDnsHandle)
{
    __DNS_HANDLE  dnsHandle;

    if (LW_NULL == pDnsHandle) {
        replyQueryMsg(OPENRT_GET_DNS_CMD, sizeof(__DNS_HANDLE), 0, NULL);
    }

    bzero(&dnsHandle, sizeof(dnsHandle));

    strncpy(dnsHandle.primaryDNS,
           inet_ntoa(*(struct in_addr *)&dns_getserver(0)->u_addr.ip4),
           sizeof(dnsHandle.primaryDNS));
    strncpy(dnsHandle.secondDNS,
           inet_ntoa(*(struct in_addr *)&dns_getserver(1)->u_addr.ip4),
           sizeof(dnsHandle.secondDNS));

    replyQueryMsg(OPENRT_GET_DNS_CMD, sizeof(__DNS_HANDLE), 1, &dnsHandle);
}
/*********************************************************************************************************
** 函数名称: dnsHandleEntry
** 函数功能: DNS 处理入口函数
** 输    入: iCommand  DNS 操作命令
**           pArgBuf   传入参数数据
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
VOID dnsHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_DNS_CMD:
        __dnsConfigDeal((__PDNS_HANDLE)pArgBuf);
        break;

    case OPENRT_GET_DNS_CMD:
        __dnsInfoGetDeal((__PDNS_HANDLE)pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
