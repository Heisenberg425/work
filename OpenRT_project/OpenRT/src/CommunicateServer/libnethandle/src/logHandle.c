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
** 文   件   名: udialHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 拨号处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "liblog_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: libUdialLogNumGet
** 功能描述:  获取网卡拨号日志个数
** 输　入  : pUdialLogInfo      拨号日志信息
** 输　出  : NONE
** 返　回  : iNum               条目个数
*********************************************************************************************************/
INT32  libUdialLogNumGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle)
{
    return  (requestConfigMsg(OPENRT_UDIAL_LOG_NUM, (PVOID)pUdialLogHandle, sizeof(__UDIAL_LOG_HANDLE)));
}
/*********************************************************************************************************
** 函数名称: libUdialLogInfoGet
** 功能描述: 获取网卡拨号日志信息 (需要调用 free 释放资源)
** 输　入  : pUdialLogInfo          拨号日志信息
** 输　出  : piNum                  个数
** 返　回  : NONE
*********************************************************************************************************/
PVOID  libUdialLogInfoGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle, INT  *piNum)
{
    if (LW_NULL == pUdialLogHandle) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_UDIAL_LOG_GET,
                             pUdialLogHandle,
                             sizeof(__UDIAL_LOG_HANDLE),
                             piNum));
}

/*********************************************************************************************************
** 函数名称: libVpnLogNumGet
** 功能描述:  获取 VPN 日志个数
** 输　入  : pVpnLogHandle      VPN 日志获取结构
** 输　出  : NONE
** 返　回  : iNum               条目个数
*********************************************************************************************************/
INT32  libVpnLogNumGet (__PVPN_LOG_HANDLE  pVpnLogHandle)
{
    return  (requestConfigMsg(OPENRT_VPN_LOG_NUM, (PVOID)pVpnLogHandle, sizeof(__VPN_LOG_HANDLE)));
}
/*********************************************************************************************************
** 函数名称: libVpnLogInfoGet
** 功能描述: 获取 VPN 日志信息 (需要调用 free 释放资源)
** 输　入  : pVpnLogHandle      VPN 日志获取结构
** 输　出  : piNum              个数
** 返　回  : 日志信息
*********************************************************************************************************/
PVOID  libVpnLogInfoGet (__PVPN_LOG_HANDLE  pVpnLogHandle, INT  *piNum)
{
    if (LW_NULL == pVpnLogHandle) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_VPN_LOG_GET,
                             pVpnLogHandle,
                             sizeof(__VPN_LOG_HANDLE),
                             piNum));
}


/*********************************************************************************************************
** 函数名称: writeOperLog
** 功能描述: 写操作日志
** 输　入  : pOperLog               操作日志信息
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
INT32  writeOperLog (__POPER_LOG  pOperLog)
{
    return  (requestConfigMsg(OPENRT_OPER_LOG_PUT, (PVOID)pOperLog, sizeof(__OPER_LOG)));
}
/*********************************************************************************************************
** 函数名称: readOperLog
** 功能描述: 读操作日志
** 输　入  : pOperLog               操作日志信息
** 输　出  : piNum                  个数
** 返　回  : NONE
*********************************************************************************************************/
PVOID  readOperLog (__POPER_LOG  pOperLog, INT  *piNum)
{
    if (LW_NULL == pOperLog) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_OPER_LOG_GET,
    		                 pOperLog,
                             sizeof(__OPER_LOG),
                             piNum));
}
/*********************************************************************************************************
** 函数名称: libUdialLogInfoDel
** 功能描述: 清除拨号日志信息
** 输　入  : pUdialLogInfo    拨号日志信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libUdialLogInfoDel (__PUDIAL_LOG_HANDLE  pUdialLogHandle)
{
    if (LW_NULL == pUdialLogHandle) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_UDIAL_LOG_DEL, pUdialLogHandle, sizeof(__UDIAL_LOG_HANDLE));
}
/*********************************************************************************************************
** 函数名称: libOperLogInfoDel
** 功能描述: 清除操作日志信息
** 输　入  : pOperLog          操作日志信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libOperLogInfoDel (__POPER_LOG  pOperLog)
{
    if (LW_NULL == pOperLog) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_OPER_LOG_DEL, pOperLog, sizeof(__OPER_LOG));
}
/*********************************************************************************************************
** 函数名称: libVpnLogInfoDel
** 功能描述: 清除 VPN 日志信息
** 输　入  : pVpnLogHandle      VPN 日志信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libVpnLogInfoDel (__PVPN_LOG_HANDLE  pVpnLogHandle)
{
    if (LW_NULL == pVpnLogHandle) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_VPN_LOG_DEL, pVpnLogHandle, sizeof(__VPN_LOG_HANDLE));
}
