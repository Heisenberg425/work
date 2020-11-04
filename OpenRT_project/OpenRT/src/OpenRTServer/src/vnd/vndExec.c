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
** 文   件   名: vndExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: 虚拟网卡操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <spawn.h>
#include "lwip/netif.h"
#include "common.h"
#include "vndExec.h"
#include "vndDB.h"
#include "../route/route_common.h"
#include "udial/udial_common.h"

/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static INT  _G_iVndID = VND_DEFAULT_ID;
/*********************************************************************************************************
** 函数名称: vndGetNextId
** 功能描述: 获取创建下一个虚拟网卡的 ID
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : iVndID             虚拟网卡 ID
*********************************************************************************************************/
INT  vndGetNextId (VOID)
{
    return  (_G_iVndID);
}
/*********************************************************************************************************
** 函数名称: vndSetNextId
** 功能描述: 设置创建下一个虚拟网卡的 ID
** 输　入  : iVndID             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  vndSetNextId (INT  iVndID)
{
    _G_iVndID = (iVndID > _G_iVndID) ? iVndID : _G_iVndID;
}
/*********************************************************************************************************
** 函数名称: vndStart
** 功能描述: 创建一个虚拟网卡
** 输　入  : pVndInfo           虚拟网卡信息
** 输　出  : piVndID            虚拟网卡 ID
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  vndStart (INT  *piVndID, __PVND_HANDLE  pVndInfo)
{
    CHAR                  cCmdBuf[50] = {0};
    INT                   iNetifIndex;
    INT                   iNetifMaxNum;
    INT                   iRet;

    if (LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    /*
     *  更新数据库信息
     */
    iNetifMaxNum = netIfMaxIndexGet();
    iNetifIndex  = netif_get_max_index();
    if (iNetifMaxNum > iNetifIndex) {
        iNetifIndex = iNetifMaxNum;
    } else {
        udialPppNumUpdate();
    }

    if (0 == pVndInfo->ucHwAddr[0] &&
        0 == pVndInfo->ucHwAddr[1] &&
        0 == pVndInfo->ucHwAddr[2] &&
        0 == pVndInfo->ucHwAddr[3] &&
        0 == pVndInfo->ucHwAddr[4] &&
        0 == pVndInfo->ucHwAddr[5]) {
        sprintf(cCmdBuf, "vnd add %d", _G_iVndID);
    } else {
        sprintf(cCmdBuf, "vnd add %d %02x:%02x:%02x:%02x:%02x:%02x",
                _G_iVndID,
                pVndInfo->ucHwAddr[0],
                pVndInfo->ucHwAddr[1],
                pVndInfo->ucHwAddr[2],
                pVndInfo->ucHwAddr[3],
                pVndInfo->ucHwAddr[4],
                pVndInfo->ucHwAddr[5]);
    }

    iRet = system(cCmdBuf);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    *piVndID              = _G_iVndID;
    pVndInfo->iVndId      = _G_iVndID;
    pVndInfo->iNetIfIndex = iNetifIndex;

    iRet = vndAddItemToDb(pVndInfo);                                    /*  更新数据库                  */
    if (PX_ERROR == iRet) {
        bzero(cCmdBuf, sizeof(cCmdBuf));

        sprintf(cCmdBuf, "vnd del %d", *piVndID);

        iRet = system(cCmdBuf);
        if (PX_ERROR == iRet) {
            return  (PX_ERROR);
        }
    }

    _G_iVndID++;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: vndDel
** 功能描述: 删除一个虚拟网卡
** 输　入  : iVndId             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  vndDel (INT   iVndID)
{
    CHAR                  cCmdBuf[50]             = {0};
    CHAR                  cIfName[NETIF_NAMESIZE] = {0};
    INT                   iNetIfIndex;
    INT                   iRet;

    udialPppNumUpdate();                                                /*  更新一次当前最大的netif num */

    sprintf(cCmdBuf, "vnd del %d", iVndID);

    /*
     *  因为创建 vnd 时会更新 ifparam.ini ,所以这里并没有删除 ini 里的信息
     */
    iRet = system(cCmdBuf);
    if (ERROR_NONE == iRet) {
        iRet = netIfIndexGetByVndId(iVndID, &iNetIfIndex);
        if (ERROR_NONE == iRet) {
            sprintf(cIfName, "vn%d", iNetIfIndex);

            iRet = staticRouteCleanFromDbByIfName(cIfName);
            if (PX_ERROR == iRet) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete static router item from db error\n");

                return  (PX_ERROR);
            }

            iRet = vndDelDBItem(iVndID);
            if (PX_ERROR == iRet) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete vnd item from db error\n");

                return  (PX_ERROR);
            }
        }
    }

    return  (iRet);
}
