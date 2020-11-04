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
** ��   ��   ��: situationExec.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 18 ��
**
** ��        ��: �龰ģʽ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include "libsituation_config.h"
#include "kidvpn/kidvpn_common.h"
#include "situation_common.h"
/*********************************************************************************************************
** ��������: situationInfoDoSave
** ��������: �龰ģʽ��Ϣ����
** �䡡��  : pSituationInfo    �龰ģʽ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  situationInfoDoSave (__PSITUATION_INFO  pSituationInfo)
{
    return  (situationAddItemToDb(pSituationInfo));
}
/*********************************************************************************************************
** ��������: wanInfoDoSave
** ��������: WAN ��Ϣ����
** �䡡��  : pWanPageInfo  WAN ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  wanInfoDoSave (__PWANPAGE_INFO  pWanPageInfo)
{
    return  (wanPageAddItemToDb(pWanPageInfo));
}
/*********************************************************************************************************
** ��������: kidVpnServerInfoDoSave
** ��������: KidVPN Server ��Ϣ����
** �䡡��  : pKidVPNServerInfo  KidVPN server ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerInfoDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    return  (kidVpnServerPageAddItemToDb(pKidVPNServerInfo));
}
/*********************************************************************************************************
** ��������: kidVpnClientInfoDoSave
** ��������: KidVPN Client ��Ϣ����
** �䡡��  : pKidVPNClientInfo  KidVPN Client ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnClientInfoDoSave (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    return  (kidVpnClientPageAddItemToDb(pKidVPNClientInfo));
}
/*********************************************************************************************************
** ��������: kidVpnServerInfoDoClean
** ��������: KidVPN Server ��Ϣ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerInfoDoClean (VOID)
{
    return  (kidVpnServerPageDeleteFromDb());
}
/*********************************************************************************************************
** ��������: kidVpnServerConfDoSave
** ��������: KidVPN Server ������Ϣ����
** �䡡��  : pKidVPNServerInfo  KidVPN server ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerConfDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    INT   iRet;
    INT   iFd;
    CHAR  cConfBuf[128] = {0};

    if (NULL == pKidVPNServerInfo) {
        return  (PX_ERROR);
    }

    if(PX_ERROR == access(KIDVPN_SERVER_CONF_PATH, F_OK)) {             /*  �����ڵ�ǰ·��              */
        mkdir(KIDVPN_SERVER_CONF_PATH, 666);
    }

    iRet = access(KIDVPN_SERVER_CONF, F_OK);
    if (PX_ERROR == iRet) {
        iFd = open(KIDVPN_SERVER_CONF, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (iFd < 0) {
            return  (PX_ERROR);
        }
    } else {
        iFd = open(KIDVPN_SERVER_CONF, O_RDWR | O_CREAT | O_TRUNC);
        if (iFd < 0) {
            return  (PX_ERROR);
        }
    }

    sprintf(cConfBuf, "Public Ip:%s\n", pKidVPNServerInfo->cPublicIp);
    write(iFd, cConfBuf, strlen(cConfBuf));

    bzero(cConfBuf, sizeof(cConfBuf));
    sprintf(cConfBuf, "KidVPN Ip:%s\n", pKidVPNServerInfo->cVndIpAddr);
    write(iFd, cConfBuf, strlen(cConfBuf));

    bzero(cConfBuf, sizeof(cConfBuf));
    sprintf(cConfBuf, "KidVPN Netmask:%s\n", pKidVPNServerInfo->cVndNetMask);
    write(iFd, cConfBuf, strlen(cConfBuf));

    bzero(cConfBuf, sizeof(cConfBuf));
    sprintf(cConfBuf, "KidVPN Gateway:%s\n", pKidVPNServerInfo->cVndGateway);
    write(iFd, cConfBuf, strlen(cConfBuf));

    bzero(cConfBuf, sizeof(cConfBuf));
    sprintf(cConfBuf, "Passwd:%s\n", pKidVPNServerInfo->cPasswd);
    write(iFd, cConfBuf, strlen(cConfBuf));

    bzero(cConfBuf, sizeof(cConfBuf));
    sprintf(cConfBuf, "AES Key:%s\n", pKidVPNServerInfo->cAesKey);
    write(iFd, cConfBuf, strlen(cConfBuf));

    close(iFd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: kidVpnDoCancel
** ��������: ���� KidVPN
** �䡡��  : iKidVpnType  KidVPN ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnDoCancel (INT  iKidVpnType)
{
    INT               iRet;
    __KIDVPN_DB_INFO  kidVpnDBInfo;
    INT               iVndId;

    bzero(&kidVpnDBInfo, sizeof(kidVpnDBInfo));

    iRet = kidvpnGetSpecInfo(&kidVpnDBInfo, "TYPE", LW_NULL, &iKidVpnType);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    iVndId = kidVpnDBInfo.kidvpnHandle.iVndID;

    return  (kidVpnDel(iVndId));
}

/*********************************************************************************************************
** ��������: optFuncDoCancel
** ��������: ȡ����ѡ����
** �䡡��  : iOptFunc  ��ѡ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  optFuncDoCancel (INT  iOptFunc)
{
    INT               iRet;
    __SITUATION_INFO  situationInfo;

    bzero(&situationInfo, sizeof(situationInfo));

    iRet = situationInfoGet(&situationInfo);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    /*
     *  ���ÿ�ѡ���ܣ���д�����ݿ�
     */
    situationInfo.iFunc &= iOptFunc;

    return  situationAddItemToDb(&situationInfo);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
