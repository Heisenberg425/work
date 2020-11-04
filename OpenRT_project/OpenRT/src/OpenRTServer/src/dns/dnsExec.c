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
** 文   件   名: dnsExec.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 31 日
**
** 描        述: shell 操作 dns
*********************************************************************************************************/
#include <SylixOS.h>
#include "dns_common.h"
/*********************************************************************************************************
** 函数名称: dnsConfigDoSet
** 函数功能: 设置 DNS
** 输    入: pDnsHandle  DNS 参数句柄
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsConfigDoSet (__PDNS_HANDLE  pDnsHandle)
{
    CHAR shellCmd[128] = {0};
    INT  iRet          = 0;

    if (LW_NULL == pDnsHandle) {
        return (PX_ERROR);
    }

    /*
     * 设置首选DNS
     */
    snprintf(shellCmd, sizeof(shellCmd), "ifconfig dns 0 %s", pDnsHandle->primaryDNS);

    iRet = system(shellCmd);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dns configure failed.\n");

        return (PX_ERROR);
    }

    /*
     * 设置备份DNS
     */
    snprintf(shellCmd, sizeof(shellCmd), "ifconfig dns 1 %s", pDnsHandle->secondDNS);

    iRet = system(shellCmd);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dns configure failed.\n");

        return (PX_ERROR);
    }

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dnsConfigDoDeal
** 函数功能: 配置 DNS
** 输    入: pDnsHandle  DNS 参数句柄
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsConfigDoDeal (__PDNS_HANDLE  pDnsHandle)
{
    INT  iRet          = 0;

    if (LW_NULL == pDnsHandle) {
        return (PX_ERROR);
    }

    iRet = dnsConfigDoSet(pDnsHandle);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dns configure set failed.\n");

        return  (PX_ERROR);
    }

    iRet = dnsInfoAddItemToDb(pDnsHandle);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dns save to database failed.\n");

        return  (PX_ERROR);
    }

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dnsRecovery
** 函数功能: DNS 恢复
** 输    入: NONE
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsRecovery (VOID)
{
    __DNS_HANDLE  dnsHandle;

    bzero(&dnsHandle, sizeof(__DNS_HANDLE));

    INIT_CHECK((dnsInfoGetFromDb(&dnsHandle)));

    return  dnsConfigDoSet(&dnsHandle);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
