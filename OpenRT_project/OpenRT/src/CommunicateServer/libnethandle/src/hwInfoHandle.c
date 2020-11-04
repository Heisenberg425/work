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
** 文   件   名: hwInfoHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 2 日
**
** 描        述: 硬件信息获取处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "libcomm.h"
#include "libhw_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: libHardWareInfoGet
** 功能描述: 获取硬件信息
** 输　入  : pHwInfo            硬件信息指针
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  libHardWareInfoGet (__PHW_INFO  pHwInfo)
{
    INT32    iNum;
    PVOID    pvBuff = LW_NULL;

    if (LW_NULL == pHwInfo) {
        return  (PX_ERROR);
    }

    pvBuff = requestQueryMsg(OPENRT_HW_INFO_GET_CMD, NULL, 0, &iNum);

    if (pvBuff) {
        memcpy(pHwInfo, pvBuff, sizeof(__HW_INFO));
        free(pvBuff);
    }

    return  (ERROR_NONE);
}
