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
** 文   件   名: situationExec.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 18 日
**
** 描        述: 情景模式操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include "libsituation_config.h"
#include "kidvpn/kidvpn_common.h"
#include "situation_common.h"
/*********************************************************************************************************
** 函数名称: situationInfoDoSave
** 功能描述: 情景模式信息保存
** 输　入  : pSituationInfo    情景模式参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  situationInfoDoSave (__PSITUATION_INFO  pSituationInfo)
{
    return  (situationAddItemToDb(pSituationInfo));
}
/*********************************************************************************************************
** 函数名称: wanInfoDoSave
** 功能描述: WAN 信息保存
** 输　入  : pWanPageInfo  WAN 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  wanInfoDoSave (__PWANPAGE_INFO  pWanPageInfo)
{
    return  (wanPageAddItemToDb(pWanPageInfo));
}
/*********************************************************************************************************
** 函数名称: kidVpnServerInfoDoSave
** 功能描述: KidVPN Server 信息保存
** 输　入  : pKidVPNServerInfo  KidVPN server 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerInfoDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    return  (kidVpnServerPageAddItemToDb(pKidVPNServerInfo));
}
/*********************************************************************************************************
** 函数名称: kidVpnClientInfoDoSave
** 功能描述: KidVPN Client 信息保存
** 输　入  : pKidVPNClientInfo  KidVPN Client 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnClientInfoDoSave (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    return  (kidVpnClientPageAddItemToDb(pKidVPNClientInfo));
}
/*********************************************************************************************************
** 函数名称: kidVpnServerInfoDoClean
** 功能描述: KidVPN Server 信息清除
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerInfoDoClean (VOID)
{
    return  (kidVpnServerPageDeleteFromDb());
}
/*********************************************************************************************************
** 函数名称: kidVpnServerConfDoSave
** 功能描述: KidVPN Server 配置信息保存
** 输　入  : pKidVPNServerInfo  KidVPN server 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerConfDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    INT   iRet;
    INT   iFd;
    CHAR  cConfBuf[128] = {0};

    if (NULL == pKidVPNServerInfo) {
        return  (PX_ERROR);
    }

    if(PX_ERROR == access(KIDVPN_SERVER_CONF_PATH, F_OK)) {             /*  不存在当前路径              */
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
** 函数名称: kidVpnDoCancel
** 功能描述: 撤销 KidVPN
** 输　入  : iKidVpnType  KidVPN 类型
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: optFuncDoCancel
** 功能描述: 取消可选功能
** 输　入  : iOptFunc  可选功能
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  设置可选功能，并写入数据库
     */
    situationInfo.iFunc &= iOptFunc;

    return  situationAddItemToDb(&situationInfo);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
