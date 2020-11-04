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
** 文   件   名: natExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述:  NAT 配置实现
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "natStatus.h"
#include "natDB.h"
#include "nat_common.h"
#include "common.h"
/*********************************************************************************************************
  用于开启 NAT 功能的空网口
*********************************************************************************************************/
#define IDEL_WAN_LAN_NAME      "null"                                   /*  空闲网口名                  */
/*********************************************************************************************************
** 函数名称: natCfgInit
** 功能描述: NAT 初始化
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  natCfgInit (VOID)
{
    /*
     *  传入一个 空 的 WAN LAN 口，开启 NAT 功能(用这种方式解决后面添加 WAN、LAN需要先开启 NAT的问题)
     */
    system("nat stop");

    return  inetNatStart(IDEL_WAN_LAN_NAME, IDEL_WAN_LAN_NAME);
}
extern VOID tmNotifyWanChange (PVOID  pvWan);
extern VOID tmNotifyLanChange (PVOID  pvLan);
/*********************************************************************************************************
** 函数名称: wanSetExistFlagCheck
** 功能描述: 检测 WAN 设置标志
** 输　入  : wanInfo            WAN 信息
**           cpcWanName         WAN 口名称
** 输　出  : NONE
** 返　回  : PX_ERROR           已设置
**           ERROR_NONE         未设置
*********************************************************************************************************/
INT  wanSetExistFlagCheck (__WAN_INFO   wanInfo, CPCHAR  cpcWanName)
{
    INT           iNum;

    for (iNum = 0; iNum < LW_CFG_NET_NAT_MAX_AP_IF; iNum++) {
        if (0 == strcmp((CPCHAR)wanInfo.cWanName[iNum], cpcWanName)) {
           return  (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: lanSetExistFlagCheck
** 功能描述: 检测 LAN 设置标志
** 输　入  : lanInfo            LAN 信息
**           cpcLanName         LAN 口名称
** 输　出  : NONE
** 返　回  : PX_ERROR           已设置
**           ERROR_NONE         未设置
*********************************************************************************************************/
INT  lanSetExistFlagCheck (__LAN_INFO   lanInfo, CPCHAR  cpcLanName)
{
    INT           iNum;

    for (iNum = 0; iNum < LW_CFG_NET_NAT_MAX_LOCAL_IF; iNum++) {
        if (0 == strcmp((CPCHAR)lanInfo.cLanName[iNum], cpcLanName)) {
           return  (ERROR_NONE);
        }
    }

    return (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: natLanCfg
** 功能描述: 配置 LAN 口状态
** 输　入  : cpcLanName         LAN 口名称
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT natLanCfg (CPCHAR  cpcLanName)
{
    INT           iRet;
    __LAN_INFO    lanInfo;


    if (LW_NULL == cpcLanName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "lan name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  获取 LAN 信息
     */
    bzero(&lanInfo, sizeof(lanInfo));
    natLanInfoGet(&lanInfo);

    iRet = lanSetExistFlagCheck(lanInfo, cpcLanName);
    if(ERROR_NONE == iRet) {                                            /*  LAN 已成功设置              */
        return  (ERROR_NONE);
    }

    iRet = inetNatLocalAdd(cpcLanName);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    natLanUpdateToDB();

    bzero(&lanInfo, sizeof(lanInfo));
    natLanInfoGet(&lanInfo);
    tmNotifyLanChange(&lanInfo);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: natWanCfg
** 功能描述: 配置 WAN 口状态
** 输　入  : cpcWanName         WAN 口名称
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT natWanCfg (CPCHAR  cpcWanName)
{
    INT          iRet;
    CHAR         cRouteCmd[128] = {0};
    __WAN_INFO   wanInfo;

    if (LW_NULL == cpcWanName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "wan name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  获取 WAN 信息
     */
    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);

    iRet = wanSetExistFlagCheck(wanInfo, cpcWanName);
    if(ERROR_NONE == iRet) {                                            /*  WAN 已成功设置              */
        return  (ERROR_NONE);
    }

    iRet = inetNatWanAdd(cpcWanName);
    if (ERROR_NONE != iRet) {
        return  (PX_ERROR);
    }

    natWanUpdateToDB();

    snprintf(cRouteCmd, sizeof(cRouteCmd), "route add default dev %s", cpcWanName);

    system(cRouteCmd);

    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);
    tmNotifyWanChange(&wanInfo);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: natWanLanDelet
** 功能描述: 删除 WAN LAN 口
** 输　入  : cpcNetIfName       网口名称
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT natWanLanDelet (CPCHAR  cpcNetIfName)
{
    INT             i;
    __LAN_INFO      lanInfo;
    __WAN_INFO      wanInfo;

    if (LW_NULL == cpcNetIfName) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "netif name is null.\n");

       return  (PX_ERROR);
    }

    /*
     *  获取 WAN LAN 口信息
     */
    bzero(&lanInfo, sizeof(lanInfo));
    bzero(&wanInfo, sizeof(wanInfo));
    natLanInfoGet(&lanInfo);
    natWanInfoGet(&wanInfo);

    inetNatStop();                                                      /*  先关闭 NAT                  */

    /*
     *  重新设置 WAN LAN 口信息
     */
    inetNatStart(IDEL_WAN_LAN_NAME, IDEL_WAN_LAN_NAME);

    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  遍历链表                    */
        if ((0 != strcmp(wanInfo.cWanName[i], cpcNetIfName)) &&
            (0 != strcmp(wanInfo.cWanName[i], IDEL_WAN_LAN_NAME))) {
            inetNatWanAdd(wanInfo.cWanName[i]);
        }
    }

    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  遍历链表                    */
        if ((0 != strcmp(lanInfo.cLanName[i], cpcNetIfName)) &&
            (0 != strcmp(lanInfo.cLanName[i], IDEL_WAN_LAN_NAME))) {
            inetNatLocalAdd(lanInfo.cLanName[i]);
        }
    }

    /*
     *  更新数据库
     */
    natWanUpdateToDB();
    natLanUpdateToDB();

    /*
     *  获取 WAN 信息，并通知流量监控 ko 模块 wan 口变化
     */
    bzero(&wanInfo, sizeof(wanInfo));
    natWanInfoGet(&wanInfo);
    tmNotifyWanChange(&wanInfo);

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: natMapCfg
** 功能描述: 配置端口映射功能
** 输　入  : pNatMapCfg         配置端口映射的信息结构
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT natMapCfg (__PNAT_MAP_HANDLE  pNatMapHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatMapHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat map info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatMapHandle->cHandle) {
        iRet = inetNatMapAdd(pNatMapHandle->cIpString,
                             pNatMapHandle->iIpCnt,
                             pNatMapHandle->iLanPort,
                             pNatMapHandle->iWanPort,
                             pNatMapHandle->cProto);
    } else if (1 == pNatMapHandle->cHandle) {
        iRet = inetNatMapDelete(pNatMapHandle->cIpString,
                                pNatMapHandle->iLanPort,
                                pNatMapHandle->iWanPort,
                                pNatMapHandle->cProto);
    }

    natMapUpdateToDB();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: natAliasCfg
** 功能描述: 配置网络别名功能
** 输　入  : pNatAliasCfg       配置网络别名的信息结构
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT natAliasCfg (__PNATALIAS_HANDLE  pNatAliasHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatAliasHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat alias info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatAliasHandle->cHandle) {                                /*  添加一则网络别名            */
        iRet = inetNatAliasAdd(pNatAliasHandle->cAlias,
                               pNatAliasHandle->cLanStart,
                               pNatAliasHandle->cLanEnd);
    } else {                                                            /*  删除一则网络别名            */
        iRet = inetNatAliasDelete(pNatAliasHandle->cAlias);
    }

    natAliasUpdateToDB();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: natIpFragCfg
** 功能描述: 配置分片包功能
** 输　入  : pNatIpFragHandle   配置分片包的信息结构
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT  natIpFragCfg(__PNAT_IPFRAG_HANDLE  pNatIpFragHandle)
{
    INT    iRet  =  PX_ERROR;

    if (LW_NULL == pNatIpFragHandle) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "nat ifrag info  is null.\n");

       return  (PX_ERROR);
    }

    if (0 == pNatIpFragHandle->cHandle) {
        iRet = inetNatIpFragSet(pNatIpFragHandle->cIpfragType, LW_FALSE);
    } else if (1 == pNatIpFragHandle->cHandle) {
        iRet = inetNatIpFragSet(pNatIpFragHandle->cIpfragType, LW_TRUE);
    }

    natIpFragUpdateToDB();

    return (iRet);
}


