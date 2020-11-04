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
** ��   ��   ��: libpower_config.h
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ��Դ״̬����ͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBPOWER_CONFIG_H
#define __LIBPOWER_CONFIG_H

#include "libserver_type.h"

/*********************************************************************************************************
  ϵͳ��Դ�������궨�� (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_POWER_OFF_CMD          (OPEN_RT_POWER_CONFIG_BASE | 1)   /*  �ر�ϵͳ����                */
#define OPENRT_POWER_OFF_ACK_CMD      (OPEN_RT_POWER_CONFIG_BASE | 2)   /*  �ر�ϵͳӦ������            */

#define OPENRT_POWER_RESTART_CMD      (OPEN_RT_POWER_CONFIG_BASE | 3)   /*  ����ϵͳ����                */
#define OPENRT_POWER_RESTART_ACK_CMD  (OPEN_RT_POWER_CONFIG_BASE | 4)   /*  ����ϵͳӦ������            */

#define OPENRT_POWER_EVT_CFG_CMD      (OPEN_RT_POWER_CONFIG_BASE | 5)   /*  ��Դ�¼���������            */
#define OPENRT_POWER_EVT_CFG_ACK_CMD  (OPEN_RT_POWER_CONFIG_BASE | 6)   /*  ��Դ�¼�����Ӧ������        */

#define OPENRT_POWER_EVT_SHOW_CMD     (OPEN_RT_POWER_CONFIG_BASE | 7)   /*  ��Դ�¼���ʾ����            */
#define OPENRT_POWER_EVT_SHOW_ACK_CMD (OPEN_RT_POWER_CONFIG_BASE | 8)   /*  ��Դ�¼���ʾӦ������        */

/*********************************************************************************************************
 ����ϵͳ��Դ״̬��Ϣ�Ľṹ
*********************************************************************************************************/
struct power_handle {
    UINT  uiEventId;                                                    /*  �¼� ID                     */
    CHAR  cEvent[16];                                                   /*  �¼�                        */
    CHAR  cDate[12];                                                    /*  ����                        */
    CHAR  cTime[12];                                                    /*  ʱ��                        */
    CHAR  cOperType[12];                                                /*  ��������                    */
    CHAR  cDesc[128];                                                   /*  ˵����Ϣ                    */
    INT   iStatus;                                                      /*  ״̬���Ƿ�����              */
};
typedef struct power_handle  __POWER_HANDLE;
typedef struct power_handle  *__PPOWER_HANDLE;
/*********************************************************************************************************
  �¼�����ڵ�
*********************************************************************************************************/
typedef struct power_node{
    LW_LIST_LINE             powerLine;
    __POWER_HANDLE           tNodeData;                                 /*  �ڵ�����                    */
} __POWER_NODE;
typedef __POWER_NODE       *__PPOWER_NODE;

#endif                                                                  /*  __LIBPOWER_CONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
