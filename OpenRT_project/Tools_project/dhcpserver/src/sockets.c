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
** ��   ��   ��: sockets.c
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��2��27��
**
** ��        ��: dhcp ����ͨ����ع���
*********************************************************************************************************/

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <string.h>
#include <sys/un.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if_arp.h>

#include <netif/etharp.h>
#include <net/if_ether.h>
#include <netpacket/packet.h>
#include <network/lwip/icmp.h>
#include <network/lwip/inet_chksum.h>
#include <net/if.h>

#include "log.h"
#include "packet.h"
#include "sockets.h"
#include "configini.h"

static INT              GiSendSocket;                                   /* �����׽���                   */
static pthread_mutex_t  GmutexSendLock;                                 /* �����׽�����                 */
/*********************************************************************************************************
** ��������: rcvsocketInit
** ��������: ��ʼ�������׽���
** ��    ��: piRcvSocket        �����׽���
**           psockaddrFrom      �׽��ֵ�ַѡ��
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  rcvsocketInit (INT                 *piRcvSocket,
                           struct sockaddr_in  *psockaddrFrom)
{
    INT          iRet;
    INT          iOpt  = 1;
    struct ifreq ifname;

    piRcvSocket[0] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);          /* ��UDP�׽���                */
    if (piRcvSocket[0] < 0) {                                           /* ��ʧ�ܷ���PX_ERROR         */
        LOG(LOG_ERR,"UDP Recv socket init failed\r\n");
        return (PX_ERROR);
    }

    iRet = setsockopt(piRcvSocket[0],                                   /* �����׽��ֿ��ظ�ʹ��ѡ��     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"UDP Recv socket setopt SO_REUSEADDR failed\r\n");
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }


    strncpy(ifname.ifr_ifrn.ifrn_name, GserverConfig.pcInteface,sizeof(ifname.ifr_ifrn.ifrn_name));
    iRet = setsockopt(piRcvSocket[0],                                   /* �����׽���bind ָ���豸     */
                      SOL_SOCKET,
                      SO_BINDTODEVICE,
                      (char *) &ifname,
                      sizeof(ifname));
    if ( iRet == PX_ERROR) {                                            /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"UDP bind to device %s failed\r\n", GserverConfig.pcInteface);
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }


    iRet = setsockopt(piRcvSocket[0],                                   /* �����׽��ֹ㲥ѡ��           */
                      SOL_SOCKET,
                      SO_BROADCAST,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"UDP Recv socket setopt SO_BROADCAST failed\r\n");
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }

    memset((void *)psockaddrFrom, 0, sizeof(struct sockaddr_in));       /* ����׽��ֵ�ַѡ��ṹ��     */
    psockaddrFrom->sin_len         = sizeof(struct sockaddr_in);
    psockaddrFrom->sin_family      = AF_INET;
    psockaddrFrom->sin_port        = htons(SERVER_PORT);                /* ���õ�ַ����˿�             */
    psockaddrFrom->sin_addr.s_addr = htons(INADDR_ANY);

    iRet = bind(piRcvSocket[0],                                         /* ���׽���                   */
                (struct sockaddr *)psockaddrFrom,
                sizeof(struct sockaddr_in));
    if (iRet < 0) {
        LOG(LOG_ERR,"UDP Recv socket bind failed\r\n");
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: sendsocketInit
** ��������: ��ʼ�������׽���
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  sendsocketInit (VOID)
{
    INT                 iRet;
    INT                 iOpt  = 1;
    struct sockaddr_in  sockaddrLocal;

    GiSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);            /* ��UDP�׽���                */
    if (GiSendSocket < 0) {                                             /* ��ʧ�ܷ���PX_ERROR         */
        LOG(LOG_ERR,"UDP Send socket init failed\r\n");
        return (PX_ERROR);
    }

    iRet = setsockopt(GiSendSocket,                                     /* �����׽��ֿ��ظ�ʹ��ѡ��     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"UDP Send socket setopt SO_REUSEADDR failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    iRet = setsockopt(GiSendSocket,                                     /* �����׽��ֹ㲥ѡ��           */
                      SOL_SOCKET,
                      SO_BROADCAST,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"UDP Send socket setopt SO_BROADCAST failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    memset((void *)&sockaddrLocal, 0, sizeof(struct sockaddr_in));      /* ����׽��ֵ�ַѡ��ṹ��     */
    sockaddrLocal.sin_len         = sizeof(struct sockaddr_in);
    sockaddrLocal.sin_family      = AF_INET;
    sockaddrLocal.sin_port        = htons(SERVER_PORT);                 /* ���õ�ַ����˿�             */
    sockaddrLocal.sin_addr.s_addr = GserverConfig.uiServer;

    iRet = bind(GiSendSocket,                                           /* ���׽���                   */
                (struct sockaddr *)&sockaddrLocal,
                sizeof(struct sockaddr_in));
    if (iRet < 0) {
        LOG(LOG_ERR,"UDP Send socket bind failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    iRet = pthread_mutex_init(&GmutexSendLock, NULL);                   /* ��ʼ�������׽��ֻ�����       */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "SendLock init failed!\r\n");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: icmpInit
** ��������: ��ʼ��ICMP PING�׽���
** ��    ��: NONE
** ��    ��: �ɹ�����socket�ļ����, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  icmpInit (VOID)
{
	register INT    iSocket;
    INT             iOpt  = 1;
    INT             iRet;

    iSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);                  /* ��ICMP�׽���               */
    if (iSocket < 0) {                                                  /* ��ʧ�ܷ���PX_ERROR         */
        return (PX_ERROR);
    }

    iRet = setsockopt(iSocket,                                          /* �����׽��ֿ��ظ�ʹ��ѡ��     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if ( iRet == PX_ERROR) {                                            /* ����ʧ�ܹر��׽���           */
        LOG(LOG_ERR,"icmp socket setopt SO_REUSEADDR failed\r\n");
        close(iSocket);
        return (PX_ERROR);
    }

    return (iSocket);
}

/*********************************************************************************************************
** ��������: socketInit
** ��������: �׽��ֳ�ʼ��
** ��    ��: piRcvSocket        �����׽���
**           psockaddrFrom      �׽��ֵ�ַѡ��
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: sendsocketInit��rcvsocketInit
*********************************************************************************************************/
INT   socketInit (INT  *piRcvSocket, struct sockaddr_in  *psockaddrFrom)
{
    if (rcvsocketInit(piRcvSocket, psockaddrFrom) != ERROR_NONE) {      /* ��ʼ�������׽���             */
        return (PX_ERROR);
    }

    if (sendsocketInit() != ERROR_NONE) {                               /* ��ʼ�������׽���             */
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: packetSendBroadcast
** ��������: �㲥�������ݰ�
** ��    ��: pdhcpmsgPacket   dhcp���ݰ�
** ��    ��: ����ERROR_NONE
*********************************************************************************************************/
INT  packetSendBroadcast (PDHCPMSG  pdhcpmsgPacket)
{
    struct sockaddr_in  sockaddrTo;

    sockaddrTo.sin_len         = sizeof(struct sockaddr_in);            /* �����׽��ֵ�ַѡ��           */
    sockaddrTo.sin_family      = AF_INET;
    sockaddrTo.sin_port        = htons(CLIENT_PORT);                    /* ���ý��ն˽��ն˿�           */
    sockaddrTo.sin_addr.s_addr =  htonl((u32_t)0xffffffffUL);           /* ���ù㲥��ַ                 */

    pthread_mutex_lock(&GmutexSendLock);

    sendto(GiSendSocket,                                                /* �������ݰ��㲥               */
           (void *)pdhcpmsgPacket,
           sizeof(DHCPMSG),
           0,
           (struct sockaddr *)&sockaddrTo,
           sizeof(sockaddrTo));

    pthread_mutex_unlock(&GmutexSendLock);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: packetSendUnicast
** ��������: �����������ݰ�
** ��    ��: pdhcpmsgPacket   dhcp���ݰ�
**           uiYiaddr         ���͵�ַ
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
INT  packetSendUnicast (PDHCPMSG  pdhcpmsgPacket, UINT uiYiaddr)
{
    struct sockaddr_in  sockaddrTo;

    sockaddrTo.sin_len         = sizeof(struct sockaddr_in);            /* �����׽��ֵ�ַѡ��           */
    sockaddrTo.sin_family      = AF_INET;
    sockaddrTo.sin_port        = htons(CLIENT_PORT);                    /* ���ý��ն˽��ն˿�           */
    sockaddrTo.sin_addr.s_addr = uiYiaddr;                              /* ���÷��͵�ַ                 */

    pthread_mutex_lock(&GmutexSendLock);

    sendto(GiSendSocket,                                                /* �������ݰ�����               */
           (void *)pdhcpmsgPacket,
           sizeof(DHCPMSG),
           0,
           (struct sockaddr *)&sockaddrTo,
           sizeof(sockaddrTo));

    pthread_mutex_unlock(&GmutexSendLock);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: sendsocketClose
** ��������: �رշ����׽���
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
VOID  sendsocketClose (VOID)
{
    pthread_mutex_lock(&GmutexSendLock);
    close(GiSendSocket);
    pthread_mutex_unlock(&GmutexSendLock);
    pthread_mutex_destroy(&GmutexSendLock);
}

/*********************************************************************************************************
** ��������: packetRecv
** ��������: �������ݰ�
** ��    ��: pdhcpmsgPacket   ����dhcp�ṹ���ַ
**           iRcvSocket       �����׽���
**           psockaddrFrom    �׽��ֵ�ַѡ��
** ��    ��: �ɹ����շ������ݳ���,����ʧ�ܷ���RECVERROR,����İ�����MSGERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  packetRecv (PDHCPMSG             pdhcpmsgPacket,
                 INT                  iRcvSocket,
                 struct sockaddr_in  *psockaddrFrom)
{
    INT        iRet;
    INT        iLen = sizeof(struct sockaddr_in);;
    INT        i;
    PUCHAR     pcVendor;
    const CHAR cBroken_vendors[][8] = {
        "MSFT 98",
        ""
    };

    memset(pdhcpmsgPacket, 0, sizeof(DHCPMSG));                         /* ��ս��սṹ������           */

    iRet = recvfrom(iRcvSocket,                                         /* ����һ�����ݰ�               */
                    (void *)pdhcpmsgPacket,
                    sizeof(DHCPMSG),
                    0,
                    (struct sockaddr *)psockaddrFrom,
                    (socklen_t *)&iLen);
    if (iRet < 0) {                                                     /* ����ʧ�ܷ���RECVERROR        */
        LOG(LOG_INFO, "couldn't read on listening socket, ignoring\r\n");
        return (RECVERROR);
    }

    if (ntohl(pdhcpmsgPacket->uiCookie) != DHCP_MAGIC) {                /* �жϽ��������Ƿ���ȷ         */
        LOG(LOG_ERR, "received bogus message, ignoring\r\n");           /* �����󷵻�MSGERROR           */
        return (MSGERROR);
    }
    LOG(LOG_INFO, "Received a packet\r\n");

    if (pdhcpmsgPacket->ucOp == BOOTREQUEST &&                          /* �ж������ṩ��               */
        (pcVendor = poptionGet(pdhcpmsgPacket, DHCP_VENDOR))) {
        for (i = 0; cBroken_vendors[i][0]; i++) {
            if (pcVendor[OPT_LEN - 2] == (unsigned char) strlen(cBroken_vendors[i]) &&
                !strncmp((PCHAR)pcVendor, cBroken_vendors[i], pcVendor[OPT_LEN - 2])) {
                LOG(LOG_INFO, "broken client (%s), forcing broadcast\r\n",
                cBroken_vendors[i]);
                pdhcpmsgPacket->usFlags |= htons(BROADCAST_FLAG);       /* �޸ķ��ͱ�־λΪ�㲥         */
            }
        }
    }

    return (iRet);
}

/*********************************************************************************************************
** ��������: icmpSend
** ��������: ����icmp��
** ��    ��: inaddr          ��ַ�ṹ��
**           pusSeqRecv      ������֤��
**           iSocket         icmp�׽���
** ��    ��: �ɹ�����ERROR_NONR,ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  icmpSend (struct in_addr  inaddr, UINT16  *pusSeqRecv, INT  iSocket)
{
    CHAR                   cBuf[100];
    long unsigned int      luiError;
    long unsigned int      luiLen       = sizeof(struct icmp_echo_hdr) + 32;
    struct icmp_echo_hdr  *picmphdrEcho = NULL;
    static UINT16          usSeqNum     = 1;
    struct sockaddr_in     sockaddrin;
     INT                   i;

    memset(cBuf, 0, sizeof(cBuf));                                      /* ��ջ�����                   */
    *pusSeqRecv          = usSeqNum;                                    /* ��֤��                       */
    picmphdrEcho         = (struct icmp_echo_hdr  *)cBuf;
    picmphdrEcho->type   = ICMP_ECHO;                                   /* ��������Ϊicmp echo        */
    picmphdrEcho->code   = 0;
    picmphdrEcho->chksum = 0;
    picmphdrEcho->id     = 0xAFAF;                                      /* ��䷢��id                   */
    picmphdrEcho->seqno  = htons(usSeqNum);                             /* �����֤��                   */
    for (i = 0; i < 32; i++) {                                          /* ����������                 */
         ((PCHAR)picmphdrEcho)[sizeof(struct icmp_echo_hdr) + i] = (CHAR)(i % 256);
    }
    picmphdrEcho->chksum  = inet_chksum(picmphdrEcho,                   /* ���У��                     */
                                        (u16_t)(32 + sizeof(struct icmp_echo_hdr)));

    sockaddrin.sin_len    = sizeof(struct sockaddr_in);                 /* ���÷��͵�ַѡ��             */
    sockaddrin.sin_family = AF_INET;
    sockaddrin.sin_port   = 0;
    sockaddrin.sin_addr   = inaddr;

    luiError = sendto(iSocket,                                          /* ����icmp echo���ݰ�          */
                      picmphdrEcho,
                      luiLen,
                      0,
                      (const struct sockaddr *)&sockaddrin,
                      sizeof(struct sockaddr_in));

    usSeqNum++;                                                         /* У��������                   */

    return ((luiError > 0) ? ERROR_NONE: PX_ERROR);                     /* ���ط��ͽ��                 */
}

/*********************************************************************************************************
** ��������: icmpRecv
** ��������: ����icmp��
** ��    ��: inaddr          ��ַ�ṹ��
**           usSeqRecv       ������֤��
**           iSocket         icmp�׽���
** ��    ��: �ɹ�����ERROR_NONR,ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  icmpRecv (struct in_addr  inaddr, UINT16  usSeqRecv, INT  iSocket)
{
    CHAR                   cBuffer[512];
    INT                    iCnt        = 20;                            /*  Ĭ�������յ����ݰ���      */
    ULONG                  ulLen;
    INT                    iAddLen     = sizeof(struct sockaddr_in);
    struct sockaddr_in     sockaddrinFrom;
    struct ip_hdr         *iphdrFrom   = NULL;
    struct icmp_echo_hdr  *icmphdrFrom = NULL;

    while ((ulLen = recvfrom(iSocket,                                   /* ����icmp �������ݰ�          */
                             cBuffer,
                             sizeof(cBuffer),
                             0,
                             (struct sockaddr *) &sockaddrinFrom,
                             (socklen_t *) &iAddLen)) > 0) {
        if (ulLen >= (sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr))) {
            iphdrFrom   = (struct ip_hdr *) cBuffer;
            icmphdrFrom = (struct icmp_echo_hdr *) (cBuffer + (IPH_HL(iphdrFrom) * 4));
            if ((icmphdrFrom->id    == 0xAFAF) &&                       /* �ж����ݰ���ȷ��             */
                (icmphdrFrom->seqno == htons(usSeqRecv))) {
                return (ERROR_NONE);
            }
        }

        iCnt--;                                                         /*  ���յ���������ݰ�̫��      */
        if (iCnt < 0) {
            break;                                                      /*  �˳�                        */
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: icmpPing
** ��������: ����ַ�Ƿ�����ռ��(��ʱ����)
** ��    ��: pinaddr        ��ַ�ṹ��
** ��    ��: ����ռ�÷���ERROR_NONE,���򷵻�PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  icmpPing (struct in_addr *pinaddr)
{
    INT           iTimeout  = 60;                                       /* ��ʱʱ��                     */
    INT           iTTL      = 64;                                       /* ���ݰ��������ʱ��           */
    INT           iSuc      = 0;
    UINT16        usSeqRecv = 0;
    register INT  iSocket;
    struct sockaddr_in sockaddrinTo;

    iSocket = icmpInit();
    if (iSocket == PX_ERROR) {
        LOG(LOG_ERR, "iPingSocket init failed!!!\r\n");
        return (ERROR_NONE);
    }

    sockaddrinTo.sin_len    = sizeof(struct sockaddr_in);               /* ���õ�ַѡ��                 */
    sockaddrinTo.sin_family = AF_INET;
    sockaddrinTo.sin_addr   = *pinaddr;

    setsockopt(iSocket,                                                 /* �����׽��ֳ�ʱѡ��           */
               SOL_SOCKET,
               SO_RCVTIMEO,
               &iTimeout,
               sizeof(INT));
    setsockopt(iSocket,                                                /* ����ip���ݽ���ѡ��           */
               IPPROTO_IP,
               IP_TTL,
               &iTTL,
               sizeof(INT));

    connect(iSocket, (struct sockaddr *) &sockaddrinTo,                 /* �����׽�������               */
            sizeof(struct sockaddr_in));

    if (icmpSend(*pinaddr, &usSeqRecv, iSocket) == ERROR_NONE) {        /* ����icmp echo��              */
        usleep(500);                                                    /* ��ʱ500us                    */
        if (icmpRecv(*pinaddr, usSeqRecv, iSocket) == ERROR_NONE) {     /* ����icmp���ذ�               */
            iSuc++;
        }
    }

    if (iSuc == 0) {
        close(iSocket);
        return (PX_ERROR);                                              /* �ղ���������PX_ERROR         */
    }

    close(iSocket);
    return (ERROR_NONE);                                                /* �յ����ذ�����ERROR_NONE     */
}

/*********************************************************************************************************
** ��������: ipConflictCheck
** ��������: ����ַ�Ƿ�����ռ��(��ʱ����)
** ��    ��: uiYiaddr  ��Ҫ����ip��ַ
** ��    ��: ����ռ�÷���ERROR_NONE,���򷵻�PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  ipConflictCheck(UINT  uiYiaddr)
{
    struct in_addr inaddr;

    inaddr.s_addr = uiYiaddr;
    if (icmpPing(&inaddr) < 0) {
        return (PX_ERROR);
    } else {
        return (ERROR_NONE);
    }

}
/*********************************************************************************************************
  END
*********************************************************************************************************/

