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
** ��   ��   ��: natHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��:  NAT ���ô���
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "libcomm.h"
#include "libnat_config.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libNatAssNodeGet
** ��������: ��ȡ NAT �˿�ʹ����Ϣ
** �䡡��  : NONE
** �䡡��  : piNatAssNodeTotal  �˿��ܸ���
**           piNatAssNodeUse    �Ѿ�ʹ�õĸ���
** ������  : NONE
*********************************************************************************************************/
INT  libNatAssNodeGet (INT  *piNatAssNodeTotal, INT  *piNatAssNodeUse)
{
    __PNAT_ASSNODE_INFO   pNatAssNodeInfo = LW_NULL;
    INT                   iNum            = 0;

    if (LW_NULL == piNatAssNodeTotal || LW_NULL == piNatAssNodeUse) {
        return  (PX_ERROR);
    }

    pNatAssNodeInfo = requestQueryMsg(OPENRT_NAT_ASSNODE_GET, NULL, 0, &iNum);
    if (LW_NULL == pNatAssNodeInfo) {
        return  (PX_ERROR);
    }

    *piNatAssNodeTotal = pNatAssNodeInfo->iNatAssNodeTotal;
    *piNatAssNodeUse   = pNatAssNodeInfo->iNatAssNodeUse;

    free(pNatAssNodeInfo);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: libNatMapNumGet
** ��������: ��ȡ NAT �˿�ӳ����Ϣ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ��Ŀ����
*********************************************************************************************************/
INT32  libNatMapNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_MAP_NUM_GET, LW_NULL, 0));
}


/*********************************************************************************************************
** ��������: libNatMapGet
** ��������: ��ȡ NAT �˿�ӳ����Ϣ (��Ҫ���� free �ͷ���Դ)
** �䡡��  : pNatInfoGet        ָ����ȡ����Ŀ��
** �䡡��  : iNum               ��Ŀ����
** ������  : pcInfoBuf          map ��Ϣ
*********************************************************************************************************/
PVOID  libNatMapGet (__PNAT_INFO_GET  pNatInfoGet, INT  *piNum)
{
    if (LW_NULL == pNatInfoGet) {
        *piNum = 0;

        return  (LW_NULL);
    }
    return  (requestQueryMsg(OPENRT_NAT_MAP_GET, pNatInfoGet, sizeof(__NAT_INFO_GET), piNum));
}

/*********************************************************************************************************
** ��������: libNatMapSet
** ��������: NAT �˿�ӳ������
** �䡡��  : pcIpString         IP ��ַ
**           iIpCnt             IP ����
**           iWanPort           �����˿�
**           iLanPort           �����˿�
**           cProto             Э������(IPPROTO_UDP  IPPROTO_TCP)
**           cNatMapHandle      ӳ���Ӧ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libNatMapSet (PCHAR  pcIpString,
                   INT    iIpCnt,
                   INT    iWanPort,
                   INT    iLanPort,
                   CHAR   cProto,
                   CHAR   cNatMapHandle)
{
    __NAT_MAP_HANDLE      natMapHandle;

    if (LW_NULL == pcIpString                             ||
        (IPPROTO_UDP != cProto  && IPPROTO_TCP != cProto) ||
        (0 != cNatMapHandle     && 1 != cNatMapHandle)) {

        return  (PX_ERROR);
    }

    /*
     *  ����Ϣ����ȥ
     */
    bzero(&natMapHandle, sizeof(natMapHandle));
    natMapHandle.cHandle  = cNatMapHandle;
    natMapHandle.cProto   = cProto;
    natMapHandle.iIpCnt   = iIpCnt;
    natMapHandle.iLanPort = iLanPort;
    natMapHandle.iWanPort = iWanPort;
    strncpy(natMapHandle.cIpString, pcIpString, sizeof(natMapHandle.cIpString));

    return  (requestConfigMsg(OPENRT_NAT_MAP_SET, (PVOID)&natMapHandle, sizeof(natMapHandle)));
}

/*********************************************************************************************************
** ��������: libNatAliasNumGet
** ��������: ��ȡ NAT ������Ϣ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ��Ŀ����
*********************************************************************************************************/
INT32  libNatAliasNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_ALI_NUM_GET, LW_NULL, 0));
}

/*********************************************************************************************************
** ��������: libNatAliasGet
** ��������: ��ȡ NAT ������Ϣ  (��Ҫ���� free �ͷ���Դ)
** �䡡��  : pNatInfoGet        ָ����ȡ����Ŀ��
** �䡡��  : iNum               ��Ŀ����
** ������  : pcInfoBuf          ������Ϣ
*********************************************************************************************************/
PVOID  libNatAliasGet (__PNAT_INFO_GET  pNatInfoGet, INT  *piNum)
{
    if (LW_NULL == pNatInfoGet) {
        *piNum = 0;

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_NAT_ALIAS_GET, pNatInfoGet, sizeof(__NAT_INFO_GET), piNum));
}

/*********************************************************************************************************
** ��������: libNatAliasSet
** ��������: ��ȡ NAT ��Ƭ������
** �䡡��  : pcAlias            ������ַ
**           pcLanStart         LAN ��ʼ��ַ
**           pcLanEnd           LAN ������ַ
**           cNatAssHandle      ������Ӧ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libNatAliasSet (PCHAR  pcAlias, PCHAR  pcLanStart, PCHAR  pcLanEnd, CHAR  cNatAssHandle)
{
    __NATALIAS_HANDLE     natAliasHandle;

    if (LW_NULL == pcAlias    ||
        LW_NULL == pcLanStart ||
        LW_NULL == pcLanEnd   ||
        (0 != cNatAssHandle   &&
         1 != cNatAssHandle)) {
        return  (PX_ERROR);
    }

    /*
     *  ����Ϣ����ȥ
     */
    bzero(&natAliasHandle, sizeof(natAliasHandle));
    natAliasHandle.cHandle = cNatAssHandle;
    strncpy(natAliasHandle.cAlias,    pcAlias,  sizeof(natAliasHandle.cAlias));
    strncpy(natAliasHandle.cLanStart, pcLanStart, sizeof(natAliasHandle.cLanStart));
    strncpy(natAliasHandle.cLanEnd,   pcLanEnd, sizeof(natAliasHandle.cLanEnd));

    return  (requestConfigMsg(OPENRT_NAT_ALIAS_SET, &natAliasHandle, sizeof(natAliasHandle)));
}

/*********************************************************************************************************
** ��������: libNatIpFragSet
** ��������: ��ȡ NAT ��Ƭ������
** �䡡��  : iIpFragType        ��Ƭ������
**           iIpFragHandle      ��Ƭ�����ʹ���ʽ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libNatIpFragSet (CHAR  cIpFragType, CHAR  cIpFragHandle)
{
    __NAT_IPFRAG_HANDLE   natIpFragHandle;

    if (IPPROTO_TCP  != cIpFragType  &&
        IPPROTO_UDP  != cIpFragType &&
        IPPROTO_ICMP != cIpFragType) {
        return  (PX_ERROR);
    }

    if (NAT_IPFRAG_ENABLE  != cIpFragHandle &&
        NAT_IPFRAG_DISABLE != cIpFragHandle) {
        return  (PX_ERROR);
    }

    /*
     *  ����Ϣ����ȥ
     */
    bzero(&natIpFragHandle, sizeof(natIpFragHandle));
    natIpFragHandle.cHandle     = cIpFragHandle;
    natIpFragHandle.cIpfragType = cIpFragType;

    return  (requestConfigMsg(OPENRT_NAT_IPFRAG_SET, &natIpFragHandle, sizeof(natIpFragHandle)));
}

/*********************************************************************************************************
** ��������: libNatIpFragGet
** ��������: ��ȡ NAT ��Ƭ������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NAT ��Ƭ����Ϣ
*********************************************************************************************************/
INT  libNatIpFragGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_IPFRAG_GET, LW_NULL, 0));
}

/*********************************************************************************************************
** ��������: libNatWanLanConfig
** ��������: WAN LAN������
** �䡡��  : cpNetIfName        ��������
**           iHandle            ����ʽ
**           iFunc              ���ڹ��� (0 : WAN  1 : LAN)
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  libNatWanLanConfig (PCHAR  pcNetIfName, INT  iNatWanLanHandle, INT  iFunc)
{
    __NAT_WANLAN_HANDLE   natWanLanHandle;
    INT                   iNatRet;

    if (LW_NULL == pcNetIfName) {
        return  (PX_ERROR);
    }

    if ((NAT_WANLAN_SET != iNatWanLanHandle  && NAT_WANLAN_DEL != iNatWanLanHandle) ||
        (NAT_FUNC_LAN   != iFunc             && NAT_FUNC_WAN != iFunc)) {

        return  (PX_ERROR);
    }

    bzero(&natWanLanHandle, sizeof(natWanLanHandle));
    natWanLanHandle.iFunc   = iFunc;
    natWanLanHandle.iHandle = iNatWanLanHandle;
    strncpy(natWanLanHandle.cpNetIfName, pcNetIfName, sizeof(natWanLanHandle.cpNetIfName));

    iNatRet = requestConfigMsg(OPENRT_NAT_WANLAN_SET, &natWanLanHandle, sizeof(natWanLanHandle));

    return  (iNatRet);
}
