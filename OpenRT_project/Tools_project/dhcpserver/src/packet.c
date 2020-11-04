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
** ��   ��   ��: packet.c
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��2��27��
**
** ��        ��: DHCP���ݰ�������
*********************************************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "log.h"
#include "packet.h"
#include "leasesfix.h"
#include "configini.h"
#include "sockets.h"

/*********************************************************************************************************
  DHCP ѡ��ṹ������
*********************************************************************************************************/
struct dhcpoption  Gdhcpopts[] = {
   /*
    * �ؼ���      ��־λ                                ��־��
    */
    {"subnet",    OPTION_IP | OPTION_REQ,               DHCP_SUBNET},
    {"timezone",  OPTION_S32,                           DHCP_TIME_OFFSET},
    {"router",    OPTION_IP | OPTION_LIST | OPTION_REQ, DHCP_ROUTER},
    {"timesvr",   OPTION_IP | OPTION_LIST,              DHCP_TIME_SERVER},
    {"namesvr",   OPTION_IP | OPTION_LIST,              DHCP_NAME_SERVER},
    {"dns",       OPTION_IP | OPTION_LIST | OPTION_REQ, DHCP_DNS_SERVER},
    {"logsvr",    OPTION_IP | OPTION_LIST,              DHCP_LOG_SERVER},
    {"cookiesvr", OPTION_IP | OPTION_LIST,              DHCP_COOKIE_SERVER},
    {"lprsvr",    OPTION_IP | OPTION_LIST,              DHCP_LPR_SERVER},
    {"hostname",  OPTION_STRING | OPTION_REQ,           DHCP_HOST_NAME},
    {"bootsize",  OPTION_U16,                           DHCP_BOOT_SIZE},
    {"domain",    OPTION_STRING | OPTION_REQ,           DHCP_DOMAIN_NAME},
    {"swapsvr",   OPTION_IP,                            DHCP_SWAP_SERVER},
    {"rootpath",  OPTION_STRING,                        DHCP_ROOT_PATH},
    {"ipttl",     OPTION_U8,                            DHCP_IP_TTL},
    {"mtu",       OPTION_U16,                           DHCP_MTU},
    {"broadcast", OPTION_IP | OPTION_REQ,               DHCP_BROADCAST},
    {"ntpsrv",    OPTION_IP | OPTION_LIST,              DHCP_NTP_SERVER},
    {"wins",      OPTION_IP | OPTION_LIST,              DHCP_WINS_SERVER},
    {"requestip", OPTION_IP,                            DHCP_REQUESTED_IP},
    {"lease",     OPTION_U32,                           DHCP_LEASE_TIME},
    {"t1",        OPTION_U32,                           DHCP_T1},
    {"t2",        OPTION_U32,                           DHCP_T2},
    {"dhcptype",  OPTION_U8,                            DHCP_MESSAGE_TYPE},
    {"serverid",  OPTION_IP,                            DHCP_SERVER_ID},
    {"message",   OPTION_STRING,                        DHCP_MESSAGE},
    {"tftp",      OPTION_STRING,                        0x42},
    {"bootfile",  OPTION_STRING,                        0x43},
    {"",          0x00,                                 0x00}
};

INT  GiOptlen[] = {
	[OPTION_IP]      =  4,
	[OPTION_IP_PAIR] =  8,
	[OPTION_BOOLEAN] =  1,
	[OPTION_STRING]  =  1,
	[OPTION_U8]      =  1,
	[OPTION_U16]     =  2,
	[OPTION_S16]     =  2,
	[OPTION_U32]     =  4,
	[OPTION_S32]     =  4
};

/*********************************************************************************************************
** ��������: optionGet
** ��������: ��ȡѡ���ֶ�
** ��    ��: pdhcpmsgPacket     dhcp���ݰ�
**           cType              ѡ������
** ��    ��: ���ض�Ӧ�ֶ�ָ��
** ����ģ��: NONE
*********************************************************************************************************/
PUCHAR  poptionGet (PDHCPMSG  pdhcpmsgPacket, CHAR  cType)
{
    INT        i           = 0;
    INT        iLength     = 308;                                       /* 312�ֽ�Option - 4�ֽ�cookie  */
    INT        iOver       = 0;
    INT        iDown       = 0;
    INT        iCurr       = OPTION_FIELD;
    PUCHAR     pcOptionptr = NULL;

    pcOptionptr = pdhcpmsgPacket->ucOptions;

    while (!iDown) {                                                    /* ��ɲ���ǰѭ������           */
        if (i >= iLength) {                                             /* ��δ�ҵ����ؿ�               */
           LOG(LOG_DEBUG,"bogus packet, option fields too long.\r\n");
           return (NULL);
        }

        if (pcOptionptr[i + OPT_CODE] == cType) {                       /* ����ҵ���Ӧ��ѡ��           */
            if (i + 1 + pcOptionptr[i + OPT_LEN] >= iLength) {          /* �ж��Ƿ�ѡ���             */
                LOG(LOG_DEBUG, "bogus packet, option fields too long.\r\n");
                return (NULL);
            }
            return (pcOptionptr + i + 2);                                 /* ����ѡ���ַ                 */
        }

        switch (pcOptionptr[i + OPT_CODE]) {                            /* �ж�ѡ������                 */

        case DHCP_PADDING:                                              /* PADDING������һ��������      */
            i++;
            break;

        case DHCP_OPTION_OVER:                                          /* ��¼option_overλ��          */
            if (i + 1 + pcOptionptr[i + OPT_LEN] >= iLength) {
                LOG(LOG_DEBUG, "bogus packet, option fields too long.\r\n");
                return (NULL);
            }
            iOver   = pcOptionptr[i + 3];
            i      += pcOptionptr[OPT_LEN] + 2;
            break;

        case DHCP_END:                                                  /* �����ѡ�������             */
            if (iCurr == OPTION_FIELD && iOver & FILE_FIELD) {          /* �ļ�·�������ض�λ����       */
                pcOptionptr = pdhcpmsgPacket->ucFile;
                i           = 0;
                iLength     = 128;
                iCurr       = FILE_FIELD;
            } else if (iCurr == FILE_FIELD && iOver & SNAME_FIELD) {
                pcOptionptr = pdhcpmsgPacket->ucSname;
                i           = 0;
                iLength     = 64;
                iCurr       = SNAME_FIELD;
            } else {
                iDown = 1;                                              /* ���ҽ���                     */
            }
            break;

        default:
            i += pcOptionptr[OPT_LEN + i] + 2;
        }
    }
    return (NULL);
}

/*********************************************************************************************************
** ��������: optionEnd
** ��������: ��λ��ѡ��ĩβ
** ��    ��: pcOptionptr   ѡ��ṹ��ָ��
** ��    ��: ����ѡ��ĩβλ��
** ����ģ��: NONE
*********************************************************************************************************/
static INT  optionEnd (PUCHAR  pcOptionptr)
{
    INT  i = 0;

    if (pcOptionptr == NULL) {                                          /* �п�                         */
        return (PX_ERROR);
    }

    while (pcOptionptr[i] != DHCP_END) {                                /* ����ѡ�ѡ�������         */
        if (pcOptionptr[i] == DHCP_PADDING) {                           /* PADDINGʱһ��������          */
            i++;
        } else {                                                        /* ��ͨѡ��ֱ������ѡ���     */
            i += pcOptionptr[i + OPT_LEN] + 2;
        }
    }

    return (i);                                                         /* ����ѡ��ĩβ��ǩ             */
}

/*********************************************************************************************************
** ��������: optionstrAdd
** ��������: ���ѡ���ַ�������
** ��    ��: pcOptionptr    ѡ��ṹָ��
**           pcString       ѡ�������ַ���
** ��    ��: �������ѡ���λ��
** ����ģ��: NONE
*********************************************************************************************************/
INT  optionstrAdd (PUCHAR  pcOptionptr,
                   PUCHAR  pcString)
{
    INT iEnd  = optionEnd(pcOptionptr);                                 /* ��λ��ѡ��ĩβ               */

    if (iEnd + pcString[OPT_LEN] + 2 + 1 >= 308) {                      /* ���ѡ���Ƿ񳬳�             */
        LOG(LOG_ERR, "Option did not fit into the packet!\r\n");
        return (0);
    }

    memcpy(pcOptionptr + iEnd, pcString, pcString[OPT_LEN] + 2);        /* ����ѡ���ַ�����ѡ��β       */
    pcOptionptr[iEnd + pcString[OPT_LEN] + 2] = DHCP_END;               /* �������ѡ�������           */

    return (pcString[OPT_LEN] + 2);                                     /* ����ѡ�����ݵ�ַ             */
}

/*********************************************************************************************************
** ��������: optionsmpAdd
** ��������: ��Ӽ�ѡ������
** ��    ��: pcOptionptr     ѡ��ṹָ��
**           ucCode          ���ѡ������
**           iData           ��ѡ������
** ��    ��: �������ѡ���λ��
** ����ģ��: NONE
*********************************************************************************************************/
INT  optionsmpAdd (PUCHAR  pcOptionptr,
                   UCHAR   ucCode,
                   UINT    iData)
{
    CHAR     cLength     = 0;
    INT      iCount;
    UCHAR    ucOption[4] = {0};
    PUCHAR   pcData;
    UINT16  *pusData     = NULL;
    UINT    *puiData     = NULL;
    UINT     iAligned;

    pcData    =  (PUCHAR)&iAligned;                                     /* ��������ǽǿת               */
    pusData   =  (UINT16 *)&iAligned;
    puiData   =  &iAligned;

    for (iCount = 0; Gdhcpopts[iCount].ucCode; iCount++) {              /* ����dhcpѡ����               */
        if (Gdhcpopts[iCount].ucCode == ucCode) {
            cLength = GiOptlen[Gdhcpopts[iCount].cFlags & TYPE_MASK];   /* ȡ�������ѡ���           */
            break;
        }
    }

    if (!cLength) {                                                     /* �жϳ��Ȼ�ȡ�Ƿ���ȷ         */
        LOG(LOG_ERR,"Could not add option\r\n");
        return (0);
    }

    ucOption[OPT_CODE] = ucCode;                                        /* ���ѡ������                 */
    ucOption[OPT_LEN]  = cLength;                                       /* ���ѡ���                 */

    switch (cLength) {                                                  /* �ж�ѡ�������Ӧѡ������ */

    case 1:
        *pcData  = iData;
        break;

    case 2:
        *pusData = iData;
        break;

    case 4:
        *puiData = iData;
        break;
    }
    memcpy(ucOption + 2, &iAligned, cLength);                           /* ѡ����������ַ���           */

    return optionstrAdd(pcOptionptr, ucOption);                         /* ����ַ���ѡ��               */
}

/*********************************************************************************************************
** ��������: dhcpMsgtypeGet
** ��������: ��ȡ��Ϣ����
** ��    ��: pdhcpmsgPacket  dhcp���ݰ�
** ��    ��: ��ȡ��Ϣ����
** ����ģ��: API
*********************************************************************************************************/
UCHAR  dhcpMsgtypeGet (PDHCPMSG  pdhcpmsgPacket)
{
    PUCHAR pcType;

    if (pdhcpmsgPacket == NULL) {                                       /* �п�                         */
        LOG(LOG_DEBUG, "packet is null\r\n");
        return (PX_ERROR);
    }

    pcType = poptionGet(pdhcpmsgPacket, DHCP_MESSAGE_TYPE);             /* ��ȡdhcp����                 */
    if (pcType == NULL) {                                               /* �жϻ�ȡ�Ƿ���ȷ             */
        LOG(LOG_DEBUG, "Get msgtype filed\r\n");
        return (PX_ERROR);
    }

    return (pcType[0]);                                                   /* �������ݰ�����               */
}

/*********************************************************************************************************
** ��������: dhcpPacketInit
** ��������: ��ȡ��Ϣ����
** ��    ��: pdhcpmsgSendPacket      Ҫ���͵����ݰ�
**           pdhcpmsgPacket          Դ���ݰ�
**           cType                   ��ʼ������
** ��    ��: ��ȡ��Ϣ����
** ����ģ��: NONE
*********************************************************************************************************/
static VOID  dhcpPacketInit (PDHCPMSG  pdhcpmsgSendPacket,
                             PDHCPMSG  pdhcpmsgPacket,
                             CHAR      cType)
{
    memset(pdhcpmsgSendPacket, 0, sizeof(DHCPMSG));                     /* ��շ������ݰ�����           */

    switch (cType) {                                                    /* �жϳ�ʼ������               */

    case DHCPDISCOVER:
    case DHCPREQUEST:
    case DHCPRELEASE:
    case DHCPINFORM:
        pdhcpmsgSendPacket->ucOp = BOOTREQUEST;                         /* ��������ΪBOOTREQUEST        */
        break;

    case DHCPOFFER:
    case DHCPACK:
    case DHCPNAK:
        pdhcpmsgSendPacket->ucOp = BOOTREPLY;                           /* ����ΪBOOTREPLY              */
        break;

    default:
        break;
    }

    pdhcpmsgSendPacket->ucHtype      = ETH_10MB;                        /* ���Ӳ������                 */
    pdhcpmsgSendPacket->ucHlen       = ETH_10MB_LEN;                    /* ���Ӳ������                 */
    pdhcpmsgSendPacket->uiCookie     = htonl(DHCP_MAGIC);               /* ���ħ��                     */
    pdhcpmsgSendPacket->ucOptions[0] = DHCP_END;                        /* ���ѡ���ֽ�����             */
    pdhcpmsgSendPacket->uiXid        = pdhcpmsgPacket->uiXid;           /* �������id                   */
    memcpy(pdhcpmsgSendPacket->ucChaddr, pdhcpmsgPacket->ucChaddr, 16); /* ���Ӳ����ַ                 */
    pdhcpmsgSendPacket->usFlags      = pdhcpmsgPacket->usFlags;         /* ������ݱ�־λ               */
    pdhcpmsgSendPacket->uiGiaddr     = pdhcpmsgPacket->uiGiaddr;        /* ���giaddr                   */
    pdhcpmsgSendPacket->uiCiaddr     = pdhcpmsgPacket->uiCiaddr;        /* ���ciaddr                   */
    optionsmpAdd(pdhcpmsgSendPacket->ucOptions,                         /* ������ݰ�����               */
                 DHCP_MESSAGE_TYPE,
                 cType);
    optionsmpAdd(pdhcpmsgSendPacket->ucOptions,                         /* ����������ַ               */
                 DHCP_SERVER_ID,
                 GserverConfig.uiServer);
}

/*********************************************************************************************************
** ��������: nakSend
** ��������: ����NAK��Ӧ
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: ����ERROR_NONE
** ����ģ��: packetSendBroadcast
*********************************************************************************************************/
static INT  nakSend (PDHCPMSG  pdhcpmsgPacket)
{
    DHCPMSG  dhcpmsgSendPacket;

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPNAK);        /* ��ʼ�����ݰ�ΪNAK����        */

    LOG(LOG_INFO,
        "Send nak %s of %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&(dhcpmsgSendPacket.uiYiaddr)),
        dhcpmsgSendPacket.ucChaddr[0],
        dhcpmsgSendPacket.ucChaddr[1],
        dhcpmsgSendPacket.ucChaddr[2],
        dhcpmsgSendPacket.ucChaddr[3],
        dhcpmsgSendPacket.ucChaddr[4],
        dhcpmsgSendPacket.ucChaddr[5]);

    return packetSendBroadcast(&dhcpmsgSendPacket);                     /* �㲥�������ݰ�               */
}

/*********************************************************************************************************
** ��������: requestSend
** ��������: ����ACK��Ӧ
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: leaseAck
*********************************************************************************************************/
static INT  requestSend (PDHCPMSG  pdhcpmsgPacket)
{
   INT                 iRet;
    DHCPMSG            dhcpmsgSendPacket;
    PUCHAR             pcRequestIp = NULL;
    UINT               uiIp;

    memset(&dhcpmsgSendPacket, 0 ,sizeof(dhcpmsgSendPacket));           /* ������ݰ�                   */

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPACK);        /* ��ʼ�����ݰ�ΪACK����        */

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* ��ȡ�ͻ��������ip           */
    if (pcRequestIp) {
        memcpy(&uiIp, pcRequestIp, 4);
    LOG(LOG_INFO,
        "Get iprequest %s from %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&uiIp),
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);
    } else {
        LOG(LOG_INFO,
            "Get iprequest from %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            pdhcpmsgPacket->ucChaddr[0],
            pdhcpmsgPacket->ucChaddr[1],
            pdhcpmsgPacket->ucChaddr[2],
            pdhcpmsgPacket->ucChaddr[3],
            pdhcpmsgPacket->ucChaddr[4],
            pdhcpmsgPacket->ucChaddr[5]);
    }

    iRet = leaseAck(pdhcpmsgPacket, &dhcpmsgSendPacket);                /* ����ACK����                  */
    if ( iRet == PX_ERROR) {                                            /* ���ACK����ʧ������NAK     */
        return nakSend(pdhcpmsgPacket);
    } else if (iRet == ERROR_NONE) {                                    /* ����Ҫ�����κδ���           */
        LOG(LOG_DEBUG, "ACK NONE !\r\n");
        return (iRet);
    } else {                                                            /* ����ҪӦ������ACK          */
        LOG(LOG_INFO,
            "Send ack %s of %02x:%02x:%02x:%02x:%02x:%02x \r\n",
			ip4addr_ntoa((const ip4_addr_t *)&(dhcpmsgSendPacket.uiYiaddr)),
            pdhcpmsgPacket->ucChaddr[0],
            pdhcpmsgPacket->ucChaddr[1],
            pdhcpmsgPacket->ucChaddr[2],
            pdhcpmsgPacket->ucChaddr[3],
            pdhcpmsgPacket->ucChaddr[4],
            pdhcpmsgPacket->ucChaddr[5]);
        return  packetSendBroadcast(&dhcpmsgSendPacket);
    }
}

/*********************************************************************************************************
** ��������: offerSend
** ��������: ����offer��
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: discoverOfferGet,packetSendBroadcast
*********************************************************************************************************/
static INT  offerSend (PDHCPMSG  pdhcpmsgPacket)
{
    DHCPMSG            dhcpmsgSendPacket;

    memset(&dhcpmsgSendPacket, 0, sizeof(dhcpmsgSendPacket));

    LOG(LOG_INFO,
        "Get discover from %02x:%02x:%02x:%02x:%02x:%02x \r\n",
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPOFFER);      /* ��ʼ�����ݰ�ΪOFFER����      */

    if (discoverOfferGet(pdhcpmsgPacket,                                /* ��ȡ������Լ                 */
                         &dhcpmsgSendPacket) == ERROR_NONE) {
        LOG(LOG_INFO,
            "sending offer %s to %02x:%02x:%02x:%02x:%02x:%02x \r\n",
			ip4addr_ntoa((const ip4_addr_t *)&(dhcpmsgSendPacket.uiYiaddr)),
            dhcpmsgSendPacket.ucChaddr[0],
            dhcpmsgSendPacket.ucChaddr[1],
            dhcpmsgSendPacket.ucChaddr[2],
            dhcpmsgSendPacket.ucChaddr[3],
            dhcpmsgSendPacket.ucChaddr[4],
            dhcpmsgSendPacket.ucChaddr[5]);
        return packetSendBroadcast(&dhcpmsgSendPacket);                 /* �㲥�������ݰ�               */
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: informSend
** ��������: ����inform��
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
**           uiYiaddr            ���͵�ַ
** ��    ��: ����ERROR_NONE
** ����ģ��: packetSendUnicast
*********************************************************************************************************/
static INT  informSend (PDHCPMSG  pdhcpmsgPacket, UINT  uiYiaddr)
{
    LOG(LOG_INFO,
        "Send inform to %s %02x:%02x:%02x:%02x:%02x:%02x\r\n",
		ip4addr_ntoa((const ip4_addr_t *)&uiYiaddr),
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);
  //  return packetSendUnicast(pdhcpmsgPacket, uiYiaddr);                 /* �����������ݰ�               */
    return packetSendBroadcast(pdhcpmsgPacket);
}

/*********************************************************************************************************
** ��������: discoverHandle
** ��������: discover������
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: offerSend
*********************************************************************************************************/
INT  discoverHandle (PDHCPMSG  pdhcpmsgPacket)
{
    return offerSend(pdhcpmsgPacket);
}

/*********************************************************************************************************
** ��������: requestHandle
** ��������: request������
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: requestSend
*********************************************************************************************************/
INT  requestHandle (PDHCPMSG  pdhcpmsgPacket)
{
    return requestSend(pdhcpmsgPacket);
}

/*********************************************************************************************************
** ��������: declienHandle
** ��������: declien������
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: leaseDecline
*********************************************************************************************************/
INT  declienHandle (PDHCPMSG  pdhcpmsgPacket)
{
    LOG(LOG_INFO,
        "Get Decline from %s %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&(pdhcpmsgPacket->uiYiaddr)),
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);

    return leaseDecline(pdhcpmsgPacket);
}

/*********************************************************************************************************
** ��������: releaseHandle
** ��������: release������
** ��    ��: pdhcpmsgSendPacket  Ҫ���͵�dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: leaseRelease
*********************************************************************************************************/
INT  releaseHandle (PDHCPMSG  pdhcpmsgPacket)
{
    LOG(LOG_INFO,
        "Get release from %s %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&(pdhcpmsgPacket->uiYiaddr)),
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);
    if (leaseRelease(pdhcpmsgPacket) == ERROR_NONE) {
        LOG(LOG_INFO,
            "Release all of %02x:%02x:%02x:%02x:%02x:%02x \r\n",
            pdhcpmsgPacket->ucChaddr[0],
            pdhcpmsgPacket->ucChaddr[1],
            pdhcpmsgPacket->ucChaddr[2],
            pdhcpmsgPacket->ucChaddr[3],
            pdhcpmsgPacket->ucChaddr[4],
            pdhcpmsgPacket->ucChaddr[5]);
        return (ERROR_NONE);
    } else {
        return (PX_ERROR);
    }
}

/*********************************************************************************************************
** ��������: informHandle
** ��������: inform������
** ��    ��: pdhcpmsgSendPacket  dhcp���ݰ�
** ��    ��: ����ERROR_NONE
** ����ģ��: informSend
*********************************************************************************************************/
INT  informHandle (PDHCPMSG  pdhcpmsgPacket)
{
    struct optionset  *poptionset       = NULL;
    DHCPMSG            dhcpmsgSendPacket;
//    PUCHAR             pcRequestIp      = NULL;
    UINT               uiIpAgain        = 0;

//    pcRequestIp = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);        /* ��ȡ���ݰ������ip           */
//    if (pcRequestIp) {
//        memcpy(&uiIpAgain, pcRequestIp, 4);
//    }
    LOG(LOG_INFO,
        "Get inform request from  %s %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&(pdhcpmsgPacket->uiCiaddr)),
        pdhcpmsgPacket->ucChaddr[0],
        pdhcpmsgPacket->ucChaddr[1],
        pdhcpmsgPacket->ucChaddr[2],
        pdhcpmsgPacket->ucChaddr[3],
        pdhcpmsgPacket->ucChaddr[4],
        pdhcpmsgPacket->ucChaddr[5]);

    uiIpAgain = pdhcpmsgPacket->uiCiaddr;

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPACK);        /* ��ʼ��inform��ΪACK����      */

    poptionset = GserverConfig.poptionsets;
    while (poptionset) {                                                /* �����Լ��ϸ���             */
        if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
            optionstrAdd((PUCHAR)(dhcpmsgSendPacket.ucOptions),(PUCHAR)(poptionset->pcData));
        }
        poptionset = poptionset->optionsetNext;
    }

    return informSend(&dhcpmsgSendPacket, uiIpAgain);                   /* ����inform���ݰ�             */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/

