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
** ��   ��   ��: natStatus.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 10 ��
**
** ��        ��: ϵͳ NAT ״̬��ȡ
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "natStatus.h"
#include "common.h"
#include "nat_common.h"
#include "fileparser/fileparser.h"
/*********************************************************************************************************
** ��������: natAssNodeUseGet
** ��������: ��ȡ NAT ����˿�ʹ����Ϣ
** �䡡��  : NONE
** �䡡��  : pAssNodeInfo       �˿�ʹ����Ϣ
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natAssNodeUseGet (__PNAT_ASSNODE_INFO  pAssNodeInfo)
{
    CHAR                 *pcBuf      = LW_NULL;
    CHAR                 *pcTmp      = LW_NULL;

    INT                   iRet;
    UINT                  uilinenum    = 1;
    UINT                  ulineOffset = 0;

    CHAR                  cFileName[50]  = {0};

    if (LW_NULL == pAssNodeInfo) {
        return  (PX_ERROR);
    }

    strncpy(cFileName, "/proc/net/nat/info", sizeof(cFileName));

    iRet = specifiedStrFind(cFileName, "Total Ass-node:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        pcTmp = pcBuf + ulineOffset + 15;                                /*  ��ȥ�ַ���ǰ������ֶ�     */
        pAssNodeInfo->iNatAssNodeTotal = atoi(pcTmp);
    } else {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    iRet = specifiedStrFind(cFileName, "Used  Ass-node:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        pcTmp = pcBuf + ulineOffset + 15;                                /*  ��ȥ�ַ���ǰ������ֶ�     */
        pAssNodeInfo->iNatAssNodeUse = atoi(pcTmp);
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: natAliasItemNumGet
** ��������: ��ȡ natAlias ��Ϣ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : iNatAliasNum       ������Ŀ����
*********************************************************************************************************/
INT  natAliasItemNumGet (VOID)
{
    CHAR                 *pcBuf          = LW_NULL;
    INT                   iRet;

    INT                   iNatAliasNum   = 0;
    UINT                  uilinenumstart = 0;
    UINT                  uilinenumEnd   = 0;
    UINT                  ulineOffset    = 0;

    PCHAR                 pcFileName     = "/proc/net/nat/info";

    /*
     *  �ҵ��˿�ӳ�����ʼ�����λ��
     */
    iRet = specifiedStrFind(pcFileName, "ALIAS", &pcBuf, &uilinenumstart, &ulineOffset);
    if (-1 != iRet) {
        free(pcBuf);
        pcBuf = LW_NULL;
    } else {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "direct", &pcBuf, &uilinenumEnd, &ulineOffset);
    if (-1 == iRet) {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    iNatAliasNum = uilinenumEnd - uilinenumstart - 3;                   /*  ��ȥ�м������ݵ� 3 ��       */

    return  (iNatAliasNum);
}

/*********************************************************************************************************
** ��������: natAliasInfoGet
** ��������: ��ȡ natAlias ��Ϣ
** �䡡��  : pNatAliasBuff      ���������Ϣ
** **        iStartIndex        ָ����ʼ��Ŀ
**           iEndIndex          ָ��������Ŀ
** �䡡��  : pNatAliasBuff      ������Ϣ
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natAliasInfoGet (PVOID  pNatAliasBuff, INT32  iStartIndex, INT32  iEndIndex)
{
    CHAR                 *pcBuf          = LW_NULL;
    INT                   iRet;

    CHAR                 *pcLineBuf      = LW_NULL;
    INT                   i, j;
    UINT32                uiInfoNum      = 0;
    UINT                  uilinenumstart = 0;
    UINT                  uilinenumEnd   = 0;
    UINT                  ulineOffset    = 0;
    size_t                sBufLen        = 0;

    __PNATALIAS_INFO      pNatAliasInfo  =  LW_NULL;
    PCHAR                 pcFileName     = "/proc/net/nat/info";

    if (LW_NULL == pNatAliasBuff) {
        return  (PX_ERROR);
    }

    pNatAliasInfo = (__PNATALIAS_INFO)pNatAliasBuff;

    /*
     *  �ҵ��˿�ӳ�����ʼ�����λ��
     */
    iRet = specifiedStrFind(pcFileName, "ALIAS", &pcBuf, &uilinenumstart, &ulineOffset);
    if (-1 != iRet) {
        free(pcBuf);
        pcBuf = LW_NULL;
    } else {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "direct", &pcBuf, &uilinenumEnd, &ulineOffset);
    if (-1 == iRet) {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    for (i = uilinenumstart + 2; i < uilinenumEnd - 1; i++)  {
        uiInfoNum++;
        if (uiInfoNum < iStartIndex || uiInfoNum > iEndIndex) {         /*  ��Ҫ���н����Ϣ�Ļ�ȡ      */
            continue;
        }

        sBufLen   = 0;
        iRet = specifiedLineRead(pcFileName, i, &pcLineBuf, &sBufLen);
        if (-1 != iRet) {
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  ȥ�����ҿո�\n            */
            pcLineBuf = strtok(pcLineBuf, " ");
            for (j = 0; j < 3; j++) {
                if (LW_NULL != pcLineBuf) {
                    switch (j) {
                    case 0:
                        strncpy(pNatAliasInfo->cAliasString,
                                pcLineBuf,
                                sizeof(pNatAliasInfo->cAliasString));
                        break;
                    case 1:
                        strncpy(pNatAliasInfo->cLoaclSString,
                                pcLineBuf,
                                sizeof(pNatAliasInfo->cLoaclSString));
                        break;
                    case 2:
                        strncpy(pNatAliasInfo->cLoaclEString,
                                pcLineBuf,
                                sizeof(pNatAliasInfo->cLoaclEString));
                        break;
                    default:
                        free(pcLineBuf);                                /*  �ͷ��ڴ�                    */

                        return  (PX_ERROR);
                    }
                    pcLineBuf = strtok(LW_NULL, " ");
                } else {                                                /*  ��û������ɳ���            */
                    free(pcLineBuf);                                    /*  �ͷ��ڴ�                    */

                    return  (PX_ERROR);
                }
            }

            pNatAliasBuff = (CHAR *)pNatAliasBuff + sizeof(__NATALIAS_INFO);
            pNatAliasInfo = (__PNATALIAS_INFO)(pNatAliasBuff);

        } else {
            free(pcLineBuf);                                            /*  �ͷ��ڴ�                    */

            return  (PX_ERROR);
        }

        free(pcLineBuf);                                                /*  �ͷ��ڴ�                    */
        pcLineBuf = LW_NULL;
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: natMaptemNumGet
** ��������: ��ȡ natmap ��Ϣ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : iNatMapNum         natmap ��Ŀ����
*********************************************************************************************************/
INT  natMapItemNumGet (VOID)
{
    CHAR                 *pcBuf          = LW_NULL;
    INT                   iRet;

    INT                   iNatMapNum     = 0;
    UINT                  uilinenumstart = 0;
    UINT                  uilinenumEnd   = 0;
    UINT                  ulineOffset    = 0;

    PCHAR                 pcFileName     = "/proc/net/nat/info";

    /*
     *  �ҵ��˿�ӳ�����ʼ�����λ��
     */
    iRet = specifiedStrFind(pcFileName, "ASS PORT", &pcBuf, &uilinenumstart, &ulineOffset);
    if (-1 != iRet) {
        free(pcBuf);
        pcBuf = LW_NULL;
    } else {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "summary", &pcBuf, &uilinenumEnd, &ulineOffset);
    if (-1 == iRet) {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    iNatMapNum = uilinenumEnd - uilinenumstart -3;                      /*  ��ȥ�м������ݵ� 3 ��       */

    return  (iNatMapNum);
}

/*********************************************************************************************************
** ��������: natMapInfoGet
** ��������: ��ȡ natmap ӳ����Ϣ
** �䡡��  : pNatMapBuff        natmap buf
**           iStartIndex        ָ����ʼ��Ŀ
**           iEndIndex          ָ��������Ŀ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natMapInfoGet (PVOID  pNatMapBuff, INT32  iStartIndex, INT32  iEndIndex)
{
    char                *pcBuf          = LW_NULL;
    char                *pcLineBuf      = LW_NULL;
    int                  iRet;
    int                  i, j;
    UINT32               uiInfoNum      = 0;
    UINT                 uilinenumstart = 0;
    UINT                 uilinenumEnd   = 0;
    unsigned int         ulineOffset    = 0;
    size_t               sBufLen        = 0;
    __PNATMAP_INFO       pNatMapInfo    =  LW_NULL;

    PCHAR                pcFileName    = "/proc/net/nat/info";

    if (LW_NULL == pNatMapBuff) {
        return  (PX_ERROR);
    }

    pNatMapInfo = (__PNATMAP_INFO)pNatMapBuff;

    /*
     *  �ҵ��˿�ӳ�����ʼ�����λ��
     */
    iRet = specifiedStrFind(pcFileName, "ASS PORT", &pcBuf, &uilinenumstart, &ulineOffset);
    if (-1 != iRet) {
        free(pcBuf);
        pcBuf = LW_NULL;
    } else {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "summary", &pcBuf, &uilinenumEnd, &ulineOffset);
    if (-1 == iRet) {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    for (i = uilinenumstart + 2; i < uilinenumEnd - 1; i++)  {
        uiInfoNum++;
        if (uiInfoNum < iStartIndex || uiInfoNum > iEndIndex) {         /*  ��Ҫ���н����Ϣ�Ļ�ȡ      */
            continue;
        }
        sBufLen   = 0;
        iRet = specifiedLineRead(pcFileName, i, &pcLineBuf, &sBufLen);
        if (-1 != iRet) {
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  ȥ�����ҿո�\n            */
            pcLineBuf = strtok(pcLineBuf, " ");
            for (j = 0; j < 5; j++) {
                if (LW_NULL != pcLineBuf) {
                    switch (j) {
                    case 0:
                        pNatMapInfo->iWanPort = atoi(pcLineBuf);
                        break;
                    case 1:
                        pNatMapInfo->iLanPort = atoi(pcLineBuf);
                        break;
                    case 2:
                        strncpy(pNatMapInfo->cIpString, pcLineBuf, sizeof(pNatMapInfo->cIpString));
                        break;
                    case 3:
                        pNatMapInfo->iIpCnt = atoi(pcLineBuf);
                        break;
                    case 4:
                        strncpy(pNatMapInfo->cProtoString, pcLineBuf, sizeof(pNatMapInfo->cProtoString));
                        break;
                    default:
                        free(pcLineBuf);                                /*  �ͷ��ڴ�                    */

                        return  (PX_ERROR);
                    }
                    pcLineBuf = strtok(LW_NULL, " ");
                } else {                                                /*  ��û������ɳ���            */
                    free(pcLineBuf);                                    /*  �ͷ��ڴ�                    */

                    return  (PX_ERROR);
                }
            }

            pNatMapBuff = (CHAR *)pNatMapBuff + sizeof(__NATMAP_INFO);
            pNatMapInfo = (__PNATMAP_INFO)(pNatMapBuff);

        } else {
            free(pcLineBuf);                                            /*  �ͷ��ڴ�                    */

            return  (PX_ERROR);
        }

        free(pcLineBuf);                                                /*  �ͷ��ڴ�                    */
        pcLineBuf = LW_NULL;
    }


    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: natIpFragGet
** ��������: ��ȡ ipfragment ״̬��Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : iIpfragmentflag    ipfragment ״̬��Ϣ
*********************************************************************************************************/
INT  natIpFragGet (VOID)
{
#define TCP_IPFRAG_ENABLE          (1)
#define UDP_IPFRAG_ENABLE          (2)
#define ICMP_IPFRAG_ENABLE         (4)

    CHAR                 *pcBuf             = LW_NULL;
    INT                   iRet;
    UINT                  uilinenum         = 1;
    UINT                  ulineOffset       = 0;
    INT                   iIpfragmentflag   = 0;

    PCHAR                 pcFileName        = "/proc/net/nat/info";

    iRet = specifiedStrFind(pcFileName, "IP Fragment:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        if (LW_NULL != strstr(pcBuf, "TCP-Enable")) {
            iIpfragmentflag |= TCP_IPFRAG_ENABLE;
        }

        if (LW_NULL != strstr(pcBuf, "UDP-Enable")) {
            iIpfragmentflag |= UDP_IPFRAG_ENABLE;
        }

        if (LW_NULL != strstr(pcBuf, "ICMP-Enable")) {
            iIpfragmentflag |= ICMP_IPFRAG_ENABLE;
        }

        free(pcBuf);
    }

    return  (iIpfragmentflag);
}

/*********************************************************************************************************
** ��������: natLanInfoGet
** ��������: ��ȡ LAN ��״̬��Ϣ(�� strtok�������̰߳�ȫ����)
** �䡡��  : pLanInfo          LAN ����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natLanInfoGet (__PLAN_INFO   pLanInfo)
{
    CHAR                 *pcBuf             = LW_NULL;
    CHAR                 *pcTmp             = LW_NULL;

    INT                   iRet;
    INT                   i                 = 0;
    UINT                  uilinenum         = 1;
    UINT                  ulineOffset       = 0;

    PCHAR                 pcFileName        = "/proc/net/nat/info";

    if (LW_NULL == pLanInfo) {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "LAN:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        pcTmp = pcBuf + ulineOffset + 4;                                /*  ��ȥ�ַ���ǰ��� LAN�� �ֶ� */
        pcTmp = compact_strtrim(pcTmp);                                 /*  ȥ�����ҿո�\n            */
        pcTmp = strtok(pcTmp, " ");
        while (LW_NULL != pcTmp) {
            bzero(pLanInfo->cLanName[i], sizeof(pLanInfo->cLanName[i]));
            strncpy(pLanInfo->cLanName[i], pcTmp, sizeof(pLanInfo->cLanName[i]));
            pcTmp = strtok(NULL, " ");
            i++;
        }
        free(pcBuf);                                                    /*  �ͷ��ڴ�                    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: natWanInfoGet
** ��������: ��ȡ WAN ��״̬��Ϣ(�� strtok�������̰߳�ȫ����)
** �䡡��  : pWanInfo          WAN ����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natWanInfoGet (__PWAN_INFO   pWanInfo)
{
    CHAR                 *pcBuf             = LW_NULL;
    CHAR                 *pcTmp             = LW_NULL;

    INT                   iRet;
    INT                   i                 = 0;
    UINT                  uilinenum         = 1;
    UINT                  ulineOffset       = 0;

    PCHAR                 pcFileName        = "/proc/net/nat/info";

    if (LW_NULL == pWanInfo) {
        return  (PX_ERROR);
    }

    iRet = specifiedStrFind(pcFileName, "WAN:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        pcTmp = pcBuf + ulineOffset + 4;                                /*  ��ȥ�ַ���ǰ��� WAN�� �ֶ� */
        pcTmp = compact_strtrim(pcTmp);                                 /*  ȥ�����ҿո�\n            */
        pcTmp = strtok(pcTmp, " ");
        while (LW_NULL != pcTmp) {
            bzero(pWanInfo->cWanName[i], sizeof(pWanInfo->cWanName[i]));
            strncpy(pWanInfo->cWanName[i], pcTmp, sizeof(pWanInfo->cWanName[i]));
            pcTmp = strtok(NULL, " ");
            i++;
        }
        free(pcBuf);                                                    /*  �ͷ��ڴ�                    */
    }

    return  (ERROR_NONE);
}
