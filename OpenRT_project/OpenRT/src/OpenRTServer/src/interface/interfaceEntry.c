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
** 文   件   名: interfaceEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 4 日
**
** 描        述: OpenRT 网口操作入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msgcomm.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "common.h"
#include "interfaceExec.h"
#include "libinterface_config.h"
/*********************************************************************************************************
** 函数名称: networkSegCollisionDetect
** 功能描述: 网段冲突检测
** 输　入  : pNetInfo         设置的网口的信息
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static INT  networkSegCollisionDetect(__PNETIF_SET  pNetInfo)
{
    PCHAR                 pIfaceBuf           = LW_NULL;
    PCHAR                 pBuffTmp            = LW_NULL;
    __PNETIF_INFO         pIfaceInfo          = LW_NULL;
    INT                  iNetIfNum   = 0;
    INT                  iBufLen     = 0;
    UINT                 uiNetId     = 0;
    UINT                 uiNetIdTmp  = 0;
    struct in_addr       inIp        = {0};
    struct in_addr       inNetmask   = {0};
    INT                  i;
    INT                  iRet;

    iNetIfNum = netIfNumGet() + vndNetIfNumGet();                       /*  获取当前的网卡个数          */
    if (0 >= iNetIfNum) {
        return  (ERROR_NONE);
    }

    /*
     *  申请内存，用于保存所有网卡的信息
     */
    iBufLen     = iNetIfNum * sizeof(__NETIF_INFO);
    pIfaceBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pIfaceBuf) {
        return  (PX_ERROR);
    }

    bzero(pIfaceBuf, iBufLen);

    iRet = netIfInfoGetAll(pIfaceBuf, iBufLen);                         /*  获取到所有信息              */
    if (ERROR_NONE != iRet) {
        if (NULL != pIfaceBuf) {
            free(pIfaceBuf);
        }

        return  (PX_ERROR);
    }

    inet_aton(pNetInfo->cIpAddr,  &inIp);
    inet_aton(pNetInfo->cNetMask, &inNetmask);

    uiNetId = (inIp.s_addr & inNetmask.s_addr);

    pBuffTmp = pIfaceBuf;
    for (i = 0; i < iNetIfNum; i++) {
        pIfaceInfo = (__PNETIF_INFO)pIfaceBuf;
        if (strstr(pIfaceInfo->cNetifName, "vn") ||
           !strcmp(pNetInfo->cNetifName, pIfaceInfo->cNetifName)) {
            pIfaceBuf += sizeof(__NETIF_INFO);

            continue;
        }

        uiNetIdTmp = pIfaceInfo->uiIp & pIfaceInfo->uiNetMask;
        if (uiNetId == uiNetIdTmp) {
            return  (RET_COLLISION);
        }

        pIfaceBuf += sizeof(__NETIF_INFO);
    }


    if (NULL != pBuffTmp) {
        free(pBuffTmp);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __netifInfoSet
** 功能描述: 设置指定网口参数入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __netifInfoSet (PVOID  pArgBuf)
{
    INT           iRet;
    __PNETIF_SET  pNetInfo = NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, PX_ERROR);

        return;
    }

    pNetInfo = (__PNETIF_SET)pArgBuf;

    iRet = networkSegCollisionDetect(pNetInfo);
    if (ERROR_NONE != iRet) {
        replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, iRet);

        return;
    }

    replyConfigMsg(OPENRT_NETINFO_SET_ACK_CMD, netIfInfoSet(pNetInfo));
}
/*********************************************************************************************************
** 函数名称: __getAllNetIfInfo
** 功能描述: 获取所有网口信息入口
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __getAllNetIfInfo (VOID)
{
    PCHAR                pcAllNetInfoBuff = LW_NULL;
    INT                  iNetIfNum        = 0;
    INT                  iBufLen          = 0;

    iNetIfNum = netIfNumGet() + vndNetIfNumGet();                       /*  获取当前的网卡个数          */
    if (0 >= iNetIfNum) {
        replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                      sizeof(__NETIF_INFO),
                      iNetIfNum,
                      pcAllNetInfoBuff);
        return;
    }

    /*
     *  申请内存，用于保存所有网卡的信息
     */
    iBufLen          = iNetIfNum * sizeof(__NETIF_INFO);
    pcAllNetInfoBuff = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcAllNetInfoBuff) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                      sizeof(__NETIF_INFO),
                      0,
                      pcAllNetInfoBuff);

        return;
    }

    bzero(pcAllNetInfoBuff, iBufLen);
    netIfInfoGetAll(pcAllNetInfoBuff, iBufLen);                         /*  获取到所有信息              */
    replyQueryMsg(OPENRT_NETINFO_GET_ACK_CMD,
                  sizeof(__NETIF_INFO),
                  iNetIfNum,
                  pcAllNetInfoBuff);

    free(pcAllNetInfoBuff);
}

/*********************************************************************************************************
** 函数名称: interFaceHandleEntry
** 功能描述: 网口操作入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID interFaceHandleEntry (INT iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_NETINFO_GET_CMD:
        __getAllNetIfInfo();
        break;

    case OPENRT_NETINFO_SET_CMD:
        __netifInfoSet(pArgBuf);
        break;

    default:
        break;
    }
}
