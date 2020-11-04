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
** 文   件   名: packet.c
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年2月27日
**
** 描        述: DHCP数据包处理功能
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
  DHCP 选项结构体数组
*********************************************************************************************************/
struct dhcpoption  Gdhcpopts[] = {
   /*
    * 关键字      标志位                                标志码
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
** 函数名称: optionGet
** 功能描述: 获取选项字段
** 输    入: pdhcpmsgPacket     dhcp数据包
**           cType              选项类型
** 输    出: 返回对应字段指针
** 调用模块: NONE
*********************************************************************************************************/
PUCHAR  poptionGet (PDHCPMSG  pdhcpmsgPacket, CHAR  cType)
{
    INT        i           = 0;
    INT        iLength     = 308;                                       /* 312字节Option - 4字节cookie  */
    INT        iOver       = 0;
    INT        iDown       = 0;
    INT        iCurr       = OPTION_FIELD;
    PUCHAR     pcOptionptr = NULL;

    pcOptionptr = pdhcpmsgPacket->ucOptions;

    while (!iDown) {                                                    /* 完成查找前循环遍历           */
        if (i >= iLength) {                                             /* 若未找到返回空               */
           LOG(LOG_DEBUG,"bogus packet, option fields too long.\r\n");
           return (NULL);
        }

        if (pcOptionptr[i + OPT_CODE] == cType) {                       /* 如果找到对应的选项           */
            if (i + 1 + pcOptionptr[i + OPT_LEN] >= iLength) {          /* 判断是否选项超长             */
                LOG(LOG_DEBUG, "bogus packet, option fields too long.\r\n");
                return (NULL);
            }
            return (pcOptionptr + i + 2);                                 /* 返回选项地址                 */
        }

        switch (pcOptionptr[i + OPT_CODE]) {                            /* 判断选项类型                 */

        case DHCP_PADDING:                                              /* PADDING数据则一个个过滤      */
            i++;
            break;

        case DHCP_OPTION_OVER:                                          /* 记录option_over位置          */
            if (i + 1 + pcOptionptr[i + OPT_LEN] >= iLength) {
                LOG(LOG_DEBUG, "bogus packet, option fields too long.\r\n");
                return (NULL);
            }
            iOver   = pcOptionptr[i + 3];
            i      += pcOptionptr[OPT_LEN] + 2;
            break;

        case DHCP_END:                                                  /* 如果是选项结束符             */
            if (iCurr == OPTION_FIELD && iOver & FILE_FIELD) {          /* 文件路径数据重定位查找       */
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
                iDown = 1;                                              /* 查找结束                     */
            }
            break;

        default:
            i += pcOptionptr[OPT_LEN + i] + 2;
        }
    }
    return (NULL);
}

/*********************************************************************************************************
** 函数名称: optionEnd
** 功能描述: 定位到选项末尾
** 输    入: pcOptionptr   选项结构体指针
** 输    出: 返回选项末尾位置
** 调用模块: NONE
*********************************************************************************************************/
static INT  optionEnd (PUCHAR  pcOptionptr)
{
    INT  i = 0;

    if (pcOptionptr == NULL) {                                          /* 判空                         */
        return (PX_ERROR);
    }

    while (pcOptionptr[i] != DHCP_END) {                                /* 遍历选项到选项结束符         */
        if (pcOptionptr[i] == DHCP_PADDING) {                           /* PADDING时一个个过滤          */
            i++;
        } else {                                                        /* 普通选项直接跳过选项长度     */
            i += pcOptionptr[i + OPT_LEN] + 2;
        }
    }

    return (i);                                                         /* 返回选项末尾标签             */
}

/*********************************************************************************************************
** 函数名称: optionstrAdd
** 功能描述: 添加选项字符串内容
** 输    入: pcOptionptr    选项结构指针
**           pcString       选项内容字符串
** 输    出: 返回添加选项的位置
** 调用模块: NONE
*********************************************************************************************************/
INT  optionstrAdd (PUCHAR  pcOptionptr,
                   PUCHAR  pcString)
{
    INT iEnd  = optionEnd(pcOptionptr);                                 /* 定位到选项末尾               */

    if (iEnd + pcString[OPT_LEN] + 2 + 1 >= 308) {                      /* 检查选项是否超长             */
        LOG(LOG_ERR, "Option did not fit into the packet!\r\n");
        return (0);
    }

    memcpy(pcOptionptr + iEnd, pcString, pcString[OPT_LEN] + 2);        /* 插入选项字符串到选项尾       */
    pcOptionptr[iEnd + pcString[OPT_LEN] + 2] = DHCP_END;               /* 重新添加选项结束符           */

    return (pcString[OPT_LEN] + 2);                                     /* 返回选项内容地址             */
}

/*********************************************************************************************************
** 函数名称: optionsmpAdd
** 功能描述: 添加简单选项内容
** 输    入: pcOptionptr     选项结构指针
**           ucCode          添加选项类型
**           iData           简单选项数据
** 输    出: 返回添加选项的位置
** 调用模块: NONE
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

    pcData    =  (PUCHAR)&iAligned;                                     /* 数据类型墙强转               */
    pusData   =  (UINT16 *)&iAligned;
    puiData   =  &iAligned;

    for (iCount = 0; Gdhcpopts[iCount].ucCode; iCount++) {              /* 遍历dhcp选项组               */
        if (Gdhcpopts[iCount].ucCode == ucCode) {
            cLength = GiOptlen[Gdhcpopts[iCount].cFlags & TYPE_MASK];   /* 取相关类型选项长度           */
            break;
        }
    }

    if (!cLength) {                                                     /* 判断长度获取是否正确         */
        LOG(LOG_ERR,"Could not add option\r\n");
        return (0);
    }

    ucOption[OPT_CODE] = ucCode;                                        /* 填充选项类型                 */
    ucOption[OPT_LEN]  = cLength;                                       /* 填充选项长度                 */

    switch (cLength) {                                                  /* 判断选项长度填充对应选项内容 */

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
    memcpy(ucOption + 2, &iAligned, cLength);                           /* 选项内容填充字符串           */

    return optionstrAdd(pcOptionptr, ucOption);                         /* 添加字符串选项               */
}

/*********************************************************************************************************
** 函数名称: dhcpMsgtypeGet
** 功能描述: 获取消息类型
** 输    入: pdhcpmsgPacket  dhcp数据包
** 输    出: 获取消息类型
** 调用模块: API
*********************************************************************************************************/
UCHAR  dhcpMsgtypeGet (PDHCPMSG  pdhcpmsgPacket)
{
    PUCHAR pcType;

    if (pdhcpmsgPacket == NULL) {                                       /* 判空                         */
        LOG(LOG_DEBUG, "packet is null\r\n");
        return (PX_ERROR);
    }

    pcType = poptionGet(pdhcpmsgPacket, DHCP_MESSAGE_TYPE);             /* 获取dhcp类型                 */
    if (pcType == NULL) {                                               /* 判断获取是否正确             */
        LOG(LOG_DEBUG, "Get msgtype filed\r\n");
        return (PX_ERROR);
    }

    return (pcType[0]);                                                   /* 返回数据包类型               */
}

/*********************************************************************************************************
** 函数名称: dhcpPacketInit
** 功能描述: 获取消息类型
** 输    入: pdhcpmsgSendPacket      要发送的数据包
**           pdhcpmsgPacket          源数据包
**           cType                   初始化类型
** 输    出: 获取消息类型
** 调用模块: NONE
*********************************************************************************************************/
static VOID  dhcpPacketInit (PDHCPMSG  pdhcpmsgSendPacket,
                             PDHCPMSG  pdhcpmsgPacket,
                             CHAR      cType)
{
    memset(pdhcpmsgSendPacket, 0, sizeof(DHCPMSG));                     /* 清空发送数据包内容           */

    switch (cType) {                                                    /* 判断初始化类型               */

    case DHCPDISCOVER:
    case DHCPREQUEST:
    case DHCPRELEASE:
    case DHCPINFORM:
        pdhcpmsgSendPacket->ucOp = BOOTREQUEST;                         /* 以上类型为BOOTREQUEST        */
        break;

    case DHCPOFFER:
    case DHCPACK:
    case DHCPNAK:
        pdhcpmsgSendPacket->ucOp = BOOTREPLY;                           /* 以上为BOOTREPLY              */
        break;

    default:
        break;
    }

    pdhcpmsgSendPacket->ucHtype      = ETH_10MB;                        /* 填充硬件类型                 */
    pdhcpmsgSendPacket->ucHlen       = ETH_10MB_LEN;                    /* 填充硬件长度                 */
    pdhcpmsgSendPacket->uiCookie     = htonl(DHCP_MAGIC);               /* 填充魔数                     */
    pdhcpmsgSendPacket->ucOptions[0] = DHCP_END;                        /* 填充选项字结束符             */
    pdhcpmsgSendPacket->uiXid        = pdhcpmsgPacket->uiXid;           /* 填充事务id                   */
    memcpy(pdhcpmsgSendPacket->ucChaddr, pdhcpmsgPacket->ucChaddr, 16); /* 填充硬件地址                 */
    pdhcpmsgSendPacket->usFlags      = pdhcpmsgPacket->usFlags;         /* 填充数据标志位               */
    pdhcpmsgSendPacket->uiGiaddr     = pdhcpmsgPacket->uiGiaddr;        /* 填充giaddr                   */
    pdhcpmsgSendPacket->uiCiaddr     = pdhcpmsgPacket->uiCiaddr;        /* 填充ciaddr                   */
    optionsmpAdd(pdhcpmsgSendPacket->ucOptions,                         /* 填充数据包类型               */
                 DHCP_MESSAGE_TYPE,
                 cType);
    optionsmpAdd(pdhcpmsgSendPacket->ucOptions,                         /* 填充服务器地址               */
                 DHCP_SERVER_ID,
                 GserverConfig.uiServer);
}

/*********************************************************************************************************
** 函数名称: nakSend
** 功能描述: 发送NAK回应
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 返回ERROR_NONE
** 调用模块: packetSendBroadcast
*********************************************************************************************************/
static INT  nakSend (PDHCPMSG  pdhcpmsgPacket)
{
    DHCPMSG  dhcpmsgSendPacket;

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPNAK);        /* 初始化数据包为NAK类型        */

    LOG(LOG_INFO,
        "Send nak %s of %02x:%02x:%02x:%02x:%02x:%02x \r\n",
		ip4addr_ntoa((const ip4_addr_t *)&(dhcpmsgSendPacket.uiYiaddr)),
        dhcpmsgSendPacket.ucChaddr[0],
        dhcpmsgSendPacket.ucChaddr[1],
        dhcpmsgSendPacket.ucChaddr[2],
        dhcpmsgSendPacket.ucChaddr[3],
        dhcpmsgSendPacket.ucChaddr[4],
        dhcpmsgSendPacket.ucChaddr[5]);

    return packetSendBroadcast(&dhcpmsgSendPacket);                     /* 广播发送数据包               */
}

/*********************************************************************************************************
** 函数名称: requestSend
** 功能描述: 发送ACK回应
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: leaseAck
*********************************************************************************************************/
static INT  requestSend (PDHCPMSG  pdhcpmsgPacket)
{
   INT                 iRet;
    DHCPMSG            dhcpmsgSendPacket;
    PUCHAR             pcRequestIp = NULL;
    UINT               uiIp;

    memset(&dhcpmsgSendPacket, 0 ,sizeof(dhcpmsgSendPacket));           /* 清空数据包                   */

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPACK);        /* 初始化数据包为ACK类型        */

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* 获取客户端申请的ip           */
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

    iRet = leaseAck(pdhcpmsgPacket, &dhcpmsgSendPacket);                /* 进行ACK处理                  */
    if ( iRet == PX_ERROR) {                                            /* 如果ACK处理失败则发送NAK     */
        return nakSend(pdhcpmsgPacket);
    } else if (iRet == ERROR_NONE) {                                    /* 不需要进行任何处理           */
        LOG(LOG_DEBUG, "ACK NONE !\r\n");
        return (iRet);
    } else {                                                            /* 如需要应答则发送ACK          */
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
** 函数名称: offerSend
** 功能描述: 发送offer包
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: discoverOfferGet,packetSendBroadcast
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

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPOFFER);      /* 初始化数据包为OFFER类型      */

    if (discoverOfferGet(pdhcpmsgPacket,                                /* 获取可用租约                 */
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
        return packetSendBroadcast(&dhcpmsgSendPacket);                 /* 广播发送数据包               */
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: informSend
** 功能描述: 发送inform包
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
**           uiYiaddr            发送地址
** 输    出: 返回ERROR_NONE
** 调用模块: packetSendUnicast
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
  //  return packetSendUnicast(pdhcpmsgPacket, uiYiaddr);                 /* 单播发送数据包               */
    return packetSendBroadcast(pdhcpmsgPacket);
}

/*********************************************************************************************************
** 函数名称: discoverHandle
** 功能描述: discover处理函数
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: offerSend
*********************************************************************************************************/
INT  discoverHandle (PDHCPMSG  pdhcpmsgPacket)
{
    return offerSend(pdhcpmsgPacket);
}

/*********************************************************************************************************
** 函数名称: requestHandle
** 功能描述: request处理函数
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: requestSend
*********************************************************************************************************/
INT  requestHandle (PDHCPMSG  pdhcpmsgPacket)
{
    return requestSend(pdhcpmsgPacket);
}

/*********************************************************************************************************
** 函数名称: declienHandle
** 功能描述: declien处理函数
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: leaseDecline
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
** 函数名称: releaseHandle
** 功能描述: release处理函数
** 输    入: pdhcpmsgSendPacket  要发送的dhcp数据包
** 输    出: 成功返回ERROR_NONE,失败返回PX_ERROR
** 调用模块: leaseRelease
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
** 函数名称: informHandle
** 功能描述: inform处理函数
** 输    入: pdhcpmsgSendPacket  dhcp数据包
** 输    出: 返回ERROR_NONE
** 调用模块: informSend
*********************************************************************************************************/
INT  informHandle (PDHCPMSG  pdhcpmsgPacket)
{
    struct optionset  *poptionset       = NULL;
    DHCPMSG            dhcpmsgSendPacket;
//    PUCHAR             pcRequestIp      = NULL;
    UINT               uiIpAgain        = 0;

//    pcRequestIp = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);        /* 获取数据包申请的ip           */
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

    dhcpPacketInit(&dhcpmsgSendPacket, pdhcpmsgPacket, DHCPACK);        /* 初始化inform包为ACK类型      */

    poptionset = GserverConfig.poptionsets;
    while (poptionset) {                                                /* 填充租约详细情况             */
        if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
            optionstrAdd((PUCHAR)(dhcpmsgSendPacket.ucOptions),(PUCHAR)(poptionset->pcData));
        }
        poptionset = poptionset->optionsetNext;
    }

    return informSend(&dhcpmsgSendPacket, uiIpAgain);                   /* 发送inform数据包             */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/

