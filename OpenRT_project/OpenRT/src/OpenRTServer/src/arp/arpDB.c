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
** 文   件   名: arpDB.c
**
** 创   建   人: Wang.JingShi ( 王京石)
**
** 文件创建日期: 2018 年 7 月 13 日
**
** 描        述: arp数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "arp_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  arp 配置信息
*********************************************************************************************************/
#define ARP_TABLE                "arp"                                  /*  arp表单名                   */
#define ARP_TABLE_HEAD           "(ID      INTEGER PRIMARY KEY AUTOINCREMENT," \
                                 "IP       TEXT," \
                                 "MAC      TEXT)"                       /*  arp表单格式                 */
/*********************************************************************************************************
** 函数名称: __arpIsExists
** 功能描述: 判断 ARP 是否存在
** 输  入  : pArpInfo     ARP 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpIsExists (__PARP_HANDLE  pArpInfo)
{
    CHAR           cSelectSql[200]  = {0};
    CHAR         **cDbResult        = LW_NULL;
    CHAR          *pcErrMsg         = LW_NULL;
    sqlite3       *pDb              = LW_NULL;
    INT            iRow             = 0;
    INT            iColumn          = 0;
    INT            isExist          = LW_FALSE;
    INT            iRet;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s WHERE IP='%s' ;",
             ARP_TABLE, inet_ntoa(*(struct in_addr *)&pArpInfo->uiIpAddr));

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        isExist = LW_TRUE;
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);

    return  (isExist);
}
/*********************************************************************************************************
** 函数名称: arpGetFromDB
** 功能描述: 从数据库获取 ARP 信息
** 输  入  : NONE
** 输  出  : piRecordNum     ARP 信息个数
** 返  回  : 成功返回 ARP 信息，失败返回 NULL
*********************************************************************************************************/
PVOID  arpGetFromDB (INT  *piRecordNum)
{
    CHAR            cSelectSql[200]  = {0};
    CHAR          **cDbResult        = LW_NULL;
    CHAR           *pcErrMsg         = LW_NULL;
    sqlite3        *pDb              = LW_NULL;
    __PARP_HANDLE   pArpHeader       = LW_NULL;
    __PARP_HANDLE   pTmpNode         = LW_NULL;
    INT             iRet;
    INT             iRow, iColumn;
    INT             i;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", ARP_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK != iRet || 0 == iRow) {
        goto  Error_Handle;
    }

    pArpHeader = (__PARP_HANDLE)malloc(sizeof(__ARP_HANDLE) * iRow);
    if (LW_NULL == pArpHeader){
        goto  Error_Handle;
    }

    bzero(pArpHeader, sizeof(__ARP_HANDLE) * iRow);

    for (i = 1; i <= iRow; i++) {                                       /*  按行获取(过滤第一行)        */
        pTmpNode = (__PARP_HANDLE)(pArpHeader + (i - 1));

        pTmpNode->uiIpAddr = inet_addr(cDbResult[i * iColumn + 1]);
        sscanf(cDbResult[i * iColumn + 2],
               "%02x:%02x:%02x:%02x:%02x:%02x",
               (UINT *)&pTmpNode->ucHwAddr[0],
               (UINT *)&pTmpNode->ucHwAddr[1],
               (UINT *)&pTmpNode->ucHwAddr[2],
               (UINT *)&pTmpNode->ucHwAddr[3],
               (UINT *)&pTmpNode->ucHwAddr[4],
               (UINT *)&pTmpNode->ucHwAddr[5]);
    }

    *piRecordNum = iRow;
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pArpHeader);

Error_Handle:
    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    *piRecordNum = 0;

    return  (LW_NULL);
}

/*********************************************************************************************************
** 函数名称: __arpInsertItem
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb       数据库句柄
**           pArpInfo  ARP 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpInsertItem (sqlite3  *pDb, __PARP_HANDLE  pArpInfo)
{
    CHAR          cTbInsertCmd[200]  = {0};
    CHAR          cMACData[20]       = {0};
    PCHAR         pIPData            = LW_NULL;
    sqlite3_stmt *pstmt              = LW_NULL;
    const CHAR   *pcTail             = LW_NULL;
    INT           iRet;

    if (LW_NULL == pArpInfo) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd),
             "INSERT INTO %s (IP, MAC) VALUES(?, ?);", ARP_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  将需要添加的信息加入数据库
     */
    pIPData = arpTypeTransform(pArpInfo, cMACData, sizeof(cMACData));
    if (NULL == pIPData) {
        sqlite3_finalize(pstmt);

        return  (PX_ERROR);
    }

    sqlite3_bind_text(pstmt,  1,  pIPData, strlen(pIPData), LW_NULL);
    sqlite3_bind_text(pstmt,  2,  cMACData, strlen(cMACData), LW_NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __arpDeleteItem
** 功能描述: 删除 arp 中的一个条目
** 输  入  : pDb       数据库句柄
**           pArpInfo  ARP 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpDeleteItem (sqlite3  *pDb, __PARP_HANDLE  pArpInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    PCHAR         pIPData       = LW_NULL;

    if (LW_NULL == pArpInfo) {
        return  (PX_ERROR);
    }

    pIPData = inet_ntoa(*(struct in_addr *)&pArpInfo->uiIpAddr);
    if (NULL == pIPData) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
             "delete from %s where IP = '%s';",
             ARP_TABLE, pIPData);

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: arpDeleteItemByIp
** 功能描述: 删除 arp 中的一个条目
** 输  入  : pArpInfo     ARP 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  arpDeleteItemByIp (__PARP_HANDLE  pArpInfo)
{
    sqlite3  *pDb = LW_NULL;

    if (LW_NULL == pArpInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb) {
        return  (PX_ERROR);
    }

    return  __arpDeleteItem(pDb, pArpInfo);
}
/*********************************************************************************************************
** 函数名称: __arpUpdateItem
** 功能描述: 更新arp中的一个条目
** 输  入  : pDb       数据库句柄
**           pArpInfo  ARP 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __arpUpdateItem (sqlite3  *pDb, __PARP_HANDLE  pArpInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    CHAR          cMACData[20]  = {0};
    PCHAR         pIPData       = LW_NULL;

    if (LW_NULL == pArpInfo) {
        return  (PX_ERROR);
    }

    pIPData = arpTypeTransform(pArpInfo, cMACData, sizeof(cMACData));
    if (NULL == pIPData) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
             "update %s set MAC = '%s' where IP = '%s';",
             ARP_TABLE, cMACData, pIPData);

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: arpUpdateToDb
** 功能描述: 向数据库中添加一个条目
** 输  入  : cpcOperType          操作类型
**           pArpInfo             arp 参数信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  arpUpdateToDb (CPCHAR  cpcOperType, __PARP_HANDLE  pArpInfo)
{
    sqlite3      *pDb;
    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (NULL == pDb || NULL == pArpInfo) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    if (!strcmp(cpcOperType, ARP_SET)) {
        iRet = __arpIsExists(pArpInfo);
        if (LW_TRUE == iRet) {
            __arpUpdateItem(pDb, pArpInfo);
        } else if (LW_FALSE == iRet) {
            __arpInsertItem(pDb, pArpInfo);
        } else {
            sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

            return  (PX_ERROR);
        }
    } else if (!strcmp(cpcOperType, ARP_DELETE)) {
        __arpDeleteItem(pDb, pArpInfo);
    } else if (!strcmp(cpcOperType, ARP_UPDATE)) {
        __arpUpdateItem(pDb, pArpInfo);
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "unknown operation.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: arpDataBaseInit
** 功能描述: 初始化arp数据库
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT arpTableCreate (VOID)
{
    sqlite3   *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, ARP_TABLE, ARP_TABLE_HEAD);
}
