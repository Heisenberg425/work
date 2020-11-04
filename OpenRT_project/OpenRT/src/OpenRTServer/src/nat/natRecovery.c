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
** 文   件   名: natRecovery.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述:  NAT 恢复操作
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nat_common.h"
#include "natDB.h"
#include "common.h"
/*********************************************************************************************************
  用于开启 NAT 功能的空网口
*********************************************************************************************************/
#define IDEL_WAN_LAN_NAME      "null"                                   /*  空闲网口名                  */

extern VOID tmNotifyWanChange (PVOID  pvWan);
extern VOID tmNotifyLanChange (PVOID  pvLan);
/*********************************************************************************************************
** 函数名称: __natLanRecovry
** 功能描述: 恢复 LAN 口状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID __natLanRecovry (VOID)
{
    CHAR             cCmdTmp[20] = {0};
    __LAN_INFO       pLan;
    INT              i;

    bzero(&pLan, sizeof(pLan));
    natLanGetFromDB(&pLan);

    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  遍历链表                    */

        /*
         *  去掉 null 网口
         */
        if (0 != strcmp(pLan.cLanName[i], IDEL_WAN_LAN_NAME) && strlen(pLan.cLanName[i])) {
            bzero(cCmdTmp, sizeof(cCmdTmp));
            sprintf(cCmdTmp, "natlocal %s", pLan.cLanName[i]);
            system(cCmdTmp);
        }

    }

    bzero(&pLan, sizeof(pLan));
    natLanInfoGet(&pLan);
    tmNotifyLanChange(&pLan);
}

/*********************************************************************************************************
** 函数名称: __natWanRecovry
** 功能描述: 恢复 WAN 口状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID __natWanRecovry (VOID)
{
    CHAR             cCmdTmp[32]    = {0};
    CHAR             cRouteCmd[128] = {0};
    __WAN_INFO       pWan;
    INT              i;

    bzero(&pWan, sizeof(pWan));
    natWanGetFromDB(&pWan);

    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  遍历链表                    */

        /*
         *  去掉 null 网口
         */
        if (0 != strcmp(pWan.cWanName[i], IDEL_WAN_LAN_NAME) && strlen(pWan.cWanName[i])) {
            bzero(cCmdTmp, sizeof(cCmdTmp));
            bzero(cRouteCmd, sizeof(cRouteCmd));

            snprintf(cCmdTmp, sizeof(cCmdTmp), "natwan %s", pWan.cWanName[i]);
            snprintf(cRouteCmd, sizeof(cRouteCmd), "route add default dev %s", pWan.cWanName[i]);

            system(cCmdTmp);                                            /*  设置 WAN                    */
            system(cRouteCmd);                                          /*  设置网口路由                */
        }
    }

    bzero(&pWan, sizeof(pWan));
    natWanInfoGet(&pWan);
    tmNotifyWanChange(&pWan);
}

/*********************************************************************************************************
** 函数名称: __natMapRecovry
** 功能描述: 端口映射恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID __natMapRecovry (VOID)
{
    CHAR             cCmdTmp[80] = {0};
    PCHAR            pcNatMapBuf = LW_NULL;
    __PNATMAP_INFO   pNatMapInfo = LW_NULL;
    __PNATMAP_INFO   pNatMapTmp  = LW_NULL;
    INT              iNatMapNum  = 0;
    INT              i;

    iNatMapNum = natMapGetDBRow();
    if (0 == iNatMapNum) {
        return;
    }

    pcNatMapBuf = natMapGetDBInfo();
    if (LW_NULL == pcNatMapBuf) {

        return;
    }

    pNatMapTmp = (__PNATMAP_INFO)pcNatMapBuf;

    pNatMapInfo = (__PNATMAP_INFO)pcNatMapBuf;
    for (i = 0; i < iNatMapNum; i++) {                                  /*  遍历链表                    */
        bzero(cCmdTmp, sizeof(cCmdTmp));
        sprintf(cCmdTmp, "natmap add %d %d %s %s %d",
                pNatMapInfo->iWanPort,
                pNatMapInfo->iLanPort,
                pNatMapInfo->cIpString,
                pNatMapInfo->cProtoString,
                pNatMapInfo->iIpCnt);
        system(cCmdTmp);

        pcNatMapBuf = pcNatMapBuf + sizeof(__NATMAP_INFO);
        pNatMapInfo = (__PNATMAP_INFO)(pcNatMapBuf);
    }
    if (NULL != pNatMapTmp) {
        free(pNatMapTmp);
    }
}

/*********************************************************************************************************
** 函数名称: __natAliasRecovry
** 功能描述: 网络别名恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID __natAliasRecovry (VOID)
{
    CHAR              cCmdTmp[100]      = {0};
    __PNATALIAS_INFO  pNatAliasInfo     = LW_NULL;
    __PNATALIAS_INFO  pNatAliasTmp      = LW_NULL;
    PCHAR             pcNatAliasInfoBuf = LW_NULL;
    INT               iNatAliasNum      = 0;
    INT               i;

    iNatAliasNum = natAliasGetDBRow();
    if (0 == iNatAliasNum) {
        return;
    }

    pcNatAliasInfoBuf = natAliasGetDBInfo();
    if (LW_NULL == pcNatAliasInfoBuf) {
        return;
    }

    pNatAliasTmp  = (__PNATALIAS_INFO)pcNatAliasInfoBuf;
    pNatAliasInfo = (__PNATALIAS_INFO)pcNatAliasInfoBuf;
    for (i = 0; i < iNatAliasNum; i++) {                               /*  遍历链表                    */
        bzero(cCmdTmp, sizeof(cCmdTmp));
        sprintf(cCmdTmp, "natalias add %s %s %s",
                pNatAliasInfo->cAliasString,
                pNatAliasInfo->cLoaclSString,
                pNatAliasInfo->cLoaclEString);
        system(cCmdTmp);

        pcNatAliasInfoBuf = (CHAR *)pcNatAliasInfoBuf + sizeof(__NATALIAS_INFO);
        pNatAliasInfo = (__PNATALIAS_INFO)(pcNatAliasInfoBuf);
    }

    if (NULL != pNatAliasTmp) {
        free(pNatAliasTmp);
    }
}

/*********************************************************************************************************
** 函数名称: __natAliasRecovry
** 功能描述: 分片包恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID __natIpFragRecovry (VOID)
{
    INT   iNatIpFragInfo  = 0;

    iNatIpFragInfo = natIpFragGetFromDB();
    if (PX_ERROR == iNatIpFragInfo) {
        return;
    }

    if (iNatIpFragInfo & 0x01) {
        system("natipfrag tcp 1");
    }

    if (iNatIpFragInfo & 0x02) {
        system("natipfrag udp 1");
    }

    if (iNatIpFragInfo & 0x04) {
        system("natipfrag icmp 1");
    }
}

/*********************************************************************************************************
** 函数名称: natRecovry
** 功能描述: 恢复 NAT 所有功能
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID natRecovry (VOID)
{
    __natWanRecovry();
    __natLanRecovry();
    __natMapRecovry();
    __natAliasRecovry();
    __natIpFragRecovry();
}
