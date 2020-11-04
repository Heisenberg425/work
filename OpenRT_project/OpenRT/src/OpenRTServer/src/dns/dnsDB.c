/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: dnsDB.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 30 ��
**
** ��        ��: dns���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include "common.h"
#include "dns_common.h"
/*********************************************************************************************************
** ��������: __dnsInfoUpdateItemToDb
** ��������: ���� DNS ���ݿ��е�һ����Ŀ
** ��  ��  : pDb                ���ݿ���
**           pDnsHandle DNS     ��Ϣָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: __dnsInfoInsertItemToDb
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pDnsHandle DNS    ��Ϣָ��
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: dnsCreateTable
** ��������: ���� DNS ���ݱ�
** ��    ��: NONE
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
** ��������: getDbDns
** ��������: �����ݿ��л�ȡ DNS ����
** ��    ��: NONE
** ��    ��: pDnsHandle  ���ڴ洢 DNS ��Ϣ
** ��    ��: ERROR_CODE
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
** ��������: dnsInfoAddItemToDb
** ��������: �������� DNS ��Ϣ
** ��    ��: pDnsHandle DNS ��Ϣָ��
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
