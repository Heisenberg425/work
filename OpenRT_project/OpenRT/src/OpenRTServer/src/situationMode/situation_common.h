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
** 文   件   名: situation_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述: 情景模式相关头文件
*********************************************************************************************************/
#ifndef __SITUATION_COMMON_H
#define __SITUATION_COMMON_H

#include "libsituation_config.h"
#include "libmonitor_config.h"

/*********************************************************************************************************
  OpenRT 模式宏定义
*********************************************************************************************************/
#define MODE_MIDDLE_ENT         "mid-Enterprise"                        /*  模式：中/小企业             */
#define MODE_LARGE_ENT          "large-Enterprise"                      /*  模式：大型企业              */
#define MODE_FAMILY             "family"                                /*  模式：家庭                  */
#define MODE_GOVERN             "govern"                                /*  模式：政府                  */
/*********************************************************************************************************
  OpenRT 状态宏定义
*********************************************************************************************************/
#define STATUS_UNCONFIG         "Noconfig"                              /*  状态：未配置                */
#define STATUS_CONFIGING        "Configing"                             /*  状态：配置中                */
#define STATUS_FINISHED         "Finished"                              /*  状态：配置完成              */
/*********************************************************************************************************
  OpenRT 位置宏定义
*********************************************************************************************************/
#define POS_STARTING_SET        "Starting-Set-Page"                     /*  位置：开始界面              */
#define POS_CONNECT_SUBMIT      "Connect-Submit-Page"                   /*  位置：外网连接提交          */
#define POS_CONNECT_INTERNET    "Connecting-Internet-Page"              /*  位置：外网接入              */
#define POS_INTRANET_SET        "Intranet-Set-Page"                     /*  位置：内网接入              */
#define POS_CLIENT_KIDVPN       "Client-KidVPN-Set-Page"                /*  位置：KidVPN 客户端         */
#define POS_SERVER_KIDVPN       "Server-KidVPN-Set-Page"                /*  位置：KidVPN 服务器         */
#define POS_CONFIG_COMPLETE     "Configure_Cmplete_Page"                /*  位置：配置完成              */
/*********************************************************************************************************
  KidVPN 服务器配置信息存放路径定义
*********************************************************************************************************/
#define KIDVPN_SERVER_CONF_PATH "/apps/goahead/web"                     /*  路径                        */
#define KIDVPN_SERVER_CONF      "/apps/goahead/web/VPN-Server-Conf.txt" /*  文件                        */
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern INT        tmStatItemCount(PTM_STATS  pCondition, INT* piFilter);
extern PTM_STATS  tmStatItemsCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition, INT * piFilter);
extern PTM_STATS  tmStatItemsSumCopy(PTM_STATS*  ppTmStat, PTM_STATS pCondition);
extern INT        getCpuUsage();
extern INT        getMemoryUsage(size_t  *pstPhyMemTotalSize, size_t  *pstUsedSize);
extern INT        tmGetOnlineUsers();
/*********************************************************************************************************
  函数声明
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
