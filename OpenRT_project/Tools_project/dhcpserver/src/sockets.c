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
** 文   件   名: sockets.c
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年2月27日
**
** 描        述: dhcp 网络通信相关功能
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

static INT              GiSendSocket;                                   /* 发送套接字                   */
static pthread_mutex_t  GmutexSendLock;                                 /* 发送套接字锁                 */
/*********************************************************************************************************
** 函数名称: rcvsocketInit
** 功能描述: 初始化接收套接字
** 输    入: piRcvSocket        接收套接字
**           psockaddrFrom      套接字地址选项
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  rcvsocketInit (INT                 *piRcvSocket,
                           struct sockaddr_in  *psockaddrFrom)
{
    INT          iRet;
    INT          iOpt  = 1;
    struct ifreq ifname;

    piRcvSocket[0] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);          /* 打开UDP套接字                */
    if (piRcvSocket[0] < 0) {                                           /* 打开失败返回PX_ERROR         */
        LOG(LOG_ERR,"UDP Recv socket init failed\r\n");
        return (PX_ERROR);
    }

    iRet = setsockopt(piRcvSocket[0],                                   /* 设置套接字可重复使用选项     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"UDP Recv socket setopt SO_REUSEADDR failed\r\n");
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }


    strncpy(ifname.ifr_ifrn.ifrn_name, GserverConfig.pcInteface,sizeof(ifname.ifr_ifrn.ifrn_name));
    iRet = setsockopt(piRcvSocket[0],                                   /* 设置套接字bind 指定设备     */
                      SOL_SOCKET,
                      SO_BINDTODEVICE,
                      (char *) &ifname,
                      sizeof(ifname));
    if ( iRet == PX_ERROR) {                                            /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"UDP bind to device %s failed\r\n", GserverConfig.pcInteface);
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }


    iRet = setsockopt(piRcvSocket[0],                                   /* 设置套接字广播选项           */
                      SOL_SOCKET,
                      SO_BROADCAST,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"UDP Recv socket setopt SO_BROADCAST failed\r\n");
        close(piRcvSocket[0]);
        return (PX_ERROR);
    }

    memset((void *)psockaddrFrom, 0, sizeof(struct sockaddr_in));       /* 清空套接字地址选项结构体     */
    psockaddrFrom->sin_len         = sizeof(struct sockaddr_in);
    psockaddrFrom->sin_family      = AF_INET;
    psockaddrFrom->sin_port        = htons(SERVER_PORT);                /* 配置地址服务端口             */
    psockaddrFrom->sin_addr.s_addr = htons(INADDR_ANY);

    iRet = bind(piRcvSocket[0],                                         /* 绑定套接字                   */
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
** 函数名称: sendsocketInit
** 功能描述: 初始化发送套接字
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  sendsocketInit (VOID)
{
    INT                 iRet;
    INT                 iOpt  = 1;
    struct sockaddr_in  sockaddrLocal;

    GiSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);            /* 打开UDP套接字                */
    if (GiSendSocket < 0) {                                             /* 打开失败返回PX_ERROR         */
        LOG(LOG_ERR,"UDP Send socket init failed\r\n");
        return (PX_ERROR);
    }

    iRet = setsockopt(GiSendSocket,                                     /* 设置套接字可重复使用选项     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"UDP Send socket setopt SO_REUSEADDR failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    iRet = setsockopt(GiSendSocket,                                     /* 设置套接字广播选项           */
                      SOL_SOCKET,
                      SO_BROADCAST,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if (iRet == PX_ERROR) {                                             /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"UDP Send socket setopt SO_BROADCAST failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    memset((void *)&sockaddrLocal, 0, sizeof(struct sockaddr_in));      /* 清空套接字地址选项结构体     */
    sockaddrLocal.sin_len         = sizeof(struct sockaddr_in);
    sockaddrLocal.sin_family      = AF_INET;
    sockaddrLocal.sin_port        = htons(SERVER_PORT);                 /* 配置地址服务端口             */
    sockaddrLocal.sin_addr.s_addr = GserverConfig.uiServer;

    iRet = bind(GiSendSocket,                                           /* 绑定套接字                   */
                (struct sockaddr *)&sockaddrLocal,
                sizeof(struct sockaddr_in));
    if (iRet < 0) {
        LOG(LOG_ERR,"UDP Send socket bind failed\r\n");
        close(GiSendSocket);
        return (PX_ERROR);
    }

    iRet = pthread_mutex_init(&GmutexSendLock, NULL);                   /* 初始化发送套接字互斥锁       */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "SendLock init failed!\r\n");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: icmpInit
** 功能描述: 初始化ICMP PING套接字
** 输    入: NONE
** 输    出: 成功返回socket文件句柄, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
INT  icmpInit (VOID)
{
	register INT    iSocket;
    INT             iOpt  = 1;
    INT             iRet;

    iSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);                  /* 打开ICMP套接字               */
    if (iSocket < 0) {                                                  /* 打开失败返回PX_ERROR         */
        return (PX_ERROR);
    }

    iRet = setsockopt(iSocket,                                          /* 设置套接字可重复使用选项     */
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (char *) &iOpt,
                      sizeof(iOpt));
    if ( iRet == PX_ERROR) {                                            /* 设置失败关闭套接字           */
        LOG(LOG_ERR,"icmp socket setopt SO_REUSEADDR failed\r\n");
        close(iSocket);
        return (PX_ERROR);
    }

    return (iSocket);
}

/*********************************************************************************************************
** 函数名称: socketInit
** 功能描述: 套接字初始化
** 输    入: piRcvSocket        接收套接字
**           psockaddrFrom      套接字地址选项
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: sendsocketInit，rcvsocketInit
*********************************************************************************************************/
INT   socketInit (INT  *piRcvSocket, struct sockaddr_in  *psockaddrFrom)
{
    if (rcvsocketInit(piRcvSocket, psockaddrFrom) != ERROR_NONE) {      /* 初始化接收套接字             */
        return (PX_ERROR);
    }

    if (sendsocketInit() != ERROR_NONE) {                               /* 初始化发送套接字             */
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: packetSendBroadcast
** 功能描述: 广播发送数据包
** 输    入: pdhcpmsgPacket   dhcp数据包
** 输    出: 返回ERROR_NONE
*********************************************************************************************************/
INT  packetSendBroadcast (PDHCPMSG  pdhcpmsgPacket)
{
    struct sockaddr_in  sockaddrTo;

    sockaddrTo.sin_len         = sizeof(struct sockaddr_in);            /* 配置套接字地址选项           */
    sockaddrTo.sin_family      = AF_INET;
    sockaddrTo.sin_port        = htons(CLIENT_PORT);                    /* 配置接收端接收端口           */
    sockaddrTo.sin_addr.s_addr =  htonl((u32_t)0xffffffffUL);           /* 配置广播地址                 */

    pthread_mutex_lock(&GmutexSendLock);

    sendto(GiSendSocket,                                                /* 发送数据包广播               */
           (void *)pdhcpmsgPacket,
           sizeof(DHCPMSG),
           0,
           (struct sockaddr *)&sockaddrTo,
           sizeof(sockaddrTo));

    pthread_mutex_unlock(&GmutexSendLock);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: packetSendUnicast
** 功能描述: 单播发送数据包
** 输    入: pdhcpmsgPacket   dhcp数据包
**           uiYiaddr         发送地址
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
INT  packetSendUnicast (PDHCPMSG  pdhcpmsgPacket, UINT uiYiaddr)
{
    struct sockaddr_in  sockaddrTo;

    sockaddrTo.sin_len         = sizeof(struct sockaddr_in);            /* 配置套接字地址选项           */
    sockaddrTo.sin_family      = AF_INET;
    sockaddrTo.sin_port        = htons(CLIENT_PORT);                    /* 配置接收端接收端口           */
    sockaddrTo.sin_addr.s_addr = uiYiaddr;                              /* 配置发送地址                 */

    pthread_mutex_lock(&GmutexSendLock);

    sendto(GiSendSocket,                                                /* 发送数据包单播               */
           (void *)pdhcpmsgPacket,
           sizeof(DHCPMSG),
           0,
           (struct sockaddr *)&sockaddrTo,
           sizeof(sockaddrTo));

    pthread_mutex_unlock(&GmutexSendLock);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: sendsocketClose
** 功能描述: 关闭发送套接字
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
VOID  sendsocketClose (VOID)
{
    pthread_mutex_lock(&GmutexSendLock);
    close(GiSendSocket);
    pthread_mutex_unlock(&GmutexSendLock);
    pthread_mutex_destroy(&GmutexSendLock);
}

/*********************************************************************************************************
** 函数名称: packetRecv
** 功能描述: 接收数据包
** 输    入: pdhcpmsgPacket   接收dhcp结构体地址
**           iRcvSocket       接收套接字
**           psockaddrFrom    套接字地址选项
** 输    出: 成功接收返回数据长度,接收失败返回RECVERROR,错误的包返回MSGERROR
** 调用模块: NONE
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

    memset(pdhcpmsgPacket, 0, sizeof(DHCPMSG));                         /* 清空接收结构体内容           */

    iRet = recvfrom(iRcvSocket,                                         /* 接收一个数据包               */
                    (void *)pdhcpmsgPacket,
                    sizeof(DHCPMSG),
                    0,
                    (struct sockaddr *)psockaddrFrom,
                    (socklen_t *)&iLen);
    if (iRet < 0) {                                                     /* 接收失败返回RECVERROR        */
        LOG(LOG_INFO, "couldn't read on listening socket, ignoring\r\n");
        return (RECVERROR);
    }

    if (ntohl(pdhcpmsgPacket->uiCookie) != DHCP_MAGIC) {                /* 判断接收内容是否正确         */
        LOG(LOG_ERR, "received bogus message, ignoring\r\n");           /* 包错误返回MSGERROR           */
        return (MSGERROR);
    }
    LOG(LOG_INFO, "Received a packet\r\n");

    if (pdhcpmsgPacket->ucOp == BOOTREQUEST &&                          /* 判断数据提供商               */
        (pcVendor = poptionGet(pdhcpmsgPacket, DHCP_VENDOR))) {
        for (i = 0; cBroken_vendors[i][0]; i++) {
            if (pcVendor[OPT_LEN - 2] == (unsigned char) strlen(cBroken_vendors[i]) &&
                !strncmp((PCHAR)pcVendor, cBroken_vendors[i], pcVendor[OPT_LEN - 2])) {
                LOG(LOG_INFO, "broken client (%s), forcing broadcast\r\n",
                cBroken_vendors[i]);
                pdhcpmsgPacket->usFlags |= htons(BROADCAST_FLAG);       /* 修改发送标志位为广播         */
            }
        }
    }

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: icmpSend
** 功能描述: 发送icmp包
** 输    入: inaddr          地址结构体
**           pusSeqRecv      发送验证字
**           iSocket         icmp套接字
** 输    出: 成功返回ERROR_NONR,失败返回PX_ERROR
** 调用模块: NONE
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

    memset(cBuf, 0, sizeof(cBuf));                                      /* 清空缓冲区                   */
    *pusSeqRecv          = usSeqNum;                                    /* 验证字                       */
    picmphdrEcho         = (struct icmp_echo_hdr  *)cBuf;
    picmphdrEcho->type   = ICMP_ECHO;                                   /* 填充包类型为icmp echo        */
    picmphdrEcho->code   = 0;
    picmphdrEcho->chksum = 0;
    picmphdrEcho->id     = 0xAFAF;                                      /* 填充发送id                   */
    picmphdrEcho->seqno  = htons(usSeqNum);                             /* 填充验证字                   */
    for (i = 0; i < 32; i++) {                                          /* 填充包数据区                 */
         ((PCHAR)picmphdrEcho)[sizeof(struct icmp_echo_hdr) + i] = (CHAR)(i % 256);
    }
    picmphdrEcho->chksum  = inet_chksum(picmphdrEcho,                   /* 添加校验                     */
                                        (u16_t)(32 + sizeof(struct icmp_echo_hdr)));

    sockaddrin.sin_len    = sizeof(struct sockaddr_in);                 /* 配置发送地址选项             */
    sockaddrin.sin_family = AF_INET;
    sockaddrin.sin_port   = 0;
    sockaddrin.sin_addr   = inaddr;

    luiError = sendto(iSocket,                                          /* 发送icmp echo数据包          */
                      picmphdrEcho,
                      luiLen,
                      0,
                      (const struct sockaddr *)&sockaddrin,
                      sizeof(struct sockaddr_in));

    usSeqNum++;                                                         /* 校验字增加                   */

    return ((luiError > 0) ? ERROR_NONE: PX_ERROR);                     /* 返回发送结果                 */
}

/*********************************************************************************************************
** 函数名称: icmpRecv
** 功能描述: 接收icmp包
** 输    入: inaddr          地址结构体
**           usSeqRecv       接收验证字
**           iSocket         icmp套接字
** 输    出: 成功返回ERROR_NONR,失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  icmpRecv (struct in_addr  inaddr, UINT16  usSeqRecv, INT  iSocket)
{
    CHAR                   cBuffer[512];
    INT                    iCnt        = 20;                            /*  默认最多接收的数据包数      */
    ULONG                  ulLen;
    INT                    iAddLen     = sizeof(struct sockaddr_in);
    struct sockaddr_in     sockaddrinFrom;
    struct ip_hdr         *iphdrFrom   = NULL;
    struct icmp_echo_hdr  *icmphdrFrom = NULL;

    while ((ulLen = recvfrom(iSocket,                                   /* 接收icmp 返回数据包          */
                             cBuffer,
                             sizeof(cBuffer),
                             0,
                             (struct sockaddr *) &sockaddrinFrom,
                             (socklen_t *) &iAddLen)) > 0) {
        if (ulLen >= (sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr))) {
            iphdrFrom   = (struct ip_hdr *) cBuffer;
            icmphdrFrom = (struct icmp_echo_hdr *) (cBuffer + (IPH_HL(iphdrFrom) * 4));
            if ((icmphdrFrom->id    == 0xAFAF) &&                       /* 判断数据包正确性             */
                (icmphdrFrom->seqno == htons(usSeqRecv))) {
                return (ERROR_NONE);
            }
        }

        iCnt--;                                                         /*  接收到错误的数据包太多      */
        if (iCnt < 0) {
            break;                                                      /*  退出                        */
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: icmpPing
** 功能描述: 检查地址是否有人占用(耗时操作)
** 输    入: pinaddr        地址结构体
** 输    出: 有人占用返回ERROR_NONE,否则返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  icmpPing (struct in_addr *pinaddr)
{
    INT           iTimeout  = 60;                                       /* 超时时间                     */
    INT           iTTL      = 64;                                       /* 数据包网络存在时间           */
    INT           iSuc      = 0;
    UINT16        usSeqRecv = 0;
    register INT  iSocket;
    struct sockaddr_in sockaddrinTo;

    iSocket = icmpInit();
    if (iSocket == PX_ERROR) {
        LOG(LOG_ERR, "iPingSocket init failed!!!\r\n");
        return (ERROR_NONE);
    }

    sockaddrinTo.sin_len    = sizeof(struct sockaddr_in);               /* 配置地址选项                 */
    sockaddrinTo.sin_family = AF_INET;
    sockaddrinTo.sin_addr   = *pinaddr;

    setsockopt(iSocket,                                                 /* 设置套接字超时选项           */
               SOL_SOCKET,
               SO_RCVTIMEO,
               &iTimeout,
               sizeof(INT));
    setsockopt(iSocket,                                                /* 配置ip数据接收选项           */
               IPPROTO_IP,
               IP_TTL,
               &iTTL,
               sizeof(INT));

    connect(iSocket, (struct sockaddr *) &sockaddrinTo,                 /* 建立套接字链接               */
            sizeof(struct sockaddr_in));

    if (icmpSend(*pinaddr, &usSeqRecv, iSocket) == ERROR_NONE) {        /* 发送icmp echo包              */
        usleep(500);                                                    /* 延时500us                    */
        if (icmpRecv(*pinaddr, usSeqRecv, iSocket) == ERROR_NONE) {     /* 接收icmp返回包               */
            iSuc++;
        }
    }

    if (iSuc == 0) {
        close(iSocket);
        return (PX_ERROR);                                              /* 收不到包返回PX_ERROR         */
    }

    close(iSocket);
    return (ERROR_NONE);                                                /* 收到返回包返回ERROR_NONE     */
}

/*********************************************************************************************************
** 函数名称: ipConflictCheck
** 功能描述: 检查地址是否有人占用(耗时操作)
** 输    入: uiYiaddr  需要检查的ip地址
** 输    出: 有人占用返回ERROR_NONE,否则返回PX_ERROR
** 调用模块: NONE
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

