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
** 文   件   名: interfaceExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 27 日
**
** 描        述: 网卡操作
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
  ifparam.ini 配置文件相关宏
*********************************************************************************************************/
#define IFPARAM_INI_FILE    "/etc/ifparam.ini"                          /*  拨号ini配置文件名称         */
#define INI_FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)     /*  拨号ini配置文件操作权限     */
#define UDIAL_INI_SECTION   "pppoe_"                                    /*  拨号ini配置文件section前缀  */
/*********************************************************************************************************
  ifparam.ini 配置文件内section对应的key
*********************************************************************************************************/
#define IFKEY_ENABLE         "enable"
#define IFKEY_IP             "ipaddr"
#define IFKEY_NETMASK        "netmask"
#define IFKEY_GATEWAY        "gateway"
#define IFKEY_DEFAULT        "default"
#define IFKEY_MAC            "mac"
#define IFKEY_DHCP           "dhcp"
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern INT  if_get_dhcp (const char *ifname);
extern INT  if_get_dhcp6 (const char *ifname);
/*********************************************************************************************************
** 函数名称: ifParamIniFileHandle
** 功能描述: 对 ifparam.ini 配置文件进行处理
** 输　入  : pIfParamInfo       ifparam 信息
** 输　出  : NONE
** 返　回  : NONE
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

    if(-1 == (access(IFPARAM_INI_FILE, F_OK))) {                        /*  不存在这个文件              */
        /*
         *  创建这个文件
         */
        iFd = open(IFPARAM_INI_FILE, O_RDWR | O_CREAT | O_TRUNC, INI_FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }

        close(iFd);
    }

    dict = iniparser_load(IFPARAM_INI_FILE);                            /*  加载配置文件                */
    if (LW_NULL == dict) {
        printf("iniparser_load file error\n");

        return  (PX_ERROR);
    }

    iSectionNum = iniparser_getnsec(dict);                              /*  获取section个数             */

    if (0 != iSectionNum) {                                             /*  判断当前的网口信息是否存在  */
        for (i = 0; i < iSectionNum; i++) {
            pcSectionBuf = (CHAR *)iniparser_getsecname(dict, i);
            if (0 == strcmp(pcSectionBuf, pIfParamInfo->cDevName)) {
                iSectionCnt = i;
                cFlag = 1;                                              /*  有对应信息                  */
                break;
            }
        }
    }

    if (INI_UPDATE_SEC == pIfParamInfo->cHandle) {                      /*  需要添加 / 修改网口         */
        if (0 == cFlag) {                                               /*  之前没有对应的信息          */
            iniparser_freedict(dict);                                   /*  把之前的dictionary先删除    */

            pFileId = fopen(IFPARAM_INI_FILE, "r+");                    /*  r+ 格式打开配置文件         */
            if (LW_NULL == pFileId) {
                printf("Create file failed.\n");

                return  (PX_ERROR);
            }

            fseek(pFileId, 0L, SEEK_END);                               /*  移动到文件末尾              */

            /*
             *  添加一个新的section
             */
            sprintf(cNewSecton, "%s", pIfParamInfo->cDevName);
            fprintf(pFileId, "\n[%s]\n", cNewSecton);                   /*  向文件中写入新 section      */

            fclose (pFileId);

            dict = iniparser_load(IFPARAM_INI_FILE);                      /*  重新获取dictionary          */
            if (LW_NULL == dict) {
                printf("iniparser_load file error\n");

                return  (PX_ERROR);
            }

            iSectionCnt = iSectionNum;                                  /*  设置最新的section的位置     */
        }

        /*
         *  至此，找到对应的 section 后，组装 entry 字符串, 并填充进 dic
         */
        /*
         *  更新使能信息
         */
        if (IF_INI_UP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryEnableBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_ENABLE);

            pcEnableBuf = (pIfParamInfo->cEnable > 0) ? "1" : "0";
            iniparser_set(dict, cEntryEnableBuf, pcEnableBuf);
        }

        /*
         *  更新默认网卡信息
         */
        if (IF_INI_DEFAULT_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryDefaultBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_DEFAULT);

            pcDefaultBuf = (pIfParamInfo->cDefault > 0) ? "1" : "0";
            iniparser_set(dict, cEntryDefaultBuf, pcDefaultBuf);
        }

        /*
         *  更新 IP 信息
         */
        if (IF_INI_IP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryIpAddrBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_IP);
            ip4addr_ntoa_r(&(pIfParamInfo->uiIpAddr), cIpAddrBuf, sizeof(cIpAddrBuf));
            iniparser_set(dict, cEntryIpAddrBuf, cIpAddrBuf);
        }

        /*
         *  更新 MASK 信息
         */
        if (IF_INI_NETMASK_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryNetMaskBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_NETMASK);
            ip4addr_ntoa_r(&(pIfParamInfo->uiNetMask), cNetMaskBuf, sizeof(cNetMaskBuf));
            iniparser_set(dict, cEntryNetMaskBuf, cNetMaskBuf);
        }

        /*
         *  更新 GW 信息
         */
        if (IF_INI_GW_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryGateWayBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_GATEWAY);
            ip4addr_ntoa_r(&(pIfParamInfo->uiGateWay), cGateWayBuf, sizeof(cGateWayBuf));
            iniparser_set(dict, cEntryGateWayBuf, cGateWayBuf);
        }

        /*
         *  更新 MAC 信息
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
         *  更新 DHCP 信息
         */
        if (IF_INI_DHCP_EN & pIfParamInfo->uiUpdateFlag) {
            sprintf(cEntryDhcpBuf, "%s:%s", iniparser_getsecname(dict, iSectionCnt), IFKEY_DHCP);

            pcDhcpBuf = (pIfParamInfo->cDhcp > 0) ? "1" : "0";
            iniparser_set(dict, cEntryDhcpBuf, pcDhcpBuf);
        }

        pFileId = LW_NULL;
        pFileId = fopen(IFPARAM_INI_FILE, "w+");                        /*  清空文件打开                */
        if (LW_NULL == pFileId) {
            printf("Create file failed, ini has cleared.\n");
            iniparser_freedict(dict);                                   /*  释放dictionar               */

            return  (PX_ERROR);
        }

        iniparser_dump_ini(dict, pFileId);                              /*  把dictionar写入ini文件      */

        iniparser_freedict(dict);                                       /*  释放dictionary              */

        fclose (pFileId);

        return  (ERROR_NONE);
    } else {                                                            /*  需要删除信息                */
        if (0 == cFlag) {                                               /*  之前没有对应的拨号信息      */
            iniparser_freedict(dict);                                   /*  释放dictionary              */
            printf("Ini file has no msg.\n");

            return  (PX_ERROR);
        } else {
            pFileId = fopen(IFPARAM_INI_FILE, "w+");                    /*  清空文件打开                */
            if (LW_NULL == pFileId) {
                printf("Create file failed.\n");
                iniparser_freedict(dict);                               /*  释放dictionar               */

                return  (PX_ERROR);
            }

            for (i = 0; i < iSectionNum; i++) {
                if (i != iSectionCnt) {                                 /*  把需要删除的section剔除     */
                    iniparser_dumpsection_ini(dict, iniparser_getsecname(dict, i), pFileId);
                }
            }

            iniparser_freedict(dict);                                   /*  释放dictionary              */

            fclose (pFileId);

            return  (ERROR_NONE);
        }
    }
}

/*********************************************************************************************************
** 函数名称: __netTypeCheck
** 功能描述: 网口功能检测
** 输　入  : cpcNetifName       网口名称
** 输　出  : NONE
** 返　回  : cNetType           网口类型
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
     *  判断是否是拨号网口
     */
    iRet = udialGetSpecInfo(&udialInfo, "NETIF", pcNetifName, LW_NULL);
    if (ERROR_NONE == iRet && udialInfo.uiPid) {                        /*  找到拨号信息并且在拨号状态  */
        return  (NETIF_ADSL);
    }

    /*
     *  判断是否是 DHCP 接入网口
     */
    if (if_get_dhcp(pcNetifName) || if_get_dhcp6(pcNetifName)) {
        return  (NETIF_DHCP);
    }

    return  (NETIF_NORMAL);
}

/*********************************************************************************************************
** 函数名称: __natFuncCheck
** 功能描述: NAT 功能检测
** 输　入  : cpcNetifName       网口名称
** 输　出  : NONE
** 返　回  : iNatFunc           NAT 功能
*********************************************************************************************************/
static INT __natFuncCheck (CPCHAR  cpcNetifName)
{
    INT                    iNatFunc = NETIF_ERROR;
    INT                    i;
    __WAN_INFO             wanInfo;
    __LAN_INFO             lanInfo;

    /*
     *  获取当前wan口和lan口对应的网口
     */
    bzero(&wanInfo, sizeof(wanInfo));
    bzero(&lanInfo, sizeof(lanInfo));
    natWanInfoGet(&wanInfo);
    natLanInfoGet(&lanInfo);

    if (LW_NULL == cpcNetifName) {
        return  (iNatFunc);
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  遍历链表                    */
        if (0 == strcmp(wanInfo.cWanName[i], cpcNetifName)) {
            iNatFunc = NETIF_WAN;
            return  (iNatFunc);
        }
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  遍历链表                    */
        if (0 == strcmp(lanInfo.cLanName[i], cpcNetifName)) {
            iNatFunc = NETIF_LAN;
            return  (iNatFunc);
        }
    }

    return  (NETIF_NORMAL);
}
/*********************************************************************************************************
** 函数名称: __vndDevNameConvertToWebName
** 功能描述: 根据设备名获取 web name
** 输　入  : cpcDevName         网口设备名称
** 输　出  : pcWebName          web name
** 返　回  : ERROR_CODE
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
** 函数名称: netIfNumGet
** 功能描述: 获取所有物理网卡数量
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 网卡个数
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
     *  获取需要多大的 buf 存放
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
     *  遍历所有的网口信息
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  获取所有的网卡的类型        */
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

    return (iNetIfNum);                                                 /*  获取总的网卡个数            */
}
/*********************************************************************************************************
** 函数名称: vndNetIfNumGet
** 功能描述: 获取虚拟网卡数量
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 虚拟网卡个数
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
     *  获取需要多大的 buf 存放
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
     *  遍历所有的网口信息
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  获取所有的网卡的类型        */
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

    return (iNetIfNum);                                                 /*  获取总的网卡个数            */
}
/*********************************************************************************************************
** 函数名称: netIfInfoSet
** 功能描述: 设置网口信息(ifparam.ini 可以设置的内容)
** 输　入  : pNetifSetInfo      网口配置参数结构
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  设置网卡的 IP 地址
     */
    iRet = ioctl(iSocketFd, SIOCSIFADDR, (void *)&ifreqInfo);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "ioctl SIOCSIFADDR error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    /*
     *  设置网卡的掩码地址
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
     *  todo: 设置网卡网关
     */
#endif

    /*
     *  配置 ip、netmask、gw
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
     *  将信息保存到ini中
     */
    bzero(&ifParamInfo, sizeof(ifParamInfo));
    ip4addr_aton(pNetifSetInfo->cIpAddr, &(ifParamInfo.uiIpAddr));
    ip4addr_aton(pNetifSetInfo->cNetMask, &(ifParamInfo.uiNetMask));
    ip4addr_aton(pNetifSetInfo->cGateWay, &(ifParamInfo.uiGateWay));

    ifParamInfo.uiUpdateFlag = IF_INI_IP_EN      |
                               IF_INI_NETMASK_EN |
                               IF_INI_GW_EN;
    ifParamInfo.cHandle      = INI_UPDATE_SEC;

    if_list_lock(LW_FALSE);                                             /*  进入临界区                  */
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
** 函数名称: __ifconfigInfoGet
** 功能描述: 获取 ifconfig 中的配置信息
** 输　入  : pIfconfigFd        ifconfig 文件信息
**           cpcNetifName       网口名称
**           pNetifInfo         保存信息的结构
** 输　出  : NONE
** 返　回  : cNetType           网口类型
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

/*  ifconfig 打印结构
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
     *  从 ifconfig 信息中找到对应的网卡
     */
    while (1) {
        if (feof(pIfconfigFd)) {                                        /*  没找到                      */
            return  (PX_ERROR);
        }

        bzero(cFileLineBuff, sizeof(cFileLineBuff));
        fgets(cFileLineBuff, sizeof(cFileLineBuff), pIfconfigFd);      /*  读取一行                     */
        if (!strstr(cFileLineBuff, pcNetifName)) {
            continue;
        } else {
            break;                                                      /*  找到对应内容                */
        }
    }

    strncpy(pNetifInfo->cNetifName, pcNetifName, sizeof(pNetifInfo->cNetifName));
    while (!feof(pIfconfigFd)) {
        switch(iLineNum) {
        case 1:                                                        /*  第一行获取 MAC               */
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

        case 2:                                                        /*  第二行获取 DEV SPEED         */
            /*
             *  获取设备名称
             */
            specifiedLineInfoGet("Dev:", "Ifidx:", cFileLineBuff, cFileLineInfo);
            strncpy(pNetifInfo->cDevName,
                    (CPCHAR)compact_strtrim(cFileLineInfo),
                    sizeof(pNetifInfo->cDevName));

            /*
              *  获取 Speed
              */
            bzero(cFileLineInfo, sizeof(cFileLineInfo));
            iRet = specifiedLineInfoGet("Spd:", "Mbps", cFileLineBuff, cFileLineInfo);
            if (ERROR_NONE == iRet) {                                   /*  这里需要判断,可能没有自协商 */
                pNetifInfo->uiSpeed = atoi(compact_strtrim(cFileLineInfo));
            }

            break;

        case 3:                                                        /*  第三行获取 IP MASK           */
            /*
             *  获取 IP
             */
            specifiedLineInfoGet("inet addr:", "netmask:", cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiIp = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            /*
             *  获取 MASK
             */
            bzero(cFileLineInfo, sizeof(cFileLineInfo));
            specifiedLineInfoGet("netmask:", LW_NULL, cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiNetMask = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            break;

        case 4:                                                        /*  第四行获取 GW                */
            /*
             *  获取 GW
             */
            specifiedLineInfoGet("gateway:", " broadcast:", cFileLineBuff, cFileLineInfo);
            pNetifInfo->uiGateWay = inet_addr((CPCHAR)compact_strtrim(cFileLineInfo));

            break;

        case 5:                                                        /*  第五行获取连接状态           */
            /*
             *  获取连接状态
             */
            if (strstr(cFileLineBuff, "inet6")) {
                /*
                 *  读取第 6 行来获取连接状态
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
        fgets(cFileLineBuff, sizeof(cFileLineBuff), pIfconfigFd);       /*  读取一行                    */

        if (iLineNum >= 5) {                                            /*  目前只需要读这么多          */
            break;
        }
        iLineNum++;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __netifInfoGetByWebIndex
** 功能描述: 根据 webindex 获取网口信息
** 输　入  : pNetifInfo         全部网口信息
**           iNetIfNum          网口数量
**           iWebIndex          web index
** 输　出  : NONE
** 返　回  : cNetType           网口类型
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
** 函数名称: netIfInfoGetAll
** 功能描述: 获取所有网卡信息
** 输　入  : pcAllNetIfInfoBuff 用于保存网口信息的内存首地址
**           iBuffLen           Buff 长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  读取 ifconfig 信息
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
     *  获取需要多大的 buf 存放
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
     *  遍历所有的网口信息
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  获取所有的网卡的类型        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");

            goto  __return;
        }

        /*
         *  只对以太网卡做统计
         */
        if (IFT_ETHER == pIfreq->ifr_type) {
            if ((pcAllNetIfInfo - pcAllInfoTmp) > iBuffLen) {           /*  内存不够                    */
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
                                      &(pNetifInfo->iWebIndex));        /*  获取 Web 网卡的信息         */

            }

            pNetifInfo->cNatFunction = __natFuncCheck(pNetifInfo->cNetifName);
            pNetifInfo->cNetType     = __netTypeCheck(pNetifInfo->cNetifName);
            if ((NETIF_WAN    == pNetifInfo->cNatFunction) &&
                (NETIF_NORMAL == pNetifInfo->cNetType)) {
                pNetifInfo->cNetType = NETIF_STATIC;                    /*  接入外网的默认方式是静态ip  */
            }

            pcAllNetIfInfo += sizeof(__NETIF_INFO);
        }
    }

    /*
     *  根据 webindex 进行网口排序
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
