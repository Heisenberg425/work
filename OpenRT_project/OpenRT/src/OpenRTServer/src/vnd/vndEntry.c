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
** 文   件   名: vndEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: OpenRT 虚拟网卡 配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "libvnd_config.h"
#include "vndExec.h"
#include "vndDB.h"
#include "msgcomm.h"
#include "interface/interface_common.h"
/*********************************************************************************************************
** 函数名称: __vndAdd
** 功能描述: 添加虚拟网卡入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndAdd (PVOID  pArgBuf)
{
    __PVND_HANDLE     pVndHandle = LW_NULL;
    __IFPARAM_INFO    ifparamInfo;
    INT               iVndRet;
    INT               iVndID     = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pVndHandle = (__PVND_HANDLE)pArgBuf;
    /*
     *  todo: 1、更新 ifparam.ini 信息 ,空格系统不识别
     *        2、两个结构转换抽出
     */
    bzero(&ifparamInfo, sizeof(ifparamInfo));
    ifparamInfo.cDefault = 0;
    ifparamInfo.cEnable  = 1;
    snprintf(ifparamInfo.cDevName, sizeof(ifparamInfo.cDevName), "vnd-%d", vndGetNextId());
    ifparamInfo.uiIpAddr.addr  = pVndHandle->uiIpAddr.addr;
    ifparamInfo.uiNetMask.addr = pVndHandle->uiNetMask.addr;
    ifparamInfo.uiGateWay.addr = pVndHandle->uiGateWay.addr;
    strncpy((PCHAR)ifparamInfo.ucMac, (const CHAR *)pVndHandle->ucHwAddr, sizeof(ifparamInfo.ucMac));
    ifparamInfo.uiUpdateFlag = INI_UPDATE_FLAG;
    ifparamInfo.cHandle      = INI_UPDATE_SEC;

    ifParamIniFileHandle(&ifparamInfo);                                 /*  将信息填入 ifparam.ini 文件 */

    /*
     *  添加一个虚拟网卡
     */
    iVndRet = vndStart(&iVndID, pVndHandle);
    if (PX_ERROR == iVndRet) {
        ifparamInfo.cHandle = INI_DEL_SEC;
        ifParamIniFileHandle(&ifparamInfo);                             /*  删除 ifparam.ini 文件的信息 */
    }

    replyConfigMsg(OPENRT_ADD_VND_ACK_CMD, iVndID);
}

/*********************************************************************************************************
** 函数名称: __vndDel
** 功能描述: 删除虚拟网卡入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndDel (PVOID  pArgBuf)
{
    INT               iVndID  = 0;

    if (LW_NULL == pArgBuf) {
        return;
    }

    iVndID = *(INT *)pArgBuf;
    replyConfigMsg(OPENRT_DEL_VND_ACK_CMD, vndDel(iVndID));
}

/*********************************************************************************************************
** 函数名称: __vndGetInfo
** 功能描述: 获取虚拟网卡信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndGetInfo (PVOID  pArgBuf)
{
    PCHAR     pcVndAllInfo    = LW_NULL;
    INT       iNum            = 0;

    iNum            = vndGetDBRow();
    pcVndAllInfo    = vndGetDBInfo();
    replyQueryMsg(OPENRT_GET_VND_ACK_CMD, sizeof(__VND_HANDLE), iNum, pcVndAllInfo);

    free(pcVndAllInfo);
}
/*********************************************************************************************************
** 函数名称: __vndGetNetIfIndex
** 功能描述: 获取虚拟网卡 NetIfIndex
** 输　入  : pArgBuf      接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndGetNetIfIndex (PVOID  pArgBuf)
{
    INT  iVndid = *(INT *)pArgBuf;
    INT  iNetIfIndex;
    INT  iRet;

    iRet = netIfIndexGetByVndId(iVndid, &iNetIfIndex);
    if (PX_ERROR == iRet) {
        replyQueryMsg(OPENRT_GET_IFINDEX_ACK_CMD, sizeof(INT), 0, NULL);

        return;
    }

    replyQueryMsg(OPENRT_GET_IFINDEX_ACK_CMD, sizeof(INT), 1, &iNetIfIndex);
}
/*********************************************************************************************************
** 函数名称: __vndGetVndid
** 功能描述: 获取虚拟网卡 VNDID
** 输　入  : pArgBuf      接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __vndGetVndid (PVOID  pArgBuf)
{
    INT  iNetIfIndex = *(INT *)pArgBuf;
    INT  iVndid;
    INT  iRet;

    iRet = vndIdGetByNetIfIndex(iNetIfIndex, &iVndid);
    if (PX_ERROR == iRet) {
        replyQueryMsg(OPENRT_GET_VNDID_ACK_CMD, sizeof(INT), 0, NULL);

        return;
    }

    replyQueryMsg(OPENRT_GET_VNDID_ACK_CMD, sizeof(INT), 1, &iVndid);
}
/*********************************************************************************************************
** 函数名称: vndHandleEntry
** 功能描述: 虚拟网卡配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID vndHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_ADD_VND_CMD:
        __vndAdd(pArgBuf);
        break;

    case OPENRT_DEL_VND_CMD:
        __vndDel(pArgBuf);
        break;

    case OPENRT_GET_VND_CMD:
        __vndGetInfo(pArgBuf);
        break;

    case OPENRT_GET_IFINDEX_CMD:
        __vndGetNetIfIndex(pArgBuf);
        break;

    case OPENRT_GET_VNDID_CMD:
        __vndGetVndid(pArgBuf);
        break;

    default:
        break;
    }
}
