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
** ��   ��   ��: domainAccess.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 1 ��
**
** ��        ��: OpenRT ��������
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "lwip/prot/ip4.h"
#include "net/lwip/lwip_hook.h"

/*********************************************************************************************************
  ��������(ʹ��Сд)
*********************************************************************************************************/
#define DOMAIN_NAME            "openrtlogin.com"                        /*  ����                        */
/*********************************************************************************************************
  DNS ����ͷ�Զ���
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
  DNS answer �ֶ������Զ���
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
#define DNSANSWER_LEN         sizeof(struct dns_answer)                 /*  DNS Ӧ�� answer ����        */
/*********************************************************************************************************
  �Զ��� IP ͷ�ṹ
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
#define IPHDR_LEN             sizeof(struct custom_iphdr)               /*  IP ����ͷ����               */
/*********************************************************************************************************
  �Զ��� UDP ͷ�ṹ
*********************************************************************************************************/
struct custom_udphdr {
    unsigned short s_port;
    unsigned short d_port;
    unsigned short length;
    unsigned short cksum;
};
#define UDPHDR_LEN            sizeof(struct custom_udphdr)              /*  UDP ����ͷ����              */
/*********************************************************************************************************
  αͷ�������ڼ���У���
*********************************************************************************************************/
struct psd_header {
    unsigned int   s_ip;
    unsigned int   d_ip;
    unsigned char  mbz;
    unsigned char  proto;
    unsigned short plen;
};
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
PCHAR  _G_pcOpenRTDomain = LW_NULL;
/*********************************************************************************************************
** ��������: __domainNamePar
** ��������: ����ת��Ϊͨ�ű�������
** ��  ��  : pcDomainName      ����
** ��  ��  : NONE
** ��  ��  : NONE
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
     *  ���������ֶ�
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
** ��������: __checkSum
** ��������: У��ͷ����
** ��  ��  : pusBuf      ��У������
**           iSize       У�����ݳ���
** ��  ��  : NONE
** ��  ��  : У���
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
** ��������: __dnsPacketGen
** ��������: ���� OpenRT ���� DNS Reply ����
** ��  ��  : pcDnsReqPacket     DNS ������
**           uiUdpSrc           UDP Դ�˿�
**           usDnsLen           DNS �����ĳ���
**           pNetif             �յ���������
**           uiDstIpAddr        Ŀ�� IP
**           uiSrcIpAddr        Դ IP
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
     *  ���뱨���ڴ�
     */
    pcPacketBuf = malloc(iPacketLen);
    if (LW_NULL == pcPacketBuf) {
        return  (PX_ERROR);
    }

    bzero(pcPacketBuf, iPacketLen);

    /*
     *  ���� socket
     */
    iSockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(0 > iSockfd) {
        free (pcPacketBuf);

        return  (PX_ERROR);
    }

    /*
     *  ���� socket ��ַ
     */
    memset(&host_addr, 0, sizeof(host_addr));

    host_addr.sin_family      = AF_INET;
    host_addr.sin_port        = htons(53);                              /*  DNS �������˿�Ϊ 53         */
    host_addr.sin_addr.s_addr = uiDstIpAddr;

    /*
     *  ���� socket Ϊ�Լ����� IP ͷ
     */
    if(setsockopt(iSockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        free (pcPacketBuf);

        return  (PX_ERROR);
    }

    struct custom_iphdr  *pIpHdr  = (struct custom_iphdr *)pcPacketBuf;
    struct custom_udphdr *pUdpHdr = (struct custom_udphdr *)(pcPacketBuf + IPHDR_LEN);

    /*
     *  ���� UDP Э��� IP ����ͷ
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
     *  ���� UDP ����ͷ
     */
    pUdpHdr->s_port         = htons(53);                                /*  DNS �������˿�Ϊ 53         */
    pUdpHdr->d_port         = uiUdpSrc;
    pUdpHdr->length         = htons(iPacketLen - IPHDR_LEN);
    pUdpHdr->cksum          = 0;

    /*
     *  ���� OpenRT ���� DNS Ӧ��������
     */
    pcDnsReplyPacket = pcPacketBuf + UDPHDR_LEN + IPHDR_LEN;
    memcpy(pcDnsReplyPacket, pcDnsReqPacket, usDnsLen);                 /*  �����յ� DNS �����󲿷����� */

    /*
     *  �޸� DNS ���󲿷ֵ�����
     */
    pDnsHdr = (struct dns_hdr *)pcDnsReplyPacket;
    pDnsHdr->usFlags         = htons(0x8180);
    pDnsHdr->usAnswerRRs     = htons(1);
    pDnsHdr->usQuestions     = htons(1);
    pDnsHdr->usAdditionalRRs = 0;
    pDnsHdr->usAuthorityRRs  = 0;

    /*
     *  ���Ӧ�𲿷�����
     */
    pDnsAnswer = (struct dns_answer *)(pcDnsReplyPacket + usDnsLen);
    pDnsAnswer->usName       = htons(0xc00c);                           /*  NAME ʹ��ָ���ʽ           */
    pDnsAnswer->usType       = htons(0x0001);                           /*  A  ����                     */
    pDnsAnswer->usClass      = htons(0x0001);                           /*  IN ģʽ                     */
    pDnsAnswer->uiTTL        = htonl(86400);                            /*  TTL Ĭ�ϸ�һ��              */
    pDnsAnswer->usDataLen    = htons(0x4);                              /*  IP ��ַ���� 4 ���ֽ�        */
    pDnsAnswer->uiAddr       = pNetif->ip_addr.u_addr.ip4.addr;         /*  IP ��ַ��Ϊ��Ӧ������       */


    /*
     *  ���� UDP αͷ��������У��ͼ���
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
** ��������: __openrtDnsHook
** ��������: OpenRT �����ٳ� hook
** �䡡��  : ip_type       IP ��������
**           hook_type     ��������
**           pBuf          ���ݰ�������
**           pIn           ����������Ϣָ��
**           pOut          ���������Ϣָ��
** �䡡��  : NONE
** ������  : ERROR_CODE
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

    struct udp_hdr  *pUdphdr = LW_NULL;                                 /*  UDP ͷ                      */
    struct dns_hdr  *pdnsHdr = LW_NULL;                                 /*  DNS ͷ                      */

    if (IP_HOOK_V4 == ip_type && IP_HT_PRE_ROUTING == hook_type) {
        struct ip_hdr  *pIphdr = (struct ip_hdr *)(pBuf->payload);      /*  IP ͷ                       */

        iphdr_hlen  = IPH_HL(pIphdr);
        iphdr_hlen *= 4;

        if (IP_PROTO_UDP == IPH_PROTO(pIphdr)) {                        /*  UDP Э��                    */
            pUdphdr   = (struct udp_hdr *)((PCHAR)(pIphdr) + iphdr_hlen);

            uiDstPort = ntohs(pUdphdr->dest);
            usUdpLen  = ntohs(pUdphdr->len);

            if (53 == uiDstPort) {
                pdnsHdr = (struct dns_hdr *)((PCHAR)(pUdphdr) + UDP_HLEN);

                if (ntohs(pdnsHdr->usFlags) == 0x0100 && ntohs(pdnsHdr->usQuestions) == 1) {

                    /*
                     *  �������С�� openrt �������ȣ�ֱ��ͨ��
                     */
                    if ((usUdpLen - sizeof(struct dns_hdr)) < strlen(_G_pcOpenRTDomain)) {
                        return  (ERROR_NONE);
                    }

                    pcDnsQueriesData = (PCHAR)pdnsHdr + sizeof(struct dns_hdr);
                    for (i = 0; i < strlen(_G_pcOpenRTDomain); i++) {
                        if (*(pcDnsQueriesData + i) == _G_pcOpenRTDomain[i]) {
                            continue;
                        } else {                                        /*  ��ƥ�䣬ֱ��ͨ��            */
                            return  (ERROR_NONE);
                        }
                    }

                    __dnsPacketReply((PCHAR)pdnsHdr,
                                     pUdphdr->src,
                                     usUdpLen - 8,
                                     pIn,
                                     pIphdr->src.addr,
                                     pIphdr->dest.addr);                /*  �ظ�������Ϣ                */
                    return  (PX_ERROR);
                }
            }
        }
    }

    return  (ERROR_NONE);                                               /*  ����                        */
}
/*********************************************************************************************************
** ��������: domainNameStart
** ��������: ���� OpenRT �����ٳ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  domainNameStart (VOID)
{
    net_ip_hook_add("openrtdns", __openrtDnsHook);

    __domainNamePar(DOMAIN_NAME);
}

/*********************************************************************************************************
** ��������: domainNameStop
** ��������: �ر� OpenRT �����ٳ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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
** ��������: module_init
** ��������: ko ģ���ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  module_init (VOID)
{
    domainNameStart();

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: module_exit
** ��������: ko ģ��ж��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  module_exit (VOID)
{
    domainNameStop();
}
