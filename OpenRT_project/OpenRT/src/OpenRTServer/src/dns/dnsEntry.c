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
** ��   ��   ��: dnsEntry.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 30 ��
**
** ��        ��: dns�������
*********************************************************************************************************/
#include <SylixOS.h>
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "dns_common.h"
/*********************************************************************************************************
** ��������: __dnsConfigDeal
** ��������: DNS ���ò���
** ��    ��: pDnsHandle  ǰ�˴���Ĳ������
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: __dnsInfoGetDeal
** ��������: ��ȡ DNS ����
** ��    ��: pDnsHandle  DNS ���
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: dnsHandleEntry
** ��������: DNS ������ں���
** ��    ��: iCommand  DNS ��������
**           pArgBuf   �����������
** ��    ��: NONE
** ��    ��: NONE
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
