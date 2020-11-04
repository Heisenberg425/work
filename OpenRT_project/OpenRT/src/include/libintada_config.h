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
** ��   ��   ��: libintada_config.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 23 ��
**
** ��        ��: ���ڶ�λ��Ϣ����ͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBINTADA_CONFIG_H
#define __LIBINTADA_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  ���ڶ�λ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_ADA_GETS_CMD         (OPEN_RT_INTADA_CONFIG_BASE | 1)    /*  ��ȡ��λ״̬                */
#define OPENRT_ADA_GETS_ACK_CMD     (OPEN_RT_INTADA_CONFIG_BASE | 2)    /*  ��ȡ��λ״̬Ӧ��            */

#define OPENRT_NETIF_GET_CMD        (OPEN_RT_INTADA_CONFIG_BASE | 3)    /*  ��ȡ netif                  */
#define OPENRT_NETIF_GET_ACK_CMD    (OPEN_RT_INTADA_CONFIG_BASE | 4)    /*  ��ȡ netif Ӧ��             */

#define OPENRT_ADA_SET_CMD          (OPEN_RT_INTADA_CONFIG_BASE | 5)    /*  ���ж�λ����                */
#define OPENRT_ADA_SET_ACK_CMD      (OPEN_RT_INTADA_CONFIG_BASE | 6)    /*  ���ж�λ����Ӧ��            */

#define OPENRT_ADA_CLEAR_CMD        (OPEN_RT_INTADA_CONFIG_BASE | 7)    /*  ɾ����λ����                */
#define OPENRT_ADA_CLEAR_ACK_CMD    (OPEN_RT_INTADA_CONFIG_BASE | 8)    /*  ɾ����λ����Ӧ��            */

#define OPENRT_WEBNETIF_GET_CMD     (OPEN_RT_INTADA_CONFIG_BASE | 9)    /*  ��ȡ web netif              */
#define OPENRT_WEBNETIF_GET_ACK_CMD (OPEN_RT_INTADA_CONFIG_BASE | 10)   /*  ��ȡ web netif Ӧ��         */

/*********************************************************************************************************
  __INT_ADA_SET flag �ֶκ�
*********************************************************************************************************/
#define INT_ADA_FLAG_FIRST          (0)                                 /*  ��һ������                  */
#define INT_ADA_FLAG_LAST           (1)                                 /*  ���һ������                */
#define INT_ADA_FLAG_MIDDLE         (2)                                 /*  �м�����                    */
/*********************************************************************************************************
  �����ù��߷��͵���Ϣ
*********************************************************************************************************/
struct int_ada_set {
    INT                iWebIndex;                                       /*  Web ��Ӧ������              */
    CHAR               cWebName[NETIF_NAMESIZE];                        /*  Web ���õ���������          */
    CHAR               cFlag;                                           /*  ˳���־                    */
};
typedef struct int_ada_set  __INT_ADA_SET;
typedef __INT_ADA_SET      *__PINT_ADA_SET;

#endif                                                                  /* __LIBINTADA_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
