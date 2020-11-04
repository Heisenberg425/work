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
** 文   件   名: intAdaptionExec.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 网卡顺序自适应执行接口
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"
#include "netdev.h"
#include "common.h"
#include "intada_common.h"
#include "intAdaptionExec.h"
#include "net/lwip/lwip_iphook.h"
/*********************************************************************************************************
** 函数名称: __onlyNetifLinkUpGet
** 功能描述: 获取所有网卡中唯一连接成功的网卡
** 输　入  : NONE
** 输　出  : pcNetifName        网卡名称
** 返　回  : error code
*********************************************************************************************************/
static INT __onlyNetifLinkUpGet (CHAR   *pcNetifName)
{
    INT                    iCount      = 0;
    INT32                  iSocketFd;
    struct  ifconf         iconf;
    INT32                  iRet, i;
    INT32                  iBuffSize;

    struct ifreq          *pIfreq     = LW_NULL;
    PCHAR                  pcIconfBuf = LW_NULL;


    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");
        return  (PX_ERROR);
    }

    /*
     *  获取需要多大的 buf 存放
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;
    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");
        close(iSocketFd);

        goto  __error;
    }

    /*
     *  遍历所有的网口信息
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  获取所有的网卡的类型        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");

            goto  __error;
        }

        /*
         * 只对物理以太网卡做统计
         */
        if (IFT_ETHER == pIfreq->ifr_type && (!strstr(pIfreq->ifr_name, "vn"))) {
            iRet = ioctl(iSocketFd, SIOCGIFFLAGS, pIfreq);              /*  获取网卡标志                */
            if (iRet < 0) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFFLAGS error.\n");

                goto  __error;
            }
            if (IFF_RUNNING == (pIfreq->ifr_flags & IFF_RUNNING)) {     /*  当前处理连接状态            */
                iCount++;
                if (iCount >= 2) {                                      /*  超过两个不是唯一            */
                    goto  __error;
                } else {
                    strncpy(pcNetifName, pIfreq->ifr_name, sizeof(pIfreq->ifr_name));
                }
            }
        }
    }

    close(iSocketFd);
    free(pcIconfBuf);
    iRet = (1 == iCount) ? ERROR_NONE : PX_ERROR;
    return  (iRet);

__error:
    close(iSocketFd);
    free(pcIconfBuf);
    return  (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: intAdaptionCheck
** 功能描述: 是否完成了网口顺序自适应
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           没完成
**           ERROR_NONE         完成
*********************************************************************************************************/
INT intAdaptionCheck (VOID)
{
    return  intAdaptionDBFindFinish();
}

/*********************************************************************************************************
** 函数名称: intAdaptionIsExist
** 功能描述: 判断前台的某个网口是否已经进行了网口定位
** 输　入  : iWebIndex          前台网口索引
** 输　出  : NONE
** 返　回  : PX_ERROR           没完成
**           ERROR_NONE         完成
*********************************************************************************************************/
INT  intAdaptionIsExist (INT  iWebIndex)
{
    __INTADAPTION_HANDLE_INFO  netInfo;

    bzero(&netInfo, sizeof(netInfo));

    netInfo.iWebIndex = iWebIndex;

    return  intAdaptionDBGetInfo(&netInfo, 0);
}

/*********************************************************************************************************
** 函数名称: intAdaptionGetNetif
** 功能描述: 通过前端网口名称获取对应的 netif 名称
** 输　入  : cpcWebNetifName    前端网口名
** 输　出  : pcNetifName        Netif名称
** 返　回  : PX_ERROR           没完成
**           ERROR_NONE         完成
*********************************************************************************************************/
INT  intAdaptionGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cWebName, cpcWebNetifName, sizeof(netInfo.cWebName));

    iRet = intAdaptionDBGetInfo(&netInfo, 2);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcNetifName, netInfo.cNetIfName, strlen(netInfo.cNetIfName));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: intAdaptionGetWebNetif
** 功能描述: 通过网口名称获取对应的WEB 网口名称
** 输　入  : cpcNetifName          Netif名称
** 输　出  : pcWebNetifName        前端网口名
** 返　回  : PX_ERROR              没完成
**           ERROR_NONE            完成
*********************************************************************************************************/
INT  intAdaptionGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcWebNetifName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cNetIfName, cpcNetifName, sizeof(netInfo.cNetIfName));

    iRet = intAdaptionDBGetInfo(&netInfo, 1);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcWebNetifName, netInfo.cWebName, strlen(netInfo.cWebName));

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: intAdaptionGetWebName
** 功能描述: 通过 netif 名称获得前端网口信息
** 输　入  : pcNetifName        Netif名称
** 输　出  : pcWebName          前端网口名称
**           piWebIndex         前端网口索引
** 返　回  : PX_ERROR           没完成
**           ERROR_NONE         完成
*********************************************************************************************************/
INT intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex)
{
    __INTADAPTION_HANDLE_INFO  netInfo;
    INT                        iRet;

    if (LW_NULL == pcNetifName || LW_NULL == pcWebName) {
        return  (PX_ERROR);
    }

    bzero(&netInfo, sizeof(netInfo));
    strncpy(netInfo.cNetIfName, pcNetifName, sizeof(netInfo.cNetIfName));
    iRet = intAdaptionDBGetInfo(&netInfo, 1);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    memcpy(pcWebName, netInfo.cWebName, strlen(netInfo.cWebName));
    *piWebIndex = netInfo.iWebIndex;

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: intAdaptionClear
** 功能描述: 清除网口定位
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID intAdaptionClear (VOID)
{
    intAdaptionDBClear();
}

/*********************************************************************************************************
** 函数名称: intAdaptionHandle
** 功能描述: 网口自适应处理
** 输　入  : pWebInfo           WEB 信息
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
*********************************************************************************************************/
INT intAdaptionHandle (__PINT_ADA_SET  pcIntAdaptionInfo)
{
    CHAR   cNetifName[NETIF_NAMESIZE] = {0};
    INT    iRet;

    __INTADAPTION_HANDLE_INFO   intadaptionHandle;
    __INTADAPTION_HANDLE_INFO   NetInfo;

    if (LW_NULL == pcIntAdaptionInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement is null\n");

       return  (PX_ERROR);
    }

    if (INT_ADA_FLAG_FIRST == pcIntAdaptionInfo->cFlag) {               /*  第一个网口需要清空数据库    */
        intAdaptionDBClear();
    }

    iRet = __onlyNetifLinkUpGet(cNetifName);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_LOG_LEVEL, "there is no only link netif\n");

        return  (PX_ERROR);
    }

    bzero(&NetInfo, sizeof(NetInfo));

    strncpy(NetInfo.cNetIfName, cNetifName, sizeof(NetInfo.cNetIfName));

    iRet = intAdaptionDBGetInfo(&NetInfo, 1);                           /*  通过 netif 名字找对应的设备 */
    if (ERROR_NONE == iRet) {                                           /*  已经存在信息                */
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "netif info has saved\n");

        return  (PX_ERROR);
    }

    strncpy(intadaptionHandle.cNetIfName, cNetifName, sizeof(intadaptionHandle.cNetIfName));
    strncpy(intadaptionHandle.cWebName,   pcIntAdaptionInfo->cWebName, sizeof(intadaptionHandle.cWebName));
    strncpy(intadaptionHandle.cStatus,    NET_ADAPTING, sizeof(intadaptionHandle.cStatus));
    intadaptionHandle.iWebIndex = pcIntAdaptionInfo->iWebIndex;

    if (INT_ADA_FLAG_LAST == pcIntAdaptionInfo->cFlag) {                /*  最后一个网口需要更新所有状态*/
        intAdaptionDBSetFinish();

        bzero(intadaptionHandle.cStatus, sizeof(intadaptionHandle.cStatus));

        strncpy(intadaptionHandle.cStatus, NET_FINISH, sizeof(intadaptionHandle.cStatus));
    }

    return (intAdaptionDBUpdate(&intadaptionHandle));
}
