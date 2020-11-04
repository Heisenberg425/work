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
** 文   件   名: packet.h
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年2月27日
**
** 描        述: dhcp数据包处理头文件
*********************************************************************************************************/
#ifndef PACKET_H_
#define PACKET_H_

#include <SylixOS.h>
#define SERVER_PORT        67                                           /* 服务器监听接口               */
#define CLIENT_PORT        68                                           /* 客户端接收接口               */

#define DHCP_MAGIC         0x63825363                                   /* DHCP魔数                     */
#define TYPE_MASK          0x0F                                         /* 掩码类型                     */

/*********************************************************************************************************
  选项信息数据类型
*********************************************************************************************************/
enum {
    OPTION_IP = 1,
    OPTION_IP_PAIR,
    OPTION_STRING,
    OPTION_BOOLEAN,
    OPTION_U8,
    OPTION_U16,
    OPTION_S16,
    OPTION_U32,
    OPTION_S32
};

#define OPTION_REQ          0x10                                        /* 选项申请                     */
#define OPTION_LIST         0x20                                        /* 选项列表                     */

/*********************************************************************************************************
  选项信息类型
*********************************************************************************************************/
#define DHCP_PADDING        0x00
#define DHCP_SUBNET         0x01
#define DHCP_TIME_OFFSET    0x02
#define DHCP_ROUTER         0x03
#define DHCP_TIME_SERVER    0x04
#define DHCP_NAME_SERVER    0x05
#define DHCP_DNS_SERVER     0x06
#define DHCP_LOG_SERVER     0x07
#define DHCP_COOKIE_SERVER  0x08
#define DHCP_LPR_SERVER     0x09
#define DHCP_HOST_NAME      0x0c
#define DHCP_BOOT_SIZE      0x0d
#define DHCP_DOMAIN_NAME    0x0f
#define DHCP_SWAP_SERVER    0x10
#define DHCP_ROOT_PATH      0x11
#define DHCP_IP_TTL         0x17
#define DHCP_MTU            0x1a
#define DHCP_BROADCAST      0x1c
#define DHCP_NTP_SERVER     0x2a
#define DHCP_WINS_SERVER    0x2c
#define DHCP_REQUESTED_IP   0x32
#define DHCP_LEASE_TIME     0x33
#define DHCP_OPTION_OVER    0x34
#define DHCP_MESSAGE_TYPE   0x35
#define DHCP_SERVER_ID      0x36
#define DHCP_PARAM_REQ      0x37
#define DHCP_MESSAGE        0x38
#define DHCP_MAX_SIZ        0x39
#define DHCP_T1             0x3a
#define DHCP_T2             0x3b
#define DHCP_VENDOR         0x3c
#define DHCP_CLIENT_ID      0x3d

#define DHCP_END            0xFF

/*********************************************************************************************************
  dhcp报文信息类型
*********************************************************************************************************/
#define BOOTREQUEST         1                                           /* bootp请求                    */
#define BOOTREPLY           2                                           /* bootp恢复                    */

#define ETH_10MB            1
#define ETH_10MB_LEN        6

#define DHCPDISCOVER        1                                           /* DISCOVER报文类型             */
#define DHCPOFFER           2                                           /* OFFER报文类型                */
#define DHCPREQUEST         3                                           /* REQUESET报文类型             */
#define DHCPDECLINE         4                                           /* DHCPDECLINE报文类型          */
#define DHCPACK             5                                           /* ACK报文类型                  */
#define DHCPNAK             6                                           /* NAK报文类型                  */
#define DHCPRELEASE         7                                           /* RELEASE报文类型              */
#define DHCPINFORM          8                                           /* DHCPINFORM报文类型           */

#define BROADCAST_FLAG      0x8000                                      /* 广播标志                     */

#define OPTION_FIELD        0
#define FILE_FIELD          1
#define SNAME_FIELD         2

#define MAC_BCAST_ADDR		(PUCHAR) "\xff\xff\xff\xff\xff\xff"         /* 广播地址                     */
#define OPT_CODE            0                                           /* 选项信息类型位置             */
#define OPT_LEN             1                                           /* 选项信息长度                 */
#define OPT_DATA            2                                           /* 选项信息内容                 */

/*********************************************************************************************************
  dhcp 报文结构体
*********************************************************************************************************/
struct dhcpMessage {
	UCHAR   ucOp;                                                       /* 信息传输方向                 */
	UCHAR   ucHtype;                                                    /* 硬件类型别                   */
	UCHAR   ucHlen;                                                     /* 硬件地址长度                 */
	UCHAR   ucHops;                                                     /* 跳数                         */
	UINT    uiXid;                                                      /* 事务ID                       */
	UINT16  usSecs;                                                     /* 地址已使用时间               */
	UINT16  usFlags;                                                    /* 标志位                       */
	UINT    uiCiaddr;                                                   /* 客户端IP地址                 */
	UINT    uiYiaddr;                                                   /* 分配给客户端的IP地址         */
	UINT    uiSiaddr;                                                   /* 下一个阶段要使用的服务器IP   */
	UINT    uiGiaddr;                                                   /* 跨网段转发地址               */
	UCHAR   ucChaddr[16];                                               /* 客户端硬件地址               */
	UCHAR   ucSname[64];                                                /* 服务器名称                   */
	UCHAR   ucFile[128];                                                /* 启动文件名                   */
	UINT    uiCookie;                                                   /* cookie                       */
	UCHAR   ucOptions[308];                                             /* 选项信息                     */
};

typedef struct dhcpMessage   DHCPMSG;
typedef struct dhcpMessage  *PDHCPMSG;

/*********************************************************************************************************
  dhcp选项结构体
*********************************************************************************************************/
struct dhcpoption {
	CHAR          cName[10];
	CHAR          cFlags;
	UCHAR         ucCode;
};

typedef struct dhcpoption   DHCPOPT;
typedef struct dhcpoption  *PDHCPOPT;

/*********************************************************************************************************
  选项全局变量
*********************************************************************************************************/
extern DHCPOPT              Gdhcpopts[];

extern INT                  GiOptlen[];

/*********************************************************************************************************
  功能函数
*********************************************************************************************************/
INT     discoverHandle (PDHCPMSG  pdhcpmsgPacket);
INT     requestHandle (PDHCPMSG  pdhcpmsgPacket);
INT     declienHandle (PDHCPMSG  pdhcpmsgPacket);
INT     releaseHandle (PDHCPMSG  pdhcpmsgPacket);
INT     informHandle (PDHCPMSG  pdhcpmsgPacket);
INT     optionstrAdd (PUCHAR  pcOptionptr, PUCHAR  pcString);
INT     optionsmpAdd (PUCHAR  pcOptionptr, UCHAR  ucCode, UINT  iData);
UCHAR   dhcpMsgtypeGet (PDHCPMSG  pdhcpmsgPacket);
PUCHAR  poptionGet (PDHCPMSG  pdhcpmsgPacket, CHAR cType);

#endif /* PACKET_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
