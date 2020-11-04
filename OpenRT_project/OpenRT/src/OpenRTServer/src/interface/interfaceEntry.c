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
** ��   ��   ��: interfaceEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 4 ��
**
** ��        ��: OpenRT ���ڲ������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "common.h"
#include "interfaceExec.h"
#include "libinterface_config.h"
/*********************************************************************************************************
** ��������: networkSegCollisionDetect
** ��������: ���γ�ͻ���
** �䡡��  : pNetInfo         ���õ����ڵ���Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static INT  networkSegCollisionDetect(__PNETIF_SET  pNetInfo)
{
    PCHAR                 pIfaceBuf           = LW_NULL;
    PCHAR                 pBuffTmp            = LW_NULL;
    __PNETIF_INFO         pIfaceInfo          = LW_NULL;
    INT                  iNetIfNum   = 0;
    INT                  iBufLen     = 0;
    UINT                 uiNetId     = 0;
    UINT                 uiNetIdTmp  = 0;
    struct in_addr       inIp        = {0};
    struct in_addr       inNetmask   = {0};
    INT                  i;
    INT                  iRet;

    iNetIfNum = netIfNumGet() + vndNetIfNumGet();                       /*  ��ȡ��ǰ����������          */
    if (0 >= iNetIfNum) {
        return  (ERROR_NONE);
    }

    /*
     *  �����ڴ棬���ڱ���������������Ϣ
     */
    iBufLen     = iNetIfNum * sizeof(__NETIF_INFO);
    pIfaceBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pIfaceBuf) {
        return  (PX_ERROR);
    }

    bzero(pIfaceBuf, iBufLen);

    iRet = netIfInfoGetAll(pIfaceBuf, iBufLen);                         /*  ��ȡ��������Ϣ              */
    if (ERROR_NONE != iRet) {
        if (NULL != pIfaceBuf) {
            free(pIfaceBuf);
        }

        return  (PX_ERROR);
    }

    inet_aton(pNetInfo->cIpAddr,  &inIp);
    inet_aton(pNetInfo->cNetMask, &inNetmask);

    uiNetId = (inIp.s_addr & inNetmask.s_addr);

    pBuffTmp = pIfaceBuf;
    for (i = 0; i < iNetIfNum; i++) {
        pIfaceInfo = (__PNETIF_INFO)pIfaceBuf;
        if (strstr(pIfaceInfo->cNetifName, "vn") ||
           !strcmp(pNetInfo->cNetifName, pIfaceInfo->cNetifName)) {
            pIfaceBuf += sizeof(__NETIF_INFO);

            continue;
        }

        uiNetIdTmp = pIfaceInfo->uiIp & pIfaceInfo->uiNetMask;
        if (uiNetId == uiNetIdTmp) {
            return  (RET_COLLISION);
        }

        pIfaceBuf += sizeof(__NETIF_INFO);
    }


    if (NULL != pBuffTmp) {
        free(pBuffTmp);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __netifInfoSet
** ��������: ����ָ�����ڲ������
** �䡡��  : pArgBuf            ���յĲ���
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __netifInfoSet (PVOID  pArgBuf)
{
    INT           iRet;
    __PNETIF_SET  pNetInfo = NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, PX_ERROR);

        return;
    }

    pNetInfo = (__PNETIF_SET)pArgBuf;

    iRet = networkSegCollisionDetect(pNetInfo);
    if (ERROR_NONE != iRet) {
        replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, iRet);

        return;
    }

    replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, netIfInfoSet(pNetInfo));
}
/*********************************************************************************************************
** ��������: __getAllNetIfInfo
** ��������: ��ȡ����������Ϣ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __getAllNetIfInfo (VOID)
{
    PCHAR                pcAllNetInfoBuff = LW_NULL;
    INT                  iNetIfNum        = 0;
    INT                  iBufLen          = 0;

    iNetIfNum = netIfNumGet() + vndNetIfNumGet();                       /*  ��ȡ��ǰ����������          */
    if (0 >= iNetIfNum) {
        replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                      sizeof(__NETIF_INFO),
                      iNetIfNum,
                      pcAllNetInfoBuff);
        return;
    }

    /*
     *  �����ڴ棬���ڱ���������������Ϣ
     */
    iBufLen          = iNetIfNum * sizeof(__NETIF_INFO);
    pcAllNetInfoBuff = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcAllNetInfoBuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                      sizeof(__NETIF_INFO),
                      0,
                      pcAllNetInfoBuff);

        return;
    }

    bzero(pcAllNetInfoBuff, iBufLen);
    netIfInfoGetAll(pcAllNetInfoBuff, iBufLen);                         /*  ��ȡ��������Ϣ              */
    replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                  sizeof(__NETIF_INFO),
                  iNetIfNum,
                  pcAllNetInfoBuff);

    free(pcAllNetInfoBuff);
}

/*********************************************************************************************************
** ��������: interFaceHandleEntry
** ��������: ���ڲ������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID interFaceHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_NETINFO_GET_CMD:
        __getAllNetIfInfo();
        break;

    case OPENRT_NETINFO_SET_CMD:
        __netifInfoSet(pArgBuf);
        break;

    default:
        break;
    }
}
