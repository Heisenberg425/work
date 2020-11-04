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
** ��   ��   ��: sockets.h
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��2��27��
**
** ��        ��: dhcp ����ͨ��ͷ�ļ�
*********************************************************************************************************/
#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <netinet/in.h>

#include "packet.h"

#define RECVERROR       -2                                              /* ���մ���                     */
#define MSGERROR        -3                                              /* ���ݰ�����                   */
/*********************************************************************************************************
  ���ܺ���
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
