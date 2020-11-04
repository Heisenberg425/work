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
** ��   ��   ��: configini.c
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017��3��14��
**
** ��        ��:
*********************************************************************************************************/
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <SylixOS.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <iniparser/iniparser.h>


#include "log.h"
#include "packet.h"
#include "configini.h"

#define IPADDRPOOL             "ipaddrpool"                             /* ��ַ�ر�ǩ�ַ���             */
#define FILEPATH               "filepath"                               /* �ļ�·����ǩ�ַ���           */
#define NETWORK                "network"                                /* ���粿�ֱ�ǩ�ַ���           */
#define OPT                    "opt"                                    /* ѡ���ǩ�ַ���               */
#define ININULL                "null"                                   /* ini���ÿ�                    */
#define IPSTART                "start"                                  /* ��ַ����ʼ��ַ��ֵƥ���ַ��� */
#define IPEND                  "end"                                    /* ��ַ�ؽ�����ַ��ֵƥ���ַ��� */
#define LEASEPATH              "leasefile"                              /* ��Լ�ļ���ֵƥ���ַ���       */
#define LOGFILE                "logfile"                                /* ��־�ļ���ֵƥ���ַ���       */
#define INTERFACE              "interface"                              /* ��������ֵƥ���ַ���         */
#define DNSOPT1                "dns1"                                   /* dns1��ֵƥ���ַ���           */
#define DNSOPT2                "dns2"                                   /* dns2��ֵƥ���ַ���           */
#define SUBNETOPT              "subnet"                                 /* ���������ֵƥ���ַ���       */
#define ROUTEROPT              "router"                                 /* Ĭ�����ؼ�ֵƥ���ַ���       */
#define DOMAINOPT              "domain"                                 /* ������ֵƥ���ַ���           */
#define LEASEOPT               "lease"                                  /* ��Լʱ���ֵƥ���ַ���       */
#define LEASET1                "t1"                                     /* ��Լʱ��50%��ֵƥ���ַ���    */
#define LEASET2                "t2"                                     /* ��Լʱ��87.5%��ֵƥ���ַ���  */
#define SECANDKEY              "%s:%s"                                  /* ��ǩ��ֵƴ��ʹ���ַ���       */
#define SECBUFFSIZE            20                                       /* ��ǩ��ֵƴ����󳤶�         */

#define ISNOTIP                0                                        /* ��IP                         */
#define IPVAILD                1                                        /* IPֵ����                     */
#define IPOK                   2                                        /* IPֵ��ȷ                     */

#define MAC_FMT                "Server Mac:    %02x:%02x:%02x:%02x:%02x:%02x \r\n"
#define addr_cmp(addr1, addr2, mask) ((addr1 & mask) == (addr2 & mask)) /* �жϵ�ַ�Ƿ���ͬһ����       */
#define SIGMASK                0xffffff00                               /* 255.255.255.0����            */

struct serverconfig  GserverConfig;                                     /* ����������ȫ�ֱ���           */

/*********************************************************************************************************
** ��������: poptionFind
** ��������: ����ѡ���ַ
** ��    ��: poptonsetlist   ѡ������б�
**           cType           ѡ������
** ��    ��: ����ѡ���ַ
** ����ģ��: NONE
*********************************************************************************************************/
static struct optionset  *poptionFind (struct optionset  *poptonsetlist, CHAR  cType)
{
    while (poptonsetlist && poptonsetlist->pcData[OPT_CODE] < cType) {  /* ���ҵ�ѡ���б��ж�Ӧ����λ�� */
        poptonsetlist = poptonsetlist->optionsetNext;
    }

    if (poptonsetlist && poptonsetlist->pcData[OPT_CODE] == cType) {    /* �ж��ǲ��ҽ�������Ƿ���ȷ   */
        return (poptonsetlist);                                         /* ����ѡ���ַ                 */
    } else {
        return (NULL);                                                  /* ���ؿ�                       */
    }
}

/*********************************************************************************************************
** ��������: optionAttach
** ��������: ����ѡ��
** ��    ��: **ppoptionsetList    ѡ������б�
**            *pdhcpoption        ѡ��Ĭ��ֵ�ṹ��
**             pcbuffer           ѡ������buffer
**             iLength            ѡ���
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static void  optionAttach (struct optionset  **ppoptionsetList,
                           struct dhcpoption  *pdhcpoption,
                           PCHAR               pcbuffer,
                           INT                 iLength)
{
    struct optionset   *poptionsetExit;
    struct optionset   *poptionsetNew;
    struct optionset  **ppoptionsetCurr;

    /*
     *  ����list���͵�ѡ�������Ҫ׷�ӣ�����ѡ�����Ͷ���һ�������
     */
    if ((poptionsetExit = poptionFind(*ppoptionsetList,                 /* ���ҵ���Ӧ���͵�ѡ��λ��     */
                                      pdhcpoption->ucCode))) {
        if (pdhcpoption->cFlags & OPTION_LIST) {                        /* �ж��Ƿ���list���͵�ѡ��     */
            if (poptionsetExit->pcData[OPT_LEN] + iLength <= 255) {     /* �ж�ѡ���Ƿ񳬳�             */
                poptionsetExit->pcData = realloc(poptionsetExit->pcData,/* �����ڴ�����С             */
                                                 poptionsetExit->pcData[OPT_LEN] + iLength + 2);
                memcpy(poptionsetExit->pcData + poptionsetExit->pcData[OPT_LEN] + 2,
                       pcbuffer,                                        /* ׷��ѡ��������               */
                       iLength);
                poptionsetExit->pcData[OPT_LEN] += iLength;             /* ����ѡ�����ݳ��Ȳ���         */
            }
        }
    } else {
        poptionsetNew                   = (struct optionset  *)malloc(sizeof(struct optionset));
        poptionsetNew->pcData           = malloc(iLength + 2);          /* ����ѡ���ڴ�                 */
        poptionsetNew->pcData[OPT_CODE] = pdhcpoption->ucCode;          /* ����ѡ������                 */
        poptionsetNew->pcData[OPT_LEN]  = iLength;                      /* ����ѡ�����ݳ���             */
        memcpy(poptionsetNew->pcData + 2, pcbuffer, iLength);           /* ����ѡ�����ݵ���Ӧ��ַ       */

        ppoptionsetCurr = ppoptionsetList;                              /* ����ѡ�����ѡ������         */
        while (*ppoptionsetCurr && (*ppoptionsetCurr)->pcData[OPT_CODE] < pdhcpoption->ucCode) {
            ppoptionsetCurr = &(*ppoptionsetCurr)->optionsetNext;
        }
        poptionsetNew->optionsetNext = *ppoptionsetCurr;
        *ppoptionsetCurr             =  poptionsetNew;
    }
}

/*********************************************************************************************************
** ��������: isIpValid
** ��������: �ж�IP��ַ��Ч��
** ��    ��: cpcIp  ip�ַ���ָ��
** ��    ��: ����IP��ַ����ISNOTIP��IP��ַ��Ч����IPVAILD��IP��ַ��������IPOK
*********************************************************************************************************/
static INT  isIpValid (CPCHAR  cpcIp)
{
    INT  i         = 0;
    INT  j         = 0;
    INT  iIpSeg[4] = {0, 0, 0, 0};
    INT  iLen      = strlen(cpcIp);
    INT  iTmp      = 0;

    for (i = 0; i < iLen + 1; i++) {
        if (cpcIp[i] == '.' || i == iLen) {                             /* ����IP��ַ�����             */
            if (iIpSeg[j] >= 0 && iIpSeg[j] <= 255) {                   /* ����IP��ַ��Χ               */
                j ++;
            } else {
                return (ISNOTIP);                                       /* ����IP��ֵ��Χ�򷵻�ISNOTIP  */
            }
        } else {
            iTmp = cpcIp[i] - '0';                                      /* IP�ַ���תΪ����             */
            if (iTmp > 9 || iTmp < 0) {
                return (ISNOTIP);                                       /* ����ֵ�ַ�������ISNOTIP      */
            } else {
                iIpSeg[j] = iIpSeg[j] * 10 + iTmp;                      /* ����ֵ����                   */
            }
        }
    }

    if (j != 4) {                                                       /* �ж�IP��ַ�����Ƿ�����       */
        return (ISNOTIP);
    }

    if (iIpSeg[0] > 0   &&                                              /* �ж�IP��Ч��                 */
        iIpSeg[0] < 255 &&
        iIpSeg[3] > 0   &&
        iIpSeg[3] < 255) {
        return (IPOK);                                                  /* ��Ч����IPOK                 */
    } else if (cpcIp[0] == 0) {
        return (ISNOTIP);                                               /* ��IP����ISNOTIP              */
    }

    return (IPVAILD);
}

/*********************************************************************************************************
** ��������: isMaskValid
** ��������: �ж�����������Ч��
** ��    ��: cpcMask  ���������ַ���ָ��
** ��    ��: ���������뷵��ERROR_NONE,���򷵻�PX_ERROR
*********************************************************************************************************/
static INT  isMaskValid (CPCHAR  cpcMask)
{
    UINT  uiIpVal  = 0;
    UINT  uiTmp[4] = {0, 0, 0, 0};
    INT   i        = 0;

    if (isIpValid(cpcMask) == IPVAILD) {
        sscanf(cpcMask,
               "%u.%u.%u.%u",
               &uiTmp[3],
               &uiTmp[2],
               &uiTmp[1],
               &uiTmp[0]);

        for (i = 0; i < 4; i++) {                                       /* ��������-->32λ�޷�������    */
            uiIpVal += uiTmp[i] << (i * 8);
        }

        uiIpVal = ~uiIpVal + 1;                                         /*   �ж������Ƿ�Ϊ2��������    */
        if ((uiIpVal & (uiIpVal - 1)) == 0) {
            return (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** ��������: optHandle
** ��������: ����ѡ��
** ��    ��: cpcType               ѡ����������
**           cpcBuf                ��ǩ��ƴ�Ӻ���ַ�����e.g "opt��dns"��
**           pdictionaryConfig     ini����ָ��
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  optHandle (CPCHAR  cpcType, CPCHAR  cpcBuf, dictionary  *pdictionaryConfig)
{
    CPCHAR           cpcConfig         = NULL;
    INT              iLen;
    ULONG            ulLeaseTmp;
    struct in_addr   inaddr;

    if (strcmp(cpcType, DNSOPT1) == 0) {                                /* �ж��Ƿ���dns1ѡ��           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* ��ȡDNSOPT1ѡ����ַ���      */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* �ж��Ƿ�ɹ���ȡ             */
            return (PX_ERROR);
        }

        if (inet_aton(cpcConfig, &inaddr)) {                            /* ת���ַ���Ϊin_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[5],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, DNSOPT2)   == 0) {                       /* �ж��Ƿ���dns2ѡ��           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* ��ȡDNSOPT2ѡ����ַ���      */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* �ж��Ƿ�ɹ���ȡ             */
            return (PX_ERROR);
        }

        if (inet_aton(cpcConfig, &inaddr)) {                            /* ת���ַ���Ϊin_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[5],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, SUBNETOPT) == 0) {                       /* �ж��Ƿ�����������ѡ��       */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* ��ȡSUBNETOPTѡ����ַ���    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* �ж��Ƿ�ɹ���ȡ             */
            return (PX_ERROR);
        }

        if (isMaskValid(cpcConfig) == ERROR_NONE &&
            inet_aton(cpcConfig, &inaddr)) {                            /* ת���ַ���Ϊin_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[0],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, ROUTEROPT) == 0) {                       /* �ж��Ƿ���Ĭ������ѡ��       */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* ��ȡROUTEROPTѡ����ַ���    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* �ж��Ƿ�ɹ���ȡ             */
            return (PX_ERROR);
        }

        if (isIpValid(cpcConfig) == IPOK &&
            inet_aton(cpcConfig, &inaddr)) {                            /* ת���ַ���Ϊin_addr          */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[2],
                         (PCHAR)&inaddr,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, DOMAINOPT) == 0) {                       /* �ж��Ƿ�������ѡ��           */
        cpcConfig = iniparser_getstring(pdictionaryConfig,              /* ��ȡDOMAINOPTѡ����ַ���    */
                                        cpcBuf,
                                        ININULL);
        if (strcmp(cpcConfig, ININULL) == 0) {                          /* �ж��Ƿ�ɹ���ȡ             */
            return (PX_ERROR);
        }

        iLen = strlen(cpcConfig);                                       /* ��ȡѡ���ַ�������           */
        if (iLen > 0) {                                                 /* �޳�254                      */
            if (iLen > 254) {
                iLen = 254;
            }
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[11],
                         (PCHAR)cpcConfig,
                         iLen);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, LEASEOPT)  == 0) {                       /* �ж��Ƿ�����Լʱ��ѡ��       */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* ��ȡLEASEOPTѡ����ֵ         */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* �ж���ֵ��Ч��               */
            GserverConfig.ulLease = ulLeaseTmp;                         /* ��ֵ��Լʱ�䵽ȫ������       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* ת����ֵΪ�����ֽ���         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[20],
                         (PCHAR)&ulLeaseTmp,
                         4);
        } else {
            return (PX_ERROR);
        }

    } else if (strcmp(cpcType, LEASET1)   == 0) {                       /* �ж��Ƿ��ǵ�һ�μ��ʱ��ѡ�� */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* ��ȡLEASET1ѡ����ֵ          */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* �ж���ֵ��Ч��               */
        	GserverConfig.ulT1 = ulLeaseTmp;                            /* ��ֵ��Լʱ�䵽ȫ������       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* ת����ֵΪ�����ֽ���         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[21],
                         (PCHAR)&ulLeaseTmp,
                         4);
        }

    } else if (strcmp(cpcType, LEASET2)   == 0) {                       /* �ж��Ƿ��ǵڶ��μ��ʱ��ѡ�� */
        ulLeaseTmp = (ULONG)iniparser_getdouble(pdictionaryConfig,      /* ��ȡLEASET2ѡ����ֵ          */
                                                cpcBuf,
                                                0);
        if (ulLeaseTmp > 0) {                                           /* �ж���ֵ��Ч��               */
        	GserverConfig.ulT1 = ulLeaseTmp;                            /* ��ֵ��Լʱ�䵽ȫ������       */
            ulLeaseTmp = htonl(ulLeaseTmp);                             /* ת����ֵΪ�����ֽ���         */
            optionAttach((PVOID)&(GserverConfig.poptionsets),           /* �����ѡ�����ѡ���б�       */
                         &Gdhcpopts[22],
                         (PCHAR)&ulLeaseTmp,
                         4);
        }
    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: optHandle
** ��������: ����ѡ��
** ��    ��: pdictionaryConfig     ini����ָ��
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static INT  readOpt (dictionary  *pdictionaryConfig)
{
    CHAR   *cOptKeyWord[]     = {DNSOPT1,                               /* ѡ��ƥ���ֵ�ַ�������      */
                                 DNSOPT2,
                                 SUBNETOPT,
                                 ROUTEROPT,
                                 DOMAINOPT,
                                 LEASEOPT,
                                 LEASET1,
                                 LEASET2};
    INT     i;
    CHAR    cBuf[SECBUFFSIZE] = {0};

    if (iniparser_find_entry(pdictionaryConfig, OPT)) {                 /* �����Ƿ�����OPTѡ���      */
        for (i = 0; i < sizeof(cOptKeyWord)/sizeof(CHAR *); i++) {
            snprintf(cBuf, SECBUFFSIZE, SECANDKEY, OPT, cOptKeyWord[i]);/* ƴ��ѡ��ںͼ�e.g("opt��dns")*/
            if (iniparser_find_entry(pdictionaryConfig, cBuf)) {        /* �ж��Ƿ���ڶ�Ӧ��ƥ���ֵ   */
                if (optHandle(cOptKeyWord[i],                           /* ����ѡ���                 */
                              cBuf,
                              pdictionaryConfig) != ERROR_NONE) {
                    return (PX_ERROR);
                }

            } else {
                if (i == 0 || i == 1) {                                 /* DNS���û�����ã������      */
                    continue;
                }
                return (PX_ERROR);
            }
        }

    } else {
        return (PX_ERROR);
    }

    return (ERROR_NONE);                                                /* ����ѡ��ɹ�                 */
}

/*********************************************************************************************************
** ��������: readNetwork
** ��������: ��ȡ����������
** ��    ��: pdictionaryConfig   INI�����ļ��ֵ�
** ��    ��: �ɹ�����ERROR��ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  readNetwork (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;

    if (iniparser_find_entry(pdictionaryConfig, NETWORK)) {             /* �ж��Ƿ���������������     */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, NETWORK, INTERFACE);     /* ƴ��ѡ��ںͼ�e.g("opt��dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* �ж��Ƿ����INTERFACE�ļ�ֵ  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* ȡ��INTERFACE�������ַ���    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* �ж��Ƿ��ȡ�ɹ�             */
                return (PX_ERROR);
            }

            if (GserverConfig.pcInteface) {                             /* Ϊ��һ��strdup׼��ָ��       */
                free(GserverConfig.pcInteface);
            }

            GserverConfig.pcInteface = strdup(cpcConfig);               /* �ַ�����ֵ                   */
            if (GserverConfig.pcInteface) {
                return (ERROR_NONE);                                    /* ���������óɹ�               */
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: readFilepath
** ��������: ��ȡ��Լ�ļ���ַ���ã����ش��󣬽��̻��˳�
** ��    ��: pdictionaryConfig   INI�����ļ��ֵ�
** ��    ��: �ɹ�����ERROR��ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  readFilepath (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;

    if (iniparser_find_entry(pdictionaryConfig, FILEPATH)) {            /* �ж��Ƿ�����ļ�·��������� */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, FILEPATH, LEASEPATH);    /* ƴ��ѡ��ںͼ�e.g("opt��dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* �ж��Ƿ����LEASEPATH�ļ�ֵ  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* ȡ��LEASEPATH�������ַ���    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* �ж��Ƿ��ȡ�ɹ�             */
                return (PX_ERROR);
            }
            if (GserverConfig.pcLeasefile) {
                free(GserverConfig.pcLeasefile);
            }

            GserverConfig.pcLeasefile    = strdup(cpcConfig);           /* �ַ�����ֵ                   */
        }

        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, FILEPATH, LOGFILE);     /* ƴ��ѡ��ںͼ�e.g("opt��dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* �ж��Ƿ����LEASEPATH�ļ�ֵ  */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* ȡ��LEASEPATH�������ַ���    */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* �ж��Ƿ��ȡ�ɹ�             */
                return (PX_ERROR);
            }
            if (GserverConfig.pclogfile) {
                free(GserverConfig.pclogfile);
            }

            GserverConfig.pclogfile    = strdup(cpcConfig);           /* �ַ�����ֵ                   */
        }

        GserverConfig.pcLeasefilebak = (CHAR *)malloc(strlen(GserverConfig.pcLeasefile) + sizeof(FILEBAK));
        if (GserverConfig.pcLeasefile && GserverConfig.pcLeasefilebak) {
            snprintf(GserverConfig.pcLeasefilebak,
                     strlen(GserverConfig.pcLeasefile) + sizeof(FILEBAK),
                     "%s%s",
                     GserverConfig.pcLeasefile,
                     FILEBAK);
            return (ERROR_NONE);                                    /* ��Լ��ַ���óɹ�             */
        }
   }
   return (PX_ERROR);

}

/*********************************************************************************************************
** ��������: readIpaddrpool
** ��������: ��ȡ��ַ������
** ��    ��: pdictionaryConfig   INI�����ļ��ֵ�
** ��    ��: �ɹ�����ERROR��ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  readIpaddrpool (dictionary  *pdictionaryConfig)
{
    CHAR            cBuf[SECBUFFSIZE] = {0};
    CPCHAR          cpcConfig         = NULL;
    struct in_addr  inaddr;

    if (iniparser_find_entry(pdictionaryConfig, IPADDRPOOL)) {          /* �ж��Ƿ���ڵ�ַ���������   */
        snprintf(cBuf, SECBUFFSIZE, SECANDKEY, IPADDRPOOL, IPSTART);    /* ƴ��ѡ��ںͼ�e.g("opt��dns")*/
        if (iniparser_find_entry(pdictionaryConfig, cBuf)) {            /* �ж��Ƿ����IPSTART�ļ�ֵ    */
            cpcConfig = iniparser_getstring(pdictionaryConfig,          /* ȡ��IPSTART�������ַ���      */
                                            cBuf,
                                            ININULL);
            if (strcmp(cpcConfig, ININULL) == 0) {                      /* �ж��Ƿ��ȡ�ɹ�             */
                return (PX_ERROR);
            }

            if (isIpValid(cpcConfig) == IPOK &&
                inet_aton(cpcConfig, &inaddr)) {                        /* ת���ַ���Ϊ���ε�ַ         */
                GserverConfig.uiStart = inaddr.s_addr;                  /* ��ֵȫ�����ýṹ��           */

                memset(cBuf, 0, sizeof(cBuf));                          /* �������buff                 */
                snprintf(cBuf, SECBUFFSIZE, SECANDKEY, IPADDRPOOL, IPEND);
                if (iniparser_find_entry(pdictionaryConfig, cBuf)) {    /* �ж��Ƿ����IPEND�ļ�ֵ      */
                    cpcConfig = iniparser_getstring(pdictionaryConfig,  /* ȡ��IPEND�������ַ���        */
                                                     cBuf,
                                                     ININULL);
                    if (strcmp(cpcConfig, ININULL) == 0) {              /* �ж��Ƿ��ȡ�ɹ�             */
                        return (PX_ERROR);
                    }

                    if (isIpValid(cpcConfig) == IPOK &&
                        inet_aton(cpcConfig, &inaddr)) {                /* ת���ַ���Ϊ���ε�ַ         */
                        GserverConfig.uiEnd = inaddr.s_addr;            /* ��ֵȫ�����ýṹ��           */
                        if (addr_cmp(ntohl(GserverConfig.uiStart),      /* �ж��Ƿ���ͬһ����           */
                                     ntohl(GserverConfig.uiEnd),
                                     SIGMASK) &&
                            ntohl(GserverConfig.uiStart) < ntohl(GserverConfig.uiEnd)) {
                            return (ERROR_NONE);                        /* ��ַ�����óɹ�               */
                        }
                    }
                }
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: localIpGet
** ��������: ��ȡ����IP��ַ
** ��    ��: iSocketFd  ���ڻ�ȡ��Ϣ���׽���
** ��    ��: �ɹ�����ERROR_NONE,ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  localIpGet (INT  iSocketFd)
{
    INT           iRet;
    struct ifreq  ifreqIp;

    memset(&ifreqIp, 0, sizeof(ifreqIp));
    strncpy(ifreqIp.ifr_name,                                           /* �������õ���������ifreq�ṹ��*/
            GserverConfig.pcInteface,
            sizeof(ifreqIp.ifr_name) - 1);

    iRet = ioctl(iSocketFd, SIOCGIFADDR, &ifreqIp);                     /* ��ȡ��Ӧ��������IP��ַ       */
    if (iRet == ERROR_NONE) {                                           /* ��ֵ��ȫ�����ýṹ����       */
        GserverConfig.uiServer = ((struct sockaddr_in *)&ifreqIp.ifr_ifru.ifru_addr)->sin_addr.s_addr;
    }

    return (iRet);
}

/*********************************************************************************************************
** ��������: localMacGet
** ��������: ��ȡ����MAC��ַ
** ��    ��: INT  iSocketFd
** ��    ��: -1 ��ʾ��ȡʧ��
** ����ģ��: API
*********************************************************************************************************/
static INT  localMacGet (INT  iSocketFd)
{
    INT           iRet;
    struct ifreq  ifreqMac;

    memset(&ifreqMac, 0, sizeof(ifreqMac));
    strncpy(ifreqMac.ifr_name,                                          /* �������õ���������ifreq�ṹ��*/
            GserverConfig.pcInteface,
            sizeof(ifreqMac.ifr_name) - 1);

    iRet = ioctl(iSocketFd, SIOCGIFHWADDR, &ifreqMac);                  /* ��ȡ��Ӧ��������MAC��ַ      */
    if (iRet == ERROR_NONE) {
        memcpy(GserverConfig.ucMac,                                     /* ��ֵ��ȫ�����ýṹ����       */
               ifreqMac.ifr_ifru.ifru_hwaddr.sa_data,
               6);
    }

    return (iRet);
}

/*********************************************************************************************************
** ��������: localNetinfoGet
** ��������: ��ȡ����Net�����Ϣ
** ��    ��: NONE
** ��    ��: ʧ�ܷ���PX_ERROR���ɹ�����ERROR_NONE
** ����ģ��: NONE
*********************************************************************************************************/
static INT  localNetinfoGet (VOID)
{
    INT  iSocketFd;
    INT  iRet;

    iSocketFd = socket(AF_INET, SOCK_STREAM, 0);                        /* ���׽���                   */
    if (iSocketFd < 0) {
        LOG(LOG_ERR, "Get loclnetinfo failed!\r\n");
        return (PX_ERROR);
    }

    iRet = localMacGet(iSocketFd);                                      /* ��ȡ����MAC                  */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Get localMac failed!\r\n");
        close(iSocketFd);
        return (PX_ERROR);
    }

    iRet = localIpGet(iSocketFd);                                       /* ��ȡ����IP                   */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Get localIp failed!\r\n");
        close(iSocketFd);
        return (PX_ERROR);
    }

    close(iSocketFd);                                                   /* �ر��׽���                   */
    return (ERROR_NONE);                                                /* ��ȡ�ɹ�                     */
}

/*********************************************************************************************************
** ��������: config_workpath_update
** ��������: �Ժ�·����ص����ã�����������·�����ºͽ���Ŀ���ļ���
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static void config_workpath_update(void)
{
    CHAR *ptr = NULL;

    ptr = malloc(strlen(GserverConfig.pcLeasefile) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pcLeasefile,GserverConfig.pcInteface);
        free(GserverConfig.pcLeasefile);
        GserverConfig.pcLeasefile = ptr;
        /*
         *  ͨ����ǰ����Լ�ļ��������ļ��н���
         */
        ptr  = strrchr(GserverConfig.pcLeasefile, '/');
        *ptr = 0;
        mkdir(GserverConfig.pcLeasefile, S_IRWXU|S_IRGRP|S_IROTH);
        *ptr = '/';
    }

    ptr = malloc(strlen(GserverConfig.pclogfile) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pclogfile,GserverConfig.pcInteface);
        free(GserverConfig.pclogfile);
        GserverConfig.pclogfile = ptr;
    }

    ptr = malloc(strlen(GserverConfig.pcLeasefilebak) + strlen(GserverConfig.pcInteface));
    if (ptr != NULL) {
        sprintf(ptr,GserverConfig.pcLeasefilebak,GserverConfig.pcInteface);
        free(GserverConfig.pcLeasefilebak);
        GserverConfig.pcLeasefilebak = ptr;
    }
}

/*********************************************************************************************************
** ��������: configPrint
** ��������: ����־�ϴ�ӡ��ǰ�������ļ�
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static VOID  configPrint (VOID)
{
    LOG(LOG_INFO, "Server Device: %s\r\n", GserverConfig.pcInteface);
    LOG(LOG_INFO, "Server IP:     %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiServer)));
    LOG(LOG_INFO, MAC_FMT, (UCHAR)GserverConfig.ucMac[0],
                           (UCHAR)GserverConfig.ucMac[1],
                           (UCHAR)GserverConfig.ucMac[2],
                           (UCHAR)GserverConfig.ucMac[3],
                           (UCHAR)GserverConfig.ucMac[4],
                           (UCHAR)GserverConfig.ucMac[5]);
    LOG(LOG_INFO, "Ip pool start: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiStart)));
    LOG(LOG_INFO, "Ip pool end:   %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&(GserverConfig.uiEnd)));
    LOG(LOG_INFO, "Leasfile path: %s\r\n", GserverConfig.pcLeasefile);
    LOG(LOG_INFO, "Logfile path: %s\r\n", GserverConfig.pclogfile);
    LOG(LOG_INFO, "Lease time:    %d\r\n", GserverConfig.ulLease);
}
/*********************************************************************************************************
** ��������: configRead
** ��������: ��ȡ���������ļ�
** ��    ��: NONE
** ��    ��: ʧ�ܷ���PX_ERROR���ɹ�����ERROR_NONE
** ����ģ��: NONE
*********************************************************************************************************/
INT  configRead (VOID *configfile)
{
    dictionary        *pdictionaryConfig;

    GserverConfig.pcInteface  = NULL;                                   /* ��Ұָ��                     */
    GserverConfig.pcLeasefile = NULL;
    GserverConfig.poptionsets = NULL;

    pdictionaryConfig = iniparser_load(configfile);                /* ����ini�����ļ�              */
    if (!pdictionaryConfig) {
        return (PX_ERROR);
    }

    if (readIpaddrpool(pdictionaryConfig) == PX_ERROR) {                /* ��ȡ��ַ��������Ϣ           */
        iniparser_freedict(pdictionaryConfig);                          /* �ͷ�ini�����ڴ�              */
        return (PX_ERROR);
    }

    if (readFilepath(pdictionaryConfig) == PX_ERROR) {                  /* ��ȡ��Լ�ļ�·��������Ϣ     */
        iniparser_freedict(pdictionaryConfig);                          /* �ͷ�ini�����ڴ�              */
        return (PX_ERROR);
    }

    if (readNetwork(pdictionaryConfig) == PX_ERROR) {                    /* ��ȡ�������������Ϣ        */
        iniparser_freedict(pdictionaryConfig);                           /* �ͷ�ini�����ڴ�             */
        return (PX_ERROR);
    }

    if (readOpt(pdictionaryConfig) == PX_ERROR) {                        /* ��ȡѡ��������Ϣ            */
        iniparser_freedict(pdictionaryConfig);                           /* �ͷ�ini�����ڴ�             */
        return (PX_ERROR);
    }

    iniparser_freedict(pdictionaryConfig);                              /* �ͷ�ini�����ڴ�              */

    GserverConfig.ulOfferWaite = OFFERWAITE;                            /* ����OFFER�ȴ�ʱ��            */

    if (localNetinfoGet() == (PX_ERROR)) {                              /* ���ñ���������Ϣ             */
        return (PX_ERROR);
    }

    config_workpath_update();                                           /* ���������Ĺ���·������       */

    configPrint();

    return (ERROR_NONE);                                                /* ���óɹ�                     */
}

/*********************************************************************************************************
** ��������: configMemFree
** ��������: �ͷű��������ڴ�
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
VOID  configMemFree (VOID)
{
    struct optionset  *poptionsetCurr = NULL;
    struct optionset  *poptionsetFree = NULL;

    if (GserverConfig.pcInteface) {                                     /* �ж�Inteface�Ƿ������ڴ�   */
        free(GserverConfig.pcInteface);                                 /* �ͷ��ڴ�ռ�                 */
    }

    if (GserverConfig.pcLeasefile) {                                    /* �ж�Leasefile�Ƿ������ڴ�  */
        free(GserverConfig.pcLeasefile);                                /* �ͷ��ڴ�ռ�                 */
    }

    if (GserverConfig.pclogfile) {                                      /* �ж�Leasefile�Ƿ������ڴ�  */
        free(GserverConfig.pclogfile);                                  /* �ͷ��ڴ�ռ�                 */
    }


    while (poptionsetCurr) {                                            /* �ͷ�option�����ڴ�           */
        poptionsetFree  = poptionsetCurr;
        poptionsetCurr  = poptionsetCurr->optionsetNext;
        if (poptionsetFree->pcData) {                                   /* �ж�Data�Ƿ������ڴ�       */
            free(poptionsetFree->pcData);                               /* �ͷ�Data�ڴ�                 */
            free(poptionsetFree);                                       /* �ͷŽṹ���ڴ�               */
        }
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/

