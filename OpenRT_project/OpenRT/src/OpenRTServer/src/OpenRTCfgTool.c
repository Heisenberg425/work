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
** ��   ��   ��: OpenRTCfgTool.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: OpenRT ���ù���
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "libcomm.h"
#include "log/log_common.h"
#include "libserver_type.h"
#include "nat/nat_common.h"
#include "hardware/hardware_common.h"
#include "kidvpn/kidvpn_common.h"
#include "vnd/vnd_common.h"
#include "udial/udial_common.h"
#include "intadaption/intada_common.h"
#include "interface/interface_common.h"
#include "route/route_common.h"
#include "vlan/vlan_common.h"
#include "flowctl/flowctl_common.h"
#include "monitor/monitor_common.h"
#include "database/database_common.h"
#include "arp/arp_common.h"
#include "qos/qos_common.h"
#include "npf/npf_common.h"
#include "dns/dns_common.h"
#include "asdefense/asdefense_common.h"
#include "dhcpserver/dhcpserver_common.h"
#include "dhcp/dhcp_common.h"
#include "login/login_common.h"
#include "timeset/timeset_common.h"
#include "power/power_common.h"
#include "devname/devname_common.h"
#include "ping/ping_common.h"
#include "updatefirmware/updatefirmware_common.h"
#include "version/version_common.h"
#include "quagga/quagga_common.h"
#include "situationMode/situation_common.h"
/*********************************************************************************************************
  ͨ�Ž��ջ������ֵ
*********************************************************************************************************/
#define OPENRT_TOOL_BUFF_LEN  (1024)                                    /*  OpenRT ����������           */
#define ARP_LOG_FILE          "/var/arpspoofdefenseLog"                 /*  arp ��־�ļ�·��            */
#define ARP_LOG_SYMLINK       "/apps/goahead/web/ArpSpoodDefenseLog"    /*  arp ��־�����ļ�·��        */
#define DHCP_LOG_SYMLINK      "/apps/goahead/web/dhcpdlog"              /*  dhcp server ��־����·��    */
#define WEB_PATH              "/apps/goahead/web"                       /*  web Ŀ¼·��                */
#define FILE_MODE             (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)   /*  �����ļ�ģʽ                */
/*********************************************************************************************************
** ��������: __OpenRTDataBaseInit
** ��������: ���ݿ��ʼ��
** �䡡��  : cpcDBFileName      ���ݿ�����
** �䡡��  : NONE
** ������  : ���ݿ�ִ�н��
*********************************************************************************************************/
static INT  __logFileSymLinkCreate (VOID)
{
    INT          iFd;
    INT          iRet = PX_ERROR;

    /*
     *  ���� DHCP server ��־�ļ�����·��
     */
    if (PX_ERROR == access(DHCPSERVER_DEF_PATH, F_OK)) {
        mkdir(DHCPSERVER_DEF_PATH, S_IRWXU|S_IRGRP|S_IROTH);
    }

    if (PX_ERROR == access(WEB_PATH, F_OK)) {
        printf("web's content not exist.\n");

        mkdir(WEB_PATH, S_IRWXU|S_IRGRP|S_IROTH);

        return  (PX_ERROR);
    }

    if (PX_ERROR == access(DHCPSERVER_DEF_LOGFILE, F_OK)) {
        iFd = open(DHCPSERVER_DEF_LOGFILE, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }
    }

    if (ERROR_NONE == access(DHCP_LOG_SYMLINK, F_OK)) {
        unlink(DHCP_LOG_SYMLINK);
    }

    iRet = symlink(DHCPSERVER_DEF_LOGFILE, DHCP_LOG_SYMLINK);
    if (PX_ERROR == iRet) {
        printf("Create DHCP link file failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ���� arp ��־�ļ�����·��
     */
    if (PX_ERROR == access(ARP_LOG_FILE, F_OK)) {
        iFd = open(ARP_LOG_FILE, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
        if (iFd < 0) {
            printf("Create file failed.\n");

            return  (PX_ERROR);
        }
    }

    if (ERROR_NONE == access(ARP_LOG_SYMLINK, F_OK)) {
        unlink(ARP_LOG_SYMLINK);
    }

    iRet = symlink(ARP_LOG_FILE, ARP_LOG_SYMLINK);
    if (PX_ERROR == iRet) {
        printf("Create ARP link file failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __OpenRTDataBaseInit
** ��������: ���ݿ��ʼ��
** �䡡��  : cpcDBFileName      ���ݿ�����
** �䡡��  : NONE
** ������  : ���ݿ�ִ�н��
*********************************************************************************************************/
static INT  __OpenRTDataBaseInit (VOID)
{
    INT          iRet  = PX_ERROR;

    _G_pOpenRTCfgDBHandle = LW_NULL;

    printf("start init DateBase.\n");

    if (PX_ERROR == access(DATABASE_PATH, F_OK)) {
        system("mkdir "DATABASE_PATH);                                  /*  �������ݿ�Ŀ¼              */
    }

    /*
     *  �����ݿ�
     */
    iRet = sqlite3_open(CFG_DB_FILE, &_G_pOpenRTCfgDBHandle);
    if(SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base open failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ��ʼ����ģ������������ݿ�
     */
    INIT_CHECK(intAdaptionTableCreate());
    INIT_CHECK(natTableCreate());
    INIT_CHECK(udialTableCreate());
    INIT_CHECK(vndTableCreate());
    INIT_CHECK(kidvpnTableCreate());
    INIT_CHECK(kidvpnLinkTableCreate());
    INIT_CHECK(routeTableCreate());
    INIT_CHECK(vlanTableCreate());
    INIT_CHECK(flowctlTableCreate());
    INIT_CHECK(arpTableCreate());
    INIT_CHECK(qosTableCreate());
    INIT_CHECK(npfTableCreate());
    INIT_CHECK(dnsTableCreate());
    INIT_CHECK(loginTableCreate());
    INIT_CHECK(devNameTableCreate());
    INIT_CHECK(powerEventTableCreate());
    INIT_CHECK(dhcpServerTableCreate());
    INIT_CHECK(quaggaTableCreate());
    INIT_CHECK(asDefenseTableCreate());
    INIT_CHECK(situationTableCreate());

    printf("Module DateBase Init Finished.\n");

    /*
     *  ��ʼ����־ϵͳ���ݿ�
     */
    INIT_CHECK(udialLogTableCreate());
    INIT_CHECK(operLogTableCreate());
    INIT_CHECK(vpnLogTableCreate());

    printf("Log DateBase Init Finished.\n");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __OpenRTToolExit
** ��������: �����Ƴ�����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __OpenRTToolExit (VOID)
{
    quaggaProcDestroy();

    if (NULL != _G_pOpenRTCfgDBHandle) {
        sqlite3_close(_G_pOpenRTCfgDBHandle);

        _G_pOpenRTCfgDBHandle = NULL;
    }
}

/*********************************************************************************************************
** ��������: __OpenRTToolInit
** ��������: ���߳�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __OpenRTToolInit (VOID)
{
    printf("OpenRT Init started.\n");
    INIT_CHECK(__OpenRTDataBaseInit());                                 /*  ���ݿ��ʼ��                */
    INIT_CHECK(initSerCommFd());                                        /*  ͨѶ�ӿڳ�ʼ��              */
    INIT_CHECK(__logFileSymLinkCreate());                               /*  ������־�ļ�������·��      */

    printf("Communicate Init finished.\n");

    /*
     *  һЩ��������ʱ�ĳ�ʼ������
     */
    INIT_CHECK(natCfgInit());
    INIT_CHECK(kidVpnInit());
    INIT_CHECK(udialInit());

    INIT_CHECK(subProRecliamInit());                                    /*  �����ӽ��̻����߳�          */
    INIT_CHECK(powerEventInit());

    printf("Module Func Init finished.\n");

    /*
     *  ���ûָ�����
     */
    INIT_CHECK(udialRecovery());
    sleep(3);
    INIT_CHECK(qosRecovery());
    INIT_CHECK(dhcpServerRecovery());
    INIT_CHECK(dnsRecovery());
    INIT_CHECK(kidvpnRecovery());
    INIT_CHECK(vlanRecovery());
    INIT_CHECK(flowctlRecovery());
    INIT_CHECK(updateFirmwareRecovery());
    INIT_CHECK(npfRecovery());
    INIT_CHECK(quaggaRecovery());
    asDefenseRecovry();
    natRecovry();
    INIT_CHECK(arpRecovery());
    INIT_CHECK(routeRecovery());

    printf("Recovery finished.\n");

    websocketInit();

    return  atexit((VOID (*)(VOID))__OpenRTToolExit);
}

/*********************************************************************************************************
** ��������: main
** ��������: ������
** �䡡��  : iArgc     ��������
**           pcArgv    ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  main (INT iArgc, PCHAR  pcArgv[])
{
    INT                     iRet;
    INT                     ipCommand;
    INT                     ipbuffLen;
    INT                     iServerTyep;
    VOID                   *pvbuff;
    INT                     i = 0;

    /*
     * ���߳�ʼ��
     */
    if (__OpenRTToolInit()) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "OpenRT Tool Init failed.\n");

        return  (PX_ERROR);
    }

    pvbuff = malloc(OPENRT_TOOL_BUFF_LEN);
    if (NULL == pvbuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error");

        return  (PX_ERROR);
    }

    printf("OpenRT Tool is working...\n");

    if (ERROR_NONE != access("/apps/goahead/goahead", F_OK)) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "goahead application not exist.\n");

        return  (PX_ERROR);
    }

    printf("goahead server startup now");

    for(i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }

    printf("\n");

    iRet = system("/apps/goahead/goahead --home /apps/goahead &");
    if (ERROR_NONE == iRet) {
        printf("goahead server is running...\n");
    } else {
        printf("goahead server start failed!\n");

        return  (PX_ERROR);
    }

    while (1) {
        iRet = serverRecv(&ipCommand, pvbuff, &ipbuffLen, OPENRT_TOOL_BUFF_LEN);
        if (COMM_ERROR == iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "serverRecv error");

            continue;
        }

        iServerTyep = GET_OPENRT_SERVER_TYPE(ipCommand);
        switch (iServerTyep) {
        case OPEN_RT_VPN_CONFIG_BASE:
            kidvpnHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_INTADA_CONFIG_BASE:
            intAdaHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_INTERFACE_CONFIG_BASE:
            interFaceHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_NAT_CONFIG_BASE:
            natHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_VND_CONFIG_BASE:
            vndHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_UDAIL_CONFIG_BASE:
            udialHandleEntry(ipCommand, pvbuff);
            break;

       case OPEN_RT_ROUTE_CONFIG_BASE:
            routeHandleEntry(ipCommand, pvbuff);
            break;

       case OPEN_RT_VLAN_CONFIG_BASE:
            vlanHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_FLOWCTL_CONFIG_BASE:
            flowctlHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_DHCP_CONFIG_BASE:
            dhcpHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_LOG_CONFIG_BASE:
            logHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_MONITOR_CONFIG_BASE:
            monitorHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_ARP_CONFIG_BASE:
            arpHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_ARP_DEFENSE_BASE:
            asDefenseHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_LOGIN_BASE:
            loginHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_TIMESET_CONFIG_BASE:
            timeHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_POWER_CONFIG_BASE:
            powerHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_DEVNAME_CONFIG_BASE:
            devNameHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_PING_TEST_CONFIG_BASE:
            pingHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_HWINFO_CONFIG_BASE:
            hwInfoHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_DHCPSER_CONF_BASE:
            dhcpServerHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_UPDATEFIRMWARE_CONFIG_BASE:
            updateFirmwareHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_NPF_CONFIG_BASE:
            npfHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_DNS_CONFIG_BASE:
            dnsHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_VERSION_CONFIG_BASE:
            versionHandleEntry(ipCommand, pvbuff);
            break;

        case OPEN_RT_QOS_CONFIG_BASE:
            qosHandleEntry(ipCommand, pvbuff);

            break;
        case OPEN_RT_SIT_CONFIG_BASE:
            situationHandleEntry(ipCommand, pvbuff);
            break;

        default :
            break;
        }
    }

    return  (PX_ERROR);
}
