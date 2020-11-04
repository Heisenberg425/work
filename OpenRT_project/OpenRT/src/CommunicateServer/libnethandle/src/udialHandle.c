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
#include "libudial_config.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libUdialStartHandle
** 功能描述: 启动拨号
** 输　入  : pcAccount          拨号账号
**           pcPassWd           拨号密码
**           pcNetifName        拨号系统网口名称
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  libUdialStartHandle (CHAR   *pcAccount, CHAR   *pcPassWd, CHAR  *pcNetifName)
{
    __UDIAL_HANDLE        udialHandle;

    if (LW_NULL == pcNetifName || LW_NULL == pcAccount || LW_NULL == pcPassWd) {

        return  (PX_ERROR);
    }

    /*
     *  将消息发过去
     */
    bzero(&udialHandle, sizeof(udialHandle));
    strncpy(udialHandle.cNetifname, pcNetifName, sizeof(udialHandle.cNetifname));
    strncpy(udialHandle.cUsrname,   pcAccount, sizeof(udialHandle.cUsrname));
    strncpy(udialHandle.cPassword,  pcPassWd, sizeof(udialHandle.cPassword));

    return  (requestConfigMsg(OPENRT_UDIAL_START, (PVOID)&udialHandle, sizeof(udialHandle)));
}

/*********************************************************************************************************
** 函数名称: libUdialDelHandle
** 功能描述: 删除拨号
** 输　入  : pcNetifName        拨号系统网口名称
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  libUdialDelHandle (PCHAR  pcNetifName)
{
    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_UDIAL_DEL, (PVOID)pcNetifName, NETIF_NAMESIZE));
}

/*********************************************************************************************************
** 函数名称: libUdialInfoGet
** 功能描述: 获取网卡拨号信息
** 输　入  : pUdialInfo            存储拨号信息的结构(需要指定netifname)
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  libUdialInfoGet (__PUDIAL_HANDLE  pUdialInfo)
{
    INT              iNum = 0;
    __PUDIAL_HANDLE  pUdialHandle = LW_NULL;

    if (LW_NULL == pUdialInfo || LW_NULL == pUdialInfo->cNetifname) {

        return  (PX_ERROR);
    }

    pUdialHandle = requestQueryMsg(OPENRT_UDIAL_INFO,
                                   pUdialInfo->cNetifname,
                                   sizeof(pUdialInfo->cNetifname),
                                   &iNum);
    if (LW_NULL == pUdialHandle) {

        return  (PX_ERROR);
    }

    memcpy(pUdialInfo, pUdialHandle, sizeof(__UDIAL_HANDLE));

    free(pUdialHandle);

    return  (ERROR_NONE);
}
