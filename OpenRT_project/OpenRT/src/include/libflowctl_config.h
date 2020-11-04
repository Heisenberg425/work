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
** ��   ��   ��: flowctl_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 29 ��
**
** ��        ��: flowctl ��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBFLOWCTL_CONFIG_H
#define __LIBFLOWCTL_CONFIG_H

#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  route ����ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_CFG_FLOWCTL_CMD     (OPEN_RT_FLOWCTL_CONFIG_BASE | 1)    /*  ��������                    */
#define OPENRT_CFG_FLOWCTL_ACK_CMD (OPEN_RT_FLOWCTL_CONFIG_BASE | 2)    /*  ��������Ӧ��                */
#define OPENRT_GET_FLOWCTL_CMD     (OPEN_RT_FLOWCTL_CONFIG_BASE | 3)    /*  ��ȡ������Ϣ                */
#define OPENRT_GET_FLOWCTL_ACK_CMD (OPEN_RT_FLOWCTL_CONFIG_BASE | 4)    /*  ��ȡ������ϢӦ��            */

#define FCT_IP                     (1)                                  /*  ��Э���������              */
#define FCT_IF                     (2)                                  /*  �����ڽ�������              */
/*********************************************************************************************************
 ���� flowctl ��Ϣ�Ľṹ
*********************************************************************************************************/
struct flowctl_handle {
    INT     iType;                                                      /*  1: ��Э���������
                                                                            2: �����ڽ�������           */
    CHAR    cEnable[8];                                                 /*  1: ʹ�� 0: ȥʹ��           */
    CHAR    cProtocol[8];                                               /*  ��ЧЭ������ tcp/udp/all    */
    CHAR    cStartIp[64];                                               /*  ������ʼ IP ��ַ            */
    CHAR    cEndIp[64];                                                 /*  ���ؽ��� IP ��ַ            */
    INT     iStartPort;                                                 /*  ������ʼ�˿�                */
    INT     iEndPort;                                                   /*  ���ؽ����˿�                */
    CHAR    cIfname[NETIF_NAMESIZE];                                    /*  ����                        */
    ULONG   ulUpRate;                                                   /*  ����������KB/s��            */
    ULONG   ulDownRate;                                                 /*  ����������KB/s��            */
    ULONG   ulBufSize;                                                  /*  ��������С(16K ~ 128M Bytes)*/
    CHAR    cIpFamily[8];                                               /*  ip ��ַ�壬IPv4 �� IPv6     */
    CHAR    cOperType[8];                                               /*  �������ͣ���ɾ��            */
};
typedef struct flowctl_handle   __FLOWCTL_HANDLE;
typedef struct flowctl_handle  *__PFLOWCTL_HANDLE;

#endif                                                                  /* __LIBFLOWCTL_CONFIG_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
