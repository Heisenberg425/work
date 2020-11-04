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
** ��   ��   ��: arpSpoofDefense.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: ARP ��ƭ��������
**               ����0.2�汾�����������ļ�����(2019��1��2��)
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <module.h>
#include <time.h>
#include "net/if_ether.h"
#include "lwip/prot/ip4.h"
#include "netdev.h"
#include "stdlib.h"
#include "netif/etharp.h"
#include "arpSpoofDefense.h"
#include "asd_cfg.h"
/*********************************************************************************************************
  �汾
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_VERSION    "0.2"                              /* ARP �����汾                 */
/*********************************************************************************************************
  ARP ��־λ
*********************************************************************************************************/
#define ARP_NORMAL_FLAG            (0x0)                                /*  Arp��Ϣ����                 */
#define ARP_NORMAL_STR             ("normal")                           /*  Arp�����ַ���               */
#define ARP_CONFLICT_FLAG          (0x01)                               /*  ����MAC��ַ��ͻ             */
#define ARP_CONFLICT_STR           ("conflict")                         /*  Arp��ͻ�ַ���               */
#define ARP_ATTACK_FLAG            (0x02)                               /*  ����MAC��ƭ                 */
#define ARP_ATTACK_STR             ("attack")                           /*  Arp�����ַ���               */
/*********************************************************************************************************
  HASH ���ú�
*********************************************************************************************************/
#define HASH_SIZE                  (512)                                /*  HASH Ͱ��                   */
/*********************************************************************************************************
  HASH ������
*********************************************************************************************************/
#if (HASH_SIZE & (HASH_SIZE - 1)) == 0
#define HASH_ENTRY(h)  ((h) & (HASH_SIZE - 1))
#else
#define HASH_ENTRY(h)  ((h) % HASH_SIZE)
#endif
/*********************************************************************************************************
  ARP ������Ϣ�ṹ��
*********************************************************************************************************/
struct as_devinfo {
    LW_LIST_LINE                    lineManag;                          /*  ��������                    */

    ip4_addr_t                      uiIpAddr;                           /*  IP��ַ                      */
    UCHAR                           ucTrueMacAddr[ETHARP_HWADDR_LEN];   /*  ��ȷMAC��ַ                 */
    UINT8                           ucTrueMacCounter;                   /*  ��ȷMAC�յ���ARPӦ�����    */
    UCHAR                           ucConMacAddr[ETHARP_HWADDR_LEN];    /*  ��ͻMAC��ַ                 */
    UINT8                           ucConMacCounter;                    /*  ��ͻMAC�յ���ARPӦ�����    */
    UINT8                           ucFlag;                             /*  ��ǰARP��Ϣ״̬��־         */
    PVOID                           pvNetif;                            /*  ���ڱ��� netif ���         */
    UINT32                          uiConflitCount;                     /*  MAC��ͻ����ֵ               */
    UINT64                          ullTimeOut;                         /*  ʱ���                      */
    UINT32                          uiTimeCount;                        /*  ��ʱ������ֵ(aliveʱ��)     */
};
typedef struct as_devinfo      __AS_DEVINFO;
typedef struct as_devinfo     *__PAS_DEVINFO;
/*********************************************************************************************************
  ����ȫ�ֱ�������
*********************************************************************************************************/
UINT8                _G_ucArpSpoofDefenseEn;                            /*  ����ʹ��                    */
LW_OBJECT_HANDLE     _G_ulAsDefenseTimer;                               /*  ��ʱ�����                  */
__ASD_CONFIG         _G_asdConfig;                                      /*  �����ļ�����                */
/*********************************************************************************************************
  ARP ��ƭ���� HASH ��
*********************************************************************************************************/
PVOID  _G_pvAsDefenseHashtable[HASH_SIZE];
/*********************************************************************************************************
  ������
*********************************************************************************************************/
static LW_OBJECT_HANDLE        _G_ulAsDefenseLock;
#define __ASDEFENSE_LOCK()     API_SemaphoreBPend(_G_ulAsDefenseLock, LW_OPTION_WAIT_INFINITE)
#define __ASDEFENSE_UNLOCK()   API_SemaphoreBPost(_G_ulAsDefenseLock)
/*********************************************************************************************************
  ��־ȫ�ֱ�������
*********************************************************************************************************/
INT32                _G_iLogFd;                                         /*  LOG �ļ����                */
CHAR                 _G_cLogTimeBuff[30] = {0};                         /*  LOG ��ʾ��ʱ��              */
INT32                _G_iFileLen;                                       /*  LOG �ļ�����                */
/*********************************************************************************************************
  LOG ����
*********************************************************************************************************/
#define FILE_MODE     (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)           /*  �ļ�����ģʽ                */
/*********************************************************************************************************
  �����ļ�
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_CFG "/etc/arpSpoofDefense.ini"                 /*  �����ļ�����                */
/*********************************************************************************************************
  LOG ���
*********************************************************************************************************/
#define __ASD_LOG(format, ...)                                                                 \
    do {                                                                                       \
        CHAR  cBuff[256] = {0};                                                                \
        __logTimeGen(_G_cLogTimeBuff);                                                         \
        snprintf(cBuff, sizeof(cBuff), "[%s] "format, _G_cLogTimeBuff, ##__VA_ARGS__);         \
        printk("%s", cBuff);                                                                   \
        __logInput(cBuff, strlen(cBuff));                                                      \
    } while (0)
/*********************************************************************************************************
** ��������: __logInit
** ��������: ��־��ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __logInit (VOID)
{
    _G_iLogFd = open(_G_asdConfig.pcLogFile, O_RDWR | O_CREAT, FILE_MODE);
    if (_G_iLogFd < 0) {
        printk("Create file failed. ret = %d\n", _G_iLogFd);
        _G_iLogFd = PX_ERROR;

        return  (PX_ERROR);
    }

    /*
     *  ƫ�Ƶ��ļ�ĩβ
     */
    _G_iFileLen = lseek(_G_iLogFd, 0, SEEK_END);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __logTimeGen
** ��������: ����һ�� log ��־��ʱ��
** �䡡��  : NONE
** �䡡��  : pcTimeBuf          ʱ���ַ���
** ������  : iTime              ʱ������ֵ
*********************************************************************************************************/
static time_t  __logTimeGen (PCHAR  pcTimeBuf)
{
    struct tm   logtm;
    time_t      iTime;

    bzero(&logtm, sizeof(logtm));
    iTime = time(NULL);

    lib_ctime_r(&iTime, pcTimeBuf);

    pcTimeBuf[strlen(pcTimeBuf) - 1] = 0;                               /*  ��Ҫɾ�����Ļ��з�        */

    return  (iTime);
}

/*********************************************************************************************************
** ��������: __logInput
** ��������: ��д��־
** �䡡��  : pcLogInfo          ��־����
**           uiLogLen           ��־����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32  __logInput (PCHAR  pcLogInfo, UINT32  uiLogLen)
{
    CHAR   cCmdString[1024] = {0};
    if  (LW_NULL == pcLogInfo) {
        return  (PX_ERROR);
    }

    /*
     *  �жϵ�ǰ�ļ��ĳ���
     */
    if (_G_asdConfig.iLogFilelen * 1024 * 1024  < _G_iFileLen + uiLogLen) {
        if(ERROR_NONE == access(_G_asdConfig.pcLogFileBk, F_OK)) {      /*  ���ڱ����ļ�                */
            snprintf(cCmdString, sizeof(cCmdString), "rm %s\n", _G_asdConfig.pcLogFileBk);
            system(cCmdString);
        }

        close(_G_iLogFd);                                               /*  �ȹرյ�ǰ�ļ�              */
        _G_iLogFd = PX_ERROR;

        bzero(cCmdString, sizeof(cCmdString));
        snprintf(cCmdString,
                 sizeof(cCmdString),
                 "mv %s %s\n",
                 _G_asdConfig.pcLogFile,
                 _G_asdConfig.pcLogFileBk);                             /*  ��ԭ�����ļ��ĳɱ���        */
        __logInit();                                                    /*  ���´���һ���µ�Log�ļ�     */
    }

    /*
     *  д��־
     */
    if (_G_iLogFd > 0) {
        write(_G_iLogFd, pcLogInfo, uiLogLen);
        fdatasync(_G_iLogFd);
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __asDefenseNodeAdd
** ��������: ����豸���
** �䡡��  : uiIpAddr           IP ��ַ
** �䡡��  : NONE
** ������  : PX_ERROR           ���ʧ��
**           ERROR_NONE         ��ӳɹ�
*********************************************************************************************************/
static INT32  __asDefenseNodeAdd (UINT32  uiIpAddr, PVOID  pvNode)
{
    UINT32            uiHashEntry = 0;

    if (LW_NULL == pvNode) {
        return  (PX_ERROR);
    }

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    _List_Line_Add_Ahead((PLW_LIST_LINE)pvNode,
                         (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[uiHashEntry]);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __asDefenseNodeDelete
** ��������: �豸���ɾ��
** �䡡��  : uiIpAddr           IP ��ַ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __asDefenseNodeDelete (UINT32  uiIpAddr)
{
    UINT32            uiHashEntry = 0;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    pLineTemp   = _G_pvAsDefenseHashtable[uiHashEntry];
    for (; LW_NULL != pLineTemp ; pLineTemp = _list_line_get_next(pLineTemp)) {
        if (((__PAS_DEVINFO)pLineTemp)->uiIpAddr.addr == uiIpAddr) {    /*  �ҵ���Ӧ�Ľ��              */
            _List_Line_Del(pLineTemp, (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[uiHashEntry]);
            free(pLineTemp);
            break;
        }
    }
}

/*********************************************************************************************************
** ��������: __asDefenseNodeFind
** ��������: �豸������
** �䡡��  : uiIpAddr           IP ��ַ
** �䡡��  : NONE
** ������  : pLineTemp          �ҵ��Ľ��
*********************************************************************************************************/
static PVOID  __asDefenseNodeFind (UINT32  uiIpAddr)
{
    UINT32            uiHashEntry = 0;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    uiHashEntry = HASH_ENTRY(uiIpAddr);
    pLineTemp   = _G_pvAsDefenseHashtable[uiHashEntry];
    for (; LW_NULL != pLineTemp; pLineTemp = _list_line_get_next(pLineTemp)) {
        if (((__PAS_DEVINFO)pLineTemp)->uiIpAddr.addr == uiIpAddr) {
            break;
        }
    }

    return  (PVOID)(pLineTemp);
}

/*********************************************************************************************************
** ��������: __asDefenseDeleteAll
** ��������: ɾ�����н��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __asDefenseDeleteAll (VOID)
{
    INT32             i;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;
    PLW_LIST_LINE     pLineNext   = LW_NULL;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i]; LW_NULL  != pLineTemp; ) {
            pLineNext    = _list_line_get_next(pLineTemp);
            _List_Line_Del(pLineTemp, (LW_LIST_LINE_HEADER *)&_G_pvAsDefenseHashtable[i]);
            free(pLineTemp);
            pLineTemp = pLineNext;
        }
    }

    bzero(_G_pvAsDefenseHashtable, sizeof(_G_pvAsDefenseHashtable));
    __ASDEFENSE_UNLOCK();
}


/*********************************************************************************************************
** ��������: __asDefenseCheckTimer
** ��������: ��ʱ����⺯��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __asDefenseCheckTimer (VOID)
{
    INT32             i;
    UINT32            uiTimerCount;
    ip4_addr_t        tIpaddr;
    __PAS_DEVINFO     pNodeInfo   = LW_NULL;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;
    PLW_LIST_LINE     pLineNext   = LW_NULL;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i]; LW_NULL  != pLineTemp; ) {
            pLineNext    = _list_line_get_next(pLineTemp);
            pNodeInfo    = (__PAS_DEVINFO)pLineTemp;
            uiTimerCount = ++pNodeInfo->uiTimeCount;
            if ((_G_asdConfig.iNodeDelTime / _G_asdConfig.iNodeCheckHz) <= uiTimerCount) {
                /*
                 *  ��Ҫ���˽��ɾ��
                 */
                __asDefenseNodeDelete(pNodeInfo->uiIpAddr.addr);        /*  ɾ�����н��                */
            } else if (ARP_ATTACK_FLAG == pNodeInfo->ucFlag &&
                       API_TimeGet64() - pNodeInfo->ullTimeOut > _G_asdConfig.iForbidTime * LW_TICK_HZ) {
                /*
                 *  ���������IP������ʱ��������򽫱�־����Ϊ conflict
                 */
                pNodeInfo->ucConMacCounter = 0;
                pNodeInfo->ucTrueMacCounter = 0;
                pNodeInfo->ullTimeOut      = 0;
                pNodeInfo->ucFlag          = ARP_CONFLICT_FLAG;

                tIpaddr.addr = htonl(pNodeInfo->uiIpAddr.addr);
                etharp_request(pNodeInfo->pvNetif, &tIpaddr);           /*  ����̽�����绷��            */
            }

            pLineTemp = pLineNext;
        }
    }
    __ASDEFENSE_UNLOCK();
}

/*********************************************************************************************************
** ��������: __asDefenseShow
** ��������: ��Ϣ��ʾ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static INT32  __asDefenseShow (INT32  iArgC, PCHAR  ppcArgV[])
{
    INT32             i;
    struct in_addr    uiIpAddr;
    __PAS_DEVINFO     pasDevInfo  = LW_NULL;
    PLW_LIST_LINE     pLineTemp   = LW_NULL;

    fprintf(stdout, "Arp Spoof Defense Info >>\n");
    fprintf(stdout, "    IP                TRUE MAC         CONFLICT MAC         STATUS       ALIAVE\n");
    fprintf(stdout, "--------------- ------------------- ------------------- ------------ ------------\n");

    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL != pLineTemp;
             pLineTemp  = _list_line_get_next(pLineTemp)) {
            pasDevInfo      = (__PAS_DEVINFO)pLineTemp;
            uiIpAddr.s_addr = htonl(pasDevInfo->uiIpAddr.addr);
            fprintf(stdout,
                    "%-15s  %02x:%02x:%02x:%02x:%02x:%02x   %02x:%02x:%02x:%02x:%02x:%02x%11s%11d\n",
                    inet_ntoa(uiIpAddr),
                    pasDevInfo->ucTrueMacAddr[0],
                    pasDevInfo->ucTrueMacAddr[1],
                    pasDevInfo->ucTrueMacAddr[2],
                    pasDevInfo->ucTrueMacAddr[3],
                    pasDevInfo->ucTrueMacAddr[4],
                    pasDevInfo->ucTrueMacAddr[5],
                    pasDevInfo->ucConMacAddr[0],
                    pasDevInfo->ucConMacAddr[1],
                    pasDevInfo->ucConMacAddr[2],
                    pasDevInfo->ucConMacAddr[3],
                    pasDevInfo->ucConMacAddr[4],
                    pasDevInfo->ucConMacAddr[5],
                    (ARP_NORMAL_FLAG == pasDevInfo->ucFlag) ? ARP_NORMAL_STR :
                    ((ARP_CONFLICT_FLAG == pasDevInfo->ucFlag) ? ARP_CONFLICT_STR : ARP_ATTACK_STR),
                    pasDevInfo->uiTimeCount);
        }
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __asDefensePacketCheck
** ��������: ARP ��ƭ���
** �䡡��  : pcPacket           ����
**           pNetdev            �����ṹ
** �䡡��  : NONE
** ������  : PX_ERROR           ��������
**           LW_TRUE            ��������ͨ��
**           LW_FALSE           ���Ĳ�����ͨ��
*********************************************************************************************************/
static INT32  __asDefensePacketCheck (UINT8  *pcPacket, struct netdev  *pNetdev)
{
    ip4_addr_t             tIpaddr;
    struct netif          *pNetif;
    UINT8                  ucArpPass    = LW_FALSE;
    UINT32                 uiSrcIpAddr, uiDesIpAddr;
    INT                    iTmpVal      = 0;

    __PAS_DEVINFO          pArpInfo = LW_NULL;
    struct etharp_hdr     *pArphdr  = LW_NULL;

    if (LW_NULL == pcPacket || LW_NULL == pNetdev) {
        return  (LW_TRUE);
    }

    pArphdr = (struct etharp_hdr *)(pcPacket + sizeof(struct eth_hdr));
    pNetif  = (struct netif *)&(pNetdev->sys[0]);

    /*
     *  ǰ��16λ���32λIP��ַ
     */
    uiSrcIpAddr  = (ntohs(pArphdr->sipaddr.addrw[0]) << 16) | ntohs(pArphdr->sipaddr.addrw[1]);
    uiDesIpAddr  = (ntohs(pArphdr->dipaddr.addrw[0]) << 16) | ntohs(pArphdr->dipaddr.addrw[1]);

    /*
     *  �Ը��Լ���ARP����������
     */
    if ((ARP_REPLY   == ntohs(pArphdr->opcode))                   ||
        (uiDesIpAddr == (ntohl(pNetif->ip_addr.u_addr.ip4.addr))  &&
        (ARP_REQUEST == ntohs(pArphdr->opcode)))) {
        __ASDEFENSE_LOCK();
        pArpInfo = __asDefenseNodeFind(uiSrcIpAddr);
        if (LW_NULL == pArpInfo) {                                      /*  �Ҳ�����Ӧ���豸��Ϣ        */
            /*
             *  ����һ���µĽ�㣬���ӽ���������
             */
            pArpInfo = (__PAS_DEVINFO)malloc(sizeof(__AS_DEVINFO));
            if (LW_NULL == pArpInfo) {                                  /*  �����ڴ�ʧ��                */
                __ASDEFENSE_UNLOCK();

                return  (LW_TRUE);                                      /*  ��������ͨ��,Э��ջ����     */
            }

            /*
             *  ��һ����������Ϊ��ͻ״̬,�Ӷ����Խ����жϵ�ǰ���豸���������绷��
             */
            bzero(pArpInfo, sizeof(__AS_DEVINFO));
            pArpInfo->uiIpAddr.addr   = uiSrcIpAddr;                    /*  ����ip��ַ                  */
            pArpInfo->ucFlag          = ARP_CONFLICT_FLAG;
            memcpy(pArpInfo->ucConMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN);
            memcpy(pArpInfo->ucTrueMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN);

            __asDefenseNodeAdd(uiSrcIpAddr, pArpInfo);

        } else {                                                        /*  ���ڵ�ǰ�豸��Ϣ            */
            pArpInfo->uiTimeCount = 0;
            if (ARP_ATTACK_FLAG == pArpInfo->ucFlag) {
                goto  __back;
            }

            /*
             *  ����MAC ���� True_mac
             */
            if (!memcmp(pArpInfo->ucTrueMacAddr, pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN)) {
                pArpInfo->ucTrueMacCounter++;
                if (ARP_NORMAL_FLAG == pArpInfo->ucFlag) {
                    ucArpPass = 1;                                      /*  ��������ͨ��                */
                }
            } else {
                if (!memcmp(pArpInfo->ucConMacAddr,pArphdr->shwaddr.addr, ETHARP_HWADDR_LEN)) {
                    /*
                     *   ����MAC ���� new_mac
                     */
                    pArpInfo->ucConMacCounter++;
                } else {                                                /*  �뵱ǰ�µ�MACҲ�����       */
                    memcpy(pArpInfo->ucConMacAddr,
                           pArphdr->shwaddr.addr,
                           ETHARP_HWADDR_LEN);                          /*  ����New MAC����             */
                    pArpInfo->ucConMacCounter = 1;
                }
            }
        }

        if (ARP_NORMAL_FLAG == pArpInfo->ucFlag) {
            if (pArpInfo->ucConMacCounter) {
                pArpInfo->ucFlag          = ARP_CONFLICT_FLAG;          /*  ������ͻ                    */
                pArpInfo->ucConMacCounter = 0;
                pArpInfo->ucTrueMacCounter = 0;
                if (pArpInfo->ullTimeOut &&
                    ((API_TimeGet64() - pArpInfo->ullTimeOut) <=
                      _G_asdConfig.iConToSpoTime * LW_TICK_HZ)) {
                    /*
                     *  ���γ�ͻ��ʱ��ܿ���������Ϊ�Ǳ�����
                     *  ����������Ϊ��ƭ�ļ���ֵ������һ�����ľͻᱻ����Ϊ��ƭ״̬
                     */
                    pArpInfo->ucConMacCounter = _G_asdConfig.iArpSpoofVal;
                    pArpInfo->ucTrueMacCounter = _G_asdConfig.iArpSpoofVal;
                } else {
                    pArpInfo->ullTimeOut = API_TimeGet64();
                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                    __ASD_LOG("warning: a device(%s) get a different MAC. "
                              "the different mac is %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                              inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                              pArpInfo->ucConMacAddr[0],
                              pArpInfo->ucConMacAddr[1],
                              pArpInfo->ucConMacAddr[2],
                              pArpInfo->ucConMacAddr[3],
                              pArpInfo->ucConMacAddr[4],
                              pArpInfo->ucConMacAddr[5]);
                }
            } else {
                pArpInfo->ucTrueMacCounter = 1;                          /*  ������ old ����һֱ����Ϊ1  */
            }
        } else {                                                        /*  ����ǳ�ͻ���              */
            if (!pArpInfo->ucTrueMacCounter) {
                /*
                 *  ��Ϊ MAC �����˱仯
                 */
                if (_G_asdConfig.iMacChangeVal <= pArpInfo->ucConMacCounter) {
                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                    __ASD_LOG("the device(%s) mac has changed from %02x:%02x:%02x:%02x:%02x:%02x to "
                              "%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                              inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                              pArpInfo->ucTrueMacAddr[0],
                              pArpInfo->ucTrueMacAddr[1],
                              pArpInfo->ucTrueMacAddr[2],
                              pArpInfo->ucTrueMacAddr[3],
                              pArpInfo->ucTrueMacAddr[4],
                              pArpInfo->ucTrueMacAddr[5],
                              pArpInfo->ucConMacAddr[0],
                              pArpInfo->ucConMacAddr[1],
                              pArpInfo->ucConMacAddr[2],
                              pArpInfo->ucConMacAddr[3],
                              pArpInfo->ucConMacAddr[4],
                              pArpInfo->ucConMacAddr[5]);
                    memcpy(pArpInfo->ucTrueMacAddr,
                           pArpInfo->ucConMacAddr,
                           sizeof(pArpInfo->ucTrueMacAddr));
                    bzero(pArpInfo->ucConMacAddr, sizeof(pArpInfo->ucConMacAddr));
                    pArpInfo->ucFlag          = ARP_NORMAL_FLAG;        /*  �ָ�����                    */
                    pArpInfo->ucConMacCounter = 0;
                    pArpInfo->ucTrueMacCounter = 0;

                    tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                }
            } else {
                if (!pArpInfo->ucConMacCounter) {
                    if (_G_asdConfig.iMacChangeVal <= pArpInfo->ucTrueMacCounter) {
                        bzero(pArpInfo->ucConMacAddr, sizeof(pArpInfo->ucConMacAddr));
                        pArpInfo->ucFlag           = ARP_NORMAL_FLAG;
                        pArpInfo->ucConMacCounter  = 0;
                        pArpInfo->ucTrueMacCounter = 0;
                    }
                } else {
                    iTmpVal = abs(pArpInfo->ucTrueMacCounter - pArpInfo->ucConMacCounter);
                    if ((_G_asdConfig.iArpSpoofVal <= pArpInfo->ucConMacCounter  &&
                         _G_asdConfig.iArpSpoofVal <= pArpInfo->ucTrueMacCounter) ||
                         _G_asdConfig.iArpSpoofVal <= iTmpVal) {        /*  ��Ϊ���ܵ�����              */
                        pArpInfo->ucFlag          = ARP_ATTACK_FLAG;
                        pArpInfo->pvNetif         = (PVOID)pNetif;
                        pArpInfo->ucConMacCounter = 0;
                        pArpInfo->ucTrueMacCounter = 0;
                        pArpInfo->ullTimeOut      = API_TimeGet64();
                        /*
                         *  Ϊ�˷�����ǰ��ϵͳ ARP �������Ŀ�Ǵ������Ϣ��
                         *  ����ɾ�� ARP ���ȫ�����ݣ��˺󣬴�����ƭ���յ�
                         *  ARP ��Ϣ�������ϵͳ ARP ��
                         */
                        system("arp -d");
                        tIpaddr.addr = htonl(pArpInfo->uiIpAddr.addr);
                        __ASD_LOG("wraning: device(%s) may be spoofed, "
                                  "the first mac is %02x:%02x:%02x:%02x:%02x:%02x, "
                                  "another mac is %02x:%02x:%02x:%02x:%02x:%02x. "
                                  "The device is blacked out.\r\n",
                                  inet_ntoa(*(struct in_addr *)&(tIpaddr)),
                                  pArpInfo->ucTrueMacAddr[0],
                                  pArpInfo->ucTrueMacAddr[1],
                                  pArpInfo->ucTrueMacAddr[2],
                                  pArpInfo->ucTrueMacAddr[3],
                                  pArpInfo->ucTrueMacAddr[4],
                                  pArpInfo->ucTrueMacAddr[5],
                                  pArpInfo->ucConMacAddr[0],
                                  pArpInfo->ucConMacAddr[1],
                                  pArpInfo->ucConMacAddr[2],
                                  pArpInfo->ucConMacAddr[3],
                                  pArpInfo->ucConMacAddr[4],
                                  pArpInfo->ucConMacAddr[5]);
                    }
                }
            }
        }

        if (ARP_CONFLICT_FLAG == pArpInfo->ucFlag) {
            tIpaddr.addr = htonl(uiSrcIpAddr);
            etharp_request(pNetif, &tIpaddr);                           /*  �ٴη���ARP����             */
        }

__back:
        __ASDEFENSE_UNLOCK();

        /*
         *  �����������״̬�������յ��ĵ�һ��ARP���ģ���������ͨ��
         */
        return  (ucArpPass ? LW_TRUE : LW_FALSE);
    }

    return  (LW_TRUE);
}

/*********************************************************************************************************
** ��������: __asDefenseEntry
** ��������: arp �������
** �䡡��  : inp                �����ṹ
**           pPbuf              ��������
** �䡡��  : �����Ƿ�eat
** ������  : PX_ERROR           ���Ĳ�����ͨ��
**           ERROR_NONE         ���Ŀ���ͨ��
*********************************************************************************************************/
static INT32 __asDefenseEntry (struct netdev  *inp, struct pbuf *pPbuf)
{
    INT                 iOffset      = 0;                               /*  ���ݵ���ʼƫ����            */
    UINT8              *pucDrvPacket = LW_NULL;                         /*  ��������                    */
    struct eth_hdr     *pEthhdr      = LW_NULL;                         /*  eth ͷ                      */

    if (ARP_SPOOF_DEFENSE_DISABLE == _G_ucArpSpoofDefenseEn) {
        goto __allow_input;                                             /*  û�п����κβ�����������ȷ  */
    }

    if (LW_NULL == pPbuf || pPbuf->tot_len < sizeof(struct eth_hdr)) {
        goto __allow_input;                                             /*  Э��ջ����                  */
    }

    pucDrvPacket = (UINT8 *)(pPbuf->payload);                           /*  ��ȡ��������                */

    /*
     *  ethernet ���˴���
     */
    if (inp->net_type & NETDEV_TYPE_ETHERNET) {                         /*  ��̫���ӿ�                  */
        pEthhdr = (struct eth_hdr *)(pucDrvPacket + iOffset);
        iOffset = sizeof(struct eth_hdr);
    }

    if (iOffset == sizeof(struct eth_hdr)) {                            /*  ǰ���� eth hdr �ֶ�         */
        /*
         *  todo: ��VLAN������Ҫ��һ���жϣ���ϸ�жϲο�npf
         */
        if (pEthhdr->type == PP_HTONS(ETHTYPE_ARP)) {
            if (LW_FALSE == __asDefensePacketCheck(pucDrvPacket, inp)) {
                goto __drop_input;
            }
        } else {
            goto    __allow_input;                                      /*  ��������, ����              */
        }
    } else {
        goto    __allow_input;                                          /*  û����̫���ֶ�, ����        */
    }

__allow_input:
    return  (0);                                                        /*  ��������                    */

__drop_input:
    netdev_linkinfo_drop_inc(inp);
    pbuf_free(pPbuf);                                                   /*  �ͷ����ݰ�                  */
    return  (1);                                                        /*  eaten                       */
}

/*********************************************************************************************************
** ��������: __asDefenseInstall
** ��������: ��װ ARP ��ƭ����
** �䡡��  : inp                �����ṹ
**           ucDrvPacket        �����յ��ı���
**           usPacketLen        ���ĳ���
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32 __asDefenseInstall (struct netdev *pNetdev,
                                 PVOID arg0, PVOID arg1, PVOID arg2,
                                 PVOID arg3, PVOID arg4, PVOID arg5)
{
    netdev_firewall(pNetdev, __asDefenseEntry);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __asDefenseUnInstall
** ��������: ж��װ ARP ��ƭ����
** �䡡��  : inp                �����ṹ
**           ucDrvPacket        �����յ��ı���
**           usPacketLen        ���ĳ���
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT32 __asDefenseUnInstall (struct netdev *pNetdev,
                                   PVOID arg0, PVOID arg1, PVOID arg2,
                                   PVOID arg3, PVOID arg4, PVOID arg5)
{
    /*
     *  todo:���￼���Ƿ�Ҫ�ָ���֮ǰ���õĽӿ�
     */
    netdev_firewall(pNetdev, LW_NULL);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __fireWallInstall
** ��������: ��װ����ǽ�ӿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
static VOID __fireWallInstall (VOID)
{
    netdev_foreache(__asDefenseInstall, 0, 0, 0, 0, 0, 0);              /*  ����������ӷ���ǽ����      */
}

/*********************************************************************************************************
** ��������: __fireWallUninstall
** ��������: ж�ط���ǽ�ӿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
static VOID __fireWallUninstall (VOID)
{
    netdev_foreache(__asDefenseUnInstall, 0, 0, 0, 0, 0, 0);            /*  ��������ж�ط���ǽ����      */
}

/*********************************************************************************************************
** ��������: __asDefenseEn
** ��������: ���� / �ر� ARP ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static INT32  __asDefenseEn (INT32  iArgC, PCHAR  ppcArgV[])
{
    if (2 != iArgC) {
        printk("\nVersion: %s\n", ARP_SPOOF_DEFENSE_VERSION);
        printk("Usage: asden 1 (enable arp spoof defense)\n");
        printk("       asden 0 (disable arp spoof defense)\n");
        printk("       asdshow (show arp spoof defense info)\n");

        return  (ERROR_NONE);
    }

    if (atoi(ppcArgV[1])) {
        _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_ENABLE;
        __fireWallInstall();                                            /*  ��װ�������ǽ              */

        __ASD_LOG("arp spoof defense start.\r\n");
    } else {
        _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;
        __fireWallUninstall();
        __asDefenseDeleteAll();

        __ASD_LOG("arp spoof defense stop.\r\n");
    }

    system("arp -d");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __iniCfgInit
** ��������: �����ļ���ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
static INT32  __iniCfgInit (VOID)
{
    PVOID    pCfg = LW_NULL;
    PVOID    pLog = LW_NULL;

    bzero(&_G_asdConfig, sizeof(_G_asdConfig));
    pCfg = asdCfgLoad(ARP_SPOOF_DEFENSE_CFG, CFG_SECTOR);
    if (!pCfg) {
        printk("[arpSpoofDefense] Load INI file Failed.\n");

        return  (PX_ERROR);
    }

    pLog = asdCfgLoad(ARP_SPOOF_DEFENSE_CFG, LOG_SECTOR);
    if (!pCfg) {
        printk("[arpSpoofDefense] Load INI file Failed.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iMacChangeVal = asdCfgGetInt(pCfg, MAC_CHANGE_VAL, -1);
    if (_G_asdConfig.iMacChangeVal < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found mac_change_val setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iArpSpoofVal = asdCfgGetInt(pCfg, ARP_SPOOF_VAL, -1);
    if (_G_asdConfig.iArpSpoofVal < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found arp_spoof_val setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iConToSpoTime = asdCfgGetInt(pCfg, CON_TO_SPO_TIME, -1);
    if (_G_asdConfig.iConToSpoTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found conflict_to_spoof_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iForbidTime = asdCfgGetInt(pCfg, SPOOF_FORBID_TIME, -1);
    if (_G_asdConfig.iForbidTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found spoof_forbid_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iNodeDelTime = asdCfgGetInt(pCfg, NODE_DEL_TIME, -1);
    if (_G_asdConfig.iNodeDelTime < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found node_del_time setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.iNodeCheckHz = asdCfgGetInt(pCfg, NODE_CHECK_HZ, -1);
    if (_G_asdConfig.iNodeCheckHz < 0) {
        asdCfgUnload(pCfg);
        printk("[arpSpoofDefense] Can't found node_check_hz setting.\n");

        return  (PX_ERROR);
    }

    asdCfgUnload(pCfg);
    _G_asdConfig.iLogFilelen = asdCfgGetInt(pLog, LOG_FILE_LEN_INI, -1);
    if (_G_asdConfig.iLogFilelen < 0) {
        asdCfgUnload(pLog);
        printk("[arpSpoofDefense] Can't found logfile_len setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.pcLogFile = (PCHAR)malloc(LOG_FILE_NAME_LEN);
    if (!_G_asdConfig.pcLogFile) {
        return  (PX_ERROR);
    }
    _G_asdConfig.pcLogFile = asdCfgGetString(pLog, LOG_FILE_INI, NULL);
    if (!_G_asdConfig.pcLogFile) {
        asdCfgUnload(pLog);
        free(_G_asdConfig.pcLogFile);
        printk("[arpSpoofDefense] Can't found logfile setting.\n");

        return  (PX_ERROR);
    }

    _G_asdConfig.pcLogFileBk = (PCHAR)malloc(LOG_FILE_NAME_LEN);
    if (!_G_asdConfig.pcLogFileBk) {
        free(_G_asdConfig.pcLogFile);

        return  (PX_ERROR);
    }
    _G_asdConfig.pcLogFileBk = asdCfgGetString(pLog, LOG_FILE_BK_INI, NULL);
    if (!_G_asdConfig.pcLogFileBk) {
        asdCfgUnload(pLog);
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);
        printk("[arpSpoofDefense] Can't found logfile setting.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: arpSpoofDefenseInit
** ��������: ARP ��ƭ������ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT32  arpSpoofDefenseInit (VOID)
{
    if (__iniCfgInit() != ERROR_NONE) {
        return  (PX_ERROR);
    }

    if (__logInit() != ERROR_NONE) {
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);

        return  (PX_ERROR);
    }

    /*
     *  ��Դ��ʼ��
     */
    bzero(_G_pvAsDefenseHashtable, sizeof(_G_pvAsDefenseHashtable));
    _G_ulAsDefenseLock = API_SemaphoreBCreate("sem_arpdefenselcok",
                                              LW_TRUE,
                                              LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL,
                                              LW_NULL);                 /*  ������                      */
    if (LW_OBJECT_HANDLE_INVALID == _G_ulAsDefenseLock) {
        fprintf(stdout, "asdefense: create lock error.\r\n");
        close(_G_iLogFd);
        free(_G_asdConfig.pcLogFile);
        free(_G_asdConfig.pcLogFileBk);

        return  (PX_ERROR);
    }

    _G_ulAsDefenseTimer = API_TimerCreate("asdefense_timer",
                                       LW_OPTION_ITIMER, LW_NULL);
    API_TimerStart(_G_ulAsDefenseTimer,
                   (LW_TICK_HZ * _G_asdConfig.iNodeCheckHz),
                   LW_OPTION_AUTO_RESTART,
                   (PTIMER_CALLBACK_ROUTINE)__asDefenseCheckTimer,
                   LW_NULL);                                            /*  ������ⶨʱ��              */

    _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;                 /*  Ĭ�Ϲر� ARP ����           */

    /*
     *  ע�� shell ����
     */
    API_TShellKeywordAdd("asdshow", __asDefenseShow);
    API_TShellHelpAdd("asdshow", "show arp spoof defense info.\n");

    API_TShellKeywordAdd("asden", __asDefenseEn);
    API_TShellHelpAdd("asden", "show arp spoof defense info.\n");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: module_init
** ��������: ARP ��ƭ����ģ�����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROE_CODE
*********************************************************************************************************/
INT32  module_init (void)
{
    INT32   iRet;
    iRet = arpSpoofDefenseInit();

    if (ERROR_NONE == iRet) {
        printk("arp spoof defense module init success.\r\n");
    } else {
        printk("arp spoof defense module init failed.\r\n");
    }

    return  (iRet);
}

/*********************************************************************************************************
** ��������: module_exit
** ��������: ARP ��ƭ����ģ��ж��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
void module_exit (void)
{
    _G_ucArpSpoofDefenseEn = ARP_SPOOF_DEFENSE_DISABLE;
    __fireWallUninstall();
    __asDefenseDeleteAll();

    API_SemaphoreBDelete(&_G_ulAsDefenseLock);
    API_TimerDelete(&_G_ulAsDefenseTimer);

    __ASD_LOG("arp spoof defense module exit.\r\n");
    if (_G_iLogFd > 0) {
        close(_G_iLogFd);
    }
}

/*********************************************************************************************************
** ��������: asDefenseStatus
** ��������: ARP ��ƭ����������ر�״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : _G_ucArpSpoofDefenseEn
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32 asDefenseStatus (VOID)
{
    return  (_G_ucArpSpoofDefenseEn);
}

/*********************************************************************************************************
** ��������: asDefenseGetSpecInfoNum
** ��������: ��ȡ ARP ��ƭ����ָ����Ϣ����
** �䡡��  : ucInfoType         ��Ϣ���� (ȫ����Ϣ / ��������Ϣ)
** �䡡��  : NONE
** ������  : uiInfoNum          ��Ϣ����
*********************************************************************************************************/
LW_SYMBOL_EXPORT UINT32 asDefenseGetSpecInfoNum (UINT8  ucInfoType)
{
    UINT32            uiInfoNum  = 0;
    PLW_LIST_LINE     pLineTemp  = LW_NULL;
    INT32             i;

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL  != pLineTemp;
             pLineTemp = _list_line_get_next(pLineTemp)) {
            if (ASDEFENSE_GET_ALL == ucInfoType) {
                uiInfoNum++;
            } else if (ASDEFENSE_GET_SPEC == ucInfoType &&
                       (ARP_NORMAL_FLAG != ((__PAS_DEVINFO)pLineTemp)->ucFlag)) {
                uiInfoNum++;
            }
        }
    }
    __ASDEFENSE_UNLOCK();
    return  (uiInfoNum);
}

/*********************************************************************************************************
** ��������: asDefenseGetSpecInfo
** ��������: ��ȡ ARP ��ƭ����ָ����Ϣ
** �䡡��  : iStartIndex        ��ʼ��Ŀ��
**           iEndIndex          ������Ŀ��
**           ucInfoType         ����
**           pvLogBuff          buff ��ַ
**           iBuffLen           buff ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32 asDefenseGetSpecInfo (INT32  iStartIndex,
                                             INT32  iEndIndex,
                                             UINT8  ucInfoType,
                                             PCHAR  pcBuff,
                                             INT32  iBuffLen)
{
    INT32                 i;
    UINT32                uiInfoNum     = 0;
    PCHAR                 pcBuffTmp     = pcBuff;
    __PAS_DEVINFO         pAsMangerInfo = LW_NULL;
    __PASDEFENSE_INFO     pAsNodeInfo   = LW_NULL;
    PLW_LIST_LINE         pLineTemp     = LW_NULL;

    if (LW_NULL == pcBuff) {
        return  (PX_ERROR);
    }

    __ASDEFENSE_LOCK();
    for (i = 0; i < HASH_SIZE; i++) {
        for (pLineTemp = _G_pvAsDefenseHashtable[i];
             LW_NULL  != pLineTemp;
             pLineTemp = _list_line_get_next(pLineTemp)) {
            /*
             *  ���ҵ���Ӧ���͵Ľ��
             */
            if (ASDEFENSE_GET_ALL == ucInfoType) {
                uiInfoNum++;
            } else if (ASDEFENSE_GET_SPEC == ucInfoType &&
                       (ARP_NORMAL_FLAG != ((__PAS_DEVINFO)pLineTemp)->ucFlag)) {
                uiInfoNum++;
            } else {
                continue;
            }

            if (uiInfoNum >= iStartIndex && uiInfoNum <= iEndIndex) {   /*  ��Ҫ���н����Ϣ�Ļ�ȡ      */
                pAsMangerInfo = (__PAS_DEVINFO)pLineTemp;
                pAsNodeInfo   = (__PASDEFENSE_INFO)pcBuff;
                if (pcBuff - pcBuffTmp > iBuffLen) {                    /*  buff ���                   */
                    __ASDEFENSE_UNLOCK();

                    return  (PX_ERROR);
                }

                /*
                 *  ��ȡ��Ϣ
                 */
                pAsNodeInfo->uiIpAddr.addr = pAsMangerInfo->uiIpAddr.addr;
                pAsNodeInfo->ucFlag        = pAsMangerInfo->ucFlag;
                memcpy(pAsNodeInfo->ucTrueMacAddr,
                       pAsMangerInfo->ucTrueMacAddr,
                       sizeof(pAsNodeInfo->ucTrueMacAddr));
                memcpy(pAsNodeInfo->ucConMacAddr,
                       pAsMangerInfo->ucConMacAddr,
                       sizeof(pAsNodeInfo->ucConMacAddr));

                pcBuff += sizeof(__ASDEFENSE_INFO);
            }
        }
    }

    __ASDEFENSE_UNLOCK();

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
