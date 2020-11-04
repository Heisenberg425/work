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
** ��   ��   ��: dhcpserveraction.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: DHCP SERVER ���ú�̨�ӿ�
** 2018.09.20    DHCP SERVER ֧���ڶ������豸���ж��������
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "lwip/netif.h"
#include "intada_info.h"
#include "dhcpserver_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  ���ص�״̬��                */
#define DEFAULT_FAILED_CJSON       "{\n\t\"code\":\t500\n}"             /*  json�ṹ����ʧ��ʱʹ��      */
#define JSON_ARGERR_CODE           (100)                                /*  �������󷵻�Json codeֵ     */
/*********************************************************************************************************
  Jason key: DHCP SERVER ������Ϣ��ʾ
*********************************************************************************************************/
#define WEB_DHCPSERVER_ENABLE     "enable"                              /*  json�ֶΣ�ʹ��              */
#define WEB_DHCPSERVER_START      "start"                               /*  json�ֶΣ���ַ����ʼ��ַ    */
#define WEB_DHCPSERVER_END        "end"                                 /*  json�ֶΣ���ַ�ؽ�����ַ    */
#define WEB_DHCPSERVER_INTERFACE  "interface"                           /*  json�ֶΣ�������          */
#define WEB_DHCPSERVER_DNS1       "dns1"                                /*  json�ֶΣ�dns1              */
#define WEB_DHCPSERVER_DNS2       "dns2"                                /*  json�ֶΣ�dns2              */
#define WEB_DHCPSERVER_MASK       "mask"                                /*  json�ֶΣ�mask              */
#define WEB_DHCPSERVER_GATEWAY    "gateway"                             /*  json�ֶΣ�����              */
#define WEB_DHCPSERVER_LEASE      "lease"                               /*  json�ֶΣ���Լʱ��          */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                  "page"                                /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                 "total"                               /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                "data"                                /*  Json�ֶΣ������˿�          */
/*********************************************************************************************************
  DHCP ������غ궨��
*********************************************************************************************************/
#define IPSTART                   "start"                               /*  ��ַ����ʼ��ַ��ֵƥ���ַ���*/
#define IPEND                     "end"                                 /*  ��ַ�ؽ�����ַ��ֵƥ���ַ���*/
#define DNSOPT1                   "dns1"                                /*  dns1 ��ֵƥ���ַ���         */
#define DNSOPT2                   "dns2"                                /*  dns2 ��ֵƥ���ַ���         */
#define SUBNETOPT                 "subnet"                              /*  ���������ֵƥ���ַ���      */
#define ROUTEROPT                 "router"                              /*  Ĭ�����ؼ�ֵƥ���ַ���      */
#define LEASEOPT                  "lease"                               /*  ��Լʱ���ֵƥ���ַ���      */
#define DNS_DEFAULT               "8.8.8.8"                             /*  Ĭ�� DNS                    */

#define ISNOTIP                   (0)                                   /*  �� IP                       */
#define IPVAILD                   (1)                                   /*  IP ֵ����                   */
#define IPOK                      (2)                                   /*  IP ֵ��ȷ                   */

#define addr_cmp(addr1, addr2, mask) \
                                  ((addr1 & mask) == (addr2 & mask))    /*  �жϵ�ַ�Ƿ���ͬһ����      */
#define SIGMASK                   (0xffffff00)                          /*  255.255.255.0 ����          */
/*********************************************************************************************************
** ��������: __isIpValid
** ��������: �ж� IP ��ַ��Ч��
** ��    ��: cpcIp  ip�ַ���ָ��
** ��    ��: NONE
** ��    ��: ���� IP ��ַ����ISNOTIP
                  IP ��ַ��Ч���� IPVAILD
                  IP ��ַ�������� IPOK
*********************************************************************************************************/
static INT  __isIpValid (CPCHAR  cpcIp)
{
    INT  i         = 0;
    INT  j         = 0;
    INT  iIpSeg[4] = {0, 0, 0, 0};
    INT  iLen      = strlen(cpcIp);
    INT  iTmp      = 0;

    for (i = 0; i < iLen + 1; i++) {
        if (cpcIp[i] == '.' || i == iLen) {                             /*  ���� IP ��ַ�����          */
            if (iIpSeg[j] >= 0 && iIpSeg[j] <= 255) {                   /*  ���� IP ��ַ��Χ            */
                j ++;
            } else {
                return  (ISNOTIP);                                      /*  ���� IP ��ֵ��Χ���� ISNOTIP*/
            }
        } else {
            iTmp = cpcIp[i] - '0';                                      /*  IP �ַ���תΪ����           */
            if (iTmp > 9 || iTmp < 0) {
                return  (ISNOTIP);                                      /*  ����ֵ�ַ������� ISNOTIP    */
            } else {
                iIpSeg[j] = iIpSeg[j] * 10 + iTmp;                      /*  ����ֵ����                  */
            }
        }
    }

    if (4 != j) {                                                       /*  �ж� IP ��ַ�����Ƿ�����    */
        return  (ISNOTIP);
    }

    if (iIpSeg[0] > 0 && iIpSeg[0] < 255 &&                             /*  �ж� IP ��Ч��              */
        iIpSeg[3] > 0 && iIpSeg[3] < 255 ) {
        return  (IPOK);                                                 /*  ��Ч���� IPOK               */
    } else if (0 == cpcIp[0]) {
        return  (ISNOTIP);                                              /*  �� IP ���� ISNOTIP          */
    }

    return  (IPVAILD);
}
/*********************************************************************************************************
** ��������: __isMaskValid
** ��������: �ж�����������Ч��
** ��    ��: cpcMask   ���������ַ���ָ��
** ��    ��: NONE
** ��    ��: ERROR_CODE
*********************************************************************************************************/
static INT  __isMaskValid (CPCHAR  cpcMask)
{
    UINT  uiIpVal  = 0;
    UINT  uiTmp[4] = {0, 0, 0, 0};
    INT   i        = 0;

    if (IPVAILD == __isIpValid(cpcMask)) {
        sscanf(cpcMask,
               "%u.%u.%u.%u",
               &uiTmp[3],
               &uiTmp[2],
               &uiTmp[1],
               &uiTmp[0]);

        for (i = 0; i < 4; i++) {                                       /*  ��������-->32 λ�޷�������  */
            uiIpVal += uiTmp[i] << (i * 8);
        }

        uiIpVal = ~uiIpVal + 1;                                         /*  �ж������Ƿ�Ϊ 2 ��������   */
        if ((uiIpVal & (uiIpVal - 1)) == 0) {
            return  (ERROR_NONE);
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __dhcpOptArgCheck
** ��������: ѡ��������
** ��    ��: cpcType               ѡ����������
**           cpcStrValue           �ַ���ֵ
**           uiIntValue            ����ֵ
** ��    ��: NONE
** ��    ��: ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpOptArgCheck (CPCHAR  cpcType, CPCHAR  cpcStrValue, UINT  uiIntValue)
{
    struct in_addr   inaddr;

    if (0 == strcmp(cpcType, DNSOPT1)) {                                /*  �ж��Ƿ��� dns1 ѡ��        */
        if (!inet_aton(cpcStrValue, &inaddr)) {                         /*  ת���ַ���Ϊ in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, DNSOPT2)) {                         /*  �ж��Ƿ��� dns2 ѡ��        */
        if (!inet_aton(cpcStrValue, &inaddr)) {                         /*  ת���ַ���Ϊ in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, SUBNETOPT)) {                       /*  �ж��Ƿ�����������ѡ��      */
        if (PX_ERROR == __isMaskValid(cpcStrValue) ||
           !inet_aton(cpcStrValue, &inaddr)) {                          /*  ת���ַ���Ϊ in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, ROUTEROPT)) {                       /*  �ж��Ƿ���Ĭ������ѡ��      */
        if (__isIpValid(cpcStrValue) != IPOK ||
           !inet_aton(cpcStrValue, &inaddr)) {                          /*  ת���ַ���Ϊ in_addr        */

            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, LEASEOPT)) {                        /*  �ж��Ƿ�����Լʱ��ѡ��      */
        if (uiIntValue <= 0) {                                          /*  �ж���ֵ��Ч��              */
            return  (PX_ERROR);
        }
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __dhcpIpValueCheck
** ��������: ��� DHCP IP ��ַ����Ч��
** ��    ��: cpcIpStart   ��ʼ IP
**           cpcIpEnd     ���� IP
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpIpValueCheck (CPCHAR  cpcIpStart, CPCHAR  cpcIpEnd)
{
    struct in_addr  inaddr;
    UINT            uiStart = 0;
    UINT            uiEnd   = 0;

    if (IPOK == __isIpValid(cpcIpStart) &&
        inet_aton(cpcIpStart, &inaddr)) {                               /*  ת���ַ���Ϊ���ε�ַ        */
        uiStart = inaddr.s_addr;                                        /*  ��ֵȫ�����ýṹ��          */

        if (IPOK == __isIpValid(cpcIpEnd) &&
            inet_aton(cpcIpEnd, &inaddr)) {                             /*  ת���ַ���Ϊ���ε�ַ        */
            uiEnd = inaddr.s_addr;                                      /*  ��ֵȫ�����ýṹ��          */

            /*
             *  �ж��Ƿ���ͬһ����
             */
            if (addr_cmp(ntohl(uiStart), ntohl(uiEnd), SIGMASK) && ntohl(uiStart) < ntohl(uiEnd)) {
                return  (ERROR_NONE);                                   /*  ��ַ�����óɹ�              */
            }
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __dhcpServerShowAction
** ��������: dhcp server ��Ϣ�ĺ�̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerShowAction (Webs  *wp)
{
    __PDHCPSERVER_HANDLE   pDhcpServerHeader         = LW_NULL;
    __PDHCPSERVER_HANDLE   pDhcpTmpNode              = LW_NULL;
    __DHCPSERVER_HANDLE    DhcpServerHandle          = {0};
    cJSON                 *pJsonDhcpArry             = LW_NULL;
    cJSON                 *pjSend                    = LW_NULL;
    cJSON                 *pRecvjRes                 = LW_NULL;
    cJSON                 *pjPage                    = LW_NULL;
    INT                    iPage                     = 0;
    INT                    iTotal                    = 0;               /*  ����Ŀ                      */
    INT                    iRecordTotal              = 0;
    CHAR                   cWebfName[NETIF_NAMESIZE] = {0};
    INT                    i;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend        = cJSON_CreateObject();                               /*  �������� Json ����          */
    pJsonDhcpArry = cJSON_CreateArray();                                /*  ���� Json ӳ����Ϣ����      */
    if (LW_NULL == pjSend || NULL == pJsonDhcpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     *  ���ò�ѯ����ӿ�
     */
    pDhcpServerHeader = dhcpServerShow(&DhcpServerHandle, &iRecordTotal);
    if (LW_NULL != pDhcpServerHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  ѭ�����÷��ص� Json ����    */
            pDhcpTmpNode = pDhcpServerHeader + i;

            /*
             *  ����Ҫ��ʾ�����ݹ���� Json ��ʽ ��һҳ��ʾ 10 ����
             */
            if ((iTotal >= (iPage - 1) * 10) && (iTotal < iPage * 10)) {

                cJSON  *pjDhcp = cJSON_CreateObject();                  /*  ����һ��ӳ�� Json ����      */
                if(NULL == pjDhcp) {
                     break;
                }

                cJSON_AddNumberToObject(pjDhcp,
                        WEB_DHCPSERVER_ENABLE,
                        pDhcpTmpNode->iEnable);
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_START,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpStart));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_END,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpEnd));

                bzero(cWebfName, sizeof(cWebfName));
                libIntAdaGetWebNetif(pDhcpTmpNode->cInterface, cWebfName, sizeof(cWebfName));

                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_INTERFACE,
                        cWebfName);
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_DNS1,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpDns1));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_DNS2,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpDns2));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_MASK,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpMask));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_GATEWAY,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpGateway));
                cJSON_AddNumberToObject(pjDhcp,
                        WEB_DHCPSERVER_LEASE,
                        pDhcpTmpNode->uiLease);

                cJSON_AddItemToArray(pJsonDhcpArry, pjDhcp);            /*  ������Ŀ��������            */
            }

            iTotal++;
        }
    }

    if (pDhcpServerHeader) {
        free(pDhcpServerHeader);
    }

    cJSON_AddItemToObject(pjSend,   ARRAY_NAME, pJsonDhcpArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,   iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,   200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonDhcpArry);
}
/*********************************************************************************************************
** ��������: __dhcpServerConfigAction
** ��������: dhcp server �����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerdcConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes           = LW_NULL;
    cJSON                *pSendjRes           = LW_NULL;
    cJSON                *pjEnable            = LW_NULL;
    cJSON                *pjIpStart           = LW_NULL;
    cJSON                *pjIpEnd             = LW_NULL;
    cJSON                *pjInterface         = LW_NULL;
    cJSON                *pjIpDns1            = LW_NULL;
    cJSON                *pjIpDns2            = LW_NULL;
    cJSON                *pjIpMask            = LW_NULL;
    cJSON                *pjIpGateway         = LW_NULL;
    cJSON                *pjLease             = LW_NULL;
    __DHCPSERVER_HANDLE   dhcpServerHandle    = {0};
    CHAR                  cDns2[16]           = {0};
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  ��ȡ post ����������        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjEnable    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_ENABLE);
    pjIpStart   = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_START);
    pjIpEnd     = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_END);
    pjInterface = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_INTERFACE);
    pjIpDns1    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_DNS1);
    pjIpMask    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_MASK);
    pjIpGateway = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_GATEWAY);
    pjLease     = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_LEASE);
    if (pjEnable     == LW_NULL ||
        pjIpStart    == LW_NULL ||
        pjIpEnd      == LW_NULL ||
        pjInterface  == LW_NULL ||
        pjIpDns1     == LW_NULL ||
        pjIpMask     == LW_NULL ||
        pjIpGateway  == LW_NULL ||
        pjLease      == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto __error;
    }

    pjIpDns2 = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_DNS2);
    if (NULL == pjIpDns2 || '\0' == (pjIpDns2->valuestring)[0]) {
        strncpy(cDns2, DNS_DEFAULT, sizeof(cDns2));
    } else {
        strncpy(cDns2, pjIpDns2->valuestring, sizeof(cDns2));
    }

    iRet = __dhcpIpValueCheck(pjIpStart->valuestring, pjIpEnd->valuestring);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto __error;
    }

    if (__dhcpOptArgCheck(DNSOPT1,   pjIpDns1->valuestring,    0) ||
        __dhcpOptArgCheck(DNSOPT2,   cDns2,                    0) ||
        __dhcpOptArgCheck(SUBNETOPT, pjIpMask->valuestring,    0) ||
        __dhcpOptArgCheck(ROUTEROPT, pjIpGateway->valuestring, 0) ||
        __dhcpOptArgCheck(LEASEOPT,  NULL, pjLease->valueint)) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto __error;
    }

    dhcpServerHandle.iEnable     = pjEnable->valueint;
    dhcpServerHandle.uiIpStart   = inet_addr(pjIpStart->valuestring);
    dhcpServerHandle.uiIpEnd     = inet_addr(pjIpEnd->valuestring);
    dhcpServerHandle.uiIpDns1    = inet_addr(pjIpDns1->valuestring);
    dhcpServerHandle.uiIpDns2    = inet_addr(cDns2);
    dhcpServerHandle.uiIpMask    = inet_addr(pjIpMask->valuestring);
    dhcpServerHandle.uiIpGateway = inet_addr(pjIpGateway->valuestring);
    dhcpServerHandle.uiLease     = pjLease->valueint;

    iRet = libIntAdaGetNetif(pjInterface->valuestring,                  /*  ��ȡ��Ӧ��ϵͳ��������      */
                             dhcpServerHandle.cInterface,
                             sizeof(dhcpServerHandle.cInterface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ������������ӿ�
     */
    iRet = dhcpServerConfig(&dhcpServerHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __dhcpServerDelAction
** ��������: ɾ�� dhcp Server ��̨�ӿ�
** �䡡��  : wp               ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerDelAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pjInterface      = LW_NULL;
    __DHCPSERVER_HANDLE   dhcpServerHandle = {0};
    INT                   iRet;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }
    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ json ��ʽ����*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjInterface = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_INTERFACE);
    if (LW_NULL == pjInterface) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iRet = libIntAdaGetNetif(pjInterface->valuestring,                  /*  ��ȡ��Ӧ��ϵͳ��������      */
                             dhcpServerHandle.cInterface,
                             sizeof(dhcpServerHandle.cInterface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    iRet = dhcpServerDel(&dhcpServerHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ��������                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  �����ɹ�                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __dhcpServerActionDefine
** ��������: dhcp server ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerActionDefine (VOID)
{
    websDefineAction("dhcpServerShowAction",   __dhcpServerShowAction);
    websDefineAction("dhcpServerConfigAction", __dhcpServerdcConfigAction);
    websDefineAction("dhcpServerDelAction",    __dhcpServerDelAction);
}
/*********************************************************************************************************
** ��������: dhcpServerActionInit
** ��������: dhcpServer��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  dhcpServerActionInit (VOID)
{
    __dhcpServerActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
