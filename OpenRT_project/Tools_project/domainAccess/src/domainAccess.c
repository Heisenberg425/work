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
** 文   件   名: domainAccess.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 8 月 1 日
**
** 描        述: OpenRT 域名设置
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "lwip/prot/ip4.h"
#include "net/lwip/lwip_hook.h"

/*********************************************************************************************************
  域名配置(使用小写)
*********************************************************************************************************/
#define DOMAIN_NAME            "openrtlogin.com"                        /*  域名                        */
/*********************************************************************************************************
  DNS 报文头自定义
*********************************************************************************************************/
#pragma pack (1)
struct dns_hdr {
    UINT16   usTranId;
    UINT16   usFlags;
    UINT16   usQuestions;
    UINT16   usAnswerRRs;
    UINT16   usAuthorityRRs;
    UINT16   usAdditionalRRs;
};
/*********************************************************************************************************
  DNS answer 字段内容自定义
*********************************************************************************************************/
#pragma pack (1)
struct dns_answer {
    UINT16   usName;
    UINT16   usType;
    UINT16   usClass;
    UINT32   uiTTL;
    UINT16   usDataLen;
    UINT32   uiAddr;
};
#define DNSANSWER_LEN         sizeof(struct dns_answer)                 /*  DNS 应答 answer 长度        */
/*********************************************************************************************************
  自定义 IP 头结构
*********************************************************************************************************/
struct custom_iphdr {
    unsigned int   ip_length :4;
    unsigned int   ip_version:4;
    unsigned char  ip_tos;
    unsigned short ip_total_length;
    unsigned short ip_id;
    unsigned short ip_flags;
    unsigned char  ip_ttl;
    unsigned char  ip_protocol;
    unsigned short ip_cksum;
    unsigned int   ip_source;
    unsigned int   ip_dest;
};
#define IPHDR_LEN             sizeof(struct custom_iphdr)               /*  IP 报文头长度               */
/*********************************************************************************************************
  自定义 UDP 头结构
*********************************************************************************************************/
struct custom_udphdr {
    unsigned short s_port;
    unsigned short d_port;
    unsigned short length;
    unsigned short cksum;
};
#define UDPHDR_LEN            sizeof(struct custom_udphdr)              /*  UDP 报文头长度              */
/*********************************************************************************************************
  伪头部，用于计算校验和
*********************************************************************************************************/
struct psd_header {
    unsigned int   s_ip;
    unsigned int   d_ip;
    unsigned char  mbz;
    unsigned char  proto;
    unsigned short plen;
};
/*********************************************************************************************************
  全局变量管理
*********************************************************************************************************/
PCHAR  _G_pcOpenRTDomain = LW_NULL;
/*********************************************************************************************************
** 函数名称: __domainNamePar
** 功能描述: 域名转换为通信报文内容
** 输  入  : pcDomainName      域名
** 输  出  : NONE
** 返  回  : NONE
*********************************************************************************************************/
static VOID __domainNamePar (PCHAR  pcDomainName)
{
    PCHAR  pcTmp      = LW_NULL;
    INT32  i          = 0;

    if (LW_NULL == pcDomainName) {
        return;
    }

    if (LW_NULL != _G_pcOpenRTDomain) {
        return;
    }

    _G_pcOpenRTDomain = malloc(strlen(pcDomainName) * 2);
    if (LW_NULL == _G_pcOpenRTDomain) {
        return;
    }

    /*
     *  解析域名字段
     */
    pcTmp = strtok(pcDomainName, ".");
    while(pcTmp) {
        _G_pcOpenRTDomain[i] = strlen(pcTmp);
        memcpy(&_G_pcOpenRTDomain[i + 1], pcTmp, strlen(pcTmp));
        i += strlen(pcTmp) + 1;
        pcTmp = strtok(NULL, ".");
    }

    _G_pcOpenRTDomain[i] = 0;
    _G_pcOpenRTDomain[i + 1] = '\0';
}
/*********************************************************************************************************
** 函数名称: __checkSum
** 功能描述: 校验头计算
** 输  入  : pusBuf      待校验数据
**           iSize       校验数据长度
** 输  出  : NONE
** 返  回  : 校验和
*********************************************************************************************************/
static UINT16  __checkSum (UINT16  *pusBuf, INT32  iSize)
{
    ULONG  ulChkSum = 0;

    while(iSize > 1) {
        ulChkSum  += *pusBuf++;
        iSize     -= sizeof(UINT16);
    }

    if(iSize) {
        ulChkSum += *(PUCHAR)pusBuf;
    }

    ulChkSum = (ulChkSum >> 16) + (ulChkSum & 0xffff);

    ulChkSum += (ulChkSum >> 16);

    return ((UINT16)(~ulChkSum));
}
/*********************************************************************************************************
** 函数名称: __dnsPacketGen
** 功能描述: 构造 OpenRT 域名 DNS Reply 报文
** 输  入  : pcDnsReqPacket     DNS 请求报文
**           uiUdpSrc           UDP 源端口
**           usDnsLen           DNS 请求报文长度
**           pNetif             收到报文网口
**           uiDstIpAddr        目的 IP
**           uiSrcIpAddr        源 IP
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT32  __dnsPacketReply (PCHAR          pcDnsReqPacket,
                                UINT16         uiUdpSrc,
                                UINT16         usDnsLen,
                                struct netif  *pNetif,
                                UINT32         uiDstIpAddr,
                                UINT32         uiSrcIpAddr)
{
    PCHAR               pcPacketBuf      = LW_NULL;
    PCHAR               pcDnsReplyPacket = LW_NULL;
    struct dns_hdr     *pDnsHdr          = LW_NULL;
    struct dns_answer  *pDnsAnswer       = LW_NULL;
    struct psd_header   psdPacket        = {0};

    INT32               iSockfd          = -1;
    struct sockaddr_in  host_addr;
    const INT32         on               = 1;

    INT32               iPacketLen       = UDPHDR_LEN + IPHDR_LEN + usDnsLen + DNSANSWER_LEN;

    CHAR                cTmpBuff[sizeof(psdPacket) + iPacketLen - IPHDR_LEN];

    /*
     *  申请报文内存
     */
    pcPacketBuf = malloc(iPacketLen);
    if (LW_NULL == pcPacketBuf) {
        return  (PX_ERROR);
    }

    bzero(pcPacketBuf, iPacketLen);

    /*
     *  创建 socket
     */
    iSockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(0 > iSockfd) {
        free (pcPacketBuf);

        return  (PX_ERROR);
    }

    /*
     *  构造 socket 地址
     */
    memset(&host_addr, 0, sizeof(host_addr));

    host_addr.sin_family      = AF_INET;
    host_addr.sin_port        = htons(53);                              /*  DNS 服务器端口为 53         */
    host_addr.sin_addr.s_addr = uiDstIpAddr;

    /*
     *  设置 socket 为自己构造 IP 头
     */
    if(setsockopt(iSockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        free (pcPacketBuf);

        return  (PX_ERROR);
    }

    struct custom_iphdr  *pIpHdr  = (struct custom_iphdr *)pcPacketBuf;
    struct custom_udphdr *pUdpHdr = (struct custom_udphdr *)(pcPacketBuf + IPHDR_LEN);

    /*
     *  构造 UDP 协议的 IP 报文头
     */
    pIpHdr->ip_length       = 5;
    pIpHdr->ip_version      = 4;
    pIpHdr->ip_tos          = 0;
    pIpHdr->ip_total_length = htons(iPacketLen);
    pIpHdr->ip_id           = 0;
    pIpHdr->ip_flags        = 0x40;
    pIpHdr->ip_ttl          = 0x40;
    pIpHdr->ip_protocol     = IP_PROTO_UDP;
    pIpHdr->ip_source       = uiSrcIpAddr;
    pIpHdr->ip_dest         = uiDstIpAddr;
    pIpHdr->ip_cksum        = __checkSum((UINT16 *)pcPacketBuf, IPHDR_LEN);

    /*
     *  构造 UDP 报文头
     */
    pUdpHdr->s_port         = htons(53);                                /*  DNS 服务器端口为 53         */
    pUdpHdr->d_port         = uiUdpSrc;
    pUdpHdr->length         = htons(iPacketLen - IPHDR_LEN);
    pUdpHdr->cksum          = 0;

    /*
     *  构造 OpenRT 域名 DNS 应答报文内容
     */
    pcDnsReplyPacket = pcPacketBuf + UDPHDR_LEN + IPHDR_LEN;
    memcpy(pcDnsReplyPacket, pcDnsReqPacket, usDnsLen);                 /*  复制收到 DNS 的请求部分内容 */

    /*
     *  修改 DNS 请求部分的内容
     */
    pDnsHdr = (struct dns_hdr *)pcDnsReplyPacket;
    pDnsHdr->usFlags         = htons(0x8180);
    pDnsHdr->usAnswerRRs     = htons(1);
    pDnsHdr->usQuestions     = htons(1);
    pDnsHdr->usAdditionalRRs = 0;
    pDnsHdr->usAuthorityRRs  = 0;

    /*
     *  添加应答部分内容
     */
    pDnsAnswer = (struct dns_answer *)(pcDnsReplyPacket + usDnsLen);
    pDnsAnswer->usName       = htons(0xc00c);                           /*  NAME 使用指针格式           */
    pDnsAnswer->usType       = htons(0x0001);                           /*  A  类型                     */
    pDnsAnswer->usClass      = htons(0x0001);                           /*  IN 模式                     */
    pDnsAnswer->uiTTL        = htonl(86400);                            /*  TTL 默认给一天              */
    pDnsAnswer->usDataLen    = htons(0x4);                              /*  IP 地址长度 4 个字节        */
    pDnsAnswer->uiAddr       = pNetif->ip_addr.u_addr.ip4.addr;         /*  IP 地址设为对应的网口       */


    /*
     *  构造 UDP 伪头部，进行校验和计算
     */
    psdPacket.s_ip          = pIpHdr->ip_source;
    psdPacket.d_ip          = pIpHdr->ip_dest;
    psdPacket.mbz           = 0;
    psdPacket.proto         = IP_PROTO_UDP;
    psdPacket.plen          = pUdpHdr->length;

    memcpy(cTmpBuff, &psdPacket, sizeof(psdPacket));
    memcpy(cTmpBuff + sizeof(psdPacket), pcPacketBuf + IPHDR_LEN, iPacketLen - IPHDR_LEN);
    pUdpHdr->cksum = __checkSum((unsigned short *)cTmpBuff, sizeof(cTmpBuff));

    sendto(iSockfd, pcPacketBuf, iPacketLen, 0, (struct sockaddr*)&host_addr, sizeof(host_addr));

    free(pcPacketBuf);
    close(iSockfd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __openrtDnsHook
** 功能描述: OpenRT 域名劫持 hook
** 输　入  : ip_type       IP 报文类型
**           hook_type     钩子类型
**           pBuf          数据包缓冲区
**           pIn           输入网口信息指针
**           pOut          输出网口信息指针
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT32  __openrtDnsHook (INT32          ip_type,
                               INT32          hook_type,
                               struct pbuf   *pBuf,
                               struct netif  *pIn,
                               struct netif  *pOut)
{
    UINT16   iphdr_hlen;
    UINT32   uiDstPort;
    UINT16   usUdpLen;
    PCHAR    pcDnsQueriesData;
    INT      i;

    struct udp_hdr  *pUdphdr = LW_NULL;                                 /*  UDP 头                      */
    struct dns_hdr  *pdnsHdr = LW_NULL;                                 /*  DNS 头                      */

    if (IP_HOOK_V4 == ip_type && IP_HT_PRE_ROUTING == hook_type) {
        struct ip_hdr  *pIphdr = (struct ip_hdr *)(pBuf->payload);      /*  IP 头                       */

        iphdr_hlen  = IPH_HL(pIphdr);
        iphdr_hlen *= 4;

        if (IP_PROTO_UDP == IPH_PROTO(pIphdr)) {                        /*  UDP 协议                    */
            pUdphdr   = (struct udp_hdr *)((PCHAR)(pIphdr) + iphdr_hlen);

            uiDstPort = ntohs(pUdphdr->dest);
            usUdpLen  = ntohs(pUdphdr->len);

            if (53 == uiDstPort) {
                pdnsHdr = (struct dns_hdr *)((PCHAR)(pUdphdr) + UDP_HLEN);

                if (ntohs(pdnsHdr->usFlags) == 0x0100 && ntohs(pdnsHdr->usQuestions) == 1) {

                    /*
                     *  如果长度小于 openrt 域名长度，直接通过
                     */
                    if ((usUdpLen - sizeof(struct dns_hdr)) < strlen(_G_pcOpenRTDomain)) {
                        return  (ERROR_NONE);
                    }

                    pcDnsQueriesData = (PCHAR)pdnsHdr + sizeof(struct dns_hdr);
                    for (i = 0; i < strlen(_G_pcOpenRTDomain); i++) {
                        if (*(pcDnsQueriesData + i) == _G_pcOpenRTDomain[i]) {
                            continue;
                        } else {                                        /*  不匹配，直接通过            */
                            return  (ERROR_NONE);
                        }
                    }

                    __dnsPacketReply((PCHAR)pdnsHdr,
                                     pUdphdr->src,
                                     usUdpLen - 8,
                                     pIn,
                                     pIphdr->src.addr,
                                     pIphdr->dest.addr);                /*  回复域名信息                */
                    return  (PX_ERROR);
                }
            }
        }
    }

    return  (ERROR_NONE);                                               /*  放行                        */
}
/*********************************************************************************************************
** 函数名称: domainNameStart
** 功能描述: 开启 OpenRT 域名劫持
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  domainNameStart (VOID)
{
    net_ip_hook_add("openrtdns", __openrtDnsHook);

    __domainNamePar(DOMAIN_NAME);
}

/*********************************************************************************************************
** 函数名称: domainNameStop
** 功能描述: 关闭 OpenRT 域名劫持
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  domainNameStop (VOID)
{
    net_ip_hook_delete(__openrtDnsHook);

    if (LW_NULL != _G_pcOpenRTDomain) {
        free(_G_pcOpenRTDomain);
    }

    _G_pcOpenRTDomain = LW_NULL;
}
/*********************************************************************************************************
** 函数名称: module_init
** 功能描述: ko 模块初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  module_init (VOID)
{
    domainNameStart();

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: module_exit
** 功能描述: ko 模块卸载
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  module_exit (VOID)
{
    domainNameStop();
}
