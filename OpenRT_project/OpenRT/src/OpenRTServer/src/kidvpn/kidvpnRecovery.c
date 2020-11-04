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
** 文   件   名: kidvpnRecovery.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 27 日
**
** 描        述: VPN 恢复
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "vnd/vnd_common.h"
#include "vnd/vndDB.h"
#include "kidvpn_common.h"
#include "kidvpnDB.h"
#include "route/route_common.h"
#include "database/database_common.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
/*********************************************************************************************************
  条目存在标志宏定义
*********************************************************************************************************/
#define ITEM_EXIST                    (1)                               /*  条目存在                    */
#define ITEM_NOT_EXIST                (0)                               /*  条目不存在                  */
/*********************************************************************************************************
** 函数名称: __vpnRouteInfoDelFromDb
** 功能描述: 从数据库中删除虚拟网卡的静态路由信息
** 输　入  : iVndId             虚拟网卡 ID
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __vpnRouteInfoDelFromDb (INT  iVndID)
{
    CHAR                  cIfName[NETIF_NAMESIZE] = {0};
    INT                   iNetIfIndex             = 0;
    INT                   iRet;

    /*
     *  获取 VND 网卡设备索引值
     */
    iRet = netIfIndexGetByVndId(iVndID, &iNetIfIndex);
    if (ERROR_NONE == iRet) {
        sprintf(cIfName, "vn%d", iNetIfIndex);

        iRet = staticRouteCleanFromDbByIfName(cIfName);
        if (PX_ERROR == iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete static router item from db error\n");

            return  (PX_ERROR);
        }
    }

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: __kidvpnItemExistFlagGet
** 功能描述: 根据 VNDID 获取存在状态
** 输  入  : iVndId     VPN ID
** 输  出  : NONE
** 返  回  : 存在标志，存在返回1，不存在返回0
*********************************************************************************************************/
static INT  __kidvpnItemExistFlagGet (INT  iVndId)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iFlag         = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "select * from %s where VNDID = %d;", KIDVPN_TABLE, iVndId);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (PX_ERROR == iFlag) {
        return  (ITEM_NOT_EXIST);
    }

    return  (ITEM_EXIST);
}
/*********************************************************************************************************
** 函数名称: __invalidVndInfoDel
** 功能描述: 从 VND 数据库中删去无效的 VND 信息
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __invalidVndInfoDel (VOID)
{
    INT                   iRet;
    INT                   iItemNum         = 0;
    __PVND_HANDLE         pVndHandleInfo   = NULL;
    __PVND_HANDLE         pTmpInfo         = NULL;
    INT                   iFlag;
    INT                   i;

    iItemNum = vndGetDBRow();
    if (0 == iItemNum) {
        return  (ERROR_NONE);
    } else if (iItemNum < 0) {
        return  (PX_ERROR);
    }

    pVndHandleInfo = (__PVND_HANDLE)vndGetDBInfo();
    if (NULL == pVndHandleInfo) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iItemNum; i++) {
        pTmpInfo = pVndHandleInfo + i;

        iFlag = __kidvpnItemExistFlagGet(pTmpInfo->iVndId);
        if (ITEM_NOT_EXIST == iFlag) {
            iRet = __vpnRouteInfoDelFromDb(pTmpInfo->iVndId);
            if (PX_ERROR == iRet) {
                free(pVndHandleInfo);

                return  (PX_ERROR);
            }

            iRet = vndDelDBItem(pTmpInfo->iVndId);
            if (PX_ERROR == iRet) {
                free(pVndHandleInfo);

                return  (PX_ERROR);
            }
        }
    }

    free(pVndHandleInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __vndItemExistFlagGet
** 功能描述: 根据 vnd 获取存在状态
** 输  入  : iVndId     VPN ID
** 输  出  : NONE
** 返  回  : 存在标志，存在返回1，不存在返回0
*********************************************************************************************************/
static INT  __vndItemExistFlagGet (INT  iVndId)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iFlag         = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "select * from %s where ID = %d;", VNDINFO_TABLE, iVndId);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (PX_ERROR == iFlag) {
        return  (ITEM_NOT_EXIST);
    }

    return  (ITEM_EXIST);
}
/*********************************************************************************************************
** 函数名称: __invalidKidVpnInfoDel
** 功能描述: 从 KIDVPN 数据库中删去无效的 KidVpn 信息
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __invalidKidVpnInfoDel (VOID)
{
    INT                   iRet;
    INT                   iItemNum         = 0;
    __PKIDVPN_DB_INFO     pKidVndInfo      = NULL;
    __PKIDVPN_DB_INFO     pTmpInfo         = NULL;
    INT                   iFlag;
    INT                   i;

    iItemNum = kidvpnGetDBRow();
    if (0 == iItemNum) {
        return  (ERROR_NONE);
    } else if (iItemNum < 0) {
        return  (PX_ERROR);
    }

    pKidVndInfo = (__PKIDVPN_DB_INFO)kidvpnGetDBInfo();
    if (NULL == pKidVndInfo) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iItemNum; i++) {
        pTmpInfo = pKidVndInfo + i;

        iFlag = __vndItemExistFlagGet(pTmpInfo->kidvpnHandle.iVndID);
        if (ITEM_NOT_EXIST == iFlag) {
            iRet = kidvpnDelDBItem(pTmpInfo->kidvpnHandle.iVndID);
            if (PX_ERROR == iRet) {
                return  (PX_ERROR);
            }
        }
    }

    free(pKidVndInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: kidvpnRecovery
** 功能描述: 恢复重启前的 VPN 连接(要确保 VND 处理线程已经开启)
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : 是否恢复成功
*********************************************************************************************************/
INT  kidvpnRecovery (VOID)
{
    CHAR                  cSelectSql[200]  = {0};
    INT                   iRet;
    CHAR                **cDbResult;
    INT                   IRow, IColumn;
    sqlite3              *pDb              = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;

    INT                   i;
    __KIDVPN_DB_INFO      kidvpnDBInfo;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

        return  (PX_ERROR);
    }

    iRet = __invalidKidVpnInfoDel();
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    iRet = __invalidVndInfoDel();
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    sprintf(cSelectSql, "SELECT * FROM %s;", KIDVPN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
            bzero(&kidvpnDBInfo, sizeof(kidvpnDBInfo));
            kidvpnDBInfo.kidvpnHandle.cHandleType = atoi(cDbResult[i * IColumn + 0]);
            kidvpnDBInfo.kidvpnHandle.iVndID      = atoi(cDbResult[i * IColumn + 1]);
            kidvpnDBInfo.kidvpnHandle.iMtu        = atoi(cDbResult[i * IColumn + 2]);
            kidvpnDBInfo.kidvpnHandle.iPort       = atoi(cDbResult[i * IColumn + 3]);

            inet_aton(cDbResult[i * IColumn + 5], (struct in_addr *)&(kidvpnDBInfo.kidvpnHandle.uiIpAddr));

            strncpy(kidvpnDBInfo.kidvpnHandle.cPassWd,
                    cDbResult[i * IColumn + 7],
                    sizeof(kidvpnDBInfo.kidvpnHandle.cPassWd));

            /*
             *  进行恢复, 先恢复虚拟网卡
             */
            iRet = vndRecovery(kidvpnDBInfo.kidvpnHandle.iVndID);
            if (ERROR_NONE == iRet) {
                if(PX_ERROR == kidVpnStart(&(kidvpnDBInfo.kidvpnHandle))) {
                    vndDel(kidvpnDBInfo.kidvpnHandle.iVndID);
                }
            } else {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "recovery vnd failed.\n");
            }
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
