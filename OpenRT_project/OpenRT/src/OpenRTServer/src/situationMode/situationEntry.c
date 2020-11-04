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
** ��   ��   ��: situationEntry.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 18 ��
**
** ��        ��: OpenRT �龰ģʽ���ô������
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
** ��������: __situationInfoSaveDeal
** ��������: ��̬·�����ô�����
** �䡡��  : pSituationInfo           ǰ�˴�����龰ģʽ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __situationInfoSaveDeal (__PSITUATION_INFO  pSituationInfo)
{
    replyConfigMsg(OPENRT_SAVE_SIT_INFO_CMD_ACK, situationInfoDoSave(pSituationInfo));
}
/*********************************************************************************************************
** ��������: __wanInfoSaveDeal
** ��������: WAN ��Ϣ���洦����
** �䡡��  : pWanPageInfo           ǰ�˴���� WAN ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __wanInfoSaveDeal (__PWANPAGE_INFO  pWanPageInfo)
{
    replyConfigMsg(OPENRT_SAVE_WAN_INFO_CMD_ACK, wanInfoDoSave(pWanPageInfo));
}
/*********************************************************************************************************
** ��������: __kidVpnServerInfoSaveDeal
** ��������: KidVPN Server ��Ϣ���洦����
** �䡡��  : pKidVPNServerInfo       ǰ�˴���� KidVPN server ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidVpnServerInfoSaveDeal (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    replyConfigMsg(OPENRT_SAVE_VPNSER_INFO_CMD_ACK, kidVpnServerInfoDoSave(pKidVPNServerInfo));
}

/*********************************************************************************************************
** ��������: __kidVpnClientInfoSaveDeal
** ��������: KidVPN Client ��Ϣ���洦����
** �䡡��  : __PKIDVPN_CLIENT_INFO   ǰ�˴���� KidVPN Client ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidVpnClientInfoSaveDeal (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    replyConfigMsg(OPENRT_SAVE_CLI_INFO_CMD_ACK, kidVpnClientInfoDoSave(pKidVPNClientInfo));
}
/*********************************************************************************************************
 ** ��������: __kidVpnServerInfoCleanDeal
 ** ��������: KidVPN Server ��Ϣ���������
 ** �䡡��  : pArgBuf     ������Ϣ
 ** �䡡��  : NONE
 ** ������  : NONE
 *********************************************************************************************************/
 static VOID  __kidVpnServerInfoCleanDeal (PVOID  pArgBuf)
 {
     replyConfigMsg(OPENRT_CLEAN_VPNSER_INFO_CMD_ACK, kidVpnServerInfoDoClean());
 }
 /*********************************************************************************************************
 ** ��������: __kidVpnServerConfSaveDeal
 ** ��������: KidVPN Server ������Ϣ���洦����
 ** �䡡��  : pKidVPNServerInfo       ǰ�˴���� KidVPN server ��Ϣ
 ** �䡡��  : NONE
 ** ������  : NONE
 *********************************************************************************************************/
 static VOID  __kidVpnServerConfSaveDeal (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
 {
     replyConfigMsg(OPENRT_SAVE_VPNSER_CONF_CMD_ACK, kidVpnServerConfDoSave(pKidVPNServerInfo));
 }
/*********************************************************************************************************
** ��������: __kidVpnCancelDeal
** ��������: ���� KidVPN ������
** �䡡��  : pArgBuf     ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidVpnCancelDeal (PVOID  pArgBuf)
{
    INT  iKidVpnType;

    iKidVpnType = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_CANCEL_KIDVPN_CMD_ACK, kidVpnDoCancel(iKidVpnType));
}
/*********************************************************************************************************
** ��������: __optionalFuncCancelDeal
** ��������: ������ѡ���ܴ�����
** �䡡��  : pArgBuf     ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __optionalFuncCancelDeal (PVOID  pArgBuf)
{
    INT  iOptFunc;

    iOptFunc = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_CANCEL_OPTFUNC_CMD_ACK, optFuncDoCancel(iOptFunc));
}
/*********************************************************************************************************
** ��������: situationHandleEntry
** ��������: �龰ģʽ�������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
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
