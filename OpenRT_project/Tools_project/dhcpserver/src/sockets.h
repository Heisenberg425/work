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
** 文   件   名: sockets.h
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年2月27日
**
** 描        述: dhcp 网络通信头文件
*********************************************************************************************************/
#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <netinet/in.h>

#include "packet.h"

#define RECVERROR       -2                                              /* 接收错误                     */
#define MSGERROR        -3                                              /* 数据包错误                   */
/*********************************************************************************************************
  功能函数
*********************************************************************************************************/
VOID  sendsocketClose (VOID);
INT   ipConflictCheck(UINT  uiYiaddr);
INT   packetRecv (PDHCPMSG pdhcpmsgPacket, INT  iRcvSocket, struct sockaddr_in  *psockaddrFrom);
INT   socketInit (INT  *piRcvSocket, struct sockaddr_in  *psockaddrFrom);
INT   packetSendUnicast (PDHCPMSG  pdhcpmsgPacket, UINT  uiYiaddr);
INT   packetSendBroadcast (PDHCPMSG  pdhcpmsgPacket);

#endif /* SOCKETS_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
