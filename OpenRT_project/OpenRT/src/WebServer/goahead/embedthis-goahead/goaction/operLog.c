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
** ��   ��   ��: operLog.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 8 �� 28 ��
**
** ��        ��: OpenRT������־��̨�ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "goahead.h"
#include "log_info.h"
#include "webHandle.h"
/*********************************************************************************************************
  ģ����������
*********************************************************************************************************/
const char *__G_pOpenRTModuleName[OPEN_RT_MAX_INDEX] = {
    "MONITOR",
    "UDIAL",
    "INTERFACE",
    "DHCPSERVER",
    "DNS",
    "ROUTE",
    "VLAN",
    "VPN",
    "VND",
    "NAT",
    "QoS",
    "DEVNAME",
    "TIMESET",
    "ACCOUNT",
    "AUTH",
    "UPDATEFIRMWARE",
    "HWINFO",
    "RESET",
    "INTADA",
    "FLOWCTL",
    "BEHAVIOUR",
    "ARP",
    "ARP_DEFENSE",
    "NPF",
    "PING_TEST",
    "SPEEDTEST",
    "UDIAL_LOG",
    "OPERA_LOG",
    "VPN_LOG",
    "ASDEFENSE_LOG",
    "DHCPSERVER_LOG",
    "QUAGGA"
};
/*********************************************************************************************************
** ��������: operLogInsert
** ��������: ������־��¼�ӿ�
** �䡡��  : wp                 ���������
**           iModuleIndex       ģ�������±� @enum OPEN_RT_MODULE_INDEX
**           pLogContent        ������־����
**           iResult            ������� 0��ʾ�ɹ�����0 ��ʾʧ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
void operLogInsert (Webs  *wp, INT  iModuleIndex, char  *pLogContent, INT  iResult)
{
    __OPER_LOG  operLog    = {0};
    PCHAR       pcUserName = NULL;

    if (wp == NULL || pLogContent == NULL) {
        return;
    }

    if (iModuleIndex >= OPEN_RT_MAX_INDEX) {
        return;
    }

    pcUserName = (PCHAR)websGetSessionVar(wp, WEBS_SESSION_USER, 0);
    if (NULL != pcUserName) {
        strncpy(operLog.cOperator, pcUserName, sizeof(operLog.cOperator));
    }

    strncpy(operLog.cClientIP, websGetRequestIpAddr(wp) , sizeof(operLog.cClientIP));
    strncpy(operLog.cModuleName, __G_pOpenRTModuleName[iModuleIndex], sizeof(operLog.cModuleName));
    strncpy(operLog.cLog, pLogContent, sizeof(operLog.cLog));
    operLog.iResult = iResult;

    writeOperLog(&operLog);
}
