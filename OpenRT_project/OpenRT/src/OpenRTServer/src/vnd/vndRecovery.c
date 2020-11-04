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
** 文   件   名: vndRecovery.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 27 日
**
** 描        述: 虚拟网卡恢复
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "interface/interface_common.h"
#include "vnd_common.h"
#include "vndDB.h"
#include "vndExec.h"
#include "libvnd_config.h"
#include "database/database_common.h"
#include "udial/udial_common.h"
/*********************************************************************************************************
** 函数名称: iVndID
** 功能描述: 恢复重启前的虚拟网卡
** 输  入  : pDb               数据库句柄
**           iVndID            虚拟网卡 ID
** 输  出  : NONE
** 返  回  : 是否恢复成功
*********************************************************************************************************/
INT  vndRecovery (INT  iVndID)
{
    INT                   iRet;
    INT                   iNetifIndex;
    INT                   iNetifMaxNum;
    __VND_HANDLE          vndHandleInfo;
    __IFPARAM_INFO        ifparamInfo;
    CHAR                  cCmdBuf[50]   = {0};
    sqlite3              *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

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

    bzero(&vndHandleInfo, sizeof(vndHandleInfo));
    iRet = vndGetInfoById(iVndID, &vndHandleInfo);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get no vnd info.\n");

        return  (PX_ERROR);
    }

    /*
     *  得到虚拟网卡的信息，进行恢复
     */
    bzero(&ifparamInfo, sizeof(ifparamInfo));
    ifparamInfo.cDefault = 0;
    ifparamInfo.cEnable  = 1;
    snprintf(ifparamInfo.cDevName, sizeof(ifparamInfo.cDevName), "vnd-%d", vndGetNextId());
    ifparamInfo.uiIpAddr.addr  = vndHandleInfo.uiIpAddr.addr;
    ifparamInfo.uiNetMask.addr = vndHandleInfo.uiNetMask.addr;
    ifparamInfo.uiGateWay.addr = vndHandleInfo.uiGateWay.addr;
    strncpy((PCHAR)ifparamInfo.ucMac, (const CHAR *)vndHandleInfo.ucHwAddr, sizeof(ifparamInfo.ucMac));
    ifparamInfo.uiUpdateFlag = INI_UPDATE_FLAG;
    ifparamInfo.cHandle      = INI_UPDATE_SEC;
    ifParamIniFileHandle(&ifparamInfo);                                 /*  将信息填入 ifparam.ini 文件 */

    if (0 == vndHandleInfo.ucHwAddr[0] &&
        0 == vndHandleInfo.ucHwAddr[1] &&
        0 == vndHandleInfo.ucHwAddr[2] &&
        0 == vndHandleInfo.ucHwAddr[3] &&
        0 == vndHandleInfo.ucHwAddr[4] &&
        0 == vndHandleInfo.ucHwAddr[5]) {
        sprintf(cCmdBuf, "vnd add %d", iVndID);
    } else {
        sprintf(cCmdBuf, "vnd add %d %02x:%02x:%02x:%02x:%02x:%02x",
                iVndID,
                vndHandleInfo.ucHwAddr[0],
                vndHandleInfo.ucHwAddr[1],
                vndHandleInfo.ucHwAddr[2],
                vndHandleInfo.ucHwAddr[3],
                vndHandleInfo.ucHwAddr[4],
                vndHandleInfo.ucHwAddr[5]);
    }

    iRet = system(cCmdBuf);                                             /*  创建虚拟网卡                */
    if (ERROR_NONE == iRet) {
        vndHandleInfo.iNetIfIndex = iNetifIndex;
        vndAddItemToDb(&vndHandleInfo);                                 /*  更新数据库                  */

        /*
         *  更新一下 _G_iVndID ，方便重启后继续从恢复的 ID 号向后创建虚拟网卡
         */
        vndSetNextId(++iVndID);
    }

    return  (iRet);
}
