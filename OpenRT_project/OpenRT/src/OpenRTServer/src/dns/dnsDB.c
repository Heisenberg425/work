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
** 文   件   名: dnsDB.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 30 日
**
** 描        述: dns数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include "common.h"
#include "dns_common.h"
/*********************************************************************************************************
** 函数名称: __dnsInfoUpdateItemToDb
** 功能描述: 更新 DNS 数据库中的一个条目
** 输  入  : pDb                数据库句柄
**           pDnsHandle DNS     信息指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dnsInfoUpdateItemToDb (sqlite3  *pDb, __PDNS_HANDLE  pDnsHandle)
{
    CHAR  cSqlCmd[128] = {0};

    if (LW_NULL == pDb || LW_NULL == pDnsHandle) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "UPDATE %s SET PRIMARYDNS='%s', SECONDDNS='%s';",
             DNS_TABLE_NAME,
             pDnsHandle->primaryDNS,
             pDnsHandle->secondDNS);

    return  dataBaseCmdExec(pDb, cSqlCmd);
}
/*********************************************************************************************************
** 函数名称: __dnsInfoInsertItemToDb
** 功能描述: 向数据库中添加一个条目
** 输  入  : pDb               数据库句柄
**           pDnsHandle DNS    信息指针
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dnsInfoInsertItemToDb (sqlite3  *pDb, __PDNS_HANDLE  pDnsHandle)
{
    CHAR  cTbInsertCmd[128] = {0};

    if (LW_NULL == pDb || LW_NULL == pDnsHandle) {
        return  (PX_ERROR);
    }

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (PRIMARYDNS, SECONDDNS) VALUES('%s', '%s');",
             DNS_TABLE_NAME,
             pDnsHandle->primaryDNS,
             pDnsHandle->secondDNS);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** 函数名称: dnsCreateTable
** 函数功能: 创建 DNS 数据表
** 输    入: NONE
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsTableCreate (VOID)
{
    sqlite3  *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, DNS_TABLE_NAME, DNS_TABLE_HEAD);
}
/*********************************************************************************************************
** 函数名称: getDbDns
** 函数功能: 从数据库中获取 DNS 配置
** 输    入: NONE
** 输    出: pDnsHandle  用于存储 DNS 信息
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsInfoGetFromDb (__PDNS_HANDLE  pDnsHandle)
{
    INT        iRet;
    sqlite3   *pDb             = LW_NULL;
    CHAR       cSelectSql[128] = {0};
    CHAR     **cDbResult;
    CHAR      *pcErrMsg;
    INT        iRow;
    INT        iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pDnsHandle) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", DNS_TABLE_NAME);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        strncpy(pDnsHandle->primaryDNS, cDbResult[iColumn + 0], sizeof(pDnsHandle->primaryDNS));
        strncpy(pDnsHandle->secondDNS,  cDbResult[iColumn + 1], sizeof(pDnsHandle->secondDNS));
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: dnsInfoAddItemToDb
** 函数功能: 插入或更新 DNS 信息
** 输    入: pDnsHandle DNS 信息指针
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  dnsInfoAddItemToDb (__PDNS_HANDLE  pDnsHandle)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;
    INT           iFlag = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pDnsHandle) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "SELECT * FROM %s;", DNS_TABLE_NAME);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (ERROR_NONE == iFlag) {
        iRet = __dnsInfoUpdateItemToDb(pDb, pDnsHandle);
    } else {
        iRet = __dnsInfoInsertItemToDb(pDb, pDnsHandle);
    }

    return  (iRet);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
