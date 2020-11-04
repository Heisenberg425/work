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
** ��   ��   ��: leasesfix.h
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��3��8��
**
** ��        ��: dhcp������Լ�����޸İ�ͷ�ļ�
*********************************************************************************************************/
#ifndef LEASESFIX_H_
#define LEASESFIX_H_
#include "packet.h"
#include "pthread.h"

#define MAXLEASES           254                                         /* �����Լ��                   */

/*********************************************************************************************************
  ״̬��־�궨��
*********************************************************************************************************/
#define LEASEFREE           0                                           /* ��Լ����״̬                 */
#define LEASETACKED         1                                           /* ��Լ��ʱ��ȡ״̬             */
#define LEASEOFFER          2                                           /* ��Լ��ʱ�ṩ״̬             */
#define LEASEOCCUPY         3                                           /* ���޳ɹ�״̬                 */
#define LEASECONFLICT       4                                           /* ��Լ��ַ��ͻ                 */
#define ACKOK               1                                           /* ��ҪACKӦ��                  */
#define TAKEERROR          -2                                           /* ��Լ��ʱ��ȡʧ��             */
#define OFFERERROR         -3                                           /* ��ԼOFFER�ṩʧ��            */
#define IPCHECKFAILE       -4                                           /* ��ַ�����ռ��               */

/*********************************************************************************************************
  ��Լ�ṹ��
*********************************************************************************************************/
struct dhcplease {
    UINT8    ucChaddr[16];                                              /* �ͻ��������ַ               */
    UINT     uiYiaddr;                                                  /* �ͻ���IP��ַ                 */
    UINT     uiExpires;                                                 /* �ͻ�����Լʱ��               */
    INT      iState;                                                    /* ��Լ״̬                     */
};

struct leaseManger {
    struct dhcplease   *pdhcplease;                                     /* ��Լ�ṹ��ָ��               */
    pthread_mutex_t     leasemutexLock;                                 /* ��Լ������                   */
};

typedef struct leaseManger  LEASEMANGER;
/*********************************************************************************************************
  ���ܺ���
*********************************************************************************************************/
INT   leaseMangerInit (VOID);
INT   leaseWriteWithProtect (VOID);
INT   leaseDecline(PDHCPMSG  pdhcpmsgPacket);
INT   leaseRelease(PDHCPMSG  pdhcpmsgPacket);
INT   discoverOfferGet(PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket);
INT   leaseAck (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket);
#endif /* LEASES_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
