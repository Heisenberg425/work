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
** 文   件   名: dhcpHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 12 日
**
** 描        述: DHCP 处理
*********************************************************************************************************/
#include <stdlib.h>
#include "dhcp_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libDhcpClientHandle
** 功能描述: 使能 / 禁能网口 DHCP 客户端
** 输　入  : pcNetifName        网口名称
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libDhcpClientHandle (PCHAR  pcNetifName, CHAR  cHandle)
{
    __DHCP_CLIENT_SET     dhcpClientHandle;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    bzero(&dhcpClientHandle, sizeof(dhcpClientHandle));
    dhcpClientHandle.cHandle = cHandle;
    strncpy(dhcpClientHandle.cNetifName, pcNetifName, sizeof(dhcpClientHandle.cNetifName));

    return  (requestConfigMsg(OPENRT_DHCPC_HANDLE, &dhcpClientHandle, sizeof(dhcpClientHandle)));
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
