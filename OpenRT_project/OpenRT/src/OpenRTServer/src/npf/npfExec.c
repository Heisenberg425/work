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
** ��   ��   ��: npfOper.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 24 ��
**
** ��        ��: npf��������
*********************************************************************************************************/
#include "npf_common.h"

/*********************************************************************************************************
** ��������:  __npfAddRule
** ��������:  ���NPF����
** ��    ��:  pNpfMsgBoby  ǰ�˴������Ϣ
** ��    ��:  NONE
** ��    ��:  ERROR_NONE or PX_ERROR
*********************************************************************************************************/
static INT  __npfAddRule (__PNPF_HANDLE  pNpfMsgBody)
{
    CHAR sCmd[256]  = {0};
    INT   iRet       = 0;

    if (LW_NULL == pNpfMsgBody) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "__npfAddRule NPF Handle is NULL\n");
        return (PX_ERROR);
    }

    switch (pNpfMsgBody->ucType) {
    case NPFRULE_MAC:
        snprintf(sCmd, sizeof(sCmd), "npfruleadd %s mac %s %s %s",
                pNpfMsgBody->cNetifName,
                pNpfMsgBody->cDirect,
                pNpfMsgBody->cMangle,
                pNpfMsgBody->cHwAddr);
        break;

    case NPFRULE_IP:
        /*
         * ��Ҫȷ��ǰ̨�����IP��ַ���ַ�����?
         */
        snprintf(sCmd, sizeof(sCmd), "npfruleadd %s ip %s %s %s %s",
                pNpfMsgBody->cNetifName,
                pNpfMsgBody->cDirect,
                pNpfMsgBody->cMangle,
                pNpfMsgBody->cIpAddrS,
                pNpfMsgBody->cIpAddrE);
        break;

    case NPFRULE_UDP:
        snprintf(sCmd, sizeof(sCmd), "npfruleadd %s udp %s %s %s %s %u %u",
                pNpfMsgBody->cNetifName,
                pNpfMsgBody->cDirect,
                pNpfMsgBody->cMangle,
                pNpfMsgBody->cIpAddrS,
                pNpfMsgBody->cIpAddrE,
                pNpfMsgBody->usPortS,
                pNpfMsgBody->usPortE);
        break;

    case NPFRULE_TCP:
        snprintf(sCmd, sizeof(sCmd), "npfruleadd %s tcp %s %s %s %s %u %u",
                pNpfMsgBody->cNetifName,
                pNpfMsgBody->cDirect,
                pNpfMsgBody->cMangle,
                pNpfMsgBody->cIpAddrS,
                pNpfMsgBody->cIpAddrE,
                pNpfMsgBody->usPortS,
                pNpfMsgBody->usPortE);
        break;

    default:
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Invaild Request,Rule is %d\n", pNpfMsgBody->ucType);
        return (PX_ERROR);
        break;
    }

    iRet = system(sCmd);                                                /*  ����NPF RULE                 */
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                               "error_code=%d.\n",sCmd, iRet);
         return (PX_ERROR);
    }

    insertDBNpfRule(pNpfMsgBody);

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __getNpfRule
** ��������: ��ȡ/proc/net/netfilter������������
** ��    ��: cbuf   һ��NPF��������
**           index  ������������λ��(�Կո���Ϊ�ָ�)
**           cValue �ⲿ����洢NPF������ĳһ������
** ��    ��: cValue ��ȡ������Ϣ
** ��    ��: NONE
*********************************************************************************************************/
static VOID  __getNpfRule (PCHAR  cbuf, INT  index, PCHAR  cValue)
{
    INT   i      = 0;
    INT   iPos   = 0;
    INT   iVaild = 0;
    INT   iVal   = 0;
    INT   iFlag  = 0;

    iPos = i;
    for (i = 0; i < strlen(cbuf); i++) {
         if (' ' == cbuf[i]) {
             iFlag = 0;

             if (0 == iVaild) {
                 continue;
             }

             if (1 == iVaild) {
                 iVaild = 0;
                 strncpy(cValue, cbuf+iPos, i-iPos);
             }
         }

         if (0 == iFlag) {
             iPos = i;
             iVal++;
         }

         iFlag = 1;
         if (index != iVal) {
             continue;
         }

         iVaild = 1;
     }

    return ;
}

/*********************************************************************************************************
** ��������: __getRuleSeq
** ��������: ��ȡNPF��������к�
** ��    ��: pNpfMsgBoby  NPF�ľ��
** ��    ��: NONE
** ��    ��: �ɹ�����NPF��������к�,ʧ�ܷ���-1
*********************************************************************************************************/
static INT  __getRuleSeq (__PNPF_HANDLE  pNpfMsgBoby)
{
#define    MANGLE_CONVERT(p)   ((0 == strcmp(p, "allow")) ? "YES" : "NO")
    FILE *fp         = LW_NULL;
    CHAR  cbuf[256]  = {0};
    INT   iRule       = 0;
    INT   iLine       = 0;
    INT   iFlag       = 0;
    CHAR  iGet        = 0;
    CHAR  cIface[6]   = {0};
    CHAR  cSeqNum[5]  = {0};
    CHAR  cRule[5]    = {0};
    CHAR  cMac[18]    = {0};
    CHAR  cIPs[16]    = {0};
    CHAR  cIPe[16]    = {0};
    CHAR  cPORTs[6]   = {0};
    CHAR  cPORTe[6]   = {0};
    CHAR  cMangle[16] = {0};
    CHAR  cLine[16]   = {0};

    fp = fopen("/proc/net/netfilter", "r");
    if (LW_NULL == fp) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "open /proc/net/netfilter failed!!!\n");

        return  (-1);
    }

    while (LW_NULL != fgets(cbuf, sizeof(cbuf), fp)) {
        iLine++;
        __getNpfRule(cbuf, 1, cLine);
        if ((iGet == 0 ) && strcmp(cLine, pNpfMsgBoby->cDirect)) {
            bzero(cLine, sizeof(cLine));
            continue;
        } else {
            iGet = 1;
        }

        if (!strcmp(cLine, "NETIF") ||
            !strcmp(cLine, "")  ||
            !strcmp(cLine, "input") ||
            !strcmp(cLine, "output")) {
            bzero(cLine, sizeof(cLine));
            if (iFlag == 1) {
                break;
            } else {
                continue;
            }
        }

        iFlag = 1;
        if (iLine > 0) {
             bzero(cIface, sizeof(cIface));
             __getNpfRule(cbuf, 1, cIface);                             /*  ��ȡ��������                */

             if (0 != strcmp(pNpfMsgBoby->cNetifName, cIface)) {
                 continue;
             }

             bzero(cRule, sizeof(cRule));
             __getNpfRule(cbuf, 4, cRule);                              /*  ��ȡNPF��������             */

             if (0 == strcmp(cRule, "MAC")) {
                 iRule = NPFRULE_MAC;
             } else if (0 == strcmp(cRule, "IP")) {
                 iRule = NPFRULE_IP;
             } else if (0 == strcmp(cRule, "UDP")) {
                 iRule = NPFRULE_UDP;
             } else if (0 == strcmp(cRule, "TCP")) {
                 iRule = NPFRULE_TCP;
             }

             if (pNpfMsgBoby->ucType != iRule) {
                 continue;
             }

             switch (iRule) {
             case NPFRULE_MAC:
                 bzero(cMac, sizeof(cMac));
                 __getNpfRule(cbuf, 6, cMac);                           /*  ��ȡMAC��ַ                 */
                 bzero(cMangle, sizeof(cMangle));
                 __getNpfRule(cbuf, 5, cMangle);

                 if (0 != strcasecmp(pNpfMsgBoby->cHwAddr, cMac) ||
                     0 != strcasecmp(MANGLE_CONVERT(pNpfMsgBoby->cMangle), cMangle)) {
                     continue;
                 }
                 break;

             case NPFRULE_IP:
                 bzero(cIPs, sizeof(cIPs));
                 __getNpfRule(cbuf, 7, cIPs);                           /*  ��ȡ��ʼIP��ַ              */

                 bzero(cIPe, sizeof(cIPe));
                 __getNpfRule(cbuf, 8, cIPe);                           /*  ��ȡ��ֹIP��ַ              */

                 bzero(cMangle, sizeof(cMangle));
                 __getNpfRule(cbuf, 5, cMangle);
                 if (0 != strcmp(pNpfMsgBoby->cIpAddrS, cIPs) ||
                     0 != strcmp(pNpfMsgBoby->cIpAddrE, cIPe) ||
                     0 != strcmp(MANGLE_CONVERT(pNpfMsgBoby->cMangle), cMangle)) {
                     continue;
                 }
                 break;

             case NPFRULE_UDP:
             case NPFRULE_TCP:
                 bzero(cIPs, sizeof(cIPs));
                 __getNpfRule(cbuf, 7, cIPs);                           /*  ��ȡ��ʼIP��ַ              */

                 bzero(cIPe, sizeof(cIPe));
                 __getNpfRule(cbuf, 8, cIPe);                           /*  ��ȡ��ֹIP��ַ              */

                 bzero(cPORTs, sizeof(cPORTs));
                 __getNpfRule(cbuf, 9, cPORTs);                         /*  ��ȡ��ʼPORT                */

                 bzero(cPORTe, sizeof(cPORTe));
                 __getNpfRule(cbuf, 10, cPORTe);                        /*  ��ȡ��ֹPORT                */

                 bzero(cMangle, sizeof(cMangle));
                 __getNpfRule(cbuf, 5, cMangle);
                 if (0 != strcmp(pNpfMsgBoby->cIpAddrS, cIPs) ||
                     0 != strcmp(pNpfMsgBoby->cIpAddrE, cIPe) ||
                     0 != strcmp(MANGLE_CONVERT(pNpfMsgBoby->cMangle), cMangle) ||
                     pNpfMsgBoby->usPortS != atoi(cPORTs)     ||
                     pNpfMsgBoby->usPortE != atoi(cPORTe)) {
                     continue;
                 }
                 break;

             default:
                 break;
             }

             __getNpfRule(cbuf, 3, cSeqNum);                            /*  ��ȡ��ʼIP��ַ              */
             fclose(fp);
#undef    MANGLE_CONVERT
             return  (atoi(cSeqNum));
         }
         iLine++;
    }
    fclose(fp);

#undef    MANGLE_CONVERT
    return  (-1);
}

/*********************************************************************************************************
** ��������: __npfDelRule
** ��������: ɾ��NPF����
** ��    ��: pNpfMsgBody  ǰ�˴������Ϣ
** ��    ��: NONE
** ��    ��: ERROR_NONE or PX_ERROR
*********************************************************************************************************/
static INT  __npfDelRule (__PNPF_HANDLE  pNpfMsgBody)
{
    CHAR  sCmd[256]  = {0};
    INT   iRet       = 0;
    INT   iRuleSeq   = 0;
    INT   iRecordNum = 0;

    if (LW_NULL == pNpfMsgBody) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "__npfDelRule NPF Handle is NULL\n");

        return  (PX_ERROR);
    }

    snprintf(sCmd, sizeof(sCmd), "ID=%d", pNpfMsgBody->iRuleSeq);
    pNpfMsgBody = selectDBNpfRule(pNpfMsgBody, &iRecordNum, sCmd);
    if (LW_NULL == pNpfMsgBody) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "No such Rule!!!\n");

        return  (PX_ERROR);
    }

    do {
        iRuleSeq = __getRuleSeq(pNpfMsgBody);

        if (-1 == iRuleSeq) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "No such Rule!!!\n");

            return  (PX_ERROR);
        }

        snprintf(sCmd, sizeof(sCmd), "npfruledel %s %s %d",
                pNpfMsgBody->cNetifName,
                pNpfMsgBody->cDirect,
                iRuleSeq);

        iRet = system(sCmd);                                           /*  ���� NPF RULE                */
        if (ERROR_NONE != iRet) {
             __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command = %s, "
                                                   "error_code=%d.\n",sCmd, iRet);
             return  (PX_ERROR);
        }

        iRuleSeq = __getRuleSeq(pNpfMsgBody);
    } while(-1 != iRuleSeq);

    delDBNpfRule(pNpfMsgBody);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __npfUpdateRule
** ��������: ����NPF����
** ��    ��: pNpfMsgBoby  ǰ�˴��������
** ��    ��: NONE
** ��    ��: ERROR_NONE or PX_ERROR
*********************************************************************************************************/
static INT  __npfUpdateRule (__PNPF_HANDLE  pNpfMsgBoby)
{
    INT   iRet  = 0;

    iRet = __npfDelRule(pNpfMsgBoby);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    iRet = __npfAddRule(pNpfMsgBoby);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: doCfgNpfRule
** ��������: ����NPF����
** ��    ��: pNpfMsgBoby  ǰ�˴��������
** ��    ��: NONE
** ��    ��: ERROR_NONE or PX_ERROR
*********************************************************************************************************/
INT  doCfgNpfRule (__PNPF_HANDLE  pNpfMsgBoby)
{
    INT  iRet = 0;

    if (LW_NULL == pNpfMsgBoby) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(pNpfMsgBoby->cHandle, NPFRULE_ADD)) {
        iRet = __npfAddRule(pNpfMsgBoby);
    } else if (0 == strcmp(pNpfMsgBoby->cHandle, NPFRULE_DEL)) {
        iRet = __npfDelRule(pNpfMsgBoby);
    } else if (0 == strcmp(pNpfMsgBoby->cHandle, NPFRULE_UPADTE)) {
        iRet = __npfUpdateRule(pNpfMsgBoby);
    }

    return  (iRet);
}

/*********************************************************************************************************
** ��������: npfRecovery
** ��������: npf���ûָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : INT ִ�н��
*********************************************************************************************************/
INT  npfRecovery (VOID)
{
    __PNPF_HANDLE pNpfHeader   = LW_NULL;
    __PNPF_HANDLE pTmpNode     = LW_NULL;
    CHAR          cCmdTmp[128] = {0};
    INT           iRet         = 0;
    INT           iRecordNum   = 0;
    INT           i            = 0;

    pNpfHeader = selectDBNpfRule(LW_NULL, &iRecordNum, "1=1");          /*  ѡ��npf������������        */
    if (LW_NULL == pNpfHeader) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iRecordNum; i++) {
        pTmpNode = pNpfHeader + i;
        bzero(cCmdTmp, sizeof(cCmdTmp));
        switch (pTmpNode->ucType) {
        case NPFRULE_MAC:
            snprintf(cCmdTmp, sizeof(cCmdTmp),
                    "npfruleadd %s mac %s %s %s",
                    pTmpNode->cNetifName,
                    pTmpNode->cDirect,
                    pTmpNode->cMangle,
                    pTmpNode->cHwAddr);
            break;

        case NPFRULE_IP:
            snprintf(cCmdTmp, sizeof(cCmdTmp),
                    "npfruleadd %s ip %s %s %s %s",
                    pTmpNode->cNetifName,
                    pTmpNode->cDirect,
                    pTmpNode->cMangle,
                    pTmpNode->cIpAddrS,
                    pTmpNode->cIpAddrE);
            break;

        case NPFRULE_UDP:
            snprintf(cCmdTmp, sizeof(cCmdTmp),
                    "npfruleadd %s udp %s %s %s %s %u %u",
                    pTmpNode->cNetifName,
                    pTmpNode->cDirect,
                    pTmpNode->cMangle,
                    pTmpNode->cIpAddrS,
                    pTmpNode->cIpAddrE,
                    pTmpNode->usPortS,
                    pTmpNode->usPortE);
            break;

        case NPFRULE_TCP:
            snprintf(cCmdTmp, sizeof(cCmdTmp),
                    "npfruleadd %s tcp %s %s %s %s %u %u",
                    pTmpNode->cNetifName,
                    pTmpNode->cDirect,
                    pTmpNode->cMangle,
                    pTmpNode->cIpAddrS,
                    pTmpNode->cIpAddrE,
                    pTmpNode->usPortS,
                    pTmpNode->usPortE);
            break;

        case NPFRULE_ATTACH:
            if (NPFRULE_ATTACH_ENABLE == pTmpNode->bEnable) {
                snprintf(cCmdTmp, sizeof(cCmdTmp),
                        "npfattach %s",
                        pTmpNode->cNetifName);
            } else {
                /*
                 * openrtcfgtool�ڳ�ʼ��ʱ������atexit,�˳�ʱ���firewall deatch
                 * �������ﲻ����detach��
                  */
//                snprintf(cCmdTmp, sizeof(cCmdTmp),
//                        "npfdetach %s",
//                        pTmpNode->cNetifName);
                continue;
            }

            usleep(10000);
            break;

        default:
            break;
        }

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet) {
               __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute recovery command = %s, "
                                                     "error_code=%d.\n",cCmdTmp, iRet);
        }
    }

    free(pNpfHeader);

    return  (iRet);
}
