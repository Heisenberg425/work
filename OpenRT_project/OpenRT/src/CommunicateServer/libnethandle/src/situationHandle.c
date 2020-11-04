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
** 文   件   名: situationHandle.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 18 日
**
** 描        述:  情景模式管理处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "situation_info.h"
#include "libvpn_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** 函数名称: libSituationInfoSave
** 功能描述: 情景模式信息保存
** 输　入  : pSituationInfo  情景模式入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libSituationInfoSave (__PSITUATION_INFO  pSituationInfo)
{
    if (NULL == pSituationInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg (OPENRT_SAVE_SIT_INFO_CMD, pSituationInfo, sizeof(__SITUATION_INFO));
}
/*********************************************************************************************************
** 函数名称: libWanInfoSave
** 功能描述: WAN 信息保存
** 输　入  : pWanPageInfo  WAN 入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libWanInfoSave (__PWANPAGE_INFO  pWanPageInfo)
{
    if (NULL == pWanPageInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg (OPENRT_SAVE_WAN_INFO_CMD, pWanPageInfo, sizeof(__WANPAGE_INFO));
}
/*********************************************************************************************************
** 函数名称: libKidVpnServerInfoSave
** 功能描述: KidVpn Server 信息保存
** 输　入  : pKidVPNServerInfo  KidVPN server 入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVpnServerInfoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    if (NULL == pKidVPNServerInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg(OPENRT_SAVE_VPNSER_INFO_CMD,
                             pKidVPNServerInfo,
                             sizeof(__KIDVPN_SERVER_INFO));
}
/*********************************************************************************************************
** 函数名称: libKidVpnServerInfoClean
** 功能描述: KidVpn Server 信息清除
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVpnServerInfoClean (VOID)
{
    return  requestConfigMsg(OPENRT_CLEAN_VPNSER_INFO_CMD, NULL, 0);
}
/*********************************************************************************************************
** 函数名称: libVpnServConfInfoSave
** 功能描述: KidVpn Server 配置信息保存
** 输　入  : pKidVPNServerInfo  KidVPN server 入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libVpnServConfInfoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    if (NULL == pKidVPNServerInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg(OPENRT_SAVE_VPNSER_CONF_CMD,
                             pKidVPNServerInfo,
                             sizeof(__KIDVPN_SERVER_INFO));
}
/*********************************************************************************************************
** 函数名称: libKidvpnCancel
** 功能描述: 取消 KidVPN
** 输　入  : iKidVpnType      KidVPN 类型
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidvpnCancel (INT  iKidVpnType)
{
    if (TYPE_ADD_VPN_S != iKidVpnType && TYPE_ADD_VPN_C != iKidVpnType) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_CANCEL_KIDVPN_CMD, &iKidVpnType, sizeof(iKidVpnType)));
}
/*********************************************************************************************************
** 函数名称: libOptionalFuncCancel
** 功能描述: 取消可选功能
** 输　入  : iOptFunc      可选功能
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libOptionalFuncCancel (INT  iOptFunc)
{
    if (iOptFunc < 0 || iOptFunc > 7) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_CANCEL_OPTFUNC_CMD, &iOptFunc, sizeof(iOptFunc)));
}
/*********************************************************************************************************
** 函数名称: libKidVpnClientInfoSave
** 功能描述: KidVpn Client 信息保存
** 输　入  : pKidVPNClientInfo  KidVPN Client 入参信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVpnClientInfoSave (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    if (NULL == pKidVPNClientInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg(OPENRT_SAVE_CLI_INFO_CMD,
                             pKidVPNClientInfo,
                             sizeof(__KIDVPN_CLIENT_INFO));
}
