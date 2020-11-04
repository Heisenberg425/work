/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: situationHandle.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 18 ��
**
** ��        ��:  �龰ģʽ������
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
** ��������: libSituationInfoSave
** ��������: �龰ģʽ��Ϣ����
** �䡡��  : pSituationInfo  �龰ģʽ�����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libSituationInfoSave (__PSITUATION_INFO  pSituationInfo)
{
    if (NULL == pSituationInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg (OPENRT_SAVE_SIT_INFO_CMD, pSituationInfo, sizeof(__SITUATION_INFO));
}
/*********************************************************************************************************
** ��������: libWanInfoSave
** ��������: WAN ��Ϣ����
** �䡡��  : pWanPageInfo  WAN �����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libWanInfoSave (__PWANPAGE_INFO  pWanPageInfo)
{
    if (NULL == pWanPageInfo) {
        return  (RET_ARG_ERROR);
    }

    return  requestConfigMsg (OPENRT_SAVE_WAN_INFO_CMD, pWanPageInfo, sizeof(__WANPAGE_INFO));
}
/*********************************************************************************************************
** ��������: libKidVpnServerInfoSave
** ��������: KidVpn Server ��Ϣ����
** �䡡��  : pKidVPNServerInfo  KidVPN server �����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: libKidVpnServerInfoClean
** ��������: KidVpn Server ��Ϣ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVpnServerInfoClean (VOID)
{
    return  requestConfigMsg(OPENRT_CLEAN_VPNSER_INFO_CMD, NULL, 0);
}
/*********************************************************************************************************
** ��������: libVpnServConfInfoSave
** ��������: KidVpn Server ������Ϣ����
** �䡡��  : pKidVPNServerInfo  KidVPN server �����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: libKidvpnCancel
** ��������: ȡ�� KidVPN
** �䡡��  : iKidVpnType      KidVPN ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidvpnCancel (INT  iKidVpnType)
{
    if (TYPE_ADD_VPN_S != iKidVpnType && TYPE_ADD_VPN_C != iKidVpnType) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_CANCEL_KIDVPN_CMD, &iKidVpnType, sizeof(iKidVpnType)));
}
/*********************************************************************************************************
** ��������: libOptionalFuncCancel
** ��������: ȡ����ѡ����
** �䡡��  : iOptFunc      ��ѡ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libOptionalFuncCancel (INT  iOptFunc)
{
    if (iOptFunc < 0 || iOptFunc > 7) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_CANCEL_OPTFUNC_CMD, &iOptFunc, sizeof(iOptFunc)));
}
/*********************************************************************************************************
** ��������: libKidVpnClientInfoSave
** ��������: KidVpn Client ��Ϣ����
** �䡡��  : pKidVPNClientInfo  KidVPN Client �����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
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
