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
** 文   件   名: vndHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: 虚拟网卡处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "vnd_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libVndCreateHandle
** 功能描述: 创建一个虚拟网卡
** 输　入  : pVndHandle         虚拟网卡信息
** 输　出  : NONE
** 返　回  : iVndID             创建的虚拟网卡 ID
*********************************************************************************************************/
INT  libVndCreate (__PVND_HANDLE  pVndHandle)
{
    if (LW_NULL == pVndHandle || LW_NULL == pVndHandle->ucHwAddr) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADD_VND_CMD, pVndHandle, sizeof(__VND_HANDLE)));
}

/*********************************************************************************************************
** 函数名称: libVndGetInfoHandle
** 功能描述: 获取 VND 信息
** 输　入  : NONE
** 输　出  : piNum              条目个数
** 返　回  : ERROR_CODE
*********************************************************************************************************/
PVOID  libVndGetInfo (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_GET_VND_CMD, NULL, 0, piNum));
}

/*********************************************************************************************************
** 函数名称: libKidVPNDel
** 功能描述: 删除 VPN
** 输　入  : pKidVPNHandle      VPN 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libVndDel (INT  iVndID)
{
    return  (requestConfigMsg(OPENRT_DEL_VND_CMD, &iVndID, sizeof(iVndID)));
}
/*********************************************************************************************************
** 函数名称: libGetVndidByNetIfIndex
** 功能描述: 根据 vndid 获取网卡索引
** 输　入  : iVndid             vndid
** 输　出  : piNetIfIndex       网卡索引
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libGetNetIfIndexByVndid (INT  iVndid, INT  *piNetIfIndex)
{
    INT       iNum                = 0;
    PCHAR     pcNetifIndexBuff = LW_NULL;

    if (LW_NULL == piNetIfIndex) {
        return  (PX_ERROR);
    }

    pcNetifIndexBuff = requestQueryMsg(OPENRT_GET_IFINDEX_CMD,
                                      (PVOID)&iVndid,
                                       sizeof(iVndid),
                                       &iNum);
    if (LW_NULL == pcNetifIndexBuff) {
        return  (PX_ERROR);
    }

    *piNetIfIndex = *(INT *)pcNetifIndexBuff;

    free(pcNetifIndexBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: libGetVndidByNetIfIndex
** 功能描述: 根据网卡索引获取 vndid
** 输　入  : iNetIfIndex       网卡索引
** 输　出  : piVndid           vndid
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libGetVndidByNetIfIndex (INT  iNetIfIndex, INT  *piVndid)
{
    INT       iNum                = 0;
    PCHAR     pcVndIdBuff = LW_NULL;

    if (LW_NULL == piVndid) {
        return  (PX_ERROR);
    }

    pcVndIdBuff = requestQueryMsg(OPENRT_GET_VNDID_CMD,
                                 (PVOID)&iNetIfIndex,
                                 sizeof(iNetIfIndex),
                                 &iNum);
    if (LW_NULL == pcVndIdBuff) {
        return  (PX_ERROR);
    }

    *piVndid = *(INT *)pcVndIdBuff;

    free(pcVndIdBuff);

    return  (ERROR_NONE);
}
