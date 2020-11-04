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
** 文   件   名: natEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 16 日
**
** 描        述: OpenRT NAT 配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "common.h"
#include "libnat_config.h"
#include "natExec.h"
#include "natStatus.h"
/*********************************************************************************************************
** 函数名称: __natAliasNumGet
** 功能描述: 获取网络别名总个数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAliasNumGet (PVOID  pArgBuf)
{
    INT32     iItermNum       = 0;

    iItermNum  = natAliasItemNumGet();
    if (iItermNum <= 0) {
        replyConfigMsg(OPENRT_NAT_ALI_NUM_GET_ACK, 0);

        return;
    }

    replyConfigMsg(OPENRT_NAT_ALI_NUM_GET_ACK, iItermNum);
}

/*********************************************************************************************************
** 函数名称: __natAliasGet
** 功能描述: 获取网络别名入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAliasGet (PVOID  pArgBuf)
{
    PCHAR                 pcNatAliasInfoNBuf = LW_NULL;
    __PNAT_INFO_GET       pNatInfoGetVar     = LW_NULL;
    INT                   iBuffLen           = 0;
    INT                   iCount             = 0;
    INT                   iNatAliasNum       = 0;

    if (LW_NULL == pArgBuf) {
        goto  __back;
    }

    pNatInfoGetVar = (__PNAT_INFO_GET)pArgBuf;
    iNatAliasNum = natAliasItemNumGet();                                /*  获取总条目个数              */
    if (0 == iNatAliasNum) {
        goto  __back;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    if (pNatInfoGetVar->iStartIndex <= iNatAliasNum) {
        if (pNatInfoGetVar->iEndIndex > iNatAliasNum) {
            pNatInfoGetVar->iEndIndex = iNatAliasNum;
            iCount = iNatAliasNum - pNatInfoGetVar->iStartIndex + 1;
        } else {
            iCount = pNatInfoGetVar->iEndIndex - pNatInfoGetVar->iStartIndex + 1;
        }
    } else {
        /*
         *  这种情况一般不会出现
         */
        goto  __back;
    }

    iBuffLen = iCount * sizeof(__NATALIAS_INFO);
    pcNatAliasInfoNBuf   = (PCHAR)malloc(iBuffLen);
    if (LW_NULL == pcNatAliasInfoNBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return;
    }

    bzero(pcNatAliasInfoNBuf, iBuffLen);
    natAliasInfoGet(pcNatAliasInfoNBuf, pNatInfoGetVar->iStartIndex, pNatInfoGetVar->iEndIndex);

__back:
    replyQueryMsg(OPENRT_NAT_ALIAS_GET_ACK, sizeof(__NATALIAS_INFO), iCount, pcNatAliasInfoNBuf);
    free(pcNatAliasInfoNBuf);
}

/*********************************************************************************************************
** 函数名称: __natAliasSet
** 功能描述: 设置网络别名入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAliasSet (PVOID  pArgBuf)
{
    __PNATALIAS_HANDLE   pNatAliasHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_ALIAS_SET_ACK, PX_ERROR);

        return;
    }
    pNatAliasHandle = (__PNATALIAS_HANDLE)pArgBuf;

    replyConfigMsg(OPENRT_NAT_ALIAS_SET_ACK, natAliasCfg(pNatAliasHandle));
}

/*********************************************************************************************************
** 函数名称: __natMapNumGet
** 功能描述: 获取 NAT 端口映射总个数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natMapNumGet (PVOID  pArgBuf)
{
    INT32     iItermNum       = 0;

    iItermNum  = natMapItemNumGet();
    if (iItermNum <= 0) {
        replyConfigMsg(OPENRT_NAT_MAP_NUM_GET_ACK, 0);

        return;
    }

    replyConfigMsg(OPENRT_NAT_MAP_NUM_GET_ACK, iItermNum);
}
/*********************************************************************************************************
** 函数名称: __natMapGet
** 功能描述: 获取 NAT 端口映射入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natMapGet (PVOID  pArgBuf)
{
    PCHAR                 pNatMapInfoNBuf  = LW_NULL;
    __PNAT_INFO_GET       pNatInfoGetVar   = LW_NULL;
    INT                   iNatMapNum       = 0;
    INT                   iBuffLen         = 0;
    INT                   iCount           = 0;

    if (LW_NULL == pArgBuf) {
        goto  __back;
    }

    pNatInfoGetVar = (__PNAT_INFO_GET)pArgBuf;
    iNatMapNum = natMapItemNumGet();                                    /*  获取总条目个数              */
    if (0 == iNatMapNum) {
        goto  __back;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    if (pNatInfoGetVar->iStartIndex <= iNatMapNum) {
        if (pNatInfoGetVar->iEndIndex > iNatMapNum) {
            pNatInfoGetVar->iEndIndex = iNatMapNum;
            iCount = iNatMapNum - pNatInfoGetVar->iStartIndex + 1;
        } else {
            iCount = pNatInfoGetVar->iEndIndex - pNatInfoGetVar->iStartIndex + 1;
        }
    } else {
        /*
         *  这种情况一般不会出现
         */
        goto  __back;
    }

    iBuffLen = iCount * sizeof(__NATMAP_INFO);
    pNatMapInfoNBuf   = (PCHAR)malloc(iBuffLen);
    if (LW_NULL == pNatMapInfoNBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return;
    }

    bzero(pNatMapInfoNBuf, iBuffLen);
    natMapInfoGet(pNatMapInfoNBuf, pNatInfoGetVar->iStartIndex, pNatInfoGetVar->iEndIndex);

__back:
    replyQueryMsg(OPENRT_NAT_MAP_GET_ACK, sizeof(__NATMAP_INFO), iCount, pNatMapInfoNBuf);
    free(pNatMapInfoNBuf);
}

/*********************************************************************************************************
** 函数名称: __natMapSet
** 功能描述: 设置NAT 端口映射入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natMapSet (PVOID  pArgBuf)
{
    __PNAT_MAP_HANDLE    pNtMapHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_MAP_SET_ACK, PX_ERROR);

        return;
    }

    pNtMapHandle = (__PNAT_MAP_HANDLE)pArgBuf;

    /*
     *  将配置是否成功告诉后台接口
     */
    replyConfigMsg(OPENRT_NAT_MAP_SET_ACK, natMapCfg(pNtMapHandle));
}

/*********************************************************************************************************
** 函数名称: __natAssNodeGet
** 功能描述: 获取 NAT 端口使用情况入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natAssNodeGet (PVOID  pArgBuf)
{
    __NAT_ASSNODE_INFO    natAssNodeUse;

    natAssNodeUseGet(&natAssNodeUse);
    replyQueryMsg(OPENRT_NAT_ASSNODE_GET_ACK, sizeof(__NAT_ASSNODE_INFO), 1, &natAssNodeUse);
}

/*********************************************************************************************************
** 函数名称: __natIpFragGet
** 功能描述: 获取 IP 分片包使能入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natIpFragGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_NAT_IPFRAG_GET_ACK, natIpFragGet());
}

/*********************************************************************************************************
** 函数名称: __natIpFragSet
** 功能描述: 设置 IP 分片包使能入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natIpFragSet (PVOID  pArgBuf)
{
    __PNAT_IPFRAG_HANDLE   pNatIpFragHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_IPFRAG_SET_ACK, PX_ERROR);

        return;
    }

    pNatIpFragHandle = (__PNAT_IPFRAG_HANDLE)pArgBuf;

    /*
     *  将配置是否成功告诉后台接口
     */
    replyConfigMsg(OPENRT_NAT_IPFRAG_SET_ACK, natIpFragCfg(pNatIpFragHandle));
}

/*********************************************************************************************************
** 函数名称: __natWanLanSet
** 功能描述: 设置 WAN LAN 口入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __natWanLanSet (PVOID  pArgBuf)
{
    INT                     iWanLanRet       = PX_ERROR;
    __PNAT_WANLAN_HANDLE    pNatWanLanHandle = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);

        return;
    }

    pNatWanLanHandle = (__PNAT_WANLAN_HANDLE)pArgBuf;

    if (0 == strcmp("NULL", pNatWanLanHandle->cpNetIfName)) {           /*  网卡名称不对直接返回        */
        replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);
    }

    if (NAT_WANLAN_SET == pNatWanLanHandle->iHandle) {                  /*  需要添加 WAN LAN 口         */
        if (NAT_FUNC_WAN == pNatWanLanHandle->iFunc) {                  /*  判断是进行Wan口配置还是Lan口*/
            iWanLanRet = natWanCfg(pNatWanLanHandle->cpNetIfName);
        } else {
            iWanLanRet = natLanCfg(pNatWanLanHandle->cpNetIfName);
        }
    } else if (NAT_WANLAN_DEL == pNatWanLanHandle->iHandle) {           /*  需要删除 WAN LAN 口         */
        iWanLanRet = natWanLanDelet(pNatWanLanHandle->cpNetIfName);
    }

    replyConfigMsg(OPENRT_NAT_WANLAN_SET_ACK, iWanLanRet);
}

/*********************************************************************************************************
** 函数名称: natHandleEntry
** 功能描述: NAT 配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID natHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_NAT_WANLAN_SET:
        __natWanLanSet(pArgBuf);
        break;

    case OPENRT_NAT_IPFRAG_SET:
        __natIpFragSet(pArgBuf);
        break;

    case OPENRT_NAT_IPFRAG_GET:
        __natIpFragGet(pArgBuf);
        break;

    case OPENRT_NAT_ASSNODE_GET:
        __natAssNodeGet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_SET:
        __natMapSet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_GET:
        __natMapGet(pArgBuf);
        break;

    case OPENRT_NAT_ALIAS_SET:
        __natAliasSet(pArgBuf);
        break;

    case OPENRT_NAT_ALIAS_GET:
        __natAliasGet(pArgBuf);
        break;

    case OPENRT_NAT_MAP_NUM_GET:
        __natMapNumGet(pArgBuf);
        break;

    case OPENRT_NAT_ALI_NUM_GET:
        __natAliasNumGet(pArgBuf);
        break;

    default:
        break;
    }
}
