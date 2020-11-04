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
** ��   ��   ��: libasdefense_config.h
**
** ��   ��   ��: Wu.Pengcheng (������)
**
** �ļ���������: 2018 �� 7 �� 27 ��
**
** ��        ��: OpenRT ARP ������Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __LIBASDEFENSE_CONFIG_H
#define __LIBASDEFENSE_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "netif/etharp.h"

/*********************************************************************************************************
  ARP �������� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_GET_ASD_EN_CMD         (OPEN_RT_ARP_DEFENSE_BASE | 1)    /*  ʹ��/ʧ�� ARP ����          */
#define OPENRT_GET_ASD_EN_ACK_CMD     (OPEN_RT_ARP_DEFENSE_BASE | 2)    /*  ʹ��/ʧ�� ARP ����Ӧ��      */

#define OPENRT_GET_ASD_GET_CMD        (OPEN_RT_ARP_DEFENSE_BASE | 3)    /*  ��ȡ ARP ������Ϣ           */
#define OPENRT_GET_ASD_GET_ACK_CMD    (OPEN_RT_ARP_DEFENSE_BASE | 4)    /*  ��ȡ ARP ������ϢӦ��       */

#define OPENRT_GET_ASD_NUM_CMD        (OPEN_RT_ARP_DEFENSE_BASE | 5)    /*  ��ȡ ARP ������Ϣ����       */
#define OPENRT_GET_ASD_NUM_ACK_CMD    (OPEN_RT_ARP_DEFENSE_BASE | 6)    /*  ��ȡ ARP ������Ϣ����Ӧ��   */

#define OPENRT_GET_ASD_STATUS_CMD     (OPEN_RT_ARP_DEFENSE_BASE | 7)    /*  ��ȡ ARP ��������״̬       */
#define OPENRT_GET_ASD_STATUS_ACK_CMD (OPEN_RT_ARP_DEFENSE_BASE | 8)    /*  ��ȡ ARP ��������״̬Ӧ��   */
/*********************************************************************************************************
  ARP ��־λ
*********************************************************************************************************/
#define ARP_NORMAL_FLAG               (0x0)                             /*  ARP ��Ϣ����                */
#define ARP_NORMAL_STR                ("normal")                        /*  ARP �����ַ���              */
#define ARP_CONFLICT_FLAG             (0x01)                            /*  ���� MAC ��ַ��ͻ           */
#define ARP_CONFLICT_STR              ("conflict")                      /*  ARP ��ͻ�ַ���              */
#define ARP_ATTACK_FLAG               (0x02)                            /*  ���� MAC ��ƭ               */
#define ARP_ATTACK_STR                ("attack")                        /*  ARP �����ַ���              */
/*********************************************************************************************************
  ����ʹ�ܱ�־��
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_ENABLE      (1)                               /*  ʹ�ܷ���                    */
#define ARP_SPOOF_DEFENSE_DISABLE     (0)                               /*  ���ܷ���                    */
/*********************************************************************************************************
  ��ȡ��Ϣ����
*********************************************************************************************************/
#define ASDEFENSE_GET_ALL             (0)                               /*  ��ȡ������Ϣ����            */
#define ASDEFENSE_GET_SPEC            (1)                               /*  ��ȡ��������Ϣ����          */
/*********************************************************************************************************
  ARP ������ȡ��Ϣ�ṹ
*********************************************************************************************************/
struct asdefense_get {
    UINT8                          ucType;                              /*  ��ȡ��Ϣ������              */
    INT32                          iStartIndex;                         /*  ��ʼ��Ŀ��                  */
    INT32                          iEndIndex;                           /*  ������Ŀ��                  */
};
typedef struct asdefense_get      __ASDEFENSE_GET;
typedef struct asdefense_get     *__PASDEFENSE_GET;
/*********************************************************************************************************
  ARP ����������Ϣ�ṹ
*********************************************************************************************************/
struct asdefense_info {
    ip4_addr_t                      uiIpAddr;                           /*  IP ��ַ                     */
    UCHAR                           ucTrueMacAddr[ETHARP_HWADDR_LEN];   /*  ��ȷ MAC ��ַ               */
    UCHAR                           ucConMacAddr[ETHARP_HWADDR_LEN];    /*  ��ͻ MAC ��ַ               */
    UINT8                           ucFlag;                             /*  ��ǰ ARP ��Ϣ״̬��־       */
};
typedef struct asdefense_info      __ASDEFENSE_INFO;
typedef struct asdefense_info     *__PASDEFENSE_INFO;
/*********************************************************************************************************
  ��ȡ ARP ������Ϣȫ�ַ��ű�
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32  asDefenseStatus (VOID);
LW_SYMBOL_EXPORT UINT32 asDefenseGetSpecInfoNum (UINT8  ucInfoType);
LW_SYMBOL_EXPORT INT32  asDefenseGetSpecInfo (INT32  iStartIndex,
                                              INT32  iEndIndex,
                                              UINT8  ucInfoType,
                                              PCHAR  pcBuff,
                                              INT32  iBuffLen);

#endif                                                                       /* __LIBASDEFENSE_CONFIG_H */
/*********************************************************************************************************
  END
*********************************************************************************************************/
