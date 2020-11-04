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
** ��   ��   ��: intada_common.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 17 ��
**
** ��        ��: ����˳������Ӧ����ͷ�ļ�
*********************************************************************************************************/
#ifndef __INTADA_COMMON_H
#define __INTADA_COMMON_H

/*********************************************************************************************************
  �����Զ���������Ϣ
*********************************************************************************************************/
struct intadaption_handle_info {
    INT                iWebIndex;                                       /*  Web ��Ӧ������              */
    CHAR               cWebName[NETIF_NAMESIZE];                        /*  Web ���õ���������          */
    CHAR               cNetIfName[NETIF_NAMESIZE];                      /*  ϵͳ��������                */
    CHAR               cStatus[12];                                     /*  ״̬��־                    */
};
typedef struct intadaption_handle_info   __INTADAPTION_HANDLE_INFO;
typedef struct intadaption_handle_info  *__PINTADAPTION_HANDLE_INFO;
/*********************************************************************************************************
  ״̬��־
*********************************************************************************************************/
#define NET_ADAPTING               "adapting"                           /*  ���ڽ�������Ӧ              */
#define NET_FINISH                 "finished"                           /*  �������Ӧ                  */
#define NET_UNUSED                 "unused"                             /*  δʹ��                      */
/*********************************************************************************************************
  EXPORT ������������
*********************************************************************************************************/
VOID intAdaHandleEntry (INT iCommand, PVOID  pArgBuf);
INT  intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex);

/*********************************************************************************************************
  EXPORT ���ݿ⺯��
*********************************************************************************************************/
INT  intAdaptionTableCreate (VOID);
/*********************************************************************************************************
  ���ݿ⺯������
*********************************************************************************************************/
VOID intAdaptionDBClear (VOID);
INT  intAdaptionDBFindFinish (VOID);
INT  intAdaptionDBGetInfo (__PINTADAPTION_HANDLE_INFO   pNetInfo, CHAR  cFlag);
INT  intadaStatusGetFromDbByName (CPCHAR  cpcNetIfName, INT  *piStatus);
INT  intAdaptionDBSetFinish (VOID);
INT  intAdaptionDBUpdate (__PINTADAPTION_HANDLE_INFO  pIntAdaptionInfo);
INT  intAdaptionDBInit (VOID);

#endif                                                                  /*  __INTADA_COMMON_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
