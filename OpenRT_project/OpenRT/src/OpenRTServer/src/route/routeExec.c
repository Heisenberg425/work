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
** 文   件   名: routeExec.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 静态路由数据库操作
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
  宏定义
*********************************************************************************************************/
#define CMD_BUFF_LEN     (256)                                          /*  route 命令缓冲区长度        */
/*********************************************************************************************************
** 函数名称: __routeInfoGet
** 功能描述: 通过route命令查询当前路由器上的路由信息
** 输　入  : cFileName              route信息生成的临时文件名
**           pRouteHandle           路由查询条件
**           cpTableHead            路由信息中分割表头
**           cpTableTail            路由信息中分割表尾
** 输　出  : piRecordNum            按条件查询到的路由条数
** 返　回  : PVOID                  路由信息数组指针
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
     *  找到数据的起始与结束位置
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
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  去除左右空格及\n            */
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
** 函数名称: updateRouteItem
** 功能描述: 用route命令查询的结果更新将插入数据库表中的路由信息
** 输　入  : pRouteHandle 路由参数信息
** 输　出  : NONE
** 返　回  : NONE
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
    pcLineBuf = compact_strtrim(pcLineBuf);                             /*  去除空格及回车符            */
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
** 函数名称: __invalidRouteInfoRemove
** 功能描述: 去除路由表中的无效网口信息
** 输　入  : pSrcRouteInfo     路由参数信息
**           iSrcRecordNum     路由信息个数
**           iNewRecordNum     有效路由个数指针
** 输　出  : NONE
** 返　回  : NONE
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
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  如果该硬件网口有效          */
                iNum++;
            } else {
                continue;
            }
        } else {                                                        /*  非硬件网口，如虚拟网口      */
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
            if (ERROR_NONE == iRet && 1 == iStatus) {                   /*  如果该硬件网口有效          */
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
** 函数名称: routeDoShow
** 功能描述: 查询路由表
** 输　入  : PROUTE_HANDLE  pRouteInfo 路由参数信息
** 输　出  : piRecordNum        路由总条数
** 返　回  : 路由表信息
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
    system(cCatCmd);                                                    /*  获取系统 route 信息         */

    if ((0 == strcmp(pRouteHandle->cIpFamily, IPv4)) &&
        (0 == strcmp(pRouteHandle->cRouteType, STATIC_ROUTE))) {        /*  获取静态路由表              */
        pRouteInfo = routeGetFromDB(&iRecordNum);                       /*  直接从 数据库 获取          */

    } else if (strcmp(pRouteHandle->cIpFamily, IPv4) == 0) {            /*  获取 IPv4 路由表            */
        pRouteInfo = __routeInfoGet(cFileName, pRouteHandle,
                                    &iRecordNum, IPv4_ROUTE_TABLE_HEAD,
                                    IPv6_ROUTE_TABLE_HEAD);

    } else if (strcmp(pRouteHandle->cIpFamily, IPv6) == 0) {            /*  获取 IPv6 路由表            */
        pRouteInfo = __routeInfoGet(cFileName, pRouteHandle,
                                    &iRecordNum, IPv6_ROUTE_TABLE_HEAD,
                                    AODV_ROUTE_TABLE_HEAD);
    }

    if (NULL != pRouteInfo && iRecordNum > 0) {                         /*  删除无效信息                */
        pRouteArrayHead = __invalidRouteInfoRemove(pRouteInfo, iRecordNum, piRecordNum);
    }

    return  (pRouteArrayHead);
}
/*********************************************************************************************************
** 函数名称: routeDoConfig
** 功能描述: 配置路由，增加或修改或删除一条静态路由
** 输　入  : pRouteInfo 路由参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     * 添加默认路由处理
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
                 *  获取对应的系统网口名称
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
         * 与设备上默认网关路由保持一致
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
                 *  获取对应的系统网口名称
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
** 函数名称: routeRecovery
** 功能描述: 静态路由恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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

    pRouteHeader = routeGetFromDB(&iRecordNum);                         /*  从数据库中获取路由信息      */
    if ((LW_NULL == pRouteHeader) && (-1 == iRecordNum)) {
        return  (PX_ERROR);
    }

    for (i = 0; i < iRecordNum; i++) {                                  /*  逐行遍历每一条信息          */
        bzero(cCmdTmp, sizeof(cCmdTmp));
        pTmpRoute = pRouteHeader + i;

        if (0 == strcmp(pTmpRoute->cRouteType, DEFAULT_ROUTE)) {        /*  如果是静态路由              */
            iStrlen = snprintf(cCmdTmp, sizeof(cCmdTmp), "route add default ");
            if (pTmpRoute->cGateway[0] !='\0' ) {
                iRestLength = CMD_BUFF_LEN - iStrlen;
                iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " gw %s", pTmpRoute->cGateway);
            }

            if (pTmpRoute->cIface[0] !='\0') {                          /*  如果包含 cIface 信息        */
                pcIfaceName = pTmpRoute->cIface;
                if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {       /*  如果是虚拟网卡              */
                    iVndid = atoi(pcIfaceName + 4);
                    iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                    if (ERROR_NONE == iRet) {                           /*  如果虚拟网卡还有效          */
                        iRestLength = CMD_BUFF_LEN - iStrlen;           /*  建立时指定虚拟网卡          */
                        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);

                    } else {                                            /*  出错时，不添加该路由        */
                        iRet = routeUpdateToDb(ROUTE_DEL, pTmpRoute);   /*  删除对应的条目              */
                        if (PX_ERROR == iRet) {
                            return  (PX_ERROR);
                        }

                        continue;                                       /*  跳过该条                    */
                    }

                } else {                                                /*  如果是物理网卡              */

                    iRet = intAdaptionGetNetif(pcIfaceName, cIface);    /*  获取对应的系统网口名称      */
                    if (PX_ERROR == iRet) {
                        return  (PX_ERROR);
                    }

                    iRestLength = CMD_BUFF_LEN - iStrlen;
                    iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev %s", cIface);
                }
            }

        } else {                                                        /*  如果不是静态路由           */
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

            if (pTmpRoute->cIface[0] !='\0') {                          /*  如果包含 cIface 信息        */
                pcIfaceName = pTmpRoute->cIface;
                if (LW_NULL != lib_strstr(pcIfaceName, "VPN-")) {       /*  如果是虚拟网卡              */
                    iVndid = atoi(pcIfaceName + 4);
                    iRet = netIfIndexGetByVndId(iVndid, &iIfIndex);
                    if (ERROR_NONE == iRet) {                           /*  如果虚拟网卡还有效          */
                        iRestLength = CMD_BUFF_LEN - iStrlen;           /*  建立时指定虚拟网卡          */
                        iStrlen    += snprintf(cCmdTmp + iStrlen, iRestLength, " dev vn%d", iIfIndex);
                    } else {                                            /*  出错时，不添加该路由        */
                        iRet = routeUpdateToDb(ROUTE_DEL, pTmpRoute);   /*  删除对应的条目              */
                        if (PX_ERROR == iRet) {
                            return  (PX_ERROR);
                        }

                        continue;                                       /*  跳过该条                    */
                    }

                } else {                                                /*  如果是物理网卡              */

                    iRet = intAdaptionGetNetif(pcIfaceName, cIface);    /*  获取对应的系统网口名称      */
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
