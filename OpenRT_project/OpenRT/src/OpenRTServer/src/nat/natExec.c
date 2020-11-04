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
** ��   ��   ��: natExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��:  NAT ����ʵ��
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "natStatus.h"
#include "natDB.h"
#include "nat_common.h"
#include "common.h"
/*********************************************************************************************************
  ���ڿ��� NAT ���ܵĿ�����
*********************************************************************************************************/
#define IDEL_WAN_LAN_NAME      "null"                                   /*  ����������                  */
/*********************************************************************************************************
** ��������: natCfgInit
** ��������: NAT ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natCfgInit (VOID)
{
    /*
     *  ����һ�� �� �� WAN LAN �ڣ����� NAT ����(�����ַ�ʽ���������� WAN��LAN��Ҫ�ȿ��� NAT������)
     */
    system("nat stop");

    return  inetNatStart(IDEL_WAN_LAN_NAME, IDEL_WAN_LAN_NAME);
}
extern VOID tmNotifyWanChange (PVOID  pvWan);
extern VOID tmNotifyLanChange (PVOID  pvLan);
/*********************************************************************************************************
** ��������: wanSetExistFlagCheck
** ��������: ��� WAN ���ñ�־
** �䡡��  : wanInfo            WAN ��Ϣ
**           cpcWanName         WAN ������
** �䡡��  : NONE
** ������  : PX_ERROR           ������
**           ERROR_NONE         δ����
*********************************************************************************************************/
INT  wanSetExistFlagCheck (__WAN_INFO   wanInfo, CPCHAR  cpcWanName)
{
    INT           iNum;

    for (iNum = 0; iNum < LW_CFG_NET_NAT_MAX_AP_IF; iNum++) {
        if (0 == strcmp((CPCHAR)wanInfo.cWanName[iNum], cpcWanName)) {
           return  (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** ��������: lanSetExistFlagCheck
** ��������: ��� LAN ���ñ�־
** �䡡��  : lanInfo            LAN ��Ϣ
**           cpcLanName         LAN ������
** �䡡��  : NONE
** ������  : PX_ERROR           ������
**           ERROR_NONE         δ����
*********************************************************************************************************/
INT  lanSetExistFlagCheck (__LAN_INFO   lanInfo, CPCHAR  cpcLanName)
{
    INT           iNum;

    for (iNum = 0; iNum < LW_CFG_NET_NAT_MAX_LOCAL_IF; iNum++) {
        if (0 == strcmp((CPCHAR)lanInfo.cLanName[iNum], cpcLanName)) {
           return  (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** ��������: natLanCfg
** ��������: ���� LAN ��״̬
** �䡡��  : cpcLanName         LAN ������
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natLanCfg (CPCHAR  cpcLanName)
{
    INT           iRet;
    __LAN_INFO    lanInfo;


    if (LW_NULL == cpcLanName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "lan name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  ��ȡ LAN ��Ϣ
     */
    bzero(&lanInfo, sizeof(lanInfo));
    natLanInfoGet(&lanInfo);

    iRet = lanSetExistFlagCheck(lanInfo, cpcLanName);
    if(ERROR_NONE == iRet) {                                            /*  LAN �ѳɹ�����              */
        return  (ERROR_NONE);
    }

    iRet = inetNatLocalAdd(cpcLanName);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    natLanUpdateToDB();

    bzero(&lanInfo, sizeof(lanInfo));
    natLanInfoGet(&lanInfo);
    tmNotifyLanChange(&lanInfo);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: natWanCfg
** ��������: ���� WAN ��״̬
** �䡡��  : cpcWanName         WAN ������
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natWanCfg (CPCHAR  cpcWanName)
{
    INT          iRet;
    CHAR         cRouteCmd[128] = {0};
    __WAN_INFO   wanInfo;

    if (LW_NULL == cpcWanName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "wan name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  ��ȡ WAN ��Ϣ
     */
    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);

    iRet = wanSetExistFlagCheck(wanInfo, cpcWanName);
    if(ERROR_NONE == iRet) {                                            /*  WAN �ѳɹ�����              */
        return  (ERROR_NONE);
    }

    iRet = inetNatWanAdd(cpcWanName);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    natWanUpdateToDB();

    snprintf(cRouteCmd, sizeof(cRouteCmd), "route add default dev %s", cpcWanName);

    system(cRouteCmd);

    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);
    tmNotifyWanChange(&wanInfo);

    return  (iRet);
}

/*********************************************************************************************************
** ��������: natWanLanDelet
** ��������: ɾ�� WAN LAN ��
** �䡡��  : cpcNetIfName       ��������
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natWanLanDelet (CPCHAR  cpcNetIfName)
{
    INT             i;
    __LAN_INFO      lanInfo;
    __WAN_INFO      wanInfo;

    if (LW_NULL == cpcNetIfName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "netif name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  ��ȡ WAN LAN ����Ϣ
     */
    bzero(&lanInfo, sizeof(lanInfo));
    bzero(&wanInfo, sizeof(wanInfo));
    natLanInfoGet(&lanInfo);
    natWanInfoGet(&wanInfo);

    inetNatStop();                                                      /*  �ȹر� NAT                  */

    /*
     *  �������� WAN LAN ����Ϣ
     */
    inetNatStart(IDEL_WAN_LAN_NAME, IDEL_WAN_LAN_NAME);

    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  ��������                    */
        if ((0 != strcmp(wanInfo.cWanName[i], cpcNetIfName)) &&
            (0 != strcmp(wanInfo.cWanName[i], IDEL_WAN_LAN_NAME))) {
            inetNatWanAdd(wanInfo.cWanName[i]);
        }
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  ��������                    */
        if ((0 != strcmp(lanInfo.cLanName[i], cpcNetIfName)) &&
            (0 != strcmp(lanInfo.cLanName[i], IDEL_WAN_LAN_NAME))) {
            inetNatLocalAdd(lanInfo.cLanName[i]);
        }
    }

    /*
     *  �������ݿ�
     */
    natWanUpdateToDB();
    natLanUpdateToDB();

    /*
     *  ��ȡ WAN ��Ϣ����֪ͨ������� ko ģ�� wan �ڱ仯
     */
    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);
    tmNotifyWanChange(&wanInfo);

    return (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natMapCfg
** ��������: ���ö˿�ӳ�书��
** �䡡��  : pNatMapCfg         ���ö˿�ӳ�����Ϣ�ṹ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natMapCfg (__PNAT_MAP_HANDLE  pNatMapHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatMapHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat map info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatMapHandle->cHandle) {
        iRet = inetNatMapAdd(pNatMapHandle->cIpString,
                             pNatMapHandle->iIpCnt,
                             pNatMapHandle->iLanPort,
                             pNatMapHandle->iWanPort,
                             pNatMapHandle->cProto);
    } else if (1 == pNatMapHandle->cHandle) {
        iRet = inetNatMapDelete(pNatMapHandle->cIpString,
                                pNatMapHandle->iLanPort,
                                pNatMapHandle->iWanPort,
                                pNatMapHandle->cProto);
    }

    natMapUpdateToDB();

    return (iRet);
}

/*********************************************************************************************************
** ��������: natAliasCfg
** ��������: ���������������
** �䡡��  : pNatAliasCfg       ���������������Ϣ�ṹ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natAliasCfg (__PNATALIAS_HANDLE  pNatAliasHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatAliasHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat alias info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatAliasHandle->cHandle) {                                /*  ���һ���������            */
        iRet = inetNatAliasAdd(pNatAliasHandle->cAlias,
                               pNatAliasHandle->cLanStart,
                               pNatAliasHandle->cLanEnd);
    } else {                                                            /*  ɾ��һ���������            */
        iRet = inetNatAliasDelete(pNatAliasHandle->cAlias);
    }

    natAliasUpdateToDB();

    return (iRet);
}

/*********************************************************************************************************
** ��������: natIpFragCfg
** ��������: ���÷�Ƭ������
** �䡡��  : pNatIpFragHandle   ���÷�Ƭ������Ϣ�ṹ
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT  natIpFragCfg(__PNAT_IPFRAG_HANDLE  pNatIpFragHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatIpFragHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat ifrag info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatIpFragHandle->cHandle) {
        iRet = inetNatIpFragSet(pNatIpFragHandle->cIpfragType, LW_FALSE);
    } else if (1 == pNatIpFragHandle->cHandle) {
        iRet = inetNatIpFragSet(pNatIpFragHandle->cIpfragType, LW_TRUE);
    }

    natIpFragUpdateToDB();

    return (iRet);
}


