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
** 文   件   名: natStatus.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 10 日
**
** 描        述: 系统 NAT 状态获取
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
** 函数名称: natAssNodeUseGet
** 功能描述: 获取 NAT 随机端口使用信息
** 输　入  : NONE
** 输　出  : pAssNodeInfo       端口使用信息
** 返　回  : ERROR_CODE
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
        pcTmp = pcBuf + ulineOffset + 15;                                /*  舍去字符串前面多余字段     */
        pAssNodeInfo->iNatAssNodeTotal = atoi(pcTmp);
    } else {
        return  (PX_ERROR);
    }

    free(pcBuf);
    pcBuf = LW_NULL;

    iRet = specifiedStrFind(cFileName, "Used  Ass-node:", &pcBuf, &uilinenum, &ulineOffset);
    if (-1 != iRet) {
        pcTmp = pcBuf + ulineOffset + 15;                                /*  舍去字符串前面多余字段     */
        pAssNodeInfo->iNatAssNodeUse = atoi(pcTmp);
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: natAliasItemNumGet
** 功能描述: 获取 natAlias 信息个数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : iNatAliasNum       别名条目个数
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
     *  找到端口映射的起始与结束位置
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

    iNatAliasNum = uilinenumEnd - uilinenumstart - 3;                   /*  减去中间无内容的 3 行       */

    return  (iNatAliasNum);
}

/*********************************************************************************************************
** 函数名称: natAliasInfoGet
** 功能描述: 获取 natAlias 信息
** 输　入  : pNatAliasBuff      网络别名信息
** **        iStartIndex        指定开始条目
**           iEndIndex          指定结束条目
** 输　出  : pNatAliasBuff      别名信息
** 返　回  : ERROR_CODE
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
     *  找到端口映射的起始与结束位置
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
        if (uiInfoNum < iStartIndex || uiInfoNum > iEndIndex) {         /*  需要进行结点信息的获取      */
            continue;
        }

        sBufLen   = 0;
        iRet = specifiedLineRead(pcFileName, i, &pcLineBuf, &sBufLen);
        if (-1 != iRet) {
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  去除左右空格及\n            */
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
                        free(pcLineBuf);                                /*  释放内存                    */

                        return  (PX_ERROR);
                    }
                    pcLineBuf = strtok(LW_NULL, " ");
                } else {                                                /*  还没解析完成出错            */
                    free(pcLineBuf);                                    /*  释放内存                    */

                    return  (PX_ERROR);
                }
            }

            pNatAliasBuff = (CHAR *)pNatAliasBuff + sizeof(__NATALIAS_INFO);
            pNatAliasInfo = (__PNATALIAS_INFO)(pNatAliasBuff);

        } else {
            free(pcLineBuf);                                            /*  释放内存                    */

            return  (PX_ERROR);
        }

        free(pcLineBuf);                                                /*  释放内存                    */
        pcLineBuf = LW_NULL;
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: natMaptemNumGet
** 功能描述: 获取 natmap 信息个数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : iNatMapNum         natmap 条目个数
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
     *  找到端口映射的起始与结束位置
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

    iNatMapNum = uilinenumEnd - uilinenumstart -3;                      /*  减去中间无内容的 3 行       */

    return  (iNatMapNum);
}

/*********************************************************************************************************
** 函数名称: natMapInfoGet
** 功能描述: 获取 natmap 映射信息
** 输　入  : pNatMapBuff        natmap buf
**           iStartIndex        指定开始条目
**           iEndIndex          指定结束条目
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
     *  找到端口映射的起始与结束位置
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
        if (uiInfoNum < iStartIndex || uiInfoNum > iEndIndex) {         /*  需要进行结点信息的获取      */
            continue;
        }
        sBufLen   = 0;
        iRet = specifiedLineRead(pcFileName, i, &pcLineBuf, &sBufLen);
        if (-1 != iRet) {
            pcLineBuf = compact_strtrim(pcLineBuf);                     /*  去除左右空格及\n            */
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
                        free(pcLineBuf);                                /*  释放内存                    */

                        return  (PX_ERROR);
                    }
                    pcLineBuf = strtok(LW_NULL, " ");
                } else {                                                /*  还没解析完成出错            */
                    free(pcLineBuf);                                    /*  释放内存                    */

                    return  (PX_ERROR);
                }
            }

            pNatMapBuff = (CHAR *)pNatMapBuff + sizeof(__NATMAP_INFO);
            pNatMapInfo = (__PNATMAP_INFO)(pNatMapBuff);

        } else {
            free(pcLineBuf);                                            /*  释放内存                    */

            return  (PX_ERROR);
        }

        free(pcLineBuf);                                                /*  释放内存                    */
        pcLineBuf = LW_NULL;
    }


    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: natIpFragGet
** 功能描述: 获取 ipfragment 状态信息
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : iIpfragmentflag    ipfragment 状态信息
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
** 函数名称: natLanInfoGet
** 功能描述: 获取 LAN 口状态信息(有 strtok，不是线程安全函数)
** 输　入  : pLanInfo          LAN 口信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        pcTmp = pcBuf + ulineOffset + 4;                                /*  舍去字符串前面的 LAN： 字段 */
        pcTmp = compact_strtrim(pcTmp);                                 /*  去除左右空格及\n            */
        pcTmp = strtok(pcTmp, " ");
        while (LW_NULL != pcTmp) {
            bzero(pLanInfo->cLanName[i], sizeof(pLanInfo->cLanName[i]));
            strncpy(pLanInfo->cLanName[i], pcTmp, sizeof(pLanInfo->cLanName[i]));
            pcTmp = strtok(NULL, " ");
            i++;
        }
        free(pcBuf);                                                    /*  释放内存                    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: natWanInfoGet
** 功能描述: 获取 WAN 口状态信息(有 strtok，不是线程安全函数)
** 输　入  : pWanInfo          WAN 口信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        pcTmp = pcBuf + ulineOffset + 4;                                /*  舍去字符串前面的 WAN： 字段 */
        pcTmp = compact_strtrim(pcTmp);                                 /*  去除左右空格及\n            */
        pcTmp = strtok(pcTmp, " ");
        while (LW_NULL != pcTmp) {
            bzero(pWanInfo->cWanName[i], sizeof(pWanInfo->cWanName[i]));
            strncpy(pWanInfo->cWanName[i], pcTmp, sizeof(pWanInfo->cWanName[i]));
            pcTmp = strtok(NULL, " ");
            i++;
        }
        free(pcBuf);                                                    /*  释放内存                    */
    }

    return  (ERROR_NONE);
}
