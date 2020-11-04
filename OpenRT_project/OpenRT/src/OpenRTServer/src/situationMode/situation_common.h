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
** ��   ��   ��: situation_common.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 10 �� 15 ��
**
** ��        ��: �龰ģʽ���ͷ�ļ�
*********************************************************************************************************/
#ifndef __SITUATION_COMMON_H
#define __SITUATION_COMMON_H

#include "libsituation_config.h"
#include "libmonitor_config.h"

/*********************************************************************************************************
  OpenRT ģʽ�궨��
*********************************************************************************************************/
#define MODE_MIDDLE_ENT         "mid-Enterprise"                        /*  ģʽ����/С��ҵ             */
#define MODE_LARGE_ENT          "large-Enterprise"                      /*  ģʽ��������ҵ              */
#define MODE_FAMILY             "family"                                /*  ģʽ����ͥ                  */
#define MODE_GOVERN             "govern"                                /*  ģʽ������                  */
/*********************************************************************************************************
  OpenRT ״̬�궨��
*********************************************************************************************************/
#define STATUS_UNCONFIG         "Noconfig"                              /*  ״̬��δ����                */
#define STATUS_CONFIGING        "Configing"                             /*  ״̬��������                */
#define STATUS_FINISHED         "Finished"                              /*  ״̬���������              */
/*********************************************************************************************************
  OpenRT λ�ú궨��
*********************************************************************************************************/
#define POS_STARTING_SET        "Starting-Set-Page"                     /*  λ�ã���ʼ����              */
#define POS_CONNECT_SUBMIT      "Connect-Submit-Page"                   /*  λ�ã����������ύ          */
#define POS_CONNECT_INTERNET    "Connecting-Internet-Page"              /*  λ�ã���������              */
#define POS_INTRANET_SET        "Intranet-Set-Page"                     /*  λ�ã���������              */
#define POS_CLIENT_KIDVPN       "Client-KidVPN-Set-Page"                /*  λ�ã�KidVPN �ͻ���         */
#define POS_SERVER_KIDVPN       "Server-KidVPN-Set-Page"                /*  λ�ã�KidVPN ������         */
#define POS_CONFIG_COMPLETE     "Configure_Cmplete_Page"                /*  λ�ã��������              */
/*********************************************************************************************************
  KidVPN ������������Ϣ���·������
*********************************************************************************************************/
#define KIDVPN_SERVER_CONF_PATH "/apps/goahead/web"                     /*  ·��                        */
#define KIDVPN_SERVER_CONF      "/apps/goahead/web/VPN-Server-Conf.txt" /*  �ļ�                        */
/*********************************************************************************************************
  �ⲿ��������
*********************************************************************************************************/
extern INT        tmStatItemCount(PTM_STATS  pCondition, INT* piFilter);
extern PTM_STATS  tmStatItemsCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
extern PTM_STATS  tmStatItemsSumCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition);
extern INT        getCpuUsage();
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
extern INT        tmGetOnlineUsers();
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT   situationInfoDoSave (__PSITUATION_INFO  pSituationInfo);
INT   situationAddItemToDb (__PSITUATION_INFO  pSituationInfo);
INT   situationInfoGet (__PSITUATION_INFO     pSituationInfo);

INT   wanInfoDoSave (__PWANPAGE_INFO     pWanPageInfo);
INT   wanPageAddItemToDb (__PWANPAGE_INFO  pWanPageInfo);
INT   wanPageGetFromDb (__PWANPAGE_INFO  pWanPageInfo);

INT   kidVpnServerInfoDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo);
INT   kidVpnServerInfoDoClean (VOID);
INT   kidVpnServerPageAddItemToDb (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo);
INT   kidVpnServerPageDeleteFromDb (VOID);
INT   kidVpnServInfoDeleteFromDbByVndid (INT  iVndId);
INT   kidVpnCliInfoDeleteFromDbByVndid (INT  iVndId);
INT   kidVpnServPageGetFromDb (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo);

INT   kidVpnServerConfDoSave (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo);

INT   kidVpnClientInfoDoSave (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo);
INT   kidVpnClientPageAddItemToDb (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo);
INT   kidVpnCliPageGetFromDb (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo);

INT   kidVpnDoCancel (INT  iKidVpnType);
INT   optFuncDoCancel (INT  iOptFunc);
INT   websocketInit (VOID);
INT   situationTableCreate (VOID);

VOID  situationHandleEntry (INT  iCommand, PVOID  pArgBuf);

#endif                                                                  /* __SITUATION_COMMON_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
