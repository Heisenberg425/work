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
** 文   件   名: situationEntry.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 18 日
**
** 描        述: OpenRT 情景模式配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "libsituation_config.h"
#include "situation_common.h"
/*********************************************************************************************************
** 函数名称: __situationInfoSaveDeal
** 功能描述: 静态路由配置处理函数
** 输　入  : pSituationInfo           前端传入的情景模式信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __situationInfoSaveDeal (__PSITUATION_INFO  pSituationInfo)
{
    replyConfigMsg(OPENRT_SAVE_SIT_INFO_CMD_ACK, situationInfoDoSave(pSituationInfo));
}
/*********************************************************************************************************
** 函数名称: __wanInfoSaveDeal
** 功能描述: WAN 信息保存处理函数
** 输　入  : pWanPageInfo           前端传入的 WAN 信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __wanInfoSaveDeal (__PWANPAGE_INFO  pWanPageInfo)
{
    replyConfigMsg(OPENRT_SAVE_WAN_INFO_CMD_ACK, wanInfoDoSave(pWanPageInfo));
}
/*********************************************************************************************************
** 函数名称: __kidVpnServerInfoSaveDeal
** 功能描述: KidVPN Server 信息保存处理函数
** 输　入  : pKidVPNServerInfo       前端传入的 KidVPN server 信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidVpnServerInfoSaveDeal (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    replyConfigMsg(OPENRT_SAVE_VPNSER_INFO_CMD_ACK, kidVpnServerInfoDoSave(pKidVPNServerInfo));
}

/*********************************************************************************************************
** 函数名称: __kidVpnClientInfoSaveDeal
** 功能描述: KidVPN Client 信息保存处理函数
** 输　入  : __PKIDVPN_CLIENT_INFO   前端传入的 KidVPN Client 信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidVpnClientInfoSaveDeal (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    replyConfigMsg(OPENRT_SAVE_CLI_INFO_CMD_ACK, kidVpnClientInfoDoSave(pKidVPNClientInfo));
}
/*********************************************************************************************************
 ** 函数名称: __kidVpnServerInfoCleanDeal
 ** 功能描述: KidVPN Server 信息清除处理函数
 ** 输　入  : pArgBuf     传入信息
 ** 输　出  : NONE
 ** 返　回  : NONE
 *********************************************************************************************************/
 static VOID  __kidVpnServerInfoCleanDeal (PVOID  pArgBuf)
 {
     replyConfigMsg(OPENRT_CLEAN_VPNSER_INFO_CMD_ACK, kidVpnServerInfoDoClean());
 }
 /*********************************************************************************************************
 ** 函数名称: __kidVpnServerConfSaveDeal
 ** 功能描述: KidVPN Server 配置信息保存处理函数
 ** 输　入  : pKidVPNServerInfo       前端传入的 KidVPN server 信息
 ** 输　出  : NONE
 ** 返　回  : NONE
 *********************************************************************************************************/
 static VOID  __kidVpnServerConfSaveDeal (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
 {
     replyConfigMsg(OPENRT_SAVE_VPNSER_CONF_CMD_ACK, kidVpnServerConfDoSave(pKidVPNServerInfo));
 }
/*********************************************************************************************************
** 函数名称: __kidVpnCancelDeal
** 功能描述: 撤销 KidVPN 处理函数
** 输　入  : pArgBuf     传入信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidVpnCancelDeal (PVOID  pArgBuf)
{
    INT  iKidVpnType;

    iKidVpnType = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_CANCEL_KIDVPN_CMD_ACK, kidVpnDoCancel(iKidVpnType));
}
/*********************************************************************************************************
** 函数名称: __optionalFuncCancelDeal
** 功能描述: 撤销可选功能处理函数
** 输　入  : pArgBuf     传入信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __optionalFuncCancelDeal (PVOID  pArgBuf)
{
    INT  iOptFunc;

    iOptFunc = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_CANCEL_OPTFUNC_CMD_ACK, optFuncDoCancel(iOptFunc));
}
/*********************************************************************************************************
** 函数名称: situationHandleEntry
** 功能描述: 情景模式配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  situationHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_SAVE_SIT_INFO_CMD:
        __situationInfoSaveDeal(pArgBuf);
        break;

    case OPENRT_SAVE_WAN_INFO_CMD:
        __wanInfoSaveDeal(pArgBuf);
        break;

    case OPENRT_SAVE_VPNSER_INFO_CMD:
        __kidVpnServerInfoSaveDeal(pArgBuf);
        break;

    case OPENRT_CLEAN_VPNSER_INFO_CMD:
        __kidVpnServerInfoCleanDeal(pArgBuf);
        break;

    case OPENRT_SAVE_VPNSER_CONF_CMD:
        __kidVpnServerConfSaveDeal(pArgBuf);
        break;

    case OPENRT_CANCEL_KIDVPN_CMD:
        __kidVpnCancelDeal(pArgBuf);
        break;

    case OPENRT_SAVE_CLI_INFO_CMD:
        __kidVpnClientInfoSaveDeal(pArgBuf);
        break;

    case OPENRT_CANCEL_OPTFUNC_CMD:
        __optionalFuncCancelDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
