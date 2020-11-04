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
** ��   ��   ��: npfAction.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 25 ��
**
** ��        ��: NPF���ú�̨�ӿ�
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
#include "npf_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  ���ص�״̬��                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json�ṹ����ʧ��ʱʹ��      */
/*********************************************************************************************************
 Jason Key: npf���ò���
*********************************************************************************************************/
#define WEB_NPF_RULE_INDEX              "id"                            /*  RULE����������              */
#define WEB_NPF_HANDLE_TYPE             "optype"                        /*  ������������                */
#define WEB_NPF_WEBNAME                 "webname"                       /*  ����ӿ�����                */
#define WEB_NPF_IFACE                   "iface"                         /*  ����ӿ�����                */
#define WEB_NPF_IFACESTR                "ifacestr"                      /*  ����ӿ��ַ���              */
#define WEB_NPF_RULE                    "rules"                         /*  NPF��������                 */
#define WEB_NPF_RULE_TYPE               "ruletype"                      /*  NPF������������             */
#define WEB_NPF_MACS                    "macs"                          /*  MAC��ַ����                 */
#define WEB_NPF_IP                      "ip"                            /*  IP��ַ����                  */
#define WEB_NPF_UDP                     "udp"                           /*  UDP����                     */
#define WEB_NPF_TCP                     "tcp"                           /*  TCP����                     */
#define WEB_NPF_ENABLE                  "enable"                        /*  NPF״̬                     */

#define WEB_NPF_MAC                     "mac"                           /*  MAC��ַ����                 */
#define WEB_NPF_IPS                     "ips"                           /*  IP��ʼ��ַ����              */
#define WEB_NPF_IPE                     "ipe"                           /*  IP������ַ����              */
#define WEB_NPF_PORTS                   "ports"                         /*  ��ʼ�˿�����                */
#define WEB_NPF_PORTE                   "porte"                         /*  �����˿�����                */
#define WEB_NPF_DIRECT                  "direct"                        /*  ���ݷ���                    */
#define WEB_NPF_MANGLE                  "mangle"                        /*  ���ݴ���ʽ                */
/*********************************************************************************************************
  Jason key: ��ҳ��ʾ��Ϣ
*********************************************************************************************************/
#define WEB_PAGE                        "page"                          /*  Json�ֶΣ���ǰ����ʾ��ҳ��  */
#define WEB_TOTAL                       "total"                         /*  Json�ֶΣ��ܹ�����Ŀ��      */
#define ARRAY_NAME                      "data"                          /*  Json�ֶΣ������˿�          */
#define WEB_PAGE_SIZE                   "pagesize"                      /*  ÿҳ��ʾ��Ŀ                */

#define NPF_GETOBJSTRING(p, key, defVal) \
                    (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                     defVal : cJSON_GetObjectItem(p, key)->valuestring  /*  ��ȡJson�������ַ�������    */
#define NPF_GETOBJINTER(p, key, defVal) \
                    (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                     defVal : cJSON_GetObjectItem(p, key)->valueint     /*  ��ȡJson��������������      */
/*********************************************************************************************************
** ��������: __parseNpfData2
** ��������: ���� NPF ����
** ��    ��: pDataHandle  NPF ��������ͷ
**           iRules       NPF ������Ŀ��
** ��    ��: NONE
** ��    ��: ������ JSON ����
*********************************************************************************************************/
static cJSON  *__parseNpfData2 (__PNPF_HANDLE  pDataHandle, INT  iRules)
{
    __PNPF_HANDLE pTmpHandle        = LW_NULL;
    INT           i                 = 0;
    CHAR          cWebNetifName[32] = {0};

    if (LW_NULL == pDataHandle) {
        return  (LW_NULL);
    }

    cJSON *pNetifObj     = cJSON_CreateObject();
    cJSON *pJsonNpfAarry = cJSON_CreateArray();

    for (i = 0; i < iRules; i++) {
       pTmpHandle = pDataHandle + i;
       cJSON  *pNpfRule = cJSON_CreateObject();

       bzero(cWebNetifName, sizeof(cWebNetifName));

       libIntAdaGetWebNetif(pTmpHandle->cNetifName, cWebNetifName, sizeof(cWebNetifName));

       cJSON_AddStringToObject(pNpfRule, WEB_NPF_IFACE, cWebNetifName);
       cJSON_AddNumberToObject(pNpfRule, WEB_NPF_ENABLE, pTmpHandle->bEnable);
       cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_TYPE, pTmpHandle->ucType);

       switch (pTmpHandle->ucType) {
       case NPFRULE_MAC:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MAC, pTmpHandle->cHwAddr);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_IP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_UDP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTS, pTmpHandle->usPortS);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTE, pTmpHandle->usPortE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_TCP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTS, pTmpHandle->usPortS);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTE, pTmpHandle->usPortE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       default:
           break;
       }

       cJSON_AddItemToArray(pJsonNpfAarry, pNpfRule);
   }

    cJSON_AddItemToObject(pNetifObj, WEB_NPF_RULE, pJsonNpfAarry);

    return (pNetifObj);
}
/*********************************************************************************************************
** ��������: __npfShowAction
** ��������: ������ʾNPF����
** ��    ��: wp  webs���
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __npfShowAction (Webs  *wp)
{
    __PNPF_HANDLE  pNpfHeader              = LW_NULL;
    __NPF_HANDLE   npfHandle               = {0};

    cJSON         *pjSend                  = LW_NULL;
    cJSON         *pRecvjRes               = LW_NULL;
    cJSON         *pjArry                  = LW_NULL;
    cJSON         *pjWebName               = LW_NULL;
    PCHAR          pcWebName               = LW_NULL;
    CHAR           cIfName[NETIF_NAMESIZE] = {0};
    INT            iTotal                  = 0;
    INT            iPage                   = 0;
    INT            iPageSize               = 0;

    if (!isUserLogin(wp)) {                                             /*  �û���Ȩ                    */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    webHeadHandle(wp, 200);                                             /*  ���� http ��ͷ              */

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
    pjArry = cJSON_CreateArray();                                       /*  ���� Json ӳ����Ϣ����      */
    if(LW_NULL == pjSend || NULL == pjArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return ;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "show npf, parser request data failed!\n", 0);

        goto  __error;
    }

    iPage = NPF_GETOBJINTER(pRecvjRes, WEB_PAGE, 1);
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageSize = NPF_GETOBJINTER(pRecvjRes, WEB_PAGE_SIZE, 10);

    npfHandle.iItemStart = (iPage -1) * iPageSize;
    npfHandle.iPageSize  = iPageSize;

    /*
     *  ��ȡ Json ��������
     */
    pjWebName = cJSON_GetObjectItem(pRecvjRes, WEB_NPF_WEBNAME);
    if (NULL == pjWebName) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    pcWebName = pjWebName->valuestring;
    if ('\0' == pcWebName[0]) {
        snprintf(npfHandle.cWhereLimit, sizeof(npfHandle.cWhereLimit), "(Type != '255')");
    } else {
        libIntAdaGetNetif(pcWebName, cIfName, sizeof(cIfName));

        snprintf(npfHandle.cWhereLimit,
                 sizeof(npfHandle.cWhereLimit),
                 "(IFNAME IN ('%s') AND Type != '255')", cIfName);
    }

    pNpfHeader = npfShow(&npfHandle, &iTotal);
    if (LW_NULL != pNpfHeader && 0 != iTotal) {
        cJSON_AddItemToObject(pjSend, ARRAY_NAME, __parseNpfData2(pNpfHeader, iTotal));
    } else {
        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pjArry);
    }

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, (NULL == pNpfHeader) ? iTotal : (pNpfHeader->iTotal));
    cJSON_AddNumberToObject(pjSend, WEB_PAGE, iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    if (NULL != pNpfHeader) {
        npfInfoFree(pNpfHeader);
    }

    operLogInsert(wp, OPEN_RT_NPF, "show npf rules success!\n", 0);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __npfConfigAction
** ��������: npf�����̨�ӿ�
** �䡡��  : wp  ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID __npfConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;
    cJSON                *ifaceArray    = LW_NULL;
    cJSON                *pArrayItem    = LW_NULL;

    cJSON                *pjWebName     = LW_NULL;
    PCHAR                 pcWebName     = LW_NULL;
    CHAR                  cDefVal[64]   = {0};
    __NPF_HANDLE          npfHandle     = {0};
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

        return ;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf, parser request data failed!\n", 0);

        goto  __error;
    }

    ifaceArray = cJSON_GetObjectItem(pRecvjRes, ARRAY_NAME);
    if (LW_NULL == ifaceArray) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules failed!\n", 0);

        goto  __error;
    }

    pArrayItem = cJSON_GetArrayItem(ifaceArray, 0);
    if (LW_NULL == pArrayItem) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules failed!\n", 0);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    bzero(&npfHandle, sizeof(npfHandle));

    pjWebName = cJSON_GetObjectItem(pArrayItem, WEB_NPF_IFACE);
    if (LW_NULL == pjWebName) {
        strncpy(npfHandle.cNetifName, "lo0", sizeof(npfHandle.cNetifName));
    } else {
        pcWebName = pjWebName->valuestring;
        if ('\0' == pcWebName[0]) {
            strncpy(npfHandle.cNetifName, "lo0", sizeof(npfHandle.cNetifName));
        } else {
            libIntAdaGetNetif(pcWebName, npfHandle.cNetifName, sizeof(npfHandle.cNetifName));
        }
    }

    strncpy(npfHandle.cHandle,                                            /*  ��ȡ������������           */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_HANDLE_TYPE, cDefVal),
           sizeof(npfHandle.cHandle));

    strncpy(npfHandle.cHwAddr,                                            /*  ��ȡ MAC ��ַ              */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_MAC, cDefVal),
           sizeof(npfHandle.cHwAddr));

    strncpy(npfHandle.cIpAddrS,                                           /*  ��ȡ��ʼIP��ַ             */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_IPS, cDefVal),
           sizeof(npfHandle.cIpAddrS));

    strncpy(npfHandle.cIpAddrE,                                           /*  ��ȡ��ֹIP��ַ             */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_IPE, cDefVal),
           sizeof(npfHandle.cIpAddrE));

    strncpy(npfHandle.cDirect,                                            /*  ��ȡ���䷽��               */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_DIRECT, cDefVal),
           sizeof(npfHandle.cDirect));

    strncpy(npfHandle.cMangle,                                            /*  ��ȡ�����ܾ���ʶ         */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_MANGLE, cDefVal),
           sizeof(npfHandle.cMangle));

    npfHandle.ucType   = NPF_GETOBJINTER(pArrayItem, WEB_NPF_RULE, 0);
    npfHandle.usPortS  = NPF_GETOBJINTER(pArrayItem, WEB_NPF_PORTS, 0);
    npfHandle.usPortE  = NPF_GETOBJINTER(pArrayItem, WEB_NPF_PORTE, 0);
    npfHandle.iRuleSeq = NPF_GETOBJINTER(pArrayItem, WEB_NPF_RULE_INDEX, 0);

    /*
     *  ������������ӿ�
     */
    iRet = npfConfig(&npfHandle);
    if (ERROR_NONE != iRet) {                                           /*  ����ʧ��                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules success!\n", 0);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __npfActionDefine
** ��������: npf��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __npfActionDefine (VOID)
{
    websDefineAction("npfShowAction",   __npfShowAction);
    websDefineAction("npfConfigAction", __npfConfigAction);
}
/*********************************************************************************************************
** ��������: npfActionInit
** ��������: npf��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  npfActionInit (VOID)
{
    __npfActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
