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
** 文   件   名: intAdaEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 16 日
**
** 描        述: 网卡定位操作入口
*********************************************************************************************************/
#include <string.h>
#include "lwip/netif.h"
#include "intada_common.h"
#include "intAdaptionExec.h"
#include "msgcomm.h"
#include "common.h"
/*********************************************************************************************************
** 函数名称: intAdaHandleEntry
** 功能描述: 网卡自适应定位入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  intAdaHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    __PINT_ADA_SET  pIntAdaSetInfo                = LW_NULL;
    CHAR            cWebNetifName[NETIF_NAMESIZE] = {0};
    CHAR            cNetIfName[NETIF_NAMESIZE]    = {0};
    INT             iRet;

    switch (iCommand) {
    case OPENRT_ADA_GETS_CMD:
        replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, intAdaptionCheck());

        break;

    case OPENRT_NETIF_GET_CMD:
        if (LW_NULL == pArgBuf) {
            goto  __get_netif_err;
        }

        if (strlen(pArgBuf) >=  NETIF_NAMESIZE) {
            goto  __get_netif_err;
        }

        strncpy(cWebNetifName, pArgBuf, sizeof(cWebNetifName));

        iRet = intAdaptionGetNetif(cWebNetifName, cNetIfName);          /*  获取网卡信息名称            */
        if (ERROR_NONE != iRet) {
            goto  __get_netif_err;
        }

        replyQueryMsg(OPENRT_NETIF_GET_ACK_CMD, sizeof(cNetIfName), 1, cNetIfName);

        break;

    case OPENRT_WEBNETIF_GET_CMD:
        if (LW_NULL == pArgBuf) {
            goto  __get_webname_err;
        }

        if (strlen(pArgBuf) >=  NETIF_NAMESIZE) {
            goto  __get_webname_err;
        }

        strncpy(cNetIfName, pArgBuf, sizeof(cNetIfName));

        iRet = intAdaptionGetWebNetif(cNetIfName, cWebNetifName);          /*  获取 web 网卡信息名称    */
        if (ERROR_NONE != iRet) {
            goto  __get_webname_err;
        }

        replyQueryMsg(OPENRT_WEBNETIF_GET_ACK_CMD, sizeof(cWebNetifName), 1, cWebNetifName);

        break;

    case OPENRT_ADA_SET_CMD:
        if (LW_NULL == pArgBuf) {
            replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, PX_ERROR);

            break;
        }

        pIntAdaSetInfo = (__PINT_ADA_SET)pArgBuf;

        /*
         *  处理定位操作
         */
        replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, intAdaptionHandle(pIntAdaSetInfo));

        break;

    case OPENRT_ADA_CLEAR_CMD:
        intAdaptionClear();

        replyConfigMsg(OPENRT_ADA_CLEAR_ACK_CMD, 0);

        break;

    default:
        break;
    }

    return;

__get_netif_err:
    replyQueryMsg(OPENRT_NETIF_GET_ACK_CMD, sizeof(cNetIfName), 0, NULL);

    return;
__get_webname_err:
    replyQueryMsg(OPENRT_WEBNETIF_GET_ACK_CMD, sizeof(cWebNetifName), 0, NULL);

    return;
}
