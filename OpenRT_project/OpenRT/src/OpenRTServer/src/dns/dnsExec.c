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
** ��   ��   ��: dnsExec.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 31 ��
**
** ��        ��: shell ���� dns
*********************************************************************************************************/
#include <SylixOS.h>
#include "dns_common.h"
/*********************************************************************************************************
** ��������: dnsConfigDoSet
** ��������: ���� DNS
** ��    ��: pDnsHandle  DNS �������
** ��    ��: NONE
** ��    ��: ERROR_CODE
*********************************************************************************************************/
INT  dnsConfigDoSet (__PDNS_HANDLE  pDnsHandle)
{
    CHAR shellCmd[128] = {0};
    INT  iRet          = 0;

    if (LW_NULL == pDnsHandle) {
        return (PX_ERROR);
    }

    /*
     * ������ѡDNS
     */
    snprintf(shellCmd, sizeof(shellCmd), "ifconfig dns 0 %s", pDnsHandle->primaryDNS);

    iRet = system(shellCmd);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "dns configure failed.\n");

        return (PX_ERROR);
    }

    /*
     * ���ñ���DNS
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
** ��������: dnsConfigDoDeal
** ��������: ���� DNS
** ��    ��: pDnsHandle  DNS �������
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
** ��������: dnsRecovery
** ��������: DNS �ָ�
** ��    ��: NONE
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
