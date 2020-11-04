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
** ��   ��   ��: packet.h
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��2��27��
**
** ��        ��: dhcp���ݰ�����ͷ�ļ�
*********************************************************************************************************/
#ifndef PACKET_H_
#define PACKET_H_

#include <SylixOS.h>
#define SERVER_PORT        67                                           /* �����������ӿ�               */
#define CLIENT_PORT        68                                           /* �ͻ��˽��սӿ�               */

#define DHCP_MAGIC         0x63825363                                   /* DHCPħ��                     */
#define TYPE_MASK          0x0F                                         /* ��������                     */

/*********************************************************************************************************
  ѡ����Ϣ��������
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

#define OPTION_REQ          0x10                                        /* ѡ������                     */
#define OPTION_LIST         0x20                                        /* ѡ���б�                     */

/*********************************************************************************************************
  ѡ����Ϣ����
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
  dhcp������Ϣ����
*********************************************************************************************************/
#define BOOTREQUEST         1                                           /* bootp����                    */
#define BOOTREPLY           2                                           /* bootp�ָ�                    */

#define ETH_10MB            1
#define ETH_10MB_LEN        6

#define DHCPDISCOVER        1                                           /* DISCOVER��������             */
#define DHCPOFFER           2                                           /* OFFER��������                */
#define DHCPREQUEST         3                                           /* REQUESET��������             */
#define DHCPDECLINE         4                                           /* DHCPDECLINE��������          */
#define DHCPACK             5                                           /* ACK��������                  */
#define DHCPNAK             6                                           /* NAK��������                  */
#define DHCPRELEASE         7                                           /* RELEASE��������              */
#define DHCPINFORM          8                                           /* DHCPINFORM��������           */

#define BROADCAST_FLAG      0x8000                                      /* �㲥��־                     */

#define OPTION_FIELD        0
#define FILE_FIELD          1
#define SNAME_FIELD         2

#define MAC_BCAST_ADDR		(PUCHAR) "\xff\xff\xff\xff\xff\xff"         /* �㲥��ַ                     */
#define OPT_CODE            0                                           /* ѡ����Ϣ����λ��             */
#define OPT_LEN             1                                           /* ѡ����Ϣ����                 */
#define OPT_DATA            2                                           /* ѡ����Ϣ����                 */

/*********************************************************************************************************
  dhcp ���Ľṹ��
*********************************************************************************************************/
struct dhcpMessage {
	UCHAR   ucOp;                                                       /* ��Ϣ���䷽��                 */
	UCHAR   ucHtype;                                                    /* Ӳ�����ͱ�                   */
	UCHAR   ucHlen;                                                     /* Ӳ����ַ����                 */
	UCHAR   ucHops;                                                     /* ����                         */
	UINT    uiXid;                                                      /* ����ID                       */
	UINT16  usSecs;                                                     /* ��ַ��ʹ��ʱ��               */
	UINT16  usFlags;                                                    /* ��־λ                       */
	UINT    uiCiaddr;                                                   /* �ͻ���IP��ַ                 */
	UINT    uiYiaddr;                                                   /* ������ͻ��˵�IP��ַ         */
	UINT    uiSiaddr;                                                   /* ��һ���׶�Ҫʹ�õķ�����IP   */
	UINT    uiGiaddr;                                                   /* ������ת����ַ               */
	UCHAR   ucChaddr[16];                                               /* �ͻ���Ӳ����ַ               */
	UCHAR   ucSname[64];                                                /* ����������                   */
	UCHAR   ucFile[128];                                                /* �����ļ���                   */
	UINT    uiCookie;                                                   /* cookie                       */
	UCHAR   ucOptions[308];                                             /* ѡ����Ϣ                     */
};

typedef struct dhcpMessage   DHCPMSG;
typedef struct dhcpMessage  *PDHCPMSG;

/*********************************************************************************************************
  dhcpѡ��ṹ��
*********************************************************************************************************/
struct dhcpoption {
	CHAR          cName[10];
	CHAR          cFlags;
	UCHAR         ucCode;
};

typedef struct dhcpoption   DHCPOPT;
typedef struct dhcpoption  *PDHCPOPT;

/*********************************************************************************************************
  ѡ��ȫ�ֱ���
*********************************************************************************************************/
extern DHCPOPT              Gdhcpopts[];

extern INT                  GiOptlen[];

/*********************************************************************************************************
  ���ܺ���
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
