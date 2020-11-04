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
** 文   件   名: operLog.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 8 月 28 日
**
** 描        述: OpenRT操作日志后台接口
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "goahead.h"
#include "log_info.h"
#include "webHandle.h"
/*********************************************************************************************************
  模块类型名称
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
** 函数名称: operLogInsert
** 功能描述: 操作日志记录接口
** 输　入  : wp                 服务器句柄
**           iModuleIndex       模块数组下标 @enum OPEN_RT_MODULE_INDEX
**           pLogContent        操作日志内容
**           iResult            操作结果 0表示成功，非0 表示失败
** 输　出  : NONE
** 返　回  : NONE
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
