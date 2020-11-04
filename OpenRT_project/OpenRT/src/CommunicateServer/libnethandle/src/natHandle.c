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
** 文   件   名: natHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述:  NAT 配置处理
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "libcomm.h"
#include "libnat_config.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libNatAssNodeGet
** 功能描述: 获取 NAT 端口使用信息
** 输　入  : NONE
** 输　出  : piNatAssNodeTotal  端口总个数
**           piNatAssNodeUse    已经使用的个数
** 返　回  : NONE
*********************************************************************************************************/
INT  libNatAssNodeGet (INT  *piNatAssNodeTotal, INT  *piNatAssNodeUse)
{
    __PNAT_ASSNODE_INFO   pNatAssNodeInfo = LW_NULL;
    INT                   iNum            = 0;

    if (LW_NULL == piNatAssNodeTotal || LW_NULL == piNatAssNodeUse) {
        return  (PX_ERROR);
    }

    pNatAssNodeInfo = requestQueryMsg(OPENRT_NAT_ASSNODE_GET, NULL, 0, &iNum);
    if (LW_NULL == pNatAssNodeInfo) {
        return  (PX_ERROR);
    }

    *piNatAssNodeTotal = pNatAssNodeInfo->iNatAssNodeTotal;
    *piNatAssNodeUse   = pNatAssNodeInfo->iNatAssNodeUse;

    free(pNatAssNodeInfo);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: libNatMapNumGet
** 功能描述: 获取 NAT 端口映射信息个数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 条目个数
*********************************************************************************************************/
INT32  libNatMapNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_MAP_NUM_GET, LW_NULL, 0));
}


/*********************************************************************************************************
** 函数名称: libNatMapGet
** 功能描述: 获取 NAT 端口映射信息 (需要调用 free 释放资源)
** 输　入  : pNatInfoGet        指定获取的条目数
** 输　出  : iNum               条目个数
** 返　回  : pcInfoBuf          map 信息
*********************************************************************************************************/
PVOID  libNatMapGet (__PNAT_INFO_GET  pNatInfoGet, INT  *piNum)
{
    if (LW_NULL == pNatInfoGet) {
        *piNum = 0;

        return  (LW_NULL);
    }
    return  (requestQueryMsg(OPENRT_NAT_MAP_GET, pNatInfoGet, sizeof(__NAT_INFO_GET), piNum));
}

/*********************************************************************************************************
** 函数名称: libNatMapSet
** 功能描述: NAT 端口映射设置
** 输　入  : pcIpString         IP 地址
**           iIpCnt             IP 个数
**           iWanPort           外网端口
**           iLanPort           内网端口
**           cProto             协议类型(IPPROTO_UDP  IPPROTO_TCP)
**           cNatMapHandle      映射对应操作
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libNatMapSet (PCHAR  pcIpString,
                   INT    iIpCnt,
                   INT    iWanPort,
                   INT    iLanPort,
                   CHAR   cProto,
                   CHAR   cNatMapHandle)
{
    __NAT_MAP_HANDLE      natMapHandle;

    if (LW_NULL == pcIpString                             ||
        (IPPROTO_UDP != cProto  && IPPROTO_TCP != cProto) ||
        (0 != cNatMapHandle     && 1 != cNatMapHandle)) {

        return  (PX_ERROR);
    }

    /*
     *  将消息发过去
     */
    bzero(&natMapHandle, sizeof(natMapHandle));
    natMapHandle.cHandle  = cNatMapHandle;
    natMapHandle.cProto   = cProto;
    natMapHandle.iIpCnt   = iIpCnt;
    natMapHandle.iLanPort = iLanPort;
    natMapHandle.iWanPort = iWanPort;
    strncpy(natMapHandle.cIpString, pcIpString, sizeof(natMapHandle.cIpString));

    return  (requestConfigMsg(OPENRT_NAT_MAP_SET, (PVOID)&natMapHandle, sizeof(natMapHandle)));
}

/*********************************************************************************************************
** 函数名称: libNatAliasNumGet
** 功能描述: 获取 NAT 别名信息个数
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 条目个数
*********************************************************************************************************/
INT32  libNatAliasNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_ALI_NUM_GET, LW_NULL, 0));
}

/*********************************************************************************************************
** 函数名称: libNatAliasGet
** 功能描述: 获取 NAT 别名信息  (需要调用 free 释放资源)
** 输　入  : pNatInfoGet        指定获取的条目数
** 输　出  : iNum               条目个数
** 返　回  : pcInfoBuf          别名信息
*********************************************************************************************************/
PVOID  libNatAliasGet (__PNAT_INFO_GET  pNatInfoGet, INT  *piNum)
{
    if (LW_NULL == pNatInfoGet) {
        *piNum = 0;

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_NAT_ALIAS_GET, pNatInfoGet, sizeof(__NAT_INFO_GET), piNum));
}

/*********************************************************************************************************
** 函数名称: libNatAliasSet
** 功能描述: 获取 NAT 分片包设置
** 输　入  : pcAlias            别名地址
**           pcLanStart         LAN 起始地址
**           pcLanEnd           LAN 结束地址
**           cNatAssHandle      别名对应操作
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libNatAliasSet (PCHAR  pcAlias, PCHAR  pcLanStart, PCHAR  pcLanEnd, CHAR  cNatAssHandle)
{
    __NATALIAS_HANDLE     natAliasHandle;

    if (LW_NULL == pcAlias    ||
        LW_NULL == pcLanStart ||
        LW_NULL == pcLanEnd   ||
        (0 != cNatAssHandle   &&
         1 != cNatAssHandle)) {
        return  (PX_ERROR);
    }

    /*
     *  将消息发过去
     */
    bzero(&natAliasHandle, sizeof(natAliasHandle));
    natAliasHandle.cHandle = cNatAssHandle;
    strncpy(natAliasHandle.cAlias,    pcAlias,  sizeof(natAliasHandle.cAlias));
    strncpy(natAliasHandle.cLanStart, pcLanStart, sizeof(natAliasHandle.cLanStart));
    strncpy(natAliasHandle.cLanEnd,   pcLanEnd, sizeof(natAliasHandle.cLanEnd));

    return  (requestConfigMsg(OPENRT_NAT_ALIAS_SET, &natAliasHandle, sizeof(natAliasHandle)));
}

/*********************************************************************************************************
** 函数名称: libNatIpFragSet
** 功能描述: 获取 NAT 分片包设置
** 输　入  : iIpFragType        分片包类型
**           iIpFragHandle      分片包类型处理方式
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libNatIpFragSet (CHAR  cIpFragType, CHAR  cIpFragHandle)
{
    __NAT_IPFRAG_HANDLE   natIpFragHandle;

    if (IPPROTO_TCP  != cIpFragType  &&
        IPPROTO_UDP  != cIpFragType &&
        IPPROTO_ICMP != cIpFragType) {
        return  (PX_ERROR);
    }

    if (NAT_IPFRAG_ENABLE  != cIpFragHandle &&
        NAT_IPFRAG_DISABLE != cIpFragHandle) {
        return  (PX_ERROR);
    }

    /*
     *  将消息发过去
     */
    bzero(&natIpFragHandle, sizeof(natIpFragHandle));
    natIpFragHandle.cHandle     = cIpFragHandle;
    natIpFragHandle.cIpfragType = cIpFragType;

    return  (requestConfigMsg(OPENRT_NAT_IPFRAG_SET, &natIpFragHandle, sizeof(natIpFragHandle)));
}

/*********************************************************************************************************
** 函数名称: libNatIpFragGet
** 功能描述: 获取 NAT 分片包设置
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NAT 分片包信息
*********************************************************************************************************/
INT  libNatIpFragGet (VOID)
{
    return  (requestConfigMsg(OPENRT_NAT_IPFRAG_GET, LW_NULL, 0));
}

/*********************************************************************************************************
** 函数名称: libNatWanLanConfig
** 功能描述: WAN LAN口配置
** 输　入  : cpNetIfName        网口名称
**           iHandle            处理方式
**           iFunc              网口功能 (0 : WAN  1 : LAN)
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  libNatWanLanConfig (PCHAR  pcNetIfName, INT  iNatWanLanHandle, INT  iFunc)
{
    __NAT_WANLAN_HANDLE   natWanLanHandle;
    INT                   iNatRet;

    if (LW_NULL == pcNetIfName) {
        return  (PX_ERROR);
    }

    if ((NAT_WANLAN_SET != iNatWanLanHandle  && NAT_WANLAN_DEL != iNatWanLanHandle) ||
        (NAT_FUNC_LAN   != iFunc             && NAT_FUNC_WAN != iFunc)) {

        return  (PX_ERROR);
    }

    bzero(&natWanLanHandle, sizeof(natWanLanHandle));
    natWanLanHandle.iFunc   = iFunc;
    natWanLanHandle.iHandle = iNatWanLanHandle;
    strncpy(natWanLanHandle.cpNetIfName, pcNetIfName, sizeof(natWanLanHandle.cpNetIfName));

    iNatRet = requestConfigMsg(OPENRT_NAT_WANLAN_SET, &natWanLanHandle, sizeof(natWanLanHandle));

    return  (iNatRet);
}
