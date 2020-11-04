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
** ��   ��   ��: interfaceExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 27 ��
**
** ��        ��: ��������
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include "common.h"
#include "netdev.h"
#include "lwip/netif.h"
#include "net/if.h"
#include "fileparser/fileParser.h"
#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"
#include "interfaceExec.h"
#include "libinterface_config.h"
#include "net/lwip/lwip_iphook.h"
#include "net/lwip/lwip_if.h"
#include "nat/nat_common.h"
#include "udial/udial_common.h"
#include "intadaption/intada_common.h"
#include "intadaption/intAdaptionExec.h"
/*********************************************************************************************************
  ifparam.ini �����ļ���غ�
*********************************************************************************************************/
#define IFPARAM_INI_FILE    "/etc/ifparam.ini"                          /*  ����ini�����ļ�����         */
#define INI_FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)     /*  ����ini�����ļ�����Ȩ��     */
#define UDIAL_INI_SECTION   "pppoe_"                                    /*  ����ini�����ļ�sectionǰ׺  */
/*********************************************************************************************************
  ifparam.ini �����ļ���section��Ӧ��key
*********************************************************************************************************/
#define IFKEY_ENABLE         "enable"
#define IFKEY_IP             "ipaddr"
#define IFKEY_NETMASK        "netmask"
#define IFKEY_GATEWAY        "gateway"
#define IFKEY_DEFAULT        "default"
#define IFKEY_MAC            "mac"
#define IFKEY_DHCP           "dhcp"
/*********************************************************************************************************
  �ⲿ��������
*********************************************************************************************************/
extern INT  if_get_dhcp (const char *ifname);
extern INT  if_get_dhcp6 (const char *ifname);
/*********************************************************************************************************
** ��������: ifParamIniFileHandle
** ��������: �� ifparam.ini �����ļ����д���
** �䡡��  : pIfParamInfo       ifparam ��Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  ifParamIniFileHandle (__PIFPARAM_INFO   pIfParamInfo)
{
    CHAR   cNewSecton[50]       = {0};
    CHAR   cEntryEnableBuf[50]  = {0};
    CHAR   cEntryIpAddrBuf[50]  = {0};
    CHAR   cEntryNetMaskBuf[50] = {0};
    CHAR   cEntryGateWayBuf[50] = {0};
    CHAR   cEntryMacBuf[50]     = {0};
    CHAR   cEntryDefaultBuf[50] = {0};
    CHAR   cEntryDhcpBuf[50]    = {0};

    PCHAR  pcEnableBuf          = LW_NULL;
    PCHAR  pcDefaultBuf         = LW_NULL;
    PCHAR  pcDhcpBuf            = LW_NULL;
    CHAR   cIpAddrBuf[20]       = {0};
    CHAR   cNetMaskBuf[20]      = {0};
    CHAR   cGateWayBuf[20]      = {0};
    CHAR   cMacBuf[20]          = {0};
    INT    iSectionNum          = 0;
    INT    iSectionCnt          = 0;

    CHAR   cFlag                = 0;

    INT          iFd;
    dictionary  *dict;
    INT          i;
    FILE        *pFileId;
    PCHAR        pcSectionBuf   = LW_NULL;


    if (LW_NULL == pIfParamInfo) {
        return  (PX_ERROR);
    }

    if(-1 == (access(IFPARAM_INI_FILE, F_OK))) {                        /*  ����������ļ�              */
        /*
         *  ��������ļ�
         */
        iFd = open(IFPARAM_INI_FILE, O_RDWR | O_CREAT | O_TRUNC, INI_FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }

        close(iFd);
    }

    dict = iniparser_load(IFPARAM_INI_FILE);                            /*  ���������ļ�                */
    if (LW_NULL == dict) {
        printf("iniparser_load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  ��ȡsection����             */

    if (0 != iSectionNum) {                                             /*  �жϵ�ǰ��������Ϣ�Ƿ����  */
        for (i = 0; i < iSectionNum; i++) {
            pcSectionBuf = (CHAR *)iniparser_getsecname(dict, i);
            if (0 == strcmp(pcSectionBuf, pIfParamInfo->cDevName)) {
                iSectionCnt = i;
                cFlag = 1;                                              /*  �ж�Ӧ��Ϣ                  */
                break;
            }
        }
    }

    if (INI_UPDATE_SEC == pIfParamInfo->cHandle) {                      /*  ��Ҫ��� / �޸�����         */
        if (0 == cFlag) {                                               /*  ֮ǰû�ж�Ӧ����Ϣ          */
            iniparser_freedict(dict);                                   /*  ��֮ǰ��dictionary��ɾ��    */

            pFileId = fopen(IFPARAM_INI_FILE, "r+");                    /*  r+ ��ʽ�������ļ�         */
            if (LW_NULL == pFileId) {
                printf("Create file failed.\n");

                return  (PX_ERROR);
            }

            fseek(pFileId, 0L, SEEK_END);                               /*  �ƶ����ļ�ĩβ              */

            /*
             *  ���һ���µ�section
             */
            sprintf(cNewSecton, "%s", pIfParamInfo->cDevName);
            fprintf(pFileId, "\n[%s]\n", cNewSecton);                   /*  ���ļ���д���� section      */

            fclose (pFileId);

            dict = iniparser_load(IFPARAM_INI_FILE);                      /*  ���»�ȡdictionary          */
            if (LW_NULL == dict) {
                printf("iniparser_load file error\n");

                return  (PX_ERROR);
            }

            iSectionCnt = iSectionNum;                                  /*  �������µ�section��λ��     */
        }

        /*
         *  ���ˣ��ҵ���Ӧ�� section ����װ entry �ַ���, ������ dic
         */
        /*
         *  ����ʹ����Ϣ
         */
        if (IF_INI_UP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryEnableBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_ENABLE);

            pcEnableBuf = (pIfParamInfo->cEnable > 0) ? "1" : "0";
            iniparser_set(dict, cEntryEnableBuf, pcEnableBuf);
        }

        /*
         *  ����Ĭ��������Ϣ
         */
        if (IF_INI_DEFAULT_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryDefaultBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_DEFAULT);

            pcDefaultBuf = (pIfParamInfo->cDefault > 0) ? "1" : "0";
            iniparser_set(dict, cEntryDefaultBuf, pcDefaultBuf);
        }

        /*
         *  ���� IP ��Ϣ
         */
        if (IF_INI_IP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryIpAddrBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_IP);
            ip4addr_ntoa_r(&(pIfParamInfo->uiIpAddr), cIpAddrBuf, sizeof(cIpAddrBuf));
            iniparser_set(dict, cEntryIpAddrBuf, cIpAddrBuf);
        }

        /*
         *  ���� MASK ��Ϣ
         */
        if (IF_INI_NETMASK_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryNetMaskBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_NETMASK);
            ip4addr_ntoa_r(&(pIfParamInfo->uiNetMask), cNetMaskBuf, sizeof(cNetMaskBuf));
            iniparser_set(dict, cEntryNetMaskBuf, cNetMaskBuf);
        }

        /*
         *  ���� GW ��Ϣ
         */
        if (IF_INI_GW_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryGateWayBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_GATEWAY);
            ip4addr_ntoa_r(&(pIfParamInfo->uiGateWay), cGateWayBuf, sizeof(cGateWayBuf));
            iniparser_set(dict, cEntryGateWayBuf, cGateWayBuf);
        }

        /*
         *  ���� MAC ��Ϣ
         */
        if (IF_INI_MAC_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryMacBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_MAC);
            sprintf(cMacBuf, "%x:%x:%x:%x:%x:%x",
                    pIfParamInfo->ucMac[0],
                    pIfParamInfo->ucMac[1],
                    pIfParamInfo->ucMac[2],
                    pIfParamInfo->ucMac[3],
                    pIfParamInfo->ucMac[4],
                    pIfParamInfo->ucMac[5]);
            iniparser_set(dict, cEntryMacBuf, cMacBuf);
        }

        /*
         *  ���� DHCP ��Ϣ
         */
        if (IF_INI_DHCP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryDhcpBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_DHCP);

            pcDhcpBuf = (pIfParamInfo->cDhcp > 0) ? "1" : "0";
            iniparser_set(dict, cEntryDhcpBuf, pcDhcpBuf);
        }

        pFileId = LW_NULL;
        pFileId = fopen(IFPARAM_INI_FILE, "w+");                        /*  ����ļ���                */
        if (LW_NULL == pFileId) {
            printf("Create file failed, ini has cleared.\n");
            iniparser_freedict(dict);                                   /*  �ͷ�dictionar               */

            return  (PX_ERROR);
        }

        iniparser_dump_ini(dict, pFileId);                              /*  ��dictionarд��ini�ļ�      */

        iniparser_freedict(dict);                                       /*  �ͷ�dictionary              */

        fclose (pFileId);

        return  (ERROR_NONE);
    } else {                                                            /*  ��Ҫɾ����Ϣ                */
        if (0 == cFlag) {                                               /*  ֮ǰû�ж�Ӧ�Ĳ�����Ϣ      */
            iniparser_freedict(dict);                                   /*  �ͷ�dictionary              */
            printf("Ini file has no msg.\n");

            return  (PX_ERROR);
        } else {
            pFileId = fopen(IFPARAM_INI_FILE, "w+");                    /*  ����ļ���                */
            if (LW_NULL == pFileId) {
                printf("Create file failed.\n");
                iniparser_freedict(dict);                               /*  �ͷ�dictionar               */

                return  (PX_ERROR);
            }

            for (i = 0; i < iSectionNum; i++) {
                if (i != iSectionCnt) {                                 /*  ����Ҫɾ����section�޳�     */
                    iniparser_dumpsection_ini(dict, iniparser_getsecname(dict, i), pFileId);
                }
            }

            iniparser_freedict(dict);                                   /*  �ͷ�dictionary              */

            fclose (pFileId);

            return  (ERROR_NONE);
        }
    }
}

/*********************************************************************************************************
** ��������: __netTypeCheck
** ��������: ���ڹ��ܼ��
** �䡡��  : cpcNetifName       ��������
** �䡡��  : NONE
** ������  : cNetType           ��������
*********************************************************************************************************/
static INT __netTypeCheck (PCHAR  pcNetifName)
{
    __UDIAL_HANDLE         udialInfo;
    INT                    iRet;
    INT                    iNetType = NETIF_ERROR;

    if (LW_NULL == pcNetifName) {
        return  (iNetType);
    }

    /*
     *  �ж��Ƿ��ǲ�������
     */
    iRet = udialGetSpecInfo(&udialInfo, "NETIF", pcNetifName, LW_NULL);
    if (ERROR_NONE == iRet && udialInfo.uiPid) {                        /*  �ҵ�������Ϣ�����ڲ���״̬  */
        return  (NETIF_ADSL);
    }

    /*
     *  �ж��Ƿ��� DHCP ��������
     */
    if (if_get_dhcp(pcNetifName) || if_get_dhcp6(pcNetifName)) {
        return  (NETIF_DHCP);
    }

    return  (NETIF_NORMAL);
}

/*********************************************************************************************************
** ��������: __natFuncCheck
** ��������: NAT ���ܼ��
** �䡡��  : cpcNetifName       ��������
** �䡡��  : NONE
** ������  : iNatFunc           NAT ����
*********************************************************************************************************/
static INT __natFuncCheck (CPCHAR  cpcNetifName)
{
    INT                    iNatFunc = NETIF_ERROR;
    INT                    i;
    __WAN_INFO             wanInfo;
    __LAN_INFO             lanInfo;

    /*
     *  ��ȡ��ǰwan�ں�lan�ڶ�Ӧ������
     */
    bzero(&wanInfo, sizeof(wanInfo));
    bzero(&lanInfo, sizeof(lanInfo));
    natWanInfoGet(&wanInfo);
    natLanInfoGet(&lanInfo);

    if (LW_NULL == cpcNetifName) {
        return  (iNatFunc);
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  ��������                    */
        if (0 == strcmp(wanInfo.cWanName[i], cpcNetifName)) {
            iNatFunc = NETIF_WAN;
            return  (iNatFunc);
        }
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  ��������                    */
        if (0 == strcmp(lanInfo.cLanName[i], cpcNetifName)) {
            iNatFunc = NETIF_LAN;
            return  (iNatFunc);
        }
    }

    return  (NETIF_NORMAL);
}
/*********************************************************************************************************
** ��������: __vndDevNameConvertToWebName
** ��������: �����豸����ȡ web name
** �䡡��  : cpcDevName         �����豸����
** �䡡��  : pcWebName          web name
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT __vndDevNameConvertToWebName (CPCHAR  cpcDevName, PCHAR  pcWebName)
{
    INT    iVndid;
    PCHAR  pcTmp = NULL;


    if (NULL == cpcDevName || NULL == pcWebName) {
        return  (PX_ERROR);
    }


    pcTmp = lib_strstr(cpcDevName, "vnd-");
    if (NULL == pcTmp) {
        return  (PX_ERROR);
    }

    pcTmp += 4;

    iVndid = atoi(pcTmp);

    sprintf(pcWebName, "VPN-%d", iVndid);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: netIfNumGet
** ��������: ��ȡ����������������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ��������
*********************************************************************************************************/
INT  netIfNumGet (VOID)
{
    INT32           iSocketFd;
    struct  ifconf  iconf;
    INT32           iRet, i;
    INT32           iBuffSize;
    INT32           iNetIfNum  = 0;
    INT32           iStatus    = 0;

    struct ifreq   *pIfreq     = LW_NULL;
    PCHAR           pcIconfBuf = LW_NULL;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");

        return  (PX_ERROR);
    }

    /*
     *  ��ȡ��Ҫ���� buf ���
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;
    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");
        close(iSocketFd);
        free(pcIconfBuf);

        return  (PX_ERROR);
    }

    /*
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");
            free(pcIconfBuf);
            close(iSocketFd);

            return  (PX_ERROR);
        }

        if (IFT_ETHER == pIfreq->ifr_type && (strstr(pIfreq->ifr_name, "en"))) {
            iRet = intadaStatusGetFromDbByName(pIfreq->ifr_name, &iStatus);
            if (ERROR_NONE == iRet && 1 == iStatus) {
                iNetIfNum++;
            } else {
                continue;
            }
        }
    }

    free(pcIconfBuf);
    close(iSocketFd);

    return (iNetIfNum);                                                 /*  ��ȡ�ܵ���������            */
}
/*********************************************************************************************************
** ��������: vndNetIfNumGet
** ��������: ��ȡ������������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ������������
*********************************************************************************************************/
INT  vndNetIfNumGet (VOID)
{
    INT32           iSocketFd;
    struct  ifconf  iconf;
    INT32           iRet, i;
    INT32           iBuffSize;
    INT32           iNetIfNum  = 0;

    struct ifreq   *pIfreq     = LW_NULL;
    PCHAR           pcIconfBuf = LW_NULL;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");

        return  (PX_ERROR);
    }

    /*
     *  ��ȡ��Ҫ���� buf ���
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;
    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");
        close(iSocketFd);
        free(pcIconfBuf);

        return  (PX_ERROR);
    }

    /*
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");
            free(pcIconfBuf);
            close(iSocketFd);

            return  (PX_ERROR);
        }

        if (strstr(pIfreq->ifr_name, "vn")) {
            iNetIfNum++;
        }
    }

    free(pcIconfBuf);
    close(iSocketFd);

    return (iNetIfNum);                                                 /*  ��ȡ�ܵ���������            */
}
/*********************************************************************************************************
** ��������: netIfInfoSet
** ��������: ����������Ϣ(ifparam.ini �������õ�����)
** �䡡��  : pNetifSetInfo      �������ò����ṹ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT32 netIfInfoSet (__PNETIF_SET   pNetifSetInfo)
{
    CHAR             cCmd[50]     = {0};
    __IFPARAM_INFO   ifParamInfo;
    INT32            iRet;
    struct netif    *pNetif;
    struct netdev   *pNetDev;

    INT32            i;

    if (LW_NULL == pNetifSetInfo) {
        return  (PX_ERROR);
    }

#if 0
    struct ifreq     ifreqInfo;
    INT32            iSocketFd;
    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        fprintf(stderr, "socket error.\n");

        return  (PX_ERROR);
    }

    bzero(&ifreqInfo, sizeof(ifreqInfo));
    strncpy(ifreqInfo.ifr_name, pNetifSetInfo->cNetifName, sizeof(ifreqInfo.ifr_name));
    ((struct sockaddr_in *)(&ifreqInfo.ifr_addr))->sin_family      = AF_INET;
    ((struct sockaddr_in *)(&ifreqInfo.ifr_addr))->sin_addr.s_addr = inet_addr(pNetifSetInfo->cIpAddr);

    /*
     *  ���������� IP ��ַ
     */
    iRet = ioctl(iSocketFd, SIOCSIFADDR, (void *)&ifreqInfo);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "ioctl SIOCSIFADDR error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    /*
     *  ���������������ַ
     */
    ((struct sockaddr_in *)(&ifreqInfo.ifr_netmask))->sin_family   = AF_INET;
    ((struct sockaddr_in *)
     (&ifreqInfo.ifr_netmask))->sin_addr.s_addr =  inet_addr(pNetifSetInfo->cNetMask);
    iRet = ioctl(iSocketFd, SIOCSIFNETMASK, (void *)&ifreqInfo);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "ioctl SIOCSIFNETMASK error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    /*
     *  todo: ������������
     */
#endif

    /*
     *  ���� ip��netmask��gw
     */
    for (i = 0; i < 3; i++) {
        bzero(cCmd, sizeof(cCmd));
        switch(i) {
        case 0:
            snprintf(cCmd,
                     sizeof(cCmd),
                     "ifconfig %s inet %s",
                     pNetifSetInfo->cNetifName,
                     pNetifSetInfo->cIpAddr);
            break;

        case 1:
            snprintf(cCmd,
                     sizeof(cCmd),
                     "ifconfig %s netmask %s",
                     pNetifSetInfo->cNetifName,
                     pNetifSetInfo->cNetMask);
            break;

        case 2:
            snprintf(cCmd,
                     sizeof(cCmd),
                     "ifconfig %s gateway %s",
                     pNetifSetInfo->cNetifName,
                     pNetifSetInfo->cGateWay);
            break;

        default:
            break;
        }

        if (PX_ERROR == system(cCmd)) {
            return  (PX_ERROR);
        }
    }

    /*
     *  ����Ϣ���浽ini��
     */
    bzero(&ifParamInfo, sizeof(ifParamInfo));
    ip4addr_aton(pNetifSetInfo->cIpAddr, &(ifParamInfo.uiIpAddr));
    ip4addr_aton(pNetifSetInfo->cNetMask, &(ifParamInfo.uiNetMask));
    ip4addr_aton(pNetifSetInfo->cGateWay, &(ifParamInfo.uiGateWay));

    ifParamInfo.uiUpdateFlag = IF_INI_IP_EN      |
                               IF_INI_NETMASK_EN |
                               IF_INI_GW_EN;
    ifParamInfo.cHandle      = INI_UPDATE_SEC;

    if_list_lock(LW_FALSE);                                             /*  �����ٽ���                  */
    pNetif = netif_find(pNetifSetInfo->cNetifName);
    if (LW_NULL == pNetif) {
#if 0
        close(iSocketFd);
#endif
        if_list_unlock();
        return  (PX_ERROR);
    }
    if_list_unlock();
    pNetDev = net_ip_hook_netif_get_netdev(pNetif);
    strncpy(ifParamInfo.cDevName, pNetDev->dev_name, sizeof(ifParamInfo.cDevName));

    iRet = ifParamIniFileHandle(&ifParamInfo);
#if 0
    close(iSocketFd);
#endif
    return  (iRet);
}

/*********************************************************************************************************
** ��������: __ifconfigInfoGet
** ��������: ��ȡ ifconfig �е�������Ϣ
** �䡡��  : pIfconfigFd        ifconfig �ļ���Ϣ
**           cpcNetifName       ��������
**           pNetifInfo         ������Ϣ�Ľṹ
** �䡡��  : NONE
** ������  : cNetType           ��������
*********************************************************************************************************/
static INT __ifconfigInfoGet (FILE  *pIfconfigFd, PCHAR  pcNetifName, __PNETIF_INFO   pNetifInfo)
{
    CHAR            cFileLineBuff[256]  = {0};
    CHAR            cFileLineInfo[256]  = {0};

    INT32           iLineNum            = 1;
    INT32           iRet;

    if (LW_NULL == pcNetifName || LW_NULL == pNetifInfo || LW_NULL == pIfconfigFd) {
        return  (PX_ERROR);
    }

/*  ifconfig ��ӡ�ṹ
 *  en1       Link encap: Ethernet HWaddr: 00:90:28:00:d8:84
 *            Dev: e1000e_0 Ifidx: 2 DHCP: D4 D6 Spd: 100 Mbps
 *            inet addr: 10.9.0.17 netmask: 255.255.0.0
 *            gateway: 0.0.0.0 broadcast: 10.9.255.255
 *            inet6 addr: fe80::290:28ff:fe00:d884 Scope:link
 *            UP BROADCAST RUNNING MULTICAST MTU:1500 METRIC:1
 *            RX ucast packets:738 nucast packets:85519 dropped:0
 *            TX ucast packets:639 nucast packets:16 dropped:0
 *            RX bytes:5967500 TX bytes:49870
 */

    /*
     *  �� ifconfig ��Ϣ���ҵ���Ӧ������
     */
    while (1) {
        if (feof(pIfconfigFd)) {                                        /*  û�ҵ�                      */
            return  (PX_ERROR);
        }

        bzero(cFileLineBuff, sizeof(cFileLineBuff));
        fgets(cFileLineBuff, sizeof(cFileLineBuff), pIfconfigFd);      /*  ��ȡһ��                     */
        if (!strstr(cFileLineBuff, pcNetifName)) {
            continue;
        } else {
            break;                                                      /*  �ҵ���Ӧ����                */
        }
    }

    strncpy(pNetifInfo->cNetifName, pcNetifName, sizeof(pNetifInfo->cNetifName));
    while (!feof(pIfconfigFd)) {
        switch(iLineNum) {
        case 1:                                                        /*  ��һ�л�ȡ MAC               */
            specifiedLineInfoGet("HWaddr:", LW_NULL, cFileLineBuff, cFileLineInfo);
            sscanf(cFileLineInfo,
                   "%02x:%02x:%02x:%02x:%02x:%02x",
                   (UINT*)&pNetifInfo->ucMac[0],
                   (UINT*)&pNetifInfo->ucMac[1],
                   (UINT*)&pNetifInfo->ucMac[2],
                   (UINT*)&pNetifInfo->ucMac[3],
                   (UINT*)&pNetifInfo->ucMac[4],
                   (UINT*)&pNetifInfo->ucMac[5]);

            break;

        case 2:                                                        /*  �ڶ��л�ȡ DEV SPEED         */
            /*
             *  ��ȡ�豸����
             */
            specifiedLineInfoGet("Dev:", "Ifidx:", cFileLineBuff, cFileLineInfo);
            strncpy(pNetifInfo->cDevName,
                    (CPCHAR)compact_strtrim(cFileLineInfo),
                    sizeof(pNetifInfo->cDevName));

            /*
              *  ��ȡ Speed
              */
            bzero(cFileLineInfo, sizeof(cFileLineInfo));
            iRet = specifiedLineInfoGet("Spd:", "Mbps", cFileLineBuff, cFileLineInfo);
            if (ERROR_NONE == iRet) {                                   /*  ������Ҫ�ж�,����û����Э�� */
                pNetifInfo->uiSpeed = atoi(compact_strtrim(cFileLineInfo));
            }

            break;

        case 3:                                                        /*  �����л�ȡ IP MASK           */
            /*
             *  ��ȡ IP
             */
            specifiedLineInfoGet("inet addr:", "netmask:", cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiIp = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            /*
             *  ��ȡ MASK
             */
            bzero(cFileLineInfo, sizeof(cFileLineInfo));
            specifiedLineInfoGet("netmask:", LW_NULL, cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiNetMask = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            break;

        case 4:                                                        /*  �����л�ȡ GW                */
            /*
             *  ��ȡ GW
             */
            specifiedLineInfoGet("gateway:", " broadcast:", cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiGateWay = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            break;

        case 5:                                                        /*  �����л�ȡ����״̬           */
            /*
             *  ��ȡ����״̬
             */
            if (strstr(cFileLineBuff, "inet6")) {
                /*
                 *  ��ȡ�� 6 ������ȡ����״̬
                 */
                bzero(cFileLineBuff, sizeof(cFileLineBuff));
                fgets(cFileLineBuff, sizeof(cFileLineBuff), pIfconfigFd);

            }

            if (strstr(cFileLineBuff, "RUNNING")) {
                pNetifInfo->iLinkFlag = IFF_RUNNING;
            }

            break;

        default :
            break;
        }

        bzero(cFileLineBuff, sizeof(cFileLineBuff));
        bzero(cFileLineInfo, sizeof(cFileLineInfo));
        fgets(cFileLineBuff, sizeof(cFileLineBuff), pIfconfigFd);       /*  ��ȡһ��                    */

        if (iLineNum >= 5) {                                            /*  Ŀǰֻ��Ҫ����ô��          */
            break;
        }
        iLineNum++;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __netifInfoGetByWebIndex
** ��������: ���� webindex ��ȡ������Ϣ
** �䡡��  : pNetifInfo         ȫ��������Ϣ
**           iNetIfNum          ��������
**           iWebIndex          web index
** �䡡��  : NONE
** ������  : cNetType           ��������
*********************************************************************************************************/
static PCHAR  __netifInfoGetByWebIndex (PCHAR   pNetifInfo, INT  iNetIfNum, INT  iWebIndex)
{
    __PNETIF_INFO  pNetInfo = LW_NULL;
    INT            iNum     = 0;

    if (LW_NULL == pNetifInfo || 0 == iNetIfNum) {
        return  (LW_NULL);
    }

    pNetInfo = (__PNETIF_INFO)pNetifInfo;

    for(iNum = 0; iNum < iNetIfNum; iNum++) {
        if (iWebIndex == pNetInfo->iWebIndex && strstr(pNetInfo->cNetifName, "en")) {
            return  (PCHAR)pNetInfo;
        } else {
            pNetInfo = pNetInfo + 1;

            continue;
        }
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: netIfInfoGetAll
** ��������: ��ȡ����������Ϣ
** �䡡��  : pcAllNetIfInfoBuff ���ڱ���������Ϣ���ڴ��׵�ַ
**           iBuffLen           Buff ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT32  netIfInfoGetAll (PCHAR   pcAllNetIfInfo, INT32  iBuffLen)
{
    INT32           iSocketFd;
    struct  ifconf  iconf;
    INT32           iRet, i;
    INT32           iBuffSize;
    INT             iNetifNum    = 0;
    INT             iWebIdxMax   = 0;
    INT             iOffset      = 0;
    INT32           iStatus      = 0;

    struct ifreq   *pIfreq       = LW_NULL;
    __PNETIF_INFO   pNetifInfo   = LW_NULL;
    PCHAR           pcIconfBuf   = LW_NULL;
    PCHAR           pcInfoTmp    = LW_NULL;
    PCHAR           pcAllInfoTmp = pcAllNetIfInfo;
    __PNETIF_INFO   pNetIfTmp    = LW_NULL;
    FILE           *pIfconfFd;

    if (LW_NULL == pcAllNetIfInfo || iBuffLen <= 0) {
        return  (PX_ERROR);
    }

    iNetifNum = iBuffLen / sizeof(__NETIF_INFO);

    pcInfoTmp = (PCHAR)malloc(iBuffLen);
    if (NULL == pcInfoTmp) {
        return  (PX_ERROR);
    }

    pIfconfFd = fopen("/tmp/ifconfig", "w+");
    if(LW_NULL == pIfconfFd) {
        goto  __error;
    }

    /*
     *  ��ȡ ifconfig ��Ϣ
     */
    iRet = system("ifconfig >/tmp/ifconfig");
    if(PX_ERROR == iRet) {
        fclose(pIfconfFd);

        goto  __error;
    }

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");
        fclose(pIfconfFd);

        goto  __error;
    }

    /*
     *  ��ȡ��Ҫ���� buf ���
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);
        fclose(pIfconfFd);

        goto  __error;
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);
        fclose(pIfconfFd);

        goto  __error;
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;
    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");

        goto  __return;
    }

    /*
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");

            goto  __return;
        }

        /*
         *  ֻ����̫������ͳ��
         */
        if (IFT_ETHER == pIfreq->ifr_type) {
            if ((pcAllNetIfInfo - pcAllInfoTmp) > iBuffLen) {           /*  �ڴ治��                    */
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "buff  to load netif info is over.\n");

                break;
            }

            if (!strstr(pIfreq->ifr_name, "en") &&
                !strstr(pIfreq->ifr_name, "vn")) {
                continue;
            }

            if (strstr(pIfreq->ifr_name, "en")) {
                iRet = intadaStatusGetFromDbByName(pIfreq->ifr_name, &iStatus);
                if (ERROR_NONE != iRet || 0 == iStatus) {
                    continue;
                }
            }

            pNetifInfo = (__PNETIF_INFO)pcAllNetIfInfo;
            __ifconfigInfoGet(pIfconfFd, pIfreq->ifr_name, pNetifInfo);

            if (strstr(pIfreq->ifr_name, "vn")) {
                __vndDevNameConvertToWebName(pNetifInfo->cDevName, pNetifInfo->cWebName);

                pNetifInfo->iWebIndex = -1;
            } else {
                intAdaptionGetWebName(pNetifInfo->cNetifName,
                                      pNetifInfo->cWebName,
                                      &(pNetifInfo->iWebIndex));        /*  ��ȡ Web ��������Ϣ         */

            }

            pNetifInfo->cNatFunction = __natFuncCheck(pNetifInfo->cNetifName);
            pNetifInfo->cNetType     = __netTypeCheck(pNetifInfo->cNetifName);
            if ((NETIF_WAN    == pNetifInfo->cNatFunction) &&
                (NETIF_NORMAL == pNetifInfo->cNetType)) {
                pNetifInfo->cNetType = NETIF_STATIC;                    /*  ����������Ĭ�Ϸ�ʽ�Ǿ�̬ip  */
            }

            pcAllNetIfInfo += sizeof(__NETIF_INFO);
        }
    }

    /*
     *  ���� webindex ������������
     */
    bzero(pcInfoTmp, iBuffLen);
    for (i = 0; i < iNetifNum; i++) {
        pNetIfTmp = (__PNETIF_INFO)__netifInfoGetByWebIndex(pcAllInfoTmp, iNetifNum, i);
        if (NULL != pNetIfTmp) {
            iOffset = i * sizeof(__NETIF_INFO);

            memcpy(pcInfoTmp + iOffset, pNetIfTmp, sizeof(__NETIF_INFO));
        } else {
            iWebIdxMax = i;

            break;
        }
    }

    if (i == iNetifNum) {
        iWebIdxMax = iNetifNum;
    }

    pNetIfTmp = (__PNETIF_INFO)pcAllInfoTmp;
    for (i = 0; i < iNetifNum; i++) {
        if (iWebIdxMax + i >= iNetifNum) {
            break;
        }

        if (-1 == pNetIfTmp->iWebIndex && strstr(pNetIfTmp->cNetifName, "vn")) {
            iOffset = (iWebIdxMax + i) * sizeof(__NETIF_INFO);

            memcpy(pcInfoTmp + iOffset, pNetIfTmp, sizeof(__NETIF_INFO));
        }

        pNetIfTmp = pNetIfTmp + 1;
    }

    bzero(pcAllInfoTmp, iBuffLen);

    memcpy(pcAllInfoTmp, pcInfoTmp, iBuffLen);

__return:
    close(iSocketFd);
    fclose(pIfconfFd);

    if (NULL != pcIconfBuf) {
        free(pcIconfBuf);
    }

    if (NULL != pcInfoTmp) {
        free(pcInfoTmp);
    }

    return  (iRet);
__error:
    if (NULL != pcInfoTmp) {
        free(pcInfoTmp);
    }

    return  (PX_ERROR);
}
