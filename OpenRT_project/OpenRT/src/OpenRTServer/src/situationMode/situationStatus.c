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
** ��   ��   ��: quaggaRecovery.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 9 �� 4 ��
**
** ��        ��: quagga �ָ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <spawn.h>
#include <wait.h>
#include <sys/un.h>
#include <socket.h>
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "common.h"
#include "situationDB.h"
#include "situation_common.h"
#include "monitor/monitor_common.h"
#include "kidvpn/kidvpn_common.h"
#include "udial/udial_common.h"
#include "database/database_common.h"
#include "intadaption/intAdaptionExec.h"
#include "cJSON.h"
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static LW_HANDLE                  _G_hWebsockTid;                       /*  �߳̾��                    */
/*********************************************************************************************************
  �����
*********************************************************************************************************/
#define SITUATION_INFO_GET        (0)                                   /*  ��ȡ�龰ģʽ��Ϣ��          */
#define WANPAGE_INFO_GET          (1)                                   /*  ��ȡ����������Ϣ��          */
#define KIDVPN_SERVER_INFO_GET    (2)                                   /*  ��ȡ KIDVPN Server ��Ϣ     */
#define DELETE_ALL_DATA           (3)                                   /*  ����������ݿ���Ϣ������    */
#define SCENEMODE_GUIDE_STAT_GET  (4)                                   /*  �龰ģʽ����״̬            */
#define SCENE_MODE_ALL_INFO_GET   (5)                                   /*  �龰ģʽ������Ϣ��ȡ        */
#define KIDVPN_CLIENT_INFO_GET    (6)                                   /*  ��ȡ KIDVPN Client ��Ϣ     */
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define REQUEST_LEN               (1024)                                /*  ����������󳤶�            */
#define WBS_SERVER_PORT           (8001)                                /*  ������Ĭ�϶˿ں�            */
#define MAX_FD(fd1, fd2)          ((fd1) > (fd2)) ? (fd1) : (fd2)       /*  ����ļ����                */
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static const CHAR    _G_ccBase[] = \
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijk" \
                     "lmnopqrstuvwxyz0123456789+/=";                    /*  �����ַ���                  */
/*********************************************************************************************************
  SHA1 ��Ϣ�ṹ����
*********************************************************************************************************/
struct  SHA1_Context {
    UINT          uiMessageDigest[5];                                   /*  ��ϢժҪ                    */
    UINT          uiLengthLow;                                          /*  ��λ����                    */
    UINT          uiLengthHigh;                                         /*  ��λ����                    */
    UCHAR         uiMessageBlock[64];                                   /*  ��Ϣ��                      */
    INT           iMessageBlockIndex;                                   /*  ��Ϣ������                  */
    INT           iComputed;                                            /*  �����־                    */
    INT           iCorrupted;                                           /*  ��������־                */
};
typedef struct SHA1_Context   __SHA1_CONTEXT;
typedef struct SHA1_Context  *__PSHA1_CONTEXT;
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static PCHAR  fetchSecKey(CPCHAR  cpcBuf);
static PCHAR  packData(CPCHAR  cpcMsg, ULONG  *pulLen);

static VOID   SHA1Reset(__PSHA1_CONTEXT   pContext);
static INT    SHA1Result(__PSHA1_CONTEXT  pContext);
static VOID   SHA1Input(__PSHA1_CONTEXT   pContext, CPCHAR  cpcMsgArray, UINT  uiLength);
static VOID   SHA1ProcessMessageBlock(__PSHA1_CONTEXT  pContext);
static VOID   SHA1PadMessage(__PSHA1_CONTEXT  pContext);
static PCHAR  sha1Hash (CPCHAR  cpcSource);

static PCHAR  base64Encode (CPCHAR  cpcData, INT  iDataLen);
static INT    hexToInt (CPCHAR  cpcStr, INT  iStart, INT  iStrLen);
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define REQUEST_LEN_MAX               (1024)                            /*  ����������󳤶�            */
#define WEB_SOCKET_KEY_LEN_MAX        (256)                             /*  KEY ��󳤶�                */
#define RESPONSE_HEADER_LEN_MAX       (1024)                            /*  Ӧ��ͷ��󳤶�              */
#define LINE_LEN_MAX                  (256)                             /*  ����󳤶�                  */

#define SHA1CircularShift(bits, word) ((((word) << (bits)) & 0xFFFFFFFF) | \
                                        ((word) >> (32-(bits))))        /*  SHA1 ѭ���ƶ�               */

#define WEBS_DIR_PATH                 "/tmp/OpenRT"                     /*  socket �ļ���Ŀ¼           */
#define WEBS_SER_ADDR                 WEBS_DIR_PATH"/unix_websock"      /*  ������ļ�                  */
#define WEBS_CLI_ADDR                 WEBS_DIR_PATH"/unix_"             /*  �ͻ����ļ�                  */
#define QUAGGA_PORT_LEN               (4)                               /*  quagga �˿ڱ�־����         */
/*********************************************************************************************************
  �龰ģʽ��Ϣ Json �궨��
*********************************************************************************************************/
#define SIT_INFO_MODE                 "mode"                            /*  �龰ģʽ                    */
#define SIT_INFO_POSITION             "position"                        /*  ������λ                  */
#define SIT_INFO_FUNC                 "func"                            /*  ���ӹ���                    */
#define SIT_INFO_STATUS               "status"                          /*  �龰ģʽ״̬                */
/*********************************************************************************************************
  OpenRT ��ѡ���ܺ궨��
*********************************************************************************************************/
#define WEB_KIDVPN_CLIENT             "kidvpn_client"                   /*  ��ѡ���ܣ�KidVPN �ͻ���     */
#define WEB_KIDVPN_SERVER             "kidvpn_server"                   /*  ��ѡ���ܣ�KidVPN �����     */
#define WEB_ARP_DEFENSE               "arp_defense"                     /*  ��ѡ���ܣ�ARP ����          */
#define FUNC_KIDVPN_CLIENT            (1 << 0)                          /*  ��ѡ���ܣ�KidVPN �ͻ���     */
#define FUNC_KIDVPN_SERVER            (1 << 1)                          /*  ��ѡ���ܣ�KidVPN �����     */
#define FUNC_ARP_DEFENSE              (1 << 2)                          /*  ��ѡ���ܣ�ARP ����          */
/*********************************************************************************************************
  ����������Ϣ Json �궨��
*********************************************************************************************************/
#define WAN_PAGE_NETIF                "webname"                         /*  ��������                    */
#define WAN_PAGE_MODE                 "mode"                            /*  ��������ģʽ                */
#define WAN_PAGE_USER                 "user"                            /*  �����˺�                    */
#define WAN_PAGE_PWD                  "pwd"                             /*  ��������                    */
#define WAN_PAGE_UDIAL_STATUS         "adslstatus"                      /*  ����״̬                    */
#define WAN_PAGE_INTERNET             "internet"                        /*  ��������״̬                */
#define WAN_PAGE_IP                   "ip"                              /*  ��̬IP                      */
#define WAN_PAGE_NETMASK              "netmask"                         /*  �����ַ                    */
#define WAN_PAGE_GATEWAY              "gateway"                         /*  ���ص�ַ                    */
/*********************************************************************************************************
  KidVPN Server ��Ϣ Json �궨��
*********************************************************************************************************/
#define VPNSERV_PUBLIC_IP             "public_ip"                       /*  ���� ip                     */
#define VPNSERV_VND_IP                "vnd_ip"                          /*  vnd ip                      */
#define VPNSERV_VND_MSK               "vnd_msk"                         /*  vnd ����                    */
#define VPNSERV_VND_GW                "vnd_gw"                          /*  vnd ����                    */
#define VPNSERV_PASSWD                "passwd"                          /*  ����                        */
#define VPNSERV_AESKEY                "aeskey"                          /*  aes key                     */
/*********************************************************************************************************
  �龰ģʽ������״̬ Json �궨��
*********************************************************************************************************/
#define SCENEMODE_GUIDE_STATUS        "status"                          /*  �龰ģʽ������״̬          */
/*********************************************************************************************************
  �龰ģʽ������ʾ��Ϣ Json ����
*********************************************************************************************************/
#define KIDVPN_CLIENT_LINK_STATUS    "vpn_cli_link_stat"                /*  KidVpn �ͻ�������״̬       */
#define KIDVPN_SERVER_LINK_NUM       "vpn_ser_link_num"                 /*  KidVpn ��������Ӹ���       */
#define SPEED_NETWORK_DOWNLOAD       "download_speed"                   /*  ���������ٶ�                */
#define SPEED_NETWORK_UPLOAD         "upload_speed"                     /*  ���������ٶ�                */
#define DATA_FLOW_DOWNLOAD           "download_data_flow"               /*  ��������                    */
#define DATA_FLOW_UPLOAD             "upload_data_flow"                 /*  ��������                    */
#define USAGE_RATE_CPU               "cpu_usage"                        /*  CPU ʹ����                  */
#define USAGE_RATE_MEMORY            "memory_usage"                     /*  �ڴ�ʹ����                  */
#define ONLINE_USERS                 "online_users"                     /*  �����û���                  */

#define WAN_LINK_TYPE                "wan_link_type"                    /*  ������������                */
#define WAN_ARG_DNS                  "wan_dns"                          /*  �� dns ��Ϣ                 */
#define WAN_ARG_DNS_SECOND           "wan_dns_sec"                      /*  ���� dns ��Ϣ               */
#define WAN_ARG_GATEWAY              "wan_gateway"                      /*  ������Ϣ                    */
#define WAN_ARG_IP                   "wan_ip"                           /*  ip ��Ϣ                     */
/*********************************************************************************************************
  ������������궨��
*********************************************************************************************************/
#define OPER_TYPE_UDIAL           (1)                                   /*  �������ͣ����Ž���          */
#define OPER_TYPE_DHCP            (2)                                   /*  �������ͣ�DHCP ����         */
#define OPER_TYPE_STATIC          (3)                                   /*  �������ͣ���̬ IP ����      */
/*********************************************************************************************************
** ��������: __wanPageJsInfoGet
** ��������: ��ȡ��������Json��Ϣ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __wanPageJsInfoGet (cJSON   *pSendjRes)
{
    __WANPAGE_INFO     wanPageInfo;
    INT32              iRet;
    __UDIAL_HANDLE     udialInfo;
    __INTERNET_STATUS  internetStatus;
    CHAR               cWebNetifName[NETIF_NAMESIZE] = {0};

    if (LW_NULL == pSendjRes) {
        return  (PX_ERROR);
    }

    bzero(&wanPageInfo, sizeof(__WANPAGE_INFO));
    iRet = wanPageGetFromDb(&wanPageInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    bzero(&udialInfo, sizeof(udialInfo));

    /*
     *  ����ǲ���״̬�����ȡ������Ϣ
     */
    if (1 == wanPageInfo.ucMode && strstr(wanPageInfo.cNetifName, "en")) {
        udialGetSpecInfo(&udialInfo, "NETIF", wanPageInfo.cNetifName, LW_NULL);
    }

    internetStatusGet(&internetStatus);                                 /*  ��ȡ��������״̬            */

#if 1
    if (strstr(wanPageInfo.cNetifName, "en")) {
        intAdaptionGetWebNetif(wanPageInfo.cNetifName, cWebNetifName);
    }
#else
    strncpy(cWebNetifName,                                              /*  ��ʱ��ʹ�� "en"             */
            wanPageInfo.cNetifName,
            strlen(wanPageInfo.cNetifName));
#endif

    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_NETIF, cWebNetifName);
    cJSON_AddNumberToObject(pSendjRes, WAN_PAGE_MODE, wanPageInfo.ucMode);
    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_USER, wanPageInfo.cPppoeUsr);
    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_PWD, wanPageInfo.cPppoePwd);
    cJSON_AddNumberToObject(pSendjRes, WAN_PAGE_UDIAL_STATUS, udialInfo.iUdialStatus);
    cJSON_AddStringToObject(pSendjRes,
                            WAN_PAGE_INTERNET,
                            INTERNET_OK == internetStatus.ucStatus ? "yes" : "no" );
    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_IP, wanPageInfo.cIpAddr);
    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_NETMASK, wanPageInfo.cNetMask);
    cJSON_AddStringToObject(pSendjRes, WAN_PAGE_GATEWAY, wanPageInfo.cGateWay);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kidvpnServPageJsInfoGet
** ��������: ��ȡ KidVPN server Json��Ϣ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __kidvpnServPageJsInfoGet (cJSON   *pSendjRes)
{
    __KIDVPN_SERVER_INFO  kidvpnServPageInfo;
    INT                   iRet;

    if (LW_NULL == pSendjRes) {
        return  (PX_ERROR);
    }

    bzero(&kidvpnServPageInfo, sizeof(__KIDVPN_SERVER_INFO));
    iRet = kidVpnServPageGetFromDb(&kidvpnServPageInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    cJSON_AddStringToObject(pSendjRes, VPNSERV_PUBLIC_IP, kidvpnServPageInfo.cPublicIp);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_VND_IP,    kidvpnServPageInfo.cVndIpAddr);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_VND_MSK,   kidvpnServPageInfo.cVndNetMask);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_VND_GW,    kidvpnServPageInfo.cVndGateway);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_PASSWD,    kidvpnServPageInfo.cPasswd);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_AESKEY,    kidvpnServPageInfo.cAesKey);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kidvpnCliPageJsInfoGet
** ��������: ��ȡ KidVPN Client Json��Ϣ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __kidvpnCliPageJsInfoGet (cJSON   *pSendjRes)
{
    __KIDVPN_CLIENT_INFO  kidvpnCliPageInfo;
    INT                   iRet;

    if (LW_NULL == pSendjRes) {
        return  (PX_ERROR);
    }

    bzero(&kidvpnCliPageInfo, sizeof(__KIDVPN_CLIENT_INFO));
    iRet = kidVpnCliPageGetFromDb(&kidvpnCliPageInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    cJSON_AddStringToObject(pSendjRes, VPNSERV_PUBLIC_IP, kidvpnCliPageInfo.cPublicIp);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_VND_IP,    kidvpnCliPageInfo.cVndIpAddr);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_VND_MSK,   kidvpnCliPageInfo.cVndNetMask);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_PASSWD,    kidvpnCliPageInfo.cPasswd);
    cJSON_AddStringToObject(pSendjRes, VPNSERV_AESKEY,    kidvpnCliPageInfo.cAesKey);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sceneModeGuideStatusGet
** ��������: �龰ģʽ����״̬��ȡ������ Json��Ϣ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __sceneModeGuideStatusGet (cJSON   *pSendjRes)
{
    sqlite3  *pCfgDb    = LW_NULL;
    INT       iRow      = 0;
    INT       iColumn   = 0;
    INT       iTotal    = 0;

    OPENRT_DB_HANDLE(pCfgDb);

    if (LW_NULL == pSendjRes ||
        LW_NULL == pCfgDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "kidvpn", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "vndinfo", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "udial", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "natwan", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "natlan", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "dhcpserver", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "arp", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "asdefense", &iRow, &iColumn));
    iTotal += iRow;

    INIT_CHECK(dataBaseGetTbRowCol(pCfgDb, "kidvpnLink", &iRow, &iColumn));
    iTotal += iRow;

    cJSON_AddNumberToObject(pSendjRes, SCENEMODE_GUIDE_STATUS, (iTotal ? 0:1));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __dbDataDelete
** ��������: ɾ���������ݲ�����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __dbDataDelete (sqlite3  *pDb, PCHAR  pcTblName)
{
    CHAR          cSqlCmd[100]  = {0};

    sprintf(cSqlCmd, "DELETE FROM %s", pcTblName);

    dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** ��������: __allDataDelete
** ��������: ɾ���������ݲ�����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __allDataDelete (VOID)
{
    sqlite3  *pCfgDb                  = LW_NULL;
    CHAR      cSituationInserSql[200] = {0};

    OPENRT_DB_HANDLE(pCfgDb);
    if (LW_NULL == pCfgDb) {
        return  ;
    }

    /*
     *  ���������龰ģʽ���ݿ�
     */
    snprintf(cSituationInserSql, sizeof(cSituationInserSql),
             "UPDATE %s  SET MODE = '%s', POSITION = '%s', FUNC = %d, STATUS = '%s';",
             SITUATION_INFO_TABLE, MODE_MIDDLE_ENT, POS_STARTING_SET, 0, STATUS_UNCONFIG);

    dataBaseCmdExec(pCfgDb, cSituationInserSql);

    /*
     *  ������ݿ�����
     */
    __dbDataDelete(pCfgDb, "kidvpn");
    __dbDataDelete(pCfgDb, "kidvpnLink");
    __dbDataDelete(pCfgDb, "vndinfo");
    __dbDataDelete(pCfgDb, "udial");
    __dbDataDelete(pCfgDb, "natwan");
    __dbDataDelete(pCfgDb, "natlan");
    __dbDataDelete(pCfgDb, "dhcpserver");
    __dbDataDelete(pCfgDb, "arp");
    __dbDataDelete(pCfgDb, "asdefense");

    __dbDataDelete(pCfgDb, "kidvpnServPageInfo");

    __dbDataDelete(pCfgDb, "kidvpnCliPageInfo");

    __dbDataDelete(pCfgDb, "WanPageInfo");

    sync();

    sleep(1);

    API_KernelReboot(LW_REBOOT_COLD);
}
/*********************************************************************************************************
** ��������: __dataFlowInfoGet
** ��������: ��ȡ��������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ��������ָ��
*********************************************************************************************************/
static PTM_STATS  __dataFlowInfoGet(VOID)
{
    PTM_STATS  pMonitorHeader;
    PTM_STATS  pMonitorReturn;
    TM_STATS   monitorHandle;

    bzero(&monitorHandle, sizeof(monitorHandle));
    monitorHandle.cType = TMT_WAN_TOTAL;

    pMonitorHeader = (PTM_STATS)malloc(sizeof(TM_STATS));
    if (NULL == pMonitorHeader) {
        return  (NULL);
    }

    bzero(pMonitorHeader, sizeof(TM_STATS));

    pMonitorReturn = tmStatItemsSumCopy(&pMonitorHeader, &monitorHandle);

    pMonitorReturn->cType = TMT_WAN_TOTAL;

    return  (pMonitorReturn);
}
/*********************************************************************************************************
** ��������: __sceneModeAllInfoGet
** ��������: �龰ģʽ������Ϣ��ȡ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __sceneModeAllInfoGet (cJSON   *pSendjRes)
{
    INT              iSerVpnLinkNum  = 0;
    INT              iCliVpnLinkStat = 0;
    double           dCpuUsedRate;
    size_t           sMemTotal;
    size_t           sMemUsed;
    double           dMemUsedRate;
    INT              iOnlineUsers;
    CHAR             cLinkType[8]    = {0};
    CHAR             cPrimaryDNS[16] = {0};
    CHAR             cSecondDNS[16]  = {0};
    PCHAR            pcPrimaryDNS    = LW_NULL;
    PCHAR            pcSecondDNS     = LW_NULL;
    PTM_STATS        pTmStats        = LW_NULL;
    __WANPAGE_INFO   wanPageInfo;

    __SITUATION_INFO situationInfo;

    INT              iKidVpnCli  = 0;
    INT              iKidVpnServ = 0;
    INT              iArpDefense = 0;
    INT32            iRet;

    if (LW_NULL == pSendjRes) {
        return  (PX_ERROR);
    }

    bzero(&wanPageInfo, sizeof(wanPageInfo));
    wanPageGetFromDb(&wanPageInfo);

    getMemoryUsage(&sMemTotal, &sMemUsed);

    dMemUsedRate    = (sMemUsed * 1.0 / sMemTotal) * 100;
    iCliVpnLinkStat = kidvpnClientStatusGetFromDB();
    iSerVpnLinkNum  = kidvpnLinkGetDBRow();
    dCpuUsedRate    = getCpuUsage() / 10.0;
    iOnlineUsers    = tmGetOnlineUsers();

    pTmStats = __dataFlowInfoGet();
    if (NULL == pTmStats) {
        return  (PX_ERROR);
    }

    bzero(&situationInfo, sizeof(__SITUATION_INFO));
    iRet = situationInfoGet(&situationInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    iKidVpnCli  = (situationInfo.iFunc & FUNC_KIDVPN_CLIENT) ? 1:0;
    iKidVpnServ = (situationInfo.iFunc & FUNC_KIDVPN_SERVER) ? 1:0;
    iArpDefense = (situationInfo.iFunc & FUNC_ARP_DEFENSE)   ? 1:0;

    cJSON_AddStringToObject(pSendjRes, KIDVPN_CLIENT_LINK_STATUS, iCliVpnLinkStat ? "ON" : "OFF");
    cJSON_AddNumberToObject(pSendjRes, KIDVPN_SERVER_LINK_NUM, iSerVpnLinkNum);
    cJSON_AddNumberToObject(pSendjRes, USAGE_RATE_CPU, dCpuUsedRate);
    cJSON_AddNumberToObject(pSendjRes, USAGE_RATE_MEMORY, dMemUsedRate);
    cJSON_AddNumberToObject(pSendjRes, ONLINE_USERS, iOnlineUsers);

    cJSON_AddNumberToObject(pSendjRes, SPEED_NETWORK_DOWNLOAD, pTmStats->iRecvRate);
    cJSON_AddNumberToObject(pSendjRes, SPEED_NETWORK_UPLOAD, pTmStats->iSendRate);
    cJSON_AddNumberToObject(pSendjRes, DATA_FLOW_DOWNLOAD, pTmStats->iRecvTotalLen);
    cJSON_AddNumberToObject(pSendjRes, DATA_FLOW_UPLOAD, pTmStats->iSendTotalLen);

    cJSON_AddNumberToObject(pSendjRes, WEB_KIDVPN_CLIENT, iKidVpnCli);
    cJSON_AddNumberToObject(pSendjRes, WEB_KIDVPN_SERVER, iKidVpnServ);
    cJSON_AddNumberToObject(pSendjRes, WEB_ARP_DEFENSE,   iArpDefense);

    switch (wanPageInfo.ucMode) {
    case OPER_TYPE_UDIAL:
        memcpy(cLinkType, "PPPoE", strlen("PPPoE"));
        break;

    case OPER_TYPE_DHCP:
        memcpy(cLinkType, "DHCP", strlen("DHCP"));
        break;

    case OPER_TYPE_STATIC:
        memcpy(cLinkType, "Static", strlen("Static"));

        pcPrimaryDNS = inet_ntoa(*(struct in_addr *)&dns_getserver(0)->u_addr.ip4);
        strncpy(cPrimaryDNS, pcPrimaryDNS, sizeof(cPrimaryDNS));
        pcSecondDNS = inet_ntoa(*(struct in_addr *)&dns_getserver(1)->u_addr.ip4);
        strncpy(cSecondDNS, pcSecondDNS, sizeof(cSecondDNS));

        cJSON_AddStringToObject(pSendjRes, WAN_ARG_DNS,        cPrimaryDNS);
        cJSON_AddStringToObject(pSendjRes, WAN_ARG_DNS_SECOND, cSecondDNS);

        cJSON_AddStringToObject(pSendjRes, WAN_ARG_GATEWAY, wanPageInfo.cGateWay);
        cJSON_AddStringToObject(pSendjRes, WAN_ARG_IP, wanPageInfo.cIpAddr);

        break;

    default:
        return  (PX_ERROR);
    }

    cJSON_AddStringToObject(pSendjRes, WAN_LINK_TYPE, cLinkType);

    free(pTmStats);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __situationJsInfoGet
** ��������: ��ȡ�龰ģʽJson��Ϣ
** �䡡��  : NONE
** �䡡��  : pSendjRes     ��ȡ�� Json ����
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __situationJsInfoGet (cJSON   *pSendjRes)
{
    __SITUATION_INFO  situationInfo;
    INT               iKidVpnCli  = 0;
    INT               iKidVpnServ = 0;
    INT               iArpDefense = 0;
    INT32             iRet;

    if (LW_NULL == pSendjRes) {
        return  (PX_ERROR);
    }

    bzero(&situationInfo, sizeof(__SITUATION_INFO));
    iRet = situationInfoGet(&situationInfo);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    iKidVpnCli  = (situationInfo.iFunc & FUNC_KIDVPN_CLIENT) ? 1:0;
    iKidVpnServ = (situationInfo.iFunc & FUNC_KIDVPN_SERVER) ? 1:0;
    iArpDefense = (situationInfo.iFunc & FUNC_ARP_DEFENSE)   ? 1:0;

    cJSON_AddStringToObject(pSendjRes, SIT_INFO_MODE,     situationInfo.ucSituationMode);
    cJSON_AddStringToObject(pSendjRes, SIT_INFO_POSITION, situationInfo.ucPosition);
    cJSON_AddStringToObject(pSendjRes, SIT_INFO_STATUS,   situationInfo.cStatus);

    cJSON_AddNumberToObject(pSendjRes, WEB_KIDVPN_CLIENT, iKidVpnCli);
    cJSON_AddNumberToObject(pSendjRes, WEB_KIDVPN_SERVER, iKidVpnServ);
    cJSON_AddNumberToObject(pSendjRes, WEB_ARP_DEFENSE,   iArpDefense);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: fetchSecKey
** ��������: ��ȡ��ȫ��Կ
** �䡡��  : cpcBuf      ���յ�������
** �䡡��  : NONE
** ������  : ��ȡ������Կ
*********************************************************************************************************/
static PCHAR  fetchSecKey (CPCHAR  cpcBuf)
 {
    PCHAR  pcKey;
    PCHAR  pcKeyBegin;
    PCHAR  pcFlag  = "Sec-WebSocket-Key: ";
    INT    iBufLen = 0;
    int    i       = 0;

    if (NULL == cpcBuf) {
        return  (NULL);
    }

    pcKey = (PCHAR)malloc(WEB_SOCKET_KEY_LEN_MAX);
    if (NULL == pcKey) {
        return  (NULL);
    }

    memset(pcKey, 0, WEB_SOCKET_KEY_LEN_MAX);

    pcKeyBegin = strstr(cpcBuf, pcFlag);
    if (NULL == pcKeyBegin) {
        return  (NULL);
    }

    pcKeyBegin += strlen(pcFlag);
    iBufLen     = strlen(cpcBuf);

    /*
     *  ��ȡ��ȫ��Կ
     */
    for (i = 0; i < iBufLen; i++) {
        if (pcKeyBegin[i] == 0x0A || pcKeyBegin[i] == 0x0D) {           /*  �������л�س�����ѭ��      */
            break;
        }

        pcKey[i] = pcKeyBegin[i];
    }

    return  (pcKey);
}
/*********************************************************************************************************
** ��������: computeAcceptKey
** ��������: ������յ����ݵ���Կ
** �䡡��  : cpcBuf      ���յ�����
** �䡡��  : NONE
** ������  : ����õ�����Կ
*********************************************************************************************************/
static PCHAR  computeAcceptKey (CPCHAR  cpcBuf)
{
    PCHAR   pcClientKey;
    PCHAR   pcServerKey;
    PCHAR   pcSha1DataTemp;
    PCHAR   pcSha1Data;
    INT     i;
    INT     iLen;
    CPCHAR  cpcGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    if (NULL == cpcBuf) {
        return  (NULL);
    }

    pcClientKey = fetchSecKey(cpcBuf);
    if (NULL == pcClientKey) {
        return  (NULL);
    }

    strcat(pcClientKey, cpcGUID);

    pcSha1DataTemp = sha1Hash(pcClientKey);
    iLen           = strlen(pcSha1DataTemp);

    pcSha1Data = (PCHAR) malloc(iLen / 2 + 1);
    memset(pcSha1Data, 0, iLen / 2 + 1);

    for (i = 0; i < iLen; i += 2) {
        pcSha1Data[i / 2] = hexToInt(pcSha1DataTemp, i, 2);
    }

    pcServerKey = base64Encode(pcSha1Data, iLen / 2);

    return  (pcServerKey);
}
/*********************************************************************************************************
** ��������: shakeHand
** ��������: ���ִ���
** �䡡��  : iConnFd        ���ӵ��׽��־��
**           cpcServerKey   ��Կ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  shakeHand (INT  iConnFd,CPCHAR  cpcServerKey)
{
    CHAR  cResponseHeader[RESPONSE_HEADER_LEN_MAX] = {0};

    if ((iConnFd <= 0) || (NULL == cpcServerKey)) {
        return;
    }

    sprintf(cResponseHeader, "HTTP/1.1 101 Switching Protocols\r\n");
    sprintf(cResponseHeader, "%sUpgrade: websocket\r\n",  cResponseHeader);
    sprintf(cResponseHeader, "%sConnection: Upgrade\r\n", cResponseHeader);
    sprintf(cResponseHeader, "%sSec-WebSocket-Accept: %s\r\n\r\n", cResponseHeader, cpcServerKey);

    write(iConnFd, cResponseHeader, strlen(cResponseHeader));
}

/*********************************************************************************************************
** ��������: analyData
** ��������: ������������
** �䡡��  : cpcBuf      ��������
**           iBufLen     ���ݳ���
** �䡡��  : NONE
** ������  : �����������
*********************************************************************************************************/
static PCHAR  analyData (CPCHAR  cpcBuf, INT  iBufLen)
{
    CHAR         cFin;
    CHAR         cMaskFlag;
    CHAR         cMasks[4];
    CHAR         cTemp[8];
    ULONG        ulLen;
    ULONG        ulPayloadLen = 0;
    int          i            = 0;
    static CHAR  cPayloadData[REQUEST_LEN_MAX + 4];

    if ((iBufLen < 2) || (NULL == cpcBuf)) {
        return  (NULL);
    }

    cFin = (cpcBuf[0] & 0x80) == 0x80;                                  /*  1bit��1 ��ʾ���һ֡        */
    if (0 == cFin) {                                                    /*  ����һ֡�ݲ�����            */
        return  (NULL);
    }

    cMaskFlag = (cpcBuf[1] & 0x80) == 0x80;                             /*  �Ƿ��������                */
    if (0 == cMaskFlag) {                                               /*  ������������ݲ�����        */
        return  (NULL);
    }

    bzero(cPayloadData, sizeof(cPayloadData));
    ulPayloadLen = cpcBuf[1] & 0x7F;                                    /*   ���ݳ���                   */
    if (ulPayloadLen == 126) {
        memcpy(cMasks, cpcBuf + 4, 4);
        ulPayloadLen = (cpcBuf[2] & 0xFF) << 8 | (cpcBuf[3] & 0xFF);

        memcpy(cPayloadData, cpcBuf + 8, ulPayloadLen);
    } else if (ulPayloadLen == 127) {
        memcpy(cMasks, cpcBuf + 10, 4);
        for (i = 0; i < 8; i++) {
            cTemp[i] = cpcBuf[9 - i];
        }

        memcpy(&ulLen, cTemp, 8);

        memcpy(cPayloadData, cpcBuf + 14, ulLen);
        ulPayloadLen = ulLen;
    } else {
        memcpy(cMasks, cpcBuf + 2, 4);

        memcpy(cPayloadData, cpcBuf + 6, ulPayloadLen);
    }

    for (i = 0; i < ulPayloadLen; i++) {
        cPayloadData[i] = (char) (cPayloadData[i] ^ cMasks[i % 4]);
    }

    return  (cPayloadData);
}
/*********************************************************************************************************
** ��������: packData
** ��������: ��װ����
** �䡡��  : cpcMsg      ����װ����
** �䡡��  : pulLen      ��װ������ݳ���
** ������  : ��װ�������
*********************************************************************************************************/
static PCHAR  packData (CPCHAR  cpcMsg, ULONG  *pulLen)
 {
    static CHAR  cData[REQUEST_LEN_MAX + 8];
    ULONG        ulLen;

    if ((NULL == cpcMsg) || (NULL == pulLen)) {
        return  (NULL);
    }

    bzero(cData, REQUEST_LEN_MAX + 8);
    ulLen = strlen(cpcMsg);
    if (ulLen < 126) {
        cData[0] = 0x81;
        cData[1] = ulLen;

        memcpy(cData + 2, cpcMsg, ulLen);
        *pulLen = ulLen + 2;
    } else if (ulLen < 0xFFFF) {
        cData[0] = 0x81;
        cData[1] = 126;
        cData[2] = (ulLen >> 8 & 0xFF);
        cData[3] = (ulLen & 0xFF);

        memcpy(cData + 4, cpcMsg, ulLen);

        *pulLen = ulLen + 4;
    } else {
        /*
         *  �ݲ�����������
         */
        *pulLen = 0;
    }

    return  (cData);
}
/*********************************************************************************************************
** ��������: SHA1Reset
** ��������: ��ʼ�� SHA1
** �䡡��  : pContext         SHA1 �ṹָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  SHA1Reset (__PSHA1_CONTEXT  pContext)
{
    pContext->uiLengthLow        = 0;
    pContext->uiLengthHigh       = 0;
    pContext->iMessageBlockIndex = 0;

    pContext->uiMessageDigest[0] = 0x67452301;
    pContext->uiMessageDigest[1] = 0xEFCDAB89;
    pContext->uiMessageDigest[2] = 0x98BADCFE;
    pContext->uiMessageDigest[3] = 0x10325476;
    pContext->uiMessageDigest[4] = 0xC3D2E1F0;

    pContext->iComputed  = 0;
    pContext->iCorrupted = 0;
}
/*********************************************************************************************************
** ��������: SHA1Result
** ��������: SHA1 ������
** �䡡��  : pContext         SHA1 �ṹָ��
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  SHA1Result (__PSHA1_CONTEXT  pContext)
{
    if (NULL == pContext) {
        return  (PX_ERROR);
    }

    if (1 == pContext->iCorrupted) {
        return  (PX_ERROR);
    }

    if (0 == pContext->iComputed) {
        SHA1PadMessage(pContext);
        pContext->iComputed = 1;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: SHA1Input
** ��������: SHA1 ����
** �䡡��  : pContext         SHA1 �ṹָ��
**           cpcMsgArray      ��Ϣ����
**           uiLength         ����
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  SHA1Input (__PSHA1_CONTEXT  pContext, CPCHAR  cpcMsgArray, UINT  uiLength)
{
    if ((0 == uiLength) || (NULL == pContext) || (NULL == cpcMsgArray)) {
        return;
    }

    if (pContext->iComputed || pContext->iCorrupted) {
        pContext->iCorrupted = 1;

        return;
    }

    while (uiLength-- && !pContext->iCorrupted) {
        pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = (*cpcMsgArray & 0xFF);

        pContext->uiLengthLow += 8;
        pContext->uiLengthLow &= 0xFFFFFFFF;

        if (pContext->uiLengthLow == 0) {
            pContext->uiLengthHigh++;
            pContext->uiLengthHigh &= 0xFFFFFFFF;

            if (pContext->uiLengthHigh == 0) {
                pContext->iCorrupted = 1;
            }
        }

        if (pContext->iMessageBlockIndex == 64) {
            SHA1ProcessMessageBlock(pContext);
        }

        cpcMsgArray++;
    }
}
/*********************************************************************************************************
** ��������: SHA1ProcessMessageBlock
** ��������: SHA1 ������Ϣ��
** �䡡��  : pContext         SHA1 �ṹָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  SHA1ProcessMessageBlock (__PSHA1_CONTEXT  pContext)
{
    const UINT K[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
    INT        t;
    unsigned   temp;
    unsigned   W[80];
    unsigned   A, B, C, D, E;

    if (NULL == pContext) {
        return;
    }

    for (t = 0; t < 16; t++) {
        W[t]  = ((unsigned) pContext->uiMessageBlock[t * 4])     << 24;
        W[t] |= ((unsigned) pContext->uiMessageBlock[t * 4 + 1]) << 16;
        W[t] |= ((unsigned) pContext->uiMessageBlock[t * 4 + 2]) << 8;
        W[t] |= ((unsigned) pContext->uiMessageBlock[t * 4 + 3]);
    }

    for (t = 16; t < 80; t++) {
        W[t] = SHA1CircularShift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
    }

    A = pContext->uiMessageDigest[0];
    B = pContext->uiMessageDigest[1];
    C = pContext->uiMessageDigest[2];
    D = pContext->uiMessageDigest[3];
    E = pContext->uiMessageDigest[4];

    for (t = 0; t < 20; t++) {
        temp = SHA1CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[t]
                + K[0];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 20; t < 40; t++) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 40; t < 60; t++) {
        temp = SHA1CircularShift(5, A) + ((B & C) | (B & D) | (C & D)) + E
                + W[t]
                + K[2];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30, B);
        B = A;
        A = temp;
    }

    for (t = 60; t < 80; t++) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30, B);
        B = A;
        A = temp;
    }

    pContext->uiMessageDigest[0] = (pContext->uiMessageDigest[0] + A) & 0xFFFFFFFF;
    pContext->uiMessageDigest[1] = (pContext->uiMessageDigest[1] + B) & 0xFFFFFFFF;
    pContext->uiMessageDigest[2] = (pContext->uiMessageDigest[2] + C) & 0xFFFFFFFF;
    pContext->uiMessageDigest[3] = (pContext->uiMessageDigest[3] + D) & 0xFFFFFFFF;
    pContext->uiMessageDigest[4] = (pContext->uiMessageDigest[4] + E) & 0xFFFFFFFF;

    pContext->iMessageBlockIndex = 0;
}
/*********************************************************************************************************
** ��������: SHA1PadMessage
** ��������: SHA1 ��װ��Ϣ
** �䡡��  : pContext         SHA1 �ṹָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  SHA1PadMessage (__PSHA1_CONTEXT  pContext)
{
    if (NULL == pContext) {
        return;
    }

    if (pContext->iMessageBlockIndex > 55) {
        pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = 0x80;

        while (pContext->iMessageBlockIndex < 64) {
            pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = 0;
        }

        SHA1ProcessMessageBlock(pContext);

        while (pContext->iMessageBlockIndex < 56) {
            pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = 0;
        }
    } else {
        pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = 0x80;

        while (pContext->iMessageBlockIndex < 56) {
            pContext->uiMessageBlock[pContext->iMessageBlockIndex++] = 0;
        }
    }

    pContext->uiMessageBlock[56] = (pContext->uiLengthHigh >> 24) & 0xFF;
    pContext->uiMessageBlock[57] = (pContext->uiLengthHigh >> 16) & 0xFF;
    pContext->uiMessageBlock[58] = (pContext->uiLengthHigh >> 8)  & 0xFF;
    pContext->uiMessageBlock[59] = (pContext->uiLengthHigh)       & 0xFF;
    pContext->uiMessageBlock[60] = (pContext->uiLengthLow >> 24)  & 0xFF;
    pContext->uiMessageBlock[61] = (pContext->uiLengthLow >> 16)  & 0xFF;
    pContext->uiMessageBlock[62] = (pContext->uiLengthLow >> 8)   & 0xFF;
    pContext->uiMessageBlock[63] = (pContext->uiLengthLow)        & 0xFF;

    SHA1ProcessMessageBlock(pContext);
}
/*********************************************************************************************************
** ��������: sha1Hash
** ��������: ���� SHA1 HASHֵ
** �䡡��  : cpcSource     SHA1 �ַ���
** �䡡��  : NONE
** ������  : ����� SHA1 ժҪ
*********************************************************************************************************/
static PCHAR  sha1Hash (CPCHAR  cpcSource)
{
    __SHA1_CONTEXT  shaConTxt;
    PCHAR           pcBuf = NULL;

    if (NULL == cpcSource) {
        return  (NULL);
    }

    SHA1Reset(&shaConTxt);
    SHA1Input(&shaConTxt, cpcSource, strlen(cpcSource));

    if (ERROR_NONE != SHA1Result(&shaConTxt)) {
        fprintf(stderr, "SHA1 ERROR: Could not compute message digest");
    } else {
        pcBuf = (PCHAR)malloc(128);

        memset(pcBuf, 0, 128);

        sprintf(pcBuf, "%08X%08X%08X%08X%08X",  shaConTxt.uiMessageDigest[0],
                  shaConTxt.uiMessageDigest[1], shaConTxt.uiMessageDigest[2],
                  shaConTxt.uiMessageDigest[3], shaConTxt.uiMessageDigest[4]);
    }

    return  (pcBuf);
}
/*********************************************************************************************************
** ��������: hexToInt
** ��������: ʮ������תʮ����
** �䡡��  : cpcStr      ʮ�������ַ���ָ��
**           iStart      �ַ�����ʼ�±�
**           iStrLen     �ַ�������
** �䡡��  : NONE
** ������  : ת�����������
*********************************************************************************************************/
static INT  hexToInt (CPCHAR  cpcStr, INT  iStart, INT  iStrLen)
{
    INT  iIndex;
    INT  iLen;
    INT  iRetDat = 0;

    if (cpcStr[0] == '0' && (cpcStr[1] == 'x' || cpcStr[1] == 'X')) {   /*  �ж��Ƿ���ǰ�� 0x ���� 0X   */
        iIndex = 2;
    } else {
        iIndex = 0;
    }

    iIndex += iStart;
    iLen    = 0;

    for (; (cpcStr[iIndex] >= '0' && cpcStr[iIndex] <= '9') ||
           (cpcStr[iIndex] >= 'a' && cpcStr[iIndex] <= 'f') ||
           (cpcStr[iIndex] >= 'A' && cpcStr[iIndex] <= 'F'); ++iIndex) {
        if(iLen >= iStrLen) {
            break;
        }

        if (tolower(cpcStr[iIndex]) > '9') {
            iRetDat = 16 * iRetDat + (10 + tolower(cpcStr[iIndex]) - 'a');
        } else {
            iRetDat = 16 * iRetDat + (tolower(cpcStr[iIndex]) - '0');
        }

        iLen++;
    }

    return  (iRetDat);
}
/*********************************************************************************************************
** ��������: base64Encode
** ��������: �Դ������ݽ��б���
** �䡡��  : cpcData     �ַ�������ָ��
**           iDataLen    ���ݳ���
** �䡡��  : NONE
** ������  : ����������ָ��
*********************************************************************************************************/
static PCHAR  base64Encode (CPCHAR  cpcData, INT  iDataLen)
{
    INT    iPrepare    = 0;
    INT    iRetLen     = 0;
    INT    iTemp       = 0;
    PCHAR  pcRetBuf    = NULL;
    PCHAR  pcFBuf      = NULL;
    INT    iTmp        = 0;
    CHAR   cChanged[4] = {0};
    INT    i           = 0;

    if (NULL == cpcData) {
        return  (NULL);
    }

    iRetLen = iDataLen / 3;
    iTemp   = iDataLen % 3;

    if (iTemp > 0) {
        iRetLen += 1;
    }

    iRetLen = iRetLen*4 + 1;

    pcRetBuf = (PCHAR)malloc(iRetLen);
    if (NULL == pcRetBuf) {
        fprintf(stderr, "No enough memory.\n");

        return  (NULL);
    }

    memset(pcRetBuf, 0, iRetLen);
    pcFBuf = pcRetBuf;

    while (iTmp < iDataLen) {
        iTemp    = 0;
        iPrepare = 0;

        bzero(cChanged, sizeof(cChanged));

        while (iTemp < 3) {
            if (iTmp >= iDataLen) {
                break;
            }

            iPrepare = ((iPrepare << 8) | (cpcData[iTmp] & 0xFF));
            iTmp++;
            iTemp++;
        }

        iPrepare = (iPrepare<<((3-iTemp)*8));

        for (i = 0; i < 4 ;i++ ) {
            if (iTemp < i) {
                cChanged[i] = 0x40;
            } else {
                cChanged[i] = (iPrepare>>((3-i)*6)) & 0x3F;
            }

            *pcFBuf = _G_ccBase[(INT)cChanged[i]];

            pcFBuf++;
        }
    }

    *pcFBuf = '\0';

    return  (pcRetBuf);
}

/*********************************************************************************************************
** ��������: __sendInfoToWeb
** ��������: ����Ϣ���ظ�ǰ��
** ��  ��  : iSockFd           socket ���
**           pSendjRes         JSON ��Ϣ
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __sendInfoToWeb (INT32  iSockFd, cJSON  *pSendjRes)
{
    ULONG           ulLen      = 0;
    PCHAR           pcJsonBuf  = LW_NULL;
    PCHAR           pcSendbuf  = LW_NULL;

    pcJsonBuf = cJSON_Print(pSendjRes);
    pcSendbuf = packData(pcJsonBuf, &ulLen);
    write(iSockFd, pcSendbuf, ulLen);
    free(pcJsonBuf);
}
/*********************************************************************************************************
** ��������: __situationInfoHandle
** ��������: �龰ģʽ��Ϣ��ȡ����
** ��  ��  : iSockFd           socket ���
**           pcData            ������Ϣ
** ��  ��  : NONE
** ��  ��  : NONE
*********************************************************************************************************/
static VOID  __situationInfoHandle (INT32  iSockFd, PCHAR  pcData)
{
    cJSON          *pSendjRes  = LW_NULL;

    pSendjRes = cJSON_CreateObject();                                   /*  ��������Json����            */
    if (LW_NULL == pSendjRes) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "cJSON_CreateObject Failed.\n");
        write(iSockFd, "error", strlen("error"));

        return;
    }

    switch(atoi(&pcData[0])) {
    case SITUATION_INFO_GET:
        __situationJsInfoGet(pSendjRes);
        break;

    case WANPAGE_INFO_GET:
        __wanPageJsInfoGet(pSendjRes);
        break;

    case KIDVPN_SERVER_INFO_GET:
        __kidvpnServPageJsInfoGet(pSendjRes);
        break;

    case DELETE_ALL_DATA:
        __allDataDelete();
        break;

    case SCENEMODE_GUIDE_STAT_GET:
        __sceneModeGuideStatusGet(pSendjRes);
        break;

    case SCENE_MODE_ALL_INFO_GET:
        __sceneModeAllInfoGet(pSendjRes);
        break;

    case KIDVPN_CLIENT_INFO_GET:
        __kidvpnCliPageJsInfoGet(pSendjRes);
        break;

    default:
        break;
    }

    __sendInfoToWeb(iSockFd, pSendjRes);

    cJSON_Delete(pSendjRes);
}

/*********************************************************************************************************
** ��������: websocketThread
** ��������: websocket �����߳�
** ��  ��  : pvArg     �̲߳���
** ��  ��  : NONE
** ��  ��  : NULL
*********************************************************************************************************/
static PVOID  websocketThread (PVOID  pvArg)
{
    INT                 iRet;
    INT                 iListenfd;
    CHAR                cBuf[REQUEST_LEN];
    PCHAR               pcData;
    PCHAR               pcSecWebSocketKey;
    INT                 iLen;
    INT                 iPort;
    UINT                uiOptval;
    INT                 iConnected = 0;
    INT                 iFdMax     = -1;
    struct timeval      timeOut    = {5, 0};
    fd_set              fdSet;
    INT                 iConnFd;

    struct sockaddr_in  sockServaddr;
    struct sockaddr_in  sockCliaddr;
    socklen_t           sockCliaddrLen;

    iListenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&sockServaddr, sizeof(sockServaddr));
    sockServaddr.sin_family      = AF_INET;
    sockServaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockServaddr.sin_port        = htons(WBS_SERVER_PORT);

    /*
     *  ���� SO_REUSEADDR ѡ��(��������������)
     */
    uiOptval = 0x1;
    setsockopt(iListenfd, SOL_SOCKET, SO_REUSEADDR, &uiOptval, 4);

    bind(iListenfd, (struct sockaddr *) &sockServaddr, sizeof(sockServaddr));

    listen(iListenfd, 20);

    printf("Listen %d\nAccepting connections ...\n", WBS_SERVER_PORT);

    sockCliaddrLen = sizeof(sockCliaddr);

    /*
     *  ׼�����ܿͻ�����������
     */
    iConnFd = accept(iListenfd, (struct sockaddr *) &sockCliaddr, &sockCliaddrLen);

    iPort = (INT)ntohs(sockCliaddr.sin_port);
    printf("From: %s at PORT: %d.\n", inet_ntoa(sockCliaddr.sin_addr), iPort);

    while (1) {
        iFdMax = -1;

        iFdMax = MAX_FD(iFdMax, iConnFd);
        iFdMax = MAX_FD(iFdMax, iListenfd);

        FD_ZERO(&fdSet);

        if (iConnFd > 0) {
            FD_SET(iConnFd, &fdSet);
        }

        if (iListenfd > 0) {
            FD_SET(iListenfd, &fdSet);
        }

        /*
         *  IO ��·���ã��ȴ��ɶ��ļ���
         */
        iRet = select(iFdMax + 1, &fdSet, NULL, NULL, &timeOut);
        if (iRet > 0) {
            if (iConnFd > 0) {
                if (FD_ISSET(iConnFd, &fdSet)) {                        /*  �ͻ��˿ɶ�, ���������ȡ����*/
                    memset(cBuf, 0, REQUEST_LEN);
                    iLen = read(iConnFd, cBuf, REQUEST_LEN);
                    if (iLen > 6) {
                        if (0 == iConnected) {                          /*  websocket ����������֤    */
                            pcSecWebSocketKey = computeAcceptKey(cBuf);
                            shakeHand(iConnFd, pcSecWebSocketKey);

                            free(pcSecWebSocketKey);

                            iConnected = 1;

                            continue;
                        }

                        /*
                         *  �����������ݣ����Ա���������Ϣ������Ӧ
                         */
                        pcData = analyData(cBuf, iLen);
                        __situationInfoHandle(iConnFd, pcData);
                    } else {
                        close(iConnFd);                                 /*  �رտͻ��������׽���        */
                        iConnFd    = 0;
                        iConnected = 0;
                    }
                }
            }

            if (iListenfd > 0) {
                if (FD_ISSET(iListenfd, &fdSet)) {                      /*  ��������ɶ���˵����������  */
                    if (iConnFd) {                                      /*  ����Ѿ�����������          */
                        close(iConnFd);                                 /*  �رտͻ��������׽���        */
                        iConnFd    = 0;
                        iConnected = 0;
                    }

                    iConnFd = accept(iListenfd, (struct sockaddr *)&sockCliaddr, &sockCliaddrLen);
                    if (iConnFd <= 0) {
                        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "accept client connect failed.\n");

                        continue;
                    }

                    iPort = (INT)ntohs(sockCliaddr.sin_port);
                    printf("From: %s at PORT: %d.\n", inet_ntoa(sockCliaddr.sin_addr), iPort);
                }
            }
        }
    }

    return  (NULL);
}

/*********************************************************************************************************
** ��������: websocketInit
** ��������: Websocket ��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  websocketInit (VOID)
{
    LW_CLASS_THREADATTR  threadAttr;

    Lw_ThreadAttr_Build(&threadAttr,
                        64 * LW_CFG_THREAD_DEFAULT_STK_SIZE,
                        LW_PRIO_NORMAL,
                        LW_OPTION_THREAD_STK_CHK,
                        NULL);

    _G_hWebsockTid = Lw_Thread_Create("t_websocket",
                                       websocketThread,
                                       &threadAttr,
                                       LW_NULL);
    if (LW_OBJECT_HANDLE_INVALID == _G_hWebsockTid) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Thread Create Failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
