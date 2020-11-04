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
** 文   件   名: vndDB.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述: 虚拟网卡数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "common.h"
#include "vnd_common.h"
#include "libvnd_config.h"
#include "network/lwip/inet.h"
#include "database/database_common.h"

/*********************************************************************************************************
** 函数名称: __vndUpdateItem
** 功能描述: 更新 vnd 数据中的一个条目
** 输  入  : pDb               数据库句柄
**           pVndInfo          虚拟网卡信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __vndUpdateItem (sqlite3  *pDb, __PVND_HANDLE  pVndInfo)
{
    CHAR          cSqlCmd[300]  = {0};
    CHAR          cMacBuf[20]   = {0};

    CHAR          cIpBuf[IP4ADDR_STRLEN_MAX]      = {0};
    CHAR          cGwBuf[IP4ADDR_STRLEN_MAX]      = {0};
    CHAR          cNetMaskBuf[IP4ADDR_STRLEN_MAX] = {0};
    INT           iRet;

    if (LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    snprintf(cMacBuf,
             sizeof(cMacBuf),
             "%02x:%02x:%02x:%02x:%02x:%02x",
             pVndInfo->ucHwAddr[0],
             pVndInfo->ucHwAddr[1],
             pVndInfo->ucHwAddr[2],
             pVndInfo->ucHwAddr[3],
             pVndInfo->ucHwAddr[4],
             pVndInfo->ucHwAddr[5]);

    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiIpAddr)), cIpBuf, sizeof(cIpBuf));
    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiNetMask)), cNetMaskBuf, sizeof(cNetMaskBuf));
    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiGateWay)), cGwBuf, sizeof(cGwBuf));

    bzero(cSqlCmd, sizeof(cSqlCmd));
    snprintf(cSqlCmd,
            sizeof(cSqlCmd),
            "UPDATE %s SET IP = '%s', "
            "NETMASK = '%s', GATEWAY = '%s', MAC = '%s', MTU = %d, NETIFINDEX = %d "
            "WHERE ID = %d;" ,
            VNDINFO_TABLE,
            cIpBuf,
            cNetMaskBuf,
            cGwBuf,
            cMacBuf,
            pVndInfo->iMtu,
            pVndInfo->iNetIfIndex,
            pVndInfo->iVndId);

    iRet = dataBaseCmdExec(pDb, cSqlCmd);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: __vndInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb               数据库句柄
**           pVndInfo          虚拟网卡信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __vndInsertItem (sqlite3 *pDb, __PVND_HANDLE  pVndInfo)
{
    CHAR               cTbInsertCmd[200]               = {0};
    CHAR               cMacBuf[20]                     = {0};
    CHAR               cIpBuf[IP4ADDR_STRLEN_MAX]      = {0};
    CHAR               cGwBuf[IP4ADDR_STRLEN_MAX]      = {0};
    CHAR               cNetMaskBuf[IP4ADDR_STRLEN_MAX] = {0};

    INT                iRet;

    if (LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    sprintf(cMacBuf,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            pVndInfo->ucHwAddr[0],
            pVndInfo->ucHwAddr[1],
            pVndInfo->ucHwAddr[2],
            pVndInfo->ucHwAddr[3],
            pVndInfo->ucHwAddr[4],
            pVndInfo->ucHwAddr[5]);

    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiIpAddr)), cIpBuf, sizeof(cIpBuf));
    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiNetMask)), cNetMaskBuf, sizeof(cNetMaskBuf));
    inet_ntoa_r(*(struct in_addr *)(&(pVndInfo->uiGateWay)), cGwBuf, sizeof(cGwBuf));

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));
    snprintf(cTbInsertCmd,
            sizeof(cTbInsertCmd),
            "INSERT INTO %s (ID, IP, NETMASK, GATEWAY, MAC, MTU, NETIFINDEX) "
            "VALUES (%d, '%s', '%s', '%s', '%s', %d, %d);",
            VNDINFO_TABLE,
            pVndInfo->iVndId,
            cIpBuf,
            cNetMaskBuf,
            cGwBuf,
            cMacBuf,
            pVndInfo->iMtu,
            pVndInfo->iNetIfIndex);

    iRet = dataBaseCmdExec(pDb, cTbInsertCmd);
    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: __vndGetFromDB
** 功能描述: 从数据库获取所有数据库的信息
** 输  入  : pcVndInfoBuf      信息存储的 buf
** 输  出  : NONE
** 返  回  : pcKidVPNInfoBuf
*********************************************************************************************************/
static  INT  __vndGetFromDB (PCHAR   pcVndInfoBuf)
{
    INT             iRet;
    INT             IRow, IColumn;
    CHAR            cSelectSql[200]      = {0};
    CHAR           *pcErrMsg             = LW_NULL;
    CHAR          **cDbResult            = LW_NULL;
    sqlite3        *pDb                  = LW_NULL;

    __PVND_HANDLE  pcVndInfo             = LW_NULL;

    INT             i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pcVndInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    pcVndInfo = (__PVND_HANDLE)pcVndInfoBuf;

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", VNDINFO_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
            pcVndInfo->iVndId = atoi(cDbResult[i * IColumn + 0]);
            inet_aton(cDbResult[i * IColumn + 1], (struct in_addr *)&(pcVndInfo->uiIpAddr));
            inet_aton(cDbResult[i * IColumn + 2], (struct in_addr *)&(pcVndInfo->uiNetMask));
            inet_aton(cDbResult[i * IColumn + 3], (struct in_addr *)&(pcVndInfo->uiGateWay));

            sscanf(cDbResult[i * IColumn + 4],
                    "%02x:%02x:%02x:%02x:%02x:%02x",
                    (UINT *)&pcVndInfo->ucHwAddr[0],
                    (UINT *)&pcVndInfo->ucHwAddr[1],
                    (UINT *)&pcVndInfo->ucHwAddr[2],
                    (UINT *)&pcVndInfo->ucHwAddr[3],
                    (UINT *)&pcVndInfo->ucHwAddr[4],
                    (UINT *)&pcVndInfo->ucHwAddr[5]);

            pcVndInfoBuf = pcVndInfoBuf + sizeof(__VND_HANDLE);
            pcVndInfo    = (__PVND_HANDLE)(pcVndInfoBuf);
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base wan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: vndGetDBRow
** 功能描述: 从数据库获取信息个数
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : iRow              信息个数
*********************************************************************************************************/
INT  vndGetDBRow (VOID)
{
    INT        iRow     = 0;
    INT        iColumn  = 0;
    INT        iRet     = PX_ERROR;
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iRet = dataBaseGetTbRowCol(pDb, VNDINFO_TABLE, &iRow, &iColumn);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    return  (iRow);
}

/*********************************************************************************************************
** 函数名称: vndGetDBInfo
** 功能描述: 从数据库获取所有数据库的信息 (需要调用者自己释放内存)
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : pcVndInfoBuf      vnd 信息
*********************************************************************************************************/
PVOID  vndGetDBInfo (VOID)
{
    PCHAR                 pcVndInfoBuf     = LW_NULL;
    INT                   iItemNum         = 0;
    INT                   iBufLen          = 0;
    INT                   iRet             = PX_ERROR;

    iItemNum = vndGetDBRow();
    if (0 == iItemNum) {

        return  (LW_NULL);
    }

    iBufLen         = iItemNum * sizeof(__VND_HANDLE);
    pcVndInfoBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcVndInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (LW_NULL);
    }

    bzero(pcVndInfoBuf, iBufLen);
    iRet = __vndGetFromDB(pcVndInfoBuf);                                /*  获取 信息                   */
    if (PX_ERROR == iRet) {
        free(pcVndInfoBuf);

        return  (LW_NULL);
    }

    return  (pcVndInfoBuf);
}

/*********************************************************************************************************
** 函数名称: vndGetInfoById
** 功能描述: 通过 ID 获取虚拟网卡信息
** 输  入  : iVndID            虚拟网卡 ID
** 输  出  : pVndInfo          虚拟网卡信息
** 返  回  : ERROR_CODE
*********************************************************************************************************/
UINT32  vndGetInfoById (INT  iVndID, __PVND_HANDLE  pVndInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    sqlite3      *pDb           = LW_NULL;

    CHAR         *cpIp          = LW_NULL;
    CHAR         *cpNetMask     = LW_NULL;
    CHAR         *cpGateWay     = LW_NULL;
    CHAR         *cpMac         = LW_NULL;

    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSqlCmd, sizeof(cSqlCmd), "select * from %s where ID = %d;", VNDINFO_TABLE, iVndID);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  条目已经存在                */
        pVndInfo->iVndId = sqlite3_column_int(pstmt, 0);

        cpIp = (char *)sqlite3_column_text(pstmt, 1);
        inet_aton(cpIp, (struct in_addr *)&(pVndInfo->uiIpAddr));

        cpNetMask = (char *)sqlite3_column_text(pstmt, 2);
        inet_aton(cpNetMask, (struct in_addr *)&(pVndInfo->uiNetMask));

        cpGateWay = (char *)sqlite3_column_text(pstmt, 3);
        inet_aton(cpGateWay, (struct in_addr *)&(pVndInfo->uiGateWay));

        cpMac = (char *)sqlite3_column_text(pstmt, 4);
        sscanf(cpMac,
               "%02x:%02x:%02x:%02x:%02x:%02x",
               (UINT *)&pVndInfo->ucHwAddr[0],
               (UINT *)&pVndInfo->ucHwAddr[1],
               (UINT *)&pVndInfo->ucHwAddr[2],
               (UINT *)&pVndInfo->ucHwAddr[3],
               (UINT *)&pVndInfo->ucHwAddr[4],
               (UINT *)&pVndInfo->ucHwAddr[5]);

        pVndInfo->iMtu   = sqlite3_column_int(pstmt, 5);

        iRet = ERROR_NONE;
    } else {
        iRet = PX_ERROR;
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: netIfIndexGetByVndId
** 功能描述: 通过 vnd ID 获取 NetIfIndex
** 输  入  : iVndid          虚拟网卡 ID
** 输  出  : piNetIfIndex    虚拟网卡索引
** 返  回  : ERROR_CODE
*********************************************************************************************************/
UINT32  netIfIndexGetByVndId (INT  iVndid, INT  *piNetIfIndex)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == piNetIfIndex) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where ID = %d;",
             VNDINFO_TABLE,
             iVndid);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  条目已经存在                */
        *piNetIfIndex = sqlite3_column_int(pstmt, 6);

        iRet = ERROR_NONE;
    } else {
        iRet = PX_ERROR;
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: vndIdGetByNetIfIndex
** 功能描述: 通过 NetIfIndex 获取 vnd ID
** 输  入  : iNetIfIndex     虚拟网卡索引
** 输  出  : piVndid         虚拟网卡 ID
** 返  回  : ERROR_CODE
*********************************************************************************************************/
UINT32  vndIdGetByNetIfIndex (INT  iNetIfIndex, INT  *piVndid)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == piVndid) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where NETIFINDEX = %d;",
             VNDINFO_TABLE,
             iNetIfIndex);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  条目已经存在                */
        *piVndid = sqlite3_column_int(pstmt, 0);

        iRet = ERROR_NONE;
    } else {
        iRet = PX_ERROR;
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: vndDelDBItem
** 功能描述: 删除数据库中指定条目
** 输  入  : pDb               数据库句柄
**           iVndID            虚拟网卡  ID
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  vndDelDBItem (INT  iVndID)
{
    CHAR               cDelCmd[200]       = {0};
    sqlite3           *pDb                = LW_NULL;

    INT                iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s WHERE ID = %d;", VNDINFO_TABLE, iVndID);

    iRet = dataBaseCmdExec(pDb, cDelCmd);

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: vndAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pVndInfo          虚拟网卡信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  vndAddItemToDb (__PVND_HANDLE  pVndInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    INT           iFlag = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
            sizeof(cSqlCmd),
            "select * from %s where ID = %d;",
            VNDINFO_TABLE,
            pVndInfo->iVndId);
    iFlag = dataBaseItemCheck(pDb, cSqlCmd);

    if (ERROR_NONE == iFlag) {
        iRet = __vndUpdateItem(pDb, pVndInfo);
    } else {
        iRet = __vndInsertItem(pDb, pVndInfo);
    }

    return  (iRet);
}

/*********************************************************************************************************
** 函数名称: vndTableCreate
** 功能描述: 初始化 vnd 数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
INT  vndTableCreate (VOID)
{
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, VNDINFO_TABLE, VNDINFO_TABLE_HEAD);
}

