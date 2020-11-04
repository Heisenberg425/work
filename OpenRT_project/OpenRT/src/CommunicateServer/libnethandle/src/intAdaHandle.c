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
** 文   件   名: intAdaHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 网口自适应定位处理
*********************************************************************************************************/
#include "stdlib.h"
#include "intada_info.h"
#include "libcomm.h"
#include "string.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libIntAdaCheck
** 功能描述: 检测是否完成定位操作
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 状态
*********************************************************************************************************/
INT  libIntAdaCheck (VOID)
{
    return  (requestConfigMsg(OPENRT_ADA_GETS_CMD, LW_NULL, 0));
}
/*********************************************************************************************************
** 函数名称: libIntAdaGetNetif
** 功能描述: 获取 Web 网口名对应的系统网卡名称
** 输　入  : cpcWebNetifName    前端网卡名称
**           iNetifLen          网卡名称 buff 长度
** 输　出  : pcNetifName        系统网卡名称
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName, INT  iNetifLen)
{
    INT       iNum            = 0;
    PCHAR     pcNetifNameBuff = LW_NULL;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(cpcWebNetifName, "lo0")) {
        memcpy(pcNetifName, "lo0", strlen("lo0"));

        return  (ERROR_NONE);
    }

    pcNetifNameBuff = requestQueryMsg(OPENRT_NETIF_GET_CMD,
                                     (PVOID)cpcWebNetifName,
                                     strlen(cpcWebNetifName)+1,
                                     &iNum);
    if (LW_NULL == pcNetifNameBuff || 0 == iNum) {
        return  (PX_ERROR);
    }

    strncpy(pcNetifName, pcNetifNameBuff, iNetifLen);
    free(pcNetifNameBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: libIntAdaGetWebNetif
** 功能描述: 获取 netif 名称获取对应的前端网卡名称
** 输　入  : pcWebNetifName    前端网卡名
**           iNetifLen         网卡名称 buff 长度
** 输　出  : cpcNetifName      网卡名称
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName, INT  iNetifLen)
{
    INT       iNum            = 0;
    PCHAR     pcWebNetifNameBuff = LW_NULL;

    if (LW_NULL == pcWebNetifName) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(cpcNetifName, "lo0")) {
        memcpy(pcWebNetifName, "lo0", strlen("lo0"));

        return  (ERROR_NONE);
    }

    pcWebNetifNameBuff = requestQueryMsg(OPENRT_WEBNETIF_GET_CMD,
                                        (PVOID)cpcNetifName,
                                        strlen(cpcNetifName)+1,
                                        &iNum);
    if (LW_NULL == pcWebNetifNameBuff || 0 == iNum) {
        return  (PX_ERROR);
    }

    strncpy(pcWebNetifName, pcWebNetifNameBuff, iNetifLen);
    free(pcWebNetifNameBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: libIntAdaSet
** 功能描述: 网口位置自定义设置
** 输　入  : pIntAdaSetInfo     定位信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaSet (__PINT_ADA_SET  pIntAdaSetInfo)
{
    if (LW_NULL == pIntAdaSetInfo ||
        (INT_ADA_FLAG_FIRST  != pIntAdaSetInfo->cFlag &&
         INT_ADA_FLAG_LAST   != pIntAdaSetInfo->cFlag &&
         INT_ADA_FLAG_MIDDLE != pIntAdaSetInfo->cFlag)) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADA_SET_CMD, pIntAdaSetInfo, sizeof(__INT_ADA_SET)));
}
/*********************************************************************************************************
** 函数名称: libIntAdaClear
** 功能描述: 网口删除操作
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaClear (VOID)
{
    requestConfigMsg(OPENRT_ADA_CLEAR_CMD, NULL, 0);
    return  (ERROR_NONE);
}
