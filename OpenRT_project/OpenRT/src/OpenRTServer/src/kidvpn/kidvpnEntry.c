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
** 文   件   名: kidvpnEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: OpenRT KidVPN 配置处理入口
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lwip/inet.h"
#include "common.h"
#include "msgcomm.h"
#include "libvpn_config.h"
#include "kidvpn_common.h"
#include "kidvpnDB.h"
#include "vnd/vnd_common.h"
/*********************************************************************************************************
** 函数名称: __indexCheck
** 功能描述: 获取总的需要显示的条目数以及最后一个条目的index
** 输　入  : llStartIndex       开始的条目index
**           llEndIndex         计划的结束条目index
**           iAllIterm          总的条目数
** 输　出  : pllEndIndex        最终转换后结束的条目index
** 返　回  : iItermCount        总共显示的条目个数
*********************************************************************************************************/
static INT32  __indexChange (INT64  llStartIndex,
                             INT64  llEndIndex,
                             INT32  iAllIterm,
                             INT64  *pllEndIndex)
{
    INT32  iItermCount = 0;

    *pllEndIndex  = llEndIndex;
    /*
     *  申请内存，用于保存所有的信息
     */
    if (llStartIndex <= iAllIterm) {
        if (llEndIndex > iAllIterm) {
            *pllEndIndex = iAllIterm;
            iItermCount  = iAllIterm - llStartIndex + 1;
        } else {
            iItermCount = llEndIndex - llStartIndex + 1;
        }
    }

    return  (iItermCount);
}
/*********************************************************************************************************
** 函数名称: __kidvpnAdd
** 功能描述: 添加 VPN 入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnAdd (PVOID  pArgBuf)
{
    __PKIDVPN_HANDLE  pKidVpnHandle = LW_NULL;
    INT               iVpnRet       = PX_ERROR;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, RET_ARG_ERROR);

        return;
    }

    pKidVpnHandle = (__PKIDVPN_HANDLE)pArgBuf;

    /*
     *  如果是创建服务器，需判断服务器 ip 地址和 port 是否已存在
     */
    if (TYPE_ADD_VPN_S == pKidVpnHandle->cHandleType) {
        iVpnRet = kidvpnServIpExistFlagGet(pKidVpnHandle);
        if (ERROR_NONE == iVpnRet) {                                    /*  服务器 ip 地址已存在        */
            kidVpnAesKeyDel(pKidVpnHandle->iVndID);

            vndDel(pKidVpnHandle->iVndID);                              /*  删除 vnd                    */

            replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, RET_ARG_ERROR);

            return;
        }
    }

    if (TYPE_ADD_VPN_C == pKidVpnHandle->cHandleType ||
        TYPE_ADD_VPN_S == pKidVpnHandle->cHandleType) {
        iVpnRet = kidVpnStart(pKidVpnHandle);
    }

    replyConfigMsg(OPENRT_ADD_VPN_ACK_CMD, iVpnRet);
}

/*********************************************************************************************************
** 函数名称: __kidvpnDel
** 功能描述: 删除 VPN 入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnDel (PVOID  pArgBuf)
{
    INT               iVndID    = 0;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_DEL_VPN_ACK_CMD, PX_ERROR);
    }

    iVndID = *(INT *)pArgBuf;

    replyConfigMsg(OPENRT_DEL_VPN_ACK_CMD, kidVpnDel(iVndID));
}
/*********************************************************************************************************
** 函数名称: __kidvpnKeySave
** 功能描述: 保存 KEY 入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeySave (PVOID  pArgBuf)
{
    INT                iVpnRet       = PX_ERROR;
    __PAES_KEY_HANDLE  pKidVpnAesKey = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyConfigMsg(OPENRT_KEY_SAVE_ACK_CMD, PX_ERROR);

        return;
    }

    pKidVpnAesKey = (__PAES_KEY_HANDLE)pArgBuf;
    iVpnRet = kidVpnAesKeySave(pKidVpnAesKey->cAesKey,
                               pKidVpnAesKey->iVndID,
                               strlen(pKidVpnAesKey->cAesKey));

    replyConfigMsg(OPENRT_KEY_SAVE_ACK_CMD, iVpnRet);
}

/*********************************************************************************************************
** 函数名称: __kidvpnKeyGen
** 功能描述: 生成 KEY 入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeyGen (PVOID  pArgBuf)
{
    INT      iKeyBit        = 0;
    CHAR     cKeyGenBuf[70] = {0};

    iKeyBit        = *(INT *)pArgBuf;

    kidVpnAesKeyGen(cKeyGenBuf, iKeyBit, sizeof(cKeyGenBuf));

    replyQueryMsg(OPENRT_KEY_GEN_ACK_CMD, sizeof(cKeyGenBuf), 1, cKeyGenBuf);
}

/*********************************************************************************************************
** 函数名称: __kidvpnKeyGet
** 功能描述: 获取本地 KEY 入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnKeyGet (PVOID  pArgBuf)
{
    INT      iVndID         = 0;
    CHAR     cKeyGenBuf[70] = {0};

    iVndID  = *(INT *)pArgBuf;
    kidVpnAesKeyGet(cKeyGenBuf, iVndID, sizeof(cKeyGenBuf));
    replyQueryMsg(OPENRT_KEY_GET_ACK_CMD, sizeof(cKeyGenBuf), 1, cKeyGenBuf);
}
/*********************************************************************************************************
** 函数名称: __kidvpnInfoGet
** 功能描述: 获取 VPN 信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnInfoGet (PVOID  pArgBuf)
{
    PCHAR     pcKidVPNAllInfo = LW_NULL;
    INT       iNum            = 0;

    iNum            = kidvpnGetDBRow();
    pcKidVPNAllInfo = kidvpnGetDBInfo();

    replyQueryMsg(OPENRT_INFO_GET_ACK_CMD, sizeof(__KIDVPN_DB_INFO), iNum, pcKidVPNAllInfo);

    free(pcKidVPNAllInfo);
}
/*********************************************************************************************************
** 函数名称: __kidvpnLinkInfoNumGet
** 功能描述: 获取 VPN 连接信息总数目入口
** 输　入  : pArgBuf      接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoNumGet (PVOID  pArgBuf)
{
    replyConfigMsg(OPENRT_LINK_NUM_ACK_CMD, kidvpnLinkGetDBRow());
}
/*********************************************************************************************************
** 函数名称: __kidvpnLinkInfoGet
** 功能描述: 获取 VPN 连接信息入口
** 输　入  : pArgBuf      接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkInfoGet (PVOID  pArgBuf)
{
    __PVPN_INFO_HANDLE   pVpnInfoHandle   = LW_NULL;
    INT32                iItermNum        = 0;
    INT32                iItermCount      = 0;
    INT32                iBufLen          = 0;
    INT                  iRet             = PX_ERROR;
    PCHAR                pcKidVPNLinkInfo = LW_NULL;

    if (LW_NULL == pArgBuf) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    pVpnInfoHandle = (__PVPN_INFO_HANDLE)pArgBuf;

    iItermNum      =  kidvpnLinkGetDBRow();
    if (iItermNum <= 0) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    /*
     *  申请内存，用于保存所有的信息
     */
    iItermCount = __indexChange(pVpnInfoHandle->llStartIndex,
                                pVpnInfoHandle->llEndIndex,
                                iItermNum,
                                &(pVpnInfoHandle->llEndIndex));
    if (!iItermCount) {
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

        return;
    }

    iBufLen    = iItermCount * sizeof(__KIDVPN_LINK);
    pcKidVPNLinkInfo = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcKidVPNLinkInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");
        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), 0, pcKidVPNLinkInfo);

        return;
    }

    bzero(pcKidVPNLinkInfo, iBufLen);

    iRet = kidvpnLinkGetSpecInfo(pVpnInfoHandle->llStartIndex,
                                 pVpnInfoHandle->llEndIndex,
                                 pcKidVPNLinkInfo,
                                 iBufLen);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get vpn link info error\n");

        replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), 0, pcKidVPNLinkInfo);

        free(pcKidVPNLinkInfo);

        return;
    }

    replyQueryMsg(OPENRT_LINK_GET_ACK_CMD, sizeof(__KIDVPN_LINK), iItermCount, pcKidVPNLinkInfo);

    free(pcKidVPNLinkInfo);
}
/*********************************************************************************************************
** 函数名称: __kidvpnLinkPositionGet
** 功能描述: 获取 VPN 连接地理信息入口
** 输　入  : pArgBuf      接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __kidvpnLinkPositionGet (PVOID  pArgBuf)
{
    INT                i;
    INT                iNum;
    INT                iRow;
    INT                iTotal;
    INT                iRet;
    static BOOL        bGetPubIp = FALSE;

    PCHAR              pcIp;
    static CHAR        cLocalIp[32]    = {0};
    CHAR               cLocalPos[128]  = {0};
    CHAR               cRemotePos[128] = {0};
    struct in_addr     inIpAddr;

    PCHAR              pcKidVPNLinkInfo = LW_NULL;
    __PKIDVPN_LINK     pcVpnLinkTmp     = LW_NULL;

    PCHAR              pKidVPNInfo      = LW_NULL;
    __PKIDVPN_DB_INFO  pKidVPNInfoTmp   = LW_NULL;

    PCHAR              pKidvpnPos       = LW_NULL;
    __PKIDVPN_POS      pKidvpnPosTmp    = LW_NULL;

    system("ifconfig dns 0 8.8.8.8");

    /*
     *  获取本机所在公网IP
     */
    if (!bGetPubIp) {
        getPublicIp(cLocalIp);

        iRet = inet_aton(cLocalIp, &inIpAddr);
        if (1 == iRet) {                                                /*  成功获得本地公网ip地址      */
            bGetPubIp = TRUE;
        } else {
            printf("Get Public ip failed.\n");
        }
    }

    /*
     *  获取本机所在地理位置
     */
    iRet = getPositionByIpAddr(cLocalIp, cLocalPos);
    if (ERROR_NONE != iRet) {
        strncpy(cLocalPos, "[\"invalid\", \"\", \"\", \"\", \"\"]", sizeof(cLocalPos));
    }

    /*
     *  获取服务器成功接收到连接的客户端个数
     */
    iNum = kidvpnLinkGetDBRow();

    /*
     *  获取本地客户端连接到服务器的信息
     */
    pKidVPNInfo = kidvpnGetLinkedCliInfoFromDB(&iRow);

    iTotal = iNum + iRow;                                               /*  总共的连接信息              */

    /*
     *  为 IP 和 地理位置信息分配内存
     */
    pKidvpnPos = (PCHAR)malloc(iTotal * sizeof(__KIDVPN_POS));
    if (NULL == pKidvpnPos) {
        free(pKidVPNInfo);

        return;
    }

    pKidVPNInfoTmp = (__PKIDVPN_DB_INFO)pKidVPNInfo;
    pKidvpnPosTmp  = (__PKIDVPN_POS)pKidvpnPos;

    bzero(pKidvpnPosTmp, iTotal * sizeof(__KIDVPN_POS));

    /*
     *  获取客户端连接信息并将 IP 转化为地理地址
     */
    for (i = 0; i < iRow; i++) {

        /*
         *  客户端本地信息
         */
        strncpy(pKidvpnPosTmp->vpnSrc.cIp,  cLocalIp,  sizeof(pKidvpnPosTmp->vpnSrc.cIp));
        strncpy(pKidvpnPosTmp->vpnSrc.cPos, cLocalPos, sizeof(pKidvpnPosTmp->vpnSrc.cPos));

        /*
         *  客户端所连接服务器信息
         */
        pcIp = inet_ntoa(*(struct in_addr *)&pKidVPNInfoTmp->kidvpnHandle.uiIpAddr);
        iRet = getPositionByIpAddr(pcIp, cRemotePos);
        if (iRet < 0) {
            strncpy(cRemotePos, "NONE", sizeof(cRemotePos));
        }

        strncpy(pKidvpnPosTmp->vpnDst.cIp,  pcIp,       sizeof(pKidvpnPosTmp->vpnDst.cIp));
        strncpy(pKidvpnPosTmp->vpnDst.cPos, cRemotePos, sizeof(pKidvpnPosTmp->vpnDst.cPos));

        pKidVPNInfoTmp += 1;
        pKidvpnPosTmp  += 1;
    }

    pcKidVPNLinkInfo = kidvpnLinkGetDBInfo();

    pcVpnLinkTmp = (__PKIDVPN_LINK)pcKidVPNLinkInfo;

    for (i = 0; i < iNum; i++) {

        /*
         *  服务器所连接客户端ip及地理位置信息
         */
        iRet = getPositionByIpAddr(pcVpnLinkTmp->cIp, cRemotePos);
        if (iRet < 0) {
            strncpy(cRemotePos, "[\"invalid\", \"\", \"\", \"\", \"\"]", sizeof(cRemotePos));
        }

        strncpy(pKidvpnPosTmp->vpnSrc.cIp,  pcVpnLinkTmp->cIp, sizeof(pKidvpnPosTmp->vpnSrc.cIp));
        strncpy(pKidvpnPosTmp->vpnSrc.cPos, cRemotePos,        sizeof(pKidvpnPosTmp->vpnSrc.cPos));

        /*
         *  服务器本地 ip 及地理位置信息
         */
        strncpy(pKidvpnPosTmp->vpnDst.cIp,  cLocalIp,  sizeof(pKidvpnPosTmp->vpnDst.cIp));
        strncpy(pKidvpnPosTmp->vpnDst.cPos, cLocalPos, sizeof(pKidvpnPosTmp->vpnDst.cPos));

        pcVpnLinkTmp  += 1;
        pKidvpnPosTmp += 1;
    }

    replyQueryMsg(OPENRT_POSIT_GET_ACK_CMD, sizeof(__KIDVPN_POS), iTotal, pKidvpnPos);

    free(pKidVPNInfo);
    free(pcKidVPNLinkInfo);
    free(pKidvpnPos);
}
/*********************************************************************************************************
** 函数名称: kidvpnHandleEntry
** 功能描述: KidVPN 配置处理入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID kidvpnHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_ADD_VPN_CMD:
        __kidvpnAdd(pArgBuf);
        break;

    case OPENRT_DEL_VPN_CMD:
        __kidvpnDel(pArgBuf);
        break;

    case OPENRT_KEY_SAVE_CMD:
        __kidvpnKeySave(pArgBuf);
        break;

    case OPENRT_KEY_GEN_CMD:
        __kidvpnKeyGen(pArgBuf);
        break;

    case OPENRT_KEY_GET_CMD:
        __kidvpnKeyGet(pArgBuf);
        break;

    case OPENRT_INFO_GET_CMD:
        __kidvpnInfoGet(pArgBuf);
        break;

    case OPENRT_LINK_NUM_CMD:
        __kidvpnLinkInfoNumGet(pArgBuf);
        break;

    case OPENRT_LINK_GET_CMD:
        __kidvpnLinkInfoGet(pArgBuf);
        break;

    case OPENRT_POSIT_GET_CMD:
        __kidvpnLinkPositionGet(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
