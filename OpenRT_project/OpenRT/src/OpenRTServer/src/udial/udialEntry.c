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
** 文   件   名: udialEntry.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 16 日
**
** 描        述: OpenRT 拨号处理入口
*********************************************************************************************************/
#include "msgcomm.h"
#include "common.h"
#include "udial_common.h"
#include "udialExec.h"
#include "udialDB.h"
#include "libudial_config.h"
/*********************************************************************************************************
** 函数名称: __udialStart
** 功能描述: 启动拨号入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialStart (PVOID  pArgBuf)
{
    __PUDIAL_HANDLE  pUdialHandleInfo = LW_NULL;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pUdialHandleInfo = (__PUDIAL_HANDLE)pArgBuf;

    replyConfigMsg(OPENRT_UDIAL_DEL_ACK, udialStart(pUdialHandleInfo));
}

/*********************************************************************************************************
** 函数名称: __udialDel
** 功能描述: 删除拨号入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialDel (PVOID  pArgBuf)
{
    PCHAR  pcNetifname = LW_NULL;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pcNetifname = (PCHAR)pArgBuf;

    replyConfigMsg(OPENRT_UDIAL_DEL_ACK, udialDel(pcNetifname));
}

/*********************************************************************************************************
** 函数名称: __udialGetInfo
** 功能描述: 获取拨号信息入口
** 输　入  : pArgBuf            接收的参数
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __udialGetInfo (PVOID  pArgBuf)
{
    PCHAR           pcNetifname = LW_NULL;
    __UDIAL_HANDLE  udialInfo;

    if (LW_NULL == pArgBuf) {
        return;
    }

    pcNetifname = (PCHAR)pArgBuf;

    bzero(&udialInfo, sizeof(udialInfo));
    udialGetSpecInfo(&udialInfo, "NETIF", pcNetifname, LW_NULL);        /*  通过物理网卡名称获取信息    */

    accountInfoGetFromIniFile(pcNetifname, udialInfo.cUsrname, udialInfo.cPassword);

    replyQueryMsg(OPENRT_UDIAL_INFO_ACK, sizeof(udialInfo), 1, &udialInfo);
}

/*********************************************************************************************************
** 函数名称: udialHandleEntry
** 功能描述: 拨号配置入口
** 输　入  : iCommand           服务命令
**           pArgBuf            处理内容
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID udialHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_UDIAL_START:
        __udialStart(pArgBuf);
        break;

    case OPENRT_UDIAL_DEL:
        __udialDel(pArgBuf);
        break;

    case OPENRT_UDIAL_INFO:
        __udialGetInfo(pArgBuf);
        break;

    default:
        break;
    }
}

