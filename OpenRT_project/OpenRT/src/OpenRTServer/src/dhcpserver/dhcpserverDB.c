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
** 文   件   名: dhcpserverDB.c
**
** 创   建   人: Wang.JingShi ( 王京石)
**
** 文件创建日期: 2018 年 7 月 19 日
**
** 描        述: DHCP Server数据库操作
** 2018.09.20    DHCP SERVER 支持在多网口设备运行多个服务器
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "dhcpserver_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  DHCP Server配置信息
*********************************************************************************************************/
#define DHCPSERVER_TABLE         "dhcpserver"                           /*  DHCP Server表单名           */
#define DHCPSERVER_TABLE_HEAD    "(ID      INTEGER PRIMARY KEY AUTOINCREMENT," \
                                 "ENABLE         INTEGER," \
                                 "IpStart        TEXT," \
                                 "IPEnd          TEXT," \
                                 "interface      TEXT," \
                                 "dns1           TEXT," \
                                 "dns2           TEXT," \
                                 "mask           TEXT," \
                                 "gateway        TEXT," \
                                 "lease          BIGINT," \
                                 "PID            INTEGER)"              /*  DHCP Server表单格式         */
/*********************************************************************************************************
** 函数名称: __dhcpServerUpdateItem
** 功能描述: 更新数据库中的一个条目
** 输  入  : pDb               数据库句柄
**           pDhcpServerInfo   DHCP server 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpServerUpdateItem (sqlite3  *pDb, __PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    CHAR          cSqlCmd[300]   = {0};
    CHAR          cIpStart[32]   = {0};
    CHAR          cIpEnd[32]     = {0};
    CHAR          cIpDns1[32]    = {0};
    CHAR          cIpDns2[32]    = {0};
    CHAR          cIpMask[32]    = {0};
    CHAR          cIpGateway[32] = {0};

    if (LW_NULL == pDhcpServerInfo) {
        return  (PX_ERROR);
    }

    strncpy(cIpStart,   inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpStart),   sizeof(cIpStart));
    strncpy(cIpEnd,     inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpEnd),     sizeof(cIpEnd));
    strncpy(cIpDns1,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns1),    sizeof(cIpDns1));
    strncpy(cIpDns2,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns2),    sizeof(cIpDns2));
    strncpy(cIpMask,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpMask),    sizeof(cIpMask));
    strncpy(cIpGateway, inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpGateway), sizeof(cIpGateway));

    bzero(cSqlCmd, sizeof(cSqlCmd));
    snprintf(cSqlCmd, sizeof(cSqlCmd), "UPDATE %s  SET ENABLE=%d, IpStart='%s', IPEnd='%s', dns1='%s'," \
             "dns2='%s', mask='%s', gateway='%s', lease=%d, PID=%d WHERE interface = '%s';" ,
              DHCPSERVER_TABLE, pDhcpServerInfo->iEnable, cIpStart, cIpEnd, cIpDns1, cIpDns2,
              cIpMask, cIpGateway, pDhcpServerInfo->uiLease, INVALID_PID, pDhcpServerInfo->cInterface);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}

/*********************************************************************************************************
** 函数名称: __dhcpServerInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb               数据库句柄
**           pDhcpServerInfo   DHCP server 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpServerInsertItem (sqlite3 *pDb, __PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    CHAR          cTbInsertCmd[400] = {0};
    CHAR          cIpStart[32]      = {0};
    CHAR          cIpEnd[32]        = {0};
    CHAR          cIpDns1[32]       = {0};
    CHAR          cIpDns2[32]       = {0};
    CHAR          cIpMask[32]       = {0};
    CHAR          cIpGateway[32]    = {0};

    if (LW_NULL == pDhcpServerInfo) {
        return  (PX_ERROR);
    }

    strncpy(cIpStart,   inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpStart),   sizeof(cIpStart));
    strncpy(cIpEnd,     inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpEnd),     sizeof(cIpEnd));
    strncpy(cIpDns1,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns1),    sizeof(cIpDns1));
    strncpy(cIpDns2,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpDns2),    sizeof(cIpDns2));
    strncpy(cIpMask,    inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpMask),    sizeof(cIpMask));
    strncpy(cIpGateway, inet_ntoa(*(struct in_addr *)&pDhcpServerInfo->uiIpGateway), sizeof(cIpGateway));

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));

    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd), "INSERT INTO %s (ENABLE, IpStart, IPEnd, " \
            "interface, dns1, dns2, mask, gateway, lease, PID)"" VALUES(%d, '%s', '%s', '%s', " \
            "'%s', '%s', '%s', '%s', %d, %d);", DHCPSERVER_TABLE, pDhcpServerInfo->iEnable,
            cIpStart, cIpEnd, pDhcpServerInfo->cInterface, cIpDns1, cIpDns2, cIpMask, cIpGateway,
            pDhcpServerInfo->uiLease, INVALID_PID);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** 函数名称: dhcpServerGetFromDB
** 功能描述: 从数据库获取 dhcpServer 信息
** 输  入  : NONE
** 输  出  : piNum     dhcpServer 信息数目
** 返  回  : 成功返回 dhcp Server 信息，失败返回 NULL
*********************************************************************************************************/
PVOID  dhcpServerGetFromDB (INT  *piNum)
{
    INT                    iRet;
    CHAR                   cSelectSql[200]   = {0};
    sqlite3               *pDb;
    CHAR                 **cDbResult;
    CHAR                  *pcErrMsg          = LW_NULL;
    INT                    iRow;
    INT                    iColumn;
    PCHAR                  pDhcpServerHeader = LW_NULL;
    __PDHCPSERVER_HANDLE   pDhcpServ         = LW_NULL;
    INT                    i;

    *piNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (LW_NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, LW_NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", DHCPSERVER_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        pDhcpServerHeader = (PCHAR)malloc(iRow * sizeof(__DHCPSERVER_HANDLE));
        if (pDhcpServerHeader == LW_NULL) {
            goto  Error_Handle;
        }

        pDhcpServ = (__PDHCPSERVER_HANDLE)pDhcpServerHeader;

        bzero(pDhcpServerHeader, iRow * sizeof(__DHCPSERVER_HANDLE));

        for (i = 1; i <= iRow; i++) {                                   /*  按行获取(过滤第一行)        */
            pDhcpServ->iEnable       = atoi(cDbResult[i * iColumn + 1]);
            pDhcpServ->uiIpStart     = inet_addr(cDbResult[i * iColumn + 2]);
            pDhcpServ->uiIpEnd       = inet_addr(cDbResult[i * iColumn + 3]);

            strncpy(pDhcpServ->cInterface, cDbResult[i * iColumn + 4], sizeof(pDhcpServ->cInterface));

            pDhcpServ->uiIpDns1      = inet_addr(cDbResult[i * iColumn + 5]);
            pDhcpServ->uiIpDns2      = inet_addr(cDbResult[i * iColumn + 6]);
            pDhcpServ->uiIpMask      = inet_addr(cDbResult[i * iColumn + 7]);
            pDhcpServ->uiIpGateway   = inet_addr(cDbResult[i * iColumn + 8]);
            pDhcpServ->uiLease       = atoi(cDbResult[i * iColumn + 9]);

            pDhcpServ += 1;
        }
    }

Error_Handle:
    *piNum = iRow;

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, LW_NULL);

    return  (pDhcpServerHeader);
}
/*********************************************************************************************************
** 函数名称: dhcpServerPidGetFromDB
** 功能描述: 从数据库获取 dhcp server 进程 pid 信息
** 输  入  : cpcIfName 网口名
** 输  出  : pPid      dhcp server 进程 id 号
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerPidGetFromDB (CPCHAR  cpcIfName, pid_t  *pPid)
{
    INT               iRet;
    CHAR              cSelectSql[200]  = {0};
    CHAR            **cDbResult;
    CHAR             *pcErrMsg         = LW_NULL;
    INT               iRow;
    INT               iColumn;
    sqlite3          *pDb              = LW_NULL;

    *pPid = INVALID_PID;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == cpcIfName || LW_NULL == pPid) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, LW_NULL);
    snprintf(cSelectSql, sizeof(cSelectSql),
            "SELECT * FROM %s WHERE interface = '%s';", DHCPSERVER_TABLE, cpcIfName);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            *pPid = atoi(cDbResult[1 * iColumn + 10]);
        } else {
            *pPid = -1;
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, LW_NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dhcpServerUpdatePidToDb
** 功能描述: 更新 dhcp server 进程 pid 进程号
** 输  入  : cpcIfName    网口名
**           pid          进程 id
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerUpdatePidToDb (CPCHAR  cpcIfName, pid_t  pid)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == cpcIfName)) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "UPDATE %s  SET PID = %d WHERE interface = '%s';" ,
            DHCPSERVER_TABLE, pid, cpcIfName);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** 函数名称: dhcpServerUpdateStatusToDb
** 功能描述: 更新 dhcp server 进程状态
** 输  入  : cpcIfName    网口名
**           iStatus      dhcp server 状态
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerUpdateStatusToDb (CPCHAR  cpcIfName, INT  iStatus)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if ((LW_NULL == pDb) || (LW_NULL == cpcIfName)) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "UPDATE %s  SET ENABLE = %d WHERE interface = '%s';" ,
            DHCPSERVER_TABLE, iStatus, cpcIfName);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** 函数名称: dhcpServerAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pDhcpServerInfo   dhcp server信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerAddItemToDb (__PDHCPSERVER_HANDLE  pDhcpServerInfo)
{
    CHAR          cSqlCmd[128]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    INT           iFlag = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pDhcpServerInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where interface = '%s';",
             DHCPSERVER_TABLE,
             pDhcpServerInfo->cInterface);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (ERROR_NONE == iFlag) {
        iRet = __dhcpServerUpdateItem(pDb, pDhcpServerInfo);
    } else {
        iRet = __dhcpServerInsertItem(pDb, pDhcpServerInfo);
    }

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: dhcpServerDelItemFromDb
** 功能描述: 从数据库中删除一个条目
** 输  入  : cpcIfName   网口名
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerDelItemFromDb (CPCHAR  cpcIfName)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == cpcIfName) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "DELETE FROM %s WHERE interface = '%s';",
             DHCPSERVER_TABLE,
             cpcIfName);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** 函数名称: dhcpServerTableCreate
** 功能描述: 初始化DHCP Server数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  dhcpServerTableCreate (VOID)
{
    sqlite3   *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, DHCPSERVER_TABLE, DHCPSERVER_TABLE_HEAD);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
