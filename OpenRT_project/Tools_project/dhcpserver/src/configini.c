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
** 文   件   名: configini.c
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年3月14日
**
** 描        述:
*********************************************************************************************************/
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <SylixOS.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <iniparser/iniparser.h>


#include "log.h"
#include "packet.h"
#include "configini.h"

#define IPADDRPOOL             "ipaddrpool"                             /* 地址池标签字符串             */
#define FILEPATH               "filepath"                               /* 文件路径标签字符串           */
#define NETWORK                "network"                                /* 网络部分标签字符串           */
#define OPT                    "opt"                                    /* 选项标签字符串               */
#define ININULL                "null"                                   /* ini配置空                    */
#define IPSTART                "start"                                  /* 地址池起始地址键值匹配字符串 */
#define IPEND                  "end"                                    /* 地址池结束地址键值匹配字符串 */
#define LEASEPATH              "leasefile"                              /* 租约文件键值匹配字符串       */
#define LOGFILE                "logfile"                                /* 日志文件键值匹配字符串       */
#define INTERFACE              "interface"                              /* 网卡名键值匹配字符串         */
#define DNSOPT1                "dns1"                                   /* dns1键值匹配字符串           */
#define DNSOPT2                "dns2"                                   /* dns2键值匹配字符串           */
#define SUBNETOPT              "subnet"                                 /* 子网掩码键值匹配字符串       */
#define ROUTEROPT              "router"                                 /* 默认网关键值匹配字符串       */
#define DOMAINOPT              "domain"                                 /* 域名键值匹配字符串           */
#define LEASEOPT               "lease"                                  /* 租约时间键值匹配字符串       */
#define LEASET1                "t1"                                     /* 租约时间50%键值匹配字符串    */
#define LEASET2                "t2"                                     /* 租约时间87.5%键值匹配字符串  */
#define SECANDKEY              "%s:%s"                                  /* 标签键值拼接使用字符串       */
#define SECBUFFSIZE            20                                       /* 标签键值拼接最大长度         */

#define ISNOTIP                0                                        /* 非IP                         */
#define IPVAILD                1                                        /* IP值错误                     */
#define IPOK                   2                                        /* IP值正确                     */

#define MAC_FMT                "Server Mac:    %02x:%02x:%02x:%02x:%02x:%02x \r\n"
#define addr_cmp(addr1, addr2, mask) ((addr1 & mask) == (addr2 & mask)) /* 判断地址是否在同一网段       */
#define SIGMASK                0xffffff00                               /* 255.255.255.0掩码            */

struct serverconfig  GserverConfig;                                     /* 服务器配置全局变量           */

/*********************************************************************************************************
** 函数名称: poptionFind
** 功能描述: 查找选项地址
** 输    入: poptonsetlist   选项参数列表
**           cType           选项类型
** 输    出: 返回选项地址
** 调用模块: NONE
*********************************************************************************************************/
static struct optionset  *poptionFind (struct optionset  *poptonsetlist, CHAR  cType)
{
    while (poptonsetlist && poptonsetlist->pcData[OPT_CODE] < cType) {  /* 查找到选项列表中对应类型位置 */
        poptonsetlist = poptonsetlist->optionsetNext;
    }

    if (poptonsetlist && poptonsetlist->pcData[OPT_CODE] == cType) {    /* 判断是查找结果类型是否正确   */
        return (poptonsetlist);                                         /* 返回选项地址                 */
    } else {
        return (NULL);                                                  /* 返回空                       */
    }
}

/*********************************************************************************************************
** 函数名称: optionAttach
** 功能描述: 插入选项
** 输    入: **ppoptionsetList    选项参数列表
**            *pdhcpoption        选项默认值结构体
**             pcbuffer           选项数据buffer
**             iLength            选项长度
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static void  optionAttach (struct optionset  **ppoptionsetList,
                           struct dhcpoption  *pdhcpoption,
                           PCHAR               pcbuffer,
                           INT                 iLength)
{
    struct optionset   *poptionsetExit;
    struct optionset   *poptionsetNew;
    struct optionset  **ppoptionsetCurr;

    /*
     *  仅有list类型的选项可能需要追加，其余选项类型都是一次性添加
     */
    if ((poptionsetExit = poptionFind(*ppoptionsetList,                 /* 查找到对应类型的选项位置     */
                                      pdhcpoption->ucCode))) {
        if (pdhcpoption->cFlags & OPTION_LIST) {                        /* 判断是否是list类型的选项     */
            if (poptionsetExit->pcData[OPT_LEN] + iLength <= 255) {     /* 判断选项是否超长             */
                poptionsetExit->pcData = realloc(poptionsetExit->pcData,/* 调整内存分配大小             */
                                                 poptionsetExit->pcData[OPT_LEN] + iLength + 2);
                memcpy(poptionsetExit->pcData + poptionsetExit->pcData[OPT_LEN] + 2,
                       pcbuffer,                                        /* 追加选项新内容               */
                       iLength);
                poptionsetExit->pcData[OPT_LEN] += iLength;             /* 重设选项数据长度参数         */
            }
        }
    } else {
        poptionsetNew                   = (struct optionset  *)malloc(sizeof(struct optionset));
        poptionsetNew->pcData           = malloc(iLength + 2);          /* 分配选项内存                 */
        poptionsetNew->pcData[OPT_CODE] = pdhcpoption->ucCode;          /* 填入选项类型                 */
        poptionsetNew->pcData[OPT_LEN]  = iLength;                      /* 填入选项数据长度             */
        memcpy(poptionsetNew->pcData + 2, pcbuffer, iLength);           /* 拷贝选项数据到对应地址       */

        ppoptionsetCurr = ppoptionsetList;                              /* 将新选项插入选项链表         */
        while (*ppoptionsetCurr && (*ppoptionsetCurr)->pcData[OPT_CODE] < pdhcpoption->ucCode) {
            ppoptionsetCurr = &(*ppoptionsetCurr)->optionsetNext;
        }
        poptionsetNew->optionsetNext = *ppoptionsetCurr;
        *ppoptionsetCurr             =  poptionsetNew;
    }
}

/*********************************************************************************************************
** 函数名称: isIpValid
** 功能描述: 判断IP地址有效性
** 输    入: cpcIp  ip字符串指针
** 输    出: 不是IP地址返回ISNOTIP，IP地址无效返回IPVAILD，IP地址正常返回IPOK
*********************************************************************************************************/
static INT  isIpValid (CPCHAR  cpcIp)
{
    INT  i         = 0;
    INT  j         = 0;
    INT  iIpSeg[4] = {0, 0, 0, 0};
    INT  iLen      = strlen(cpcIp);
    INT  iTmp      = 0;

    for (i = 0; i < iLen + 1; i++) {
        if (cpcIp[i] == '.' || i == iLen) {                             /* 处理IP地址分配符             */
            if (iIpSeg[j] >= 0 && iIpSeg[j] <= 255) {                   /* 处理IP地址范围               */
                j ++;
            } else {
                return (ISNOTIP);                                       /* 不在IP数值范围则返回ISNOTIP  */
            }
        } else {
            iTmp = cpcIp[i] - '0';                                      /* IP字符串转为整形             */
            if (iTmp > 9 || iTmp < 0) {
                return (ISNOTIP);                                       /* 非数值字符串返回ISNOTIP      */
            } else {
                iIpSeg[j] = iIpSeg[j] * 10 + iTmp;                      /* 整形值处理                   */
            }
        }
    }

    if (j != 4) {                                                       /* 判断IP地址段数是否正常       */
        return (ISNOTIP);
    }

    if (iIpSeg[0] > 0   &&                                              /* 判断IP有效性                 */
        iIpSeg[0] < 255 &&
        iIpSeg[3] > 0   &&
        iIpSeg[3] < 255) {
        return (IPOK);                                                  /* 有效返回IPOK                 */
    } else if (cpcIp[0] == 0) {
        return (ISNOTIP);                                               /* 非IP返回ISNOTIP              */
    }

    return (IPVAILD);
}

/*********************************************************************************************************
** 函数名称: isMaskValid
** 功能描述: 判断子网掩码有效性
** 输    入: cpcMask  子网掩码字符串指针
** 输    出: 是子网掩码返回ERROR_NONE,否则返回PX_ERROR
*********************************************************************************************************/
static INT  isMaskValid (CPCHAR  cpcMask)
{
    UINT  uiIpVal  = 0;
    UINT  uiTmp[4] = {0, 0, 0, 0};
    INT   i        = 0;

    if (isIpValid(cpcMask) == IPVAILD) {
        sscanf(cpcMask,
               "%u.%u.%u.%u",
               &uiTmp[3],
               &uiTmp[2],
               &uiTmp[1],
               &uiTmp[0]);

        for (i = 0; i < 4; i++) {                                       /* 子网掩码-->32位无符号整型    */
            uiIpVal += uiTmp[i] << (i * 8);
        }

        uiIpVal = ~uiIpVal + 1;                                         /*   判断掩码是否为2的整数幂    */
        if ((uiIpVal & (uiIpVal - 1)) == 0) {
            return (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: optHandle
** 功能描述: 插入选项
** 输    入: cpcType               选项配置类型
**           cpcBuf                标签键拼接后的字符串（e.g "opt：dns"）
**           pdictionaryConfig     ini对象指针
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  optHandle (CPCHAR  cpcType, CPCHAR  cpcBuf, dictionary  *pdictionaryConfig)
{
    CPCHAR           cpcConfig         = NULL;
    INT              iLen;
    ULONG            ulLeaseTmp;
    struct in_addr   inaddr;

    if (strcmp(cpcType, DNSOPT1) == 0) {                                /* 判断是否是dns1选项           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* 获取DNSOPT1选项的字符串      */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* 判断是否成功获取             */
            return (PX_ERROR);
        }

        if (inet_aton(cpcConfig, &inaddr)) {                            /* 转换字符串为in_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[5],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, DNSOPT2)   == 0) {                       /* 判断是否是dns2选项           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* 获取DNSOPT2选项的字符串      */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* 判断是否成功获取             */
            return (PX_ERROR);
        }

        if (inet_aton(cpcConfig, &inaddr)) {                            /* 转换字符串为in_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[5],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, SUBNETOPT) == 0) {                       /* 判断是否是子网掩码选项       */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* 获取SUBNETOPT选项的字符串    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* 判断是否成功获取             */
            return (PX_ERROR);
        }

        if (isMaskValid(cpcConfig) == ERROR_NONE &&
            inet_aton(cpcConfig, &inaddr)) {                            /* 转换字符串为in_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[0],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, ROUTEROPT) == 0) {                       /* 判断是否是默认网关选项       */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* 获取ROUTEROPT选项的字符串    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* 判断是否成功获取             */
            return (PX_ERROR);
        }

        if (isIpValid(cpcConfig) == IPOK &&
            inet_aton(cpcConfig, &inaddr)) {                            /* 转换字符串为in_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[2],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, DOMAINOPT) == 0) {                       /* 判断是否是域名选项           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* 获取DOMAINOPT选项的字符串    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* 判断是否成功获取             */
            return (PX_ERROR);
        }

        iLen = strlen(cpcConfig);                                       /* 获取选项字符串长度           */
        if (iLen > 0) {                                                 /* 限长254                      */
            if (iLen > 254) {
                iLen = 254;
            }
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[11],
                         (PCHAR)cpcConfig,
                         iLen);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, LEASEOPT)  == 0) {                       /* 判断是否是租约时间选项       */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* 获取LEASEOPT选项数值         */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* 判断数值有效性               */
            GserverConfig.ulLease = ulLeaseTmp;                         /* 赋值租约时间到全局配置       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* 转换数值为网络字节序         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[20],
                         (PCHAR)&ulLeaseTmp,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, LEASET1)   == 0) {                       /* 判断是否是第一次检查时间选项 */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* 获取LEASET1选项数值          */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* 判断数值有效性               */
        	GserverConfig.ulT1 = ulLeaseTmp;                            /* 赋值租约时间到全局配置       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* 转换数值为网络字节序         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[21],
                         (PCHAR)&ulLeaseTmp,
                         4);
        }

    } else if (strcmp(cpcType, LEASET2)   == 0) {                       /* 判断是否是第二次检查时间选项 */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* 获取LEASET2选项数值          */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* 判断数值有效性               */
        	GserverConfig.ulT1 = ulLeaseTmp;                            /* 赋值租约时间到全局配置       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* 转换数值为网络字节序         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* 将相关选项插入选项列表       */
                         &Gdhcpopts[22],
                         (PCHAR)&ulLeaseTmp,
                         4);
        }
    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: optHandle
** 功能描述: 插入选项
** 输    入: pdictionaryConfig     ini对象指针
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static INT  readOpt (dictionary  *pdictionaryConfig)
{
    CHAR   *cOptKeyWord[]     = {DNSOPT1,                               /* 选项匹配键值字符串数组      */
                                 DNSOPT2,
                                 SUBNETOPT,
                                 ROUTEROPT,
                                 DOMAINOPT,
                                 LEASEOPT,
                                 LEASET1,
                                 LEASET2};
    INT     i;
    CHAR    cBuf[SECBUFFSIZE] = {0};

    if (iniparser_find_entry(pdictionaryConfig, OPT)) {                 /* 查找是否含有有OPT选项节      */
        for (i = 0; i < sizeof(cOptKeyWord)/sizeof(CHAR *); i++) {
            snprintf(cBuf, SECBUFFSIZE, SECANDKEY, OPT, cOptKeyWord[i]);/* 拼接选项节和键e.g("opt：dns")*/
            if (iniparser_find_entry(pdictionaryConfig, cBuf)) {        /* 判断是否存在对应的匹配键值   */
                if (optHandle(cOptKeyWord[i],                           /* 进行选项处理                 */
                              cBuf,
                              pdictionaryConfig) != ERROR_NONE) {
                    return (PX_ERROR);
                }

            } else {
                if (i == 0 || i == 1) {                                 /* DNS如果没有配置，则继续      */
                    continue;
                }
                return (PX_ERROR);
            }
        }

    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);                                                /* 配置选项成功                 */
}

/*********************************************************************************************************
** 函数名称: readNetwork
** 功能描述: 读取网卡名配置
** 输    入: pdictionaryConfig   INI配置文件字典
** 输    出: 成功返回ERROR，失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  readNetwork (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;

    if (iniparser_find_entry(pdictionaryConfig, NETWORK)) {             /* 判断是否存在网络相关配置     */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, NETWORK, INTERFACE);     /* 拼接选项节和键e.g("opt：dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* 判断是否存在INTERFACE的键值  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* 取得INTERFACE的配置字符串    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* 判断是否读取成功             */
                return (PX_ERROR);
            }

            if (GserverConfig.pcInteface) {                             /* 为下一步strdup准备指针       */
                free(GserverConfig.pcInteface);
            }

            GserverConfig.pcInteface = strdup(cpcConfig);               /* 字符串赋值                   */
            if (GserverConfig.pcInteface) {
                return (ERROR_NONE);                                    /* 网卡名配置成功               */
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: readFilepath
** 功能描述: 读取租约文件地址配置，返回错误，进程会退出
** 输    入: pdictionaryConfig   INI配置文件字典
** 输    出: 成功返回ERROR，失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  readFilepath (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;

    if (iniparser_find_entry(pdictionaryConfig, FILEPATH)) {            /* 判断是否存在文件路径相关配置 */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, FILEPATH, LEASEPATH);    /* 拼接选项节和键e.g("opt：dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* 判断是否存在LEASEPATH的键值  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* 取得LEASEPATH的配置字符串    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* 判断是否读取成功             */
                return (PX_ERROR);
            }
            if (GserverConfig.pcLeasefile) {
                free(GserverConfig.pcLeasefile);
            }

            GserverConfig.pcLeasefile    = strdup(cpcConfig);           /* 字符串赋值                   */
        }

        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, FILEPATH, LOGFILE);     /* 拼接选项节和键e.g("opt：dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* 判断是否存在LEASEPATH的键值  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* 取得LEASEPATH的配置字符串    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* 判断是否读取成功             */
                return (PX_ERROR);
            }
            if (GserverConfig.pclogfile) {
                free(GserverConfig.pclogfile);
            }

            GserverConfig.pclogfile    = strdup(cpcConfig);           /* 字符串赋值                   */
        }

        GserverConfig.pcLeasefilebak = (CHAR *)malloc(strlen(GserverConfig.pcLeasefile) + sizeof(FILEBAK));
        if (GserverConfig.pcLeasefile && GserverConfig.pcLeasefilebak) {
            snprintf(GserverConfig.pcLeasefilebak,
                     strlen(GserverConfig.pcLeasefile) + sizeof(FILEBAK),
                     "%s%s",
                     GserverConfig.pcLeasefile,
                     FILEBAK);
            return (ERROR_NONE);                                    /* 租约地址配置成功             */
        }
   }
   return (PX_ERROR);

}

/*********************************************************************************************************
** 函数名称: readIpaddrpool
** 功能描述: 读取地址池配置
** 输    入: pdictionaryConfig   INI配置文件字典
** 输    出: 成功返回ERROR，失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  readIpaddrpool (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;
    struct in_addr  inaddr;

    if (iniparser_find_entry(pdictionaryConfig, IPADDRPOOL)) {          /* 判断是否存在地址池相关配置   */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, IPADDRPOOL, IPSTART);    /* 拼接选项节和键e.g("opt：dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* 判断是否存在IPSTART的键值    */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* 取得IPSTART的配置字符串      */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* 判断是否读取成功             */
                return (PX_ERROR);
            }

            if (isIpValid(cpcConfig) == IPOK &&
                inet_aton(cpcConfig, &inaddr)) {                        /* 转换字符串为整形地址         */
                GserverConfig.uiStart = inaddr.s_addr;                  /* 赋值全局配置结构体           */

                memset(cBuf, 0, sizeof(cBuf));                          /* 清空数组buff                 */
                snprintf(cBuf, SECBUFFSIZE, SECANDKEY, IPADDRPOOL, IPEND);
                if (iniparser_find_entry(pdictionaryConfig, cBuf)) {    /* 判断是否存在IPEND的键值      */
                    cpcConfig = iniparser_getstring(pdictionaryConfig,  /* 取得IPEND的配置字符串        */
                                                     cBuf,
                                                     ININULL);
                    if (strcmp(cpcConfig, ININULL) == 0) {              /* 判断是否读取成功             */
                        return (PX_ERROR);
                    }

                    if (isIpValid(cpcConfig) == IPOK &&
                        inet_aton(cpcConfig, &inaddr)) {                /* 转换字符串为整形地址         */
                        GserverConfig.uiEnd = inaddr.s_addr;            /* 赋值全局配置结构体           */
                        if (addr_cmp(ntohl(GserverConfig.uiStart),      /* 判断是否在同一网段           */
                                     ntohl(GserverConfig.uiEnd),
                                     SIGMASK) &&
                            ntohl(GserverConfig.uiStart) < ntohl(GserverConfig.uiEnd)) {
                            return (ERROR_NONE);                        /* 地址池配置成功               */
                        }
                    }
                }
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: localIpGet
** 功能描述: 获取本地IP地址
** 输    入: iSocketFd  用于获取信息的套接字
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  localIpGet (INT  iSocketFd)
{
    INT           iRet;
    struct ifreq  ifreqIp;

    memset(&ifreqIp, 0, sizeof(ifreqIp));
    strncpy(ifreqIp.ifr_name,                                           /* 拷贝配置的网卡名到ifreq结构体*/
            GserverConfig.pcInteface,
            sizeof(ifreqIp.ifr_name) - 1);

    iRet = ioctl(iSocketFd, SIOCGIFADDR, &ifreqIp);                     /* 获取对应网卡名的IP地址       */
    if (iRet == ERROR_NONE) {                                           /* 赋值到全局配置结构体中       */
        GserverConfig.uiServer = ((struct sockaddr_in *)&ifreqIp.ifr_ifru.ifru_addr)->sin_addr.s_addr;
    }

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: localMacGet
** 功能描述: 获取本地MAC地址
** 输    入: INT  iSocketFd
** 输    出: -1 表示获取失败
** 调用模块: API
*********************************************************************************************************/
static INT  localMacGet (INT  iSocketFd)
{
    INT           iRet;
    struct ifreq  ifreqMac;

    memset(&ifreqMac, 0, sizeof(ifreqMac));
    strncpy(ifreqMac.ifr_name,                                          /* 拷贝配置的网卡名到ifreq结构体*/
            GserverConfig.pcInteface,
            sizeof(ifreqMac.ifr_name) - 1);

    iRet = ioctl(iSocketFd, SIOCGIFHWADDR, &ifreqMac);                  /* 获取对应网卡名的MAC地址      */
    if (iRet == ERROR_NONE) {
        memcpy(GserverConfig.ucMac,                                     /* 赋值到全局配置结构体中       */
               ifreqMac.ifr_ifru.ifru_hwaddr.sa_data,
               6);
    }

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: localNetinfoGet
** 功能描述: 获取本地Net相关信息
** 输    入: NONE
** 输    出: 失败返回PX_ERROR，成功返回ERROR_NONE
** 调用模块: NONE
*********************************************************************************************************/
static INT  localNetinfoGet (VOID)
{
    INT  iSocketFd;
    INT  iRet;

    iSocketFd = socket(AF_INET, SOCK_STREAM, 0);                        /* 打开套接字                   */
    if (iSocketFd < 0) {
        LOG(LOG_ERR, "Get loclnetinfo failed!\r\n");
        return (PX_ERROR);
    }

    iRet = localMacGet(iSocketFd);                                      /* 获取本地MAC                  */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Get localMac failed!\r\n");
        close(iSocketFd);
        return (PX_ERROR);
    }

    iRet = localIpGet(iSocketFd);                                       /* 获取本地IP                   */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Get localIp failed!\r\n");
        close(iSocketFd);
        return (PX_ERROR);
    }

    close(iSocketFd);                                                   /* 关闭套接字                   */
    return (ERROR_NONE);                                                /* 获取成功                     */
}

/*********************************************************************************************************
** 函数名称: config_workpath_update
** 功能描述: 对和路径相关的配置，增加网口名路径更新和建立目的文件夹
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static void config_workpath_update(void)
{
    CHAR *ptr = NULL;

    ptr = malloc(strlen(GserverConfig.pcLeasefile) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pcLeasefile,GserverConfig.pcInteface);
        free(GserverConfig.pcLeasefile);
        GserverConfig.pcLeasefile = ptr;
        /*
         *  通过当前的租约文件，进行文件夹建立
         */
        ptr  = strrchr(GserverConfig.pcLeasefile, '/');
        *ptr = 0;
        mkdir(GserverConfig.pcLeasefile, S_IRWXU|S_IRGRP|S_IROTH);
        *ptr = '/';
    }

    ptr = malloc(strlen(GserverConfig.pclogfile) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pclogfile,GserverConfig.pcInteface);
        free(GserverConfig.pclogfile);
        GserverConfig.pclogfile = ptr;
    }

    ptr = malloc(strlen(GserverConfig.pcLeasefilebak) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pcLeasefilebak,GserverConfig.pcInteface);
        free(GserverConfig.pcLeasefilebak);
        GserverConfig.pcLeasefilebak = ptr;
    }
}

/*********************************************************************************************************
** 函数名称: configPrint
** 功能描述: 在日志上打印当前的配置文件
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static VOID  configPrint (VOID)
{
    LOG(LOG_INFO, "Server Device: %s\r\n", GserverConfig.pcInteface);
    LOG(LOG_INFO, "Server IP:     %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiServer)));
    LOG(LOG_INFO, MAC_FMT, (UCHAR)GserverConfig.ucMac[0],
                           (UCHAR)GserverConfig.ucMac[1],
                           (UCHAR)GserverConfig.ucMac[2],
                           (UCHAR)GserverConfig.ucMac[3],
                           (UCHAR)GserverConfig.ucMac[4],
                           (UCHAR)GserverConfig.ucMac[5]);
    LOG(LOG_INFO, "Ip pool start: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiStart)));
    LOG(LOG_INFO, "Ip pool end:   %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiEnd)));
    LOG(LOG_INFO, "Leasfile path: %s\r\n", GserverConfig.pcLeasefile);
    LOG(LOG_INFO, "Logfile path: %s\r\n", GserverConfig.pclogfile);
    LOG(LOG_INFO, "Lease time:    %d\r\n", GserverConfig.ulLease);
}
/*********************************************************************************************************
** 函数名称: configRead
** 功能描述: 读取本地配置文件
** 输    入: NONE
** 输    出: 失败返回PX_ERROR，成功返回ERROR_NONE
** 调用模块: NONE
*********************************************************************************************************/
INT  configRead (VOID *configfile)
{
    dictionary        *pdictionaryConfig;

    GserverConfig.pcInteface  = NULL;                                   /* 防野指针                     */
    GserverConfig.pcLeasefile = NULL;
    GserverConfig.poptionsets = NULL;

    pdictionaryConfig = iniparser_load(configfile);                /* 加载ini配置文件              */
    if (!pdictionaryConfig) {
        return (PX_ERROR);
    }

    if (readIpaddrpool(pdictionaryConfig) == PX_ERROR) {                /* 读取地址池配置信息           */
        iniparser_freedict(pdictionaryConfig);                          /* 释放ini对象内存              */
        return (PX_ERROR);
    }

    if (readFilepath(pdictionaryConfig) == PX_ERROR) {                  /* 读取租约文件路径配置信息     */
        iniparser_freedict(pdictionaryConfig);                          /* 释放ini对象内存              */
        return (PX_ERROR);
    }

    if (readNetwork(pdictionaryConfig) == PX_ERROR) {                    /* 读取网卡相关配置信息        */
        iniparser_freedict(pdictionaryConfig);                           /* 释放ini对象内存             */
        return (PX_ERROR);
    }

    if (readOpt(pdictionaryConfig) == PX_ERROR) {                        /* 读取选项配置信息            */
        iniparser_freedict(pdictionaryConfig);                           /* 释放ini对象内存             */
        return (PX_ERROR);
    }

    iniparser_freedict(pdictionaryConfig);                              /* 释放ini对象内存              */

    GserverConfig.ulOfferWaite = OFFERWAITE;                            /* 配置OFFER等待时间            */

    if (localNetinfoGet() == (PX_ERROR)) {                              /* 配置本地网络信息             */
        return (PX_ERROR);
    }

    config_workpath_update();                                           /* 带网口名的工作路径更新       */

    configPrint();

    return (ERROR_NONE);                                                /* 配置成功                     */
}

/*********************************************************************************************************
** 函数名称: configMemFree
** 功能描述: 释放本地配置内存
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
VOID  configMemFree (VOID)
{
    struct optionset  *poptionsetCurr = NULL;
    struct optionset  *poptionsetFree = NULL;

    if (GserverConfig.pcInteface) {                                     /* 判断Inteface是否分配过内存   */
        free(GserverConfig.pcInteface);                                 /* 释放内存空间                 */
    }

    if (GserverConfig.pcLeasefile) {                                    /* 判断Leasefile是否分配过内存  */
        free(GserverConfig.pcLeasefile);                                /* 释放内存空间                 */
    }

    if (GserverConfig.pclogfile) {                                      /* 判断Leasefile是否分配过内存  */
        free(GserverConfig.pclogfile);                                  /* 释放内存空间                 */
    }


    while (poptionsetCurr) {                                            /* 释放option链表内存           */
        poptionsetFree  = poptionsetCurr;
        poptionsetCurr  = poptionsetCurr->optionsetNext;
        if (poptionsetFree->pcData) {                                   /* 判断Data是否分配过内存       */
            free(poptionsetFree->pcData);                               /* 释放Data内存                 */
            free(poptionsetFree);                                       /* 释放结构体内存               */
        }
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/

