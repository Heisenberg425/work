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
** ��   ��   ��: routeExec.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: ��̬·�����ݿ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
#include <pthread.h>
#include "fileparser/fileparser.h"
#include "libroute_config.h"
#include "route_common.h"
#include "intadaption/intada_common.h"
#include "intadaption/intAdaptionExec.h"
#include "vnd/vndDB.h"
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define CMD_BUFF_LEN     (256)                                          /*  route �����������        */
/*********************************************************************************************************
** ��������: __routeInfoGet
** ��������: ͨ��route�����ѯ��ǰ·�����ϵ�·����Ϣ
** �䡡��  : cFileName              route��Ϣ���ɵ���ʱ�ļ���
**           pRouteHandle           ·�ɲ�ѯ����
**           cpTableHead            ·����Ϣ�зָ��ͷ
**           cpTableTail            ·����Ϣ�зָ��β
** �䡡��  : piRecordNum            ��������ѯ����·������
** ������  : PVOID                  ·����Ϣ����ָ��
*********************************************************************************************************/
static PVOID  __routeInfoGet (PCHAR            cFileName,
                              __PROUTE_HANDLE  pRouteHandle,
                              INT             *piRecordNum,
                              PCHAR            cpTableHead,
                              PCHAR            cpTableTail)
{
    INT                  iRet;
    PCHAR                pcBuf          = LW_NULL;
    PCHAR                pcLineBuf      = LW_NULL;
    UINT                 uiLineNumStart = 0;
    UINT                 uiLineNumEnd   = 0;
    UINT                 uiLineOffset   = 0;
    size_t               sBufLen        = 0;
    __PROUTE_HANDLE      pRouteNode     = LW_NULL;
    __PROUTE_HANDLE      pTmpNode       = LW_NULL;
    INT                  i, j;
    INT                  k              = 0;

    /*
     *  �ҵ����ݵ���ʼ�����λ��
     */
    iRet = specifiedStrFind(cFileName, cpTableHead, &pcBuf, &uiLineNumStart, &uiLineOffset);
    if (-1 != iRet) {
        free(pcBuf);
    } else {
        return  (LW_NULL);
    }

    iRet = specifiedStrFind(cFileName, cpTableTail, &pcBuf, &uiLineNumEnd, &uiLineOffset);
    if (-1 == iRet) {
        return  (LW_NULL);
    }

    free(pcBuf);

    *piRecordNum = (uiLineNumEnd - 1) - (uiLineNumStart + 2);
    pRouteNode   = (__PROUTE_HANDLE)malloc((*piRecordNum) * sizeof(__ROUTE_HANDLE));

    if (LW_NULL == pRouteNode) {
        return  (LW_NULL);
    }

    bzero(pRouteNode, (*piRecordNum)* sizeof(__ROUTE_HANDLE));

    for (i = uiLineNumStart + 2, k = 0; i < uiLineNumEnd - 1; i++, k++)  {
        pcLineBuf = LW_NULL;
        sBufLen   = 0;
        pTmpNode = pRouteNode + k;
        iRet = specifiedLineRead(cFileName, i, &pcLineBuf, &sBufLen);
        if (-1 != iRet) {
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  ȥ�����ҿո�\n            */
            pcLineBuf = strtok(pcLineBuf, " ");
            for (j = 0; j < 8; j++) {
                if (LW_NULL != pcLineBuf) {
                    switch (j) {
                    case 0:
                        strncpy(pTmpNode->cDestination, pcLineBuf, sizeof(pTmpNode->cDestination));

                        break;
                    case 1:
                        strncpy(pTmpNode->cGateway, pcLineBuf, sizeof(pTmpNode->cGateway));

                        break;
                    case 2:
                        if ( 0 == strcmp(pRouteHandle->cIpFamily, IPv4)) {
                            strncpy(pTmpNode->cGenmask, pcLineBuf, sizeof(pTmpNode->cGenmask));
                        } else {
                            pTmpNode->iPrefix = atoi(pcLineBuf);
                        }

                        break;
                    case 3:
                        strncpy(pTmpNode->cFlags, pcLineBuf, sizeof(pTmpNode->cFlags));

                        break;
                    case 4:
                        pTmpNode->iMetric = atoi(pcLineBuf);

                        break;
                    case 5:
                        pTmpNode->iRef = atoi(pcLineBuf);

                        break;
                    case 6:
                        pTmpNode->iUse = atoi(pcLineBuf);

                        break;
                    case 7:
                        strncpy(pTmpNode->cIface,  pcLineBuf, sizeof(pTmpNode->cIface));

                        break;
                    default:
                        free(pcLineBuf);

                        return  (LW_NULL);
                    }
                    pcLineBuf = strtok(LW_NULL, " ");
                } else {
                    free(pcLineBuf);

                    return  (LW_NULL);
                }
            }

            strncpy(pTmpNode->cIpFamily, pRouteHandle->cIpFamily, sizeof(pTmpNode->cIpFamily));
        } else {
            free(pcLineBuf);

            return  (LW_NULL);
        }

        free(pcLineBuf);
    }

    free(pcLineBuf);

    return  (pRouteNode);
}
/*********************************************************************************************************
** ��������: updateRouteItem
** ��������: ��route�����ѯ�Ľ�����½��������ݿ���е�·����Ϣ
** �䡡��  : pRouteHandle ·�ɲ�����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  updateRouteItem (pthread_t   tid, __PROUTE_HANDLE  pRouteHandle)
{
    char     *pcBuf         = LW_NULL;
    char     *pcLineBuf     = LW_NULL;
    INT      iRet           = 0;
    UINT     ulinenumstart  = 0;
    UINT     ulineOffset    = 0;
    size_t   sBufLen        = 0;
    INT      j;
    CHAR     cFileName[50]  = {0};
    CHAR     cCatCmd[80]    = {0};

    snprintf(cFileName, sizeof(cFileName), "/tmp/route_info_update");
    snprintf(cCatCmd, sizeof(cCatCmd),"route >%s", cFileName);
    system(cCatCmd);

    iRet = specifiedStrFind(cFileName, pRouteHandle->cDestination, &pcBuf, &ulinenumstart, &ulineOffset);
    if (-1 == iRet) {
        return;
    }

    free(pcBuf);

    iRet      = specifiedLineRead(cFileName, ulinenumstart, &pcLineBuf, &sBufLen);
    pcLineBuf = compact_strtrim(pcLineBuf);                             /*  ȥ���ո񼰻س���            */
    pcLineBuf = strtok(pcLineBuf, " ");

    for (j = 0; j < 8; j++) {
        if (LW_NULL != pcLineBuf) {
            switch(j){
            case 3:
                strncpy(pRouteHandle->cFlags,  pcLineBuf, sizeof(pRouteHandle->cFlags));
                break;

            case 4:
                pRouteHandle->iMetric = atoi(pcLineBuf);
                break;

            default:
                break;
            }

            pcLineBuf = strtok(LW_NULL, " ");
        }
    }

    free(pcLineBuf);
}
/*********************************************************************************************************
** ��������: __invalidRouteInfoRemove
** ��������: ȥ��·�ɱ��е���Ч������Ϣ
** �䡡��  : pSrcRouteInfo     ·�ɲ�����Ϣ
**           iSrcRecordNum     ·����Ϣ����
**           iNewRecordNum     ��Ч·�ɸ���ָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
PVOID  __invalidRouteInfoRemove (__PROUTE_HANDLE  pSrcRouteInfo,
                                 INT              iSrcRecordNum,
                                 INT             *piNewRecordNum)
{
    INT              i;
    INT              iRet;
    INT              iNum            = 0;
    INT              iStatus         = 0;
    __PROUTE_HANDLE  pRouteInfo      = LW_NULL;
    __PROUTE_HANDLE  pValidRouteInfo = LW_NULL;
    __PROUTE_HANDLE  pValidRouteTmp  = LW_NULL;

    *piNewRecordNum = 0;

    if (NULL == pSrcRouteInfo) {
        return  (NULL);
    }

    for (i = 0; i < iSrcRecordNum; i++) {
        pRouteInfo = pSrcRouteInfo + i;

        if (strstr(pRouteInfo->cIface, "en")) {
            iRet = intadaStatusGetFromDbByName(pRouteInfo->cIface, &iStatus);
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  �����Ӳ��������Ч          */
                iNum++;
            } else {
                continue;
            }
        } else {                                                        /*  ��Ӳ�����ڣ�����������      */
            iNum++;
        }
    }

    pValidRouteInfo = (__PROUTE_HANDLE)malloc(iNum * sizeof(__ROUTE_HANDLE));
    if (NULL == pValidRouteInfo) {
        return  (NULL);
    }

    pValidRouteTmp = pValidRouteInfo;

    for (i = 0; i < iSrcRecordNum; i++) {
        pRouteInfo = pSrcRouteInfo + i;

        if (strstr(pRouteInfo->cIface, "en")) {
            iRet = intadaStatusGetFromDbByName(pRouteInfo->cIface, &iStatus);
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  �����Ӳ��������Ч          */
                memcpy(pValidRouteTmp, pRouteInfo, sizeof(__ROUTE_HANDLE));

                pValidRouteTmp += 1;
            } else {
                continue;
            }
        } else {
            memcpy(pValidRouteTmp, pRouteInfo, sizeof(__ROUTE_HANDLE));

            pValidRouteTmp += 1;
        }
    }

    *piNewRecordNum = iNum;

    return  (pValidRouteInfo);
}
/*********************************************************************************************************
** ��������: routeDoShow
** ��������: ��ѯ·�ɱ�
** �䡡��  : PROUTE_HANDLE  pRouteInfo ·�ɲ�����Ϣ
** �䡡��  : piRecordNum        ·��������
** ������  : ·�ɱ���Ϣ
*********************************************************************************************************/
PVOID  routeDoShow (__PROUTE_HANDLE  pRouteHandle, INT  *piRecordNum)
{
    __PROUTE_HANDLE  pRouteInfo         = LW_NULL;
    __PROUTE_HANDLE  pRouteArrayHead    = LW_NULL;
    CHAR             cFileName[50]      = {0};
    CHAR             cCatCmd[80]        = {0};
    INT              iRecordNum         = 0;

    snprintf(cFileName, sizeof(cFileName), "/tmp/route_info");
    snprintf(cCatCmd, sizeof(cCatCmd), "route >%s", cFileName);
    system(cCatCmd);                                                    /*  ��ȡϵͳ route ��Ϣ         */

    if ((0 == strcmp(pRouteHandle->cIpFamily, IPv4)) &&
        (0 == strcmp(pRouteHandle->cRouteType, STATIC_ROUTE))) {        /*  ��ȡ��̬·�ɱ�              */
        pRouteInfo = routeGetFromDB(&iRecordNum);                       /*  ֱ�Ӵ� ���ݿ� ��ȡ          */

    } else if (strcmp(pRouteHandle->cIpFamily, IPv4) == 0) {            /*  ��ȡ IPv4 ·�ɱ�            */
        pRouteInfo = __routeInfoGet(cFileName, pRouteHandle,
                                    &iRecordNum, IPv4_ROUTE_TABLE_HEAD,
                                    IPv6_ROUTE_TABLE_HEAD);

    } else if (strcmp(pRouteHandle->cIpFamily, IPv6) == 0) {            /*  ��ȡ IPv6 ·�ɱ�            */
        pRouteInfo = __routeInfoGet(cFileName, pRouteHandle,
                                    &iRecordNum, IPv6_ROUTE_TABLE_HEAD,
                                    AODV_ROUTE_TABLE_HEAD);
    }

    if (NULL != pRouteInfo && iRecordNum > 0) {                         /*  ɾ����Ч��Ϣ                */
        pRouteArrayHead = __invalidRouteInfoRemove(pRouteInfo, iRecordNum, piRecordNum);
    }

    return  (pRouteArrayHead);
}
/*********************************************************************************************************
** ��������: routeDoConfig
** ��������: ����·�ɣ����ӻ��޸Ļ�ɾ��һ����̬·��
** �䡡��  : pRouteInfo ·�ɲ�����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  routeDoConfig (__PROUTE_HANDLE  pRouteInfo)
{
    INT    iRet;
    CHAR   cCmdTmp[CMD_BUFF_LEN] = {0};
    PCHAR  pcIfaceName;
    CHAR   cIface[NETIF_NAMESIZE];
    INT    iStrlen;
    INT    iRestLength;
    INT    iVndid;
    INT    iIfIndex;

    if (LW_NULL == pRouteInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "config route parameter is null.\n");

       return  (PX_ERROR);
    }

    iStrlen = snprintf(cCmdTmp, sizeof(cCmdTmp), "route %s ", pRouteInfo->cOperType);

    /*
     * ���Ĭ��·�ɴ���
     */
    if (0 == strcmp(pRouteInfo->cRouteType, DEFAULT_ROUTE)) {
        if (0 != strcmp(pRouteInfo->cOperType, ROUTE_DEL)) {
            iRestLength = CMD_BUFF_LEN - iStrlen;
            iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength , "%s", pRouteInfo->cRouteType);
        }

        if (pRouteInfo->cGateway[0] !='\0' &&
            strlen(compact_strtrim(pRouteInfo->cGateway)) > 0 ) {
            iRestLength = CMD_BUFF_LEN - iStrlen;
            iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " gw %s", pRouteInfo->cGateway);
        }

        if (pRouteInfo->cIface[0] !='\0' &&
            strlen(compact_strtrim(pRouteInfo->cIface)) > 0 ) {
            pcIfaceName = pRouteInfo->cIface;
            if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {
                iVndid = atoi(pcIfaceName + 4);

                iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                if (PX_ERROR == iRet) {
                    return  (PX_ERROR);
                }

                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);

            } else {

                /*
                 *  ��ȡ��Ӧ��ϵͳ��������
                 */
                iRet = intAdaptionGetNetif(pcIfaceName, cIface);
                if (PX_ERROR == iRet) {
                    return  (PX_ERROR);
                }

                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev %s", cIface);
            }
        }

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute command %s.\n",cCmdTmp);

            return  (iRet);
        }

        /*
         * ���豸��Ĭ������·�ɱ���һ��
         */
        strncpy(pRouteInfo->cDestination, "0.0.0.0", sizeof(pRouteInfo->cDestination));
        strncpy(pRouteInfo->cGenmask, "0.0.0.0", sizeof(pRouteInfo->cGenmask));
        updateRouteItem(pthread_self(), pRouteInfo);

        iRet = routeUpdateToDb(ROUTE_CHG, pRouteInfo);

        return  (iRet);
    }

    iRestLength = CMD_BUFF_LEN - iStrlen;
    iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, "%s ", pRouteInfo->cRouteType);

    if (pRouteInfo->cDestination[0] != '\0') {
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " %s", pRouteInfo->cDestination);
    }

    if (pRouteInfo->cGenmask[0] !='\0') {
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " mask %s", pRouteInfo->cGenmask);
    }

    if (pRouteInfo->cGateway[0] !='\0') {
        iRestLength = CMD_BUFF_LEN - iStrlen;
        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " %s", pRouteInfo->cGateway);
    }

    if (0 != strcmp(pRouteInfo->cOperType, ROUTE_DEL)) {
        if (pRouteInfo->iMetric != 0 ) {
            iRestLength = CMD_BUFF_LEN - iStrlen;
            iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " metric %d", pRouteInfo->iMetric);
        }

        if (pRouteInfo->cIface[0] !='\0') {
            pcIfaceName = pRouteInfo->cIface;
            if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {
                iVndid = atoi(pcIfaceName + 4);
                iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                if (PX_ERROR == iRet) {
                    return  (PX_ERROR);
                }

                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);

            } else {

                /*
                 *  ��ȡ��Ӧ��ϵͳ��������
                 */
                iRet = intAdaptionGetNetif(pcIfaceName, cIface);
                if (PX_ERROR == iRet) {
                    return  (PX_ERROR);
                }

                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev %s", cIface);
            }
        }
    }

    iRet = system(cCmdTmp);
    if (ERROR_NONE != iRet) {
         __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute recovery command = %s, "
                                               "error_code=%d.\n",cCmdTmp, iRet);
         return  (iRet);
    }

    if (0 == strcmp(pRouteInfo->cOperType, ROUTE_ADD)) {
        updateRouteItem(pthread_self(), pRouteInfo);
    }

    return  routeUpdateToDb(pRouteInfo->cOperType, pRouteInfo);
}
/*********************************************************************************************************
** ��������: routeRecovery
** ��������: ��̬·�ɻָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  routeRecovery (VOID)
{
    __PROUTE_HANDLE  pRouteHeader          = LW_NULL;
    __PROUTE_HANDLE  pTmpRoute             = LW_NULL;
    PCHAR            pcIfaceName;
    INT              iVndid;
    INT              iIfIndex;
    CHAR             cIface[NETIF_NAMESIZE];
    CHAR             cCmdTmp[CMD_BUFF_LEN] = {0};
    INT              iStrlen               = 0;
    INT              iRet                  = 0;
    INT              iRecordNum            = 0;
    INT              i                     = 0;
    INT              iRestLength           = 0;

    pRouteHeader = routeGetFromDB(&iRecordNum);                         /*  �����ݿ��л�ȡ·����Ϣ      */
    if ((LW_NULL == pRouteHeader) && (-1 == iRecordNum)) {
        return  (PX_ERROR);
    }

    for (i = 0; i < iRecordNum; i++) {                                  /*  ���б���ÿһ����Ϣ          */
        bzero(cCmdTmp, sizeof(cCmdTmp));
        pTmpRoute = pRouteHeader + i;

        if (0 == strcmp(pTmpRoute->cRouteType, DEFAULT_ROUTE)) {        /*  ����Ǿ�̬·��              */
            iStrlen = snprintf(cCmdTmp, sizeof(cCmdTmp), "route add default ");
            if (pTmpRoute->cGateway[0] !='\0' ) {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " gw %s", pTmpRoute->cGateway);
            }

            if (pTmpRoute->cIface[0] !='\0') {                          /*  ������� cIface ��Ϣ        */
                pcIfaceName = pTmpRoute->cIface;
                if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {       /*  �������������              */
                    iVndid = atoi(pcIfaceName + 4);
                    iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                    if (ERROR_NONE == iRet) {                           /*  ���������������Ч          */
                        iRestLength = CMD_BUFF_LEN - iStrlen;           /*  ����ʱָ����������          */
                        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);

                    } else {                                            /*  ����ʱ������Ӹ�·��        */
                        iRet = routeUpdateToDb(ROUTE_DEL, pTmpRoute);   /*  ɾ����Ӧ����Ŀ              */
                        if (PX_ERROR == iRet) {
                            return  (PX_ERROR);
                        }

                        continue;                                       /*  ��������                    */
                    }

                } else {                                                /*  �������������              */

                    iRet = intAdaptionGetNetif(pcIfaceName, cIface);    /*  ��ȡ��Ӧ��ϵͳ��������      */
                    if (PX_ERROR == iRet) {
                        return  (PX_ERROR);
                    }

                    iRestLength = CMD_BUFF_LEN - iStrlen;
                    iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev %s", cIface);
                }
            }

        } else {                                                        /*  ������Ǿ�̬·��           */
            iStrlen = snprintf(cCmdTmp, sizeof (cCmdTmp), "route add %s ", pTmpRoute->cRouteType);

            if (pTmpRoute->cDestination[0] != '\0') {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " %s", pTmpRoute->cDestination);
            }

            if (pTmpRoute->cGenmask[0] !='\0') {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " mask %s", pTmpRoute->cGenmask);
            }

            if (pTmpRoute->cGateway[0] !='\0') {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " %s", pTmpRoute->cGateway);
            }
            if (pTmpRoute->iMetric != 0) {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " metric %d", pTmpRoute->iMetric);
            }

            if (pTmpRoute->cIface[0] !='\0') {                          /*  ������� cIface ��Ϣ        */
                pcIfaceName = pTmpRoute->cIface;
                if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {       /*  �������������              */
                    iVndid = atoi(pcIfaceName + 4);
                    iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                    if (ERROR_NONE == iRet) {                           /*  ���������������Ч          */
                        iRestLength = CMD_BUFF_LEN - iStrlen;           /*  ����ʱָ����������          */
                        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);
                    } else {                                            /*  ����ʱ������Ӹ�·��        */
                        iRet = routeUpdateToDb(ROUTE_DEL, pTmpRoute);   /*  ɾ����Ӧ����Ŀ              */
                        if (PX_ERROR == iRet) {
                            return  (PX_ERROR);
                        }

                        continue;                                       /*  ��������                    */
                    }

                } else {                                                /*  �������������              */

                    iRet = intAdaptionGetNetif(pcIfaceName, cIface);    /*  ��ȡ��Ӧ��ϵͳ��������      */
                    if (PX_ERROR == iRet) {
                        return  (PX_ERROR);
                    }

                    iRestLength = CMD_BUFF_LEN - iStrlen;
                    iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev %s", cIface);
                }
            }
        }

        iRet = system(cCmdTmp);

        if (ERROR_NONE != iRet){
             __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute recovery command = %s, "
                                                   "error_code=%d.\n", cCmdTmp, iRet);
        }
    }

    free(pRouteHeader);

    return  (iRet);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
