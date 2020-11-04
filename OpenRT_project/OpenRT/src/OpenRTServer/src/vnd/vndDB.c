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
** ��   ��   ��: vndDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��: �����������ݿ����
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
** ��������: __vndUpdateItem
** ��������: ���� vnd �����е�һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pVndInfo          ����������Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: __vndInsertItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pVndInfo          ����������Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: __vndGetFromDB
** ��������: �����ݿ��ȡ�������ݿ����Ϣ
** ��  ��  : pcVndInfoBuf      ��Ϣ�洢�� buf
** ��  ��  : NONE
** ��  ��  : pcKidVPNInfoBuf
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
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
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
** ��������: vndGetDBRow
** ��������: �����ݿ��ȡ��Ϣ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iRow              ��Ϣ����
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
** ��������: vndGetDBInfo
** ��������: �����ݿ��ȡ�������ݿ����Ϣ (��Ҫ�������Լ��ͷ��ڴ�)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : pcVndInfoBuf      vnd ��Ϣ
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
    iRet = __vndGetFromDB(pcVndInfoBuf);                                /*  ��ȡ ��Ϣ                   */
    if (PX_ERROR == iRet) {
        free(pcVndInfoBuf);

        return  (LW_NULL);
    }

    return  (pcVndInfoBuf);
}

/*********************************************************************************************************
** ��������: vndGetInfoById
** ��������: ͨ�� ID ��ȡ����������Ϣ
** ��  ��  : iVndID            �������� ID
** ��  ��  : pVndInfo          ����������Ϣ
** ��  ��  : ERROR_CODE
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
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
** ��������: netIfIndexGetByVndId
** ��������: ͨ�� vnd ID ��ȡ NetIfIndex
** ��  ��  : iVndid          �������� ID
** ��  ��  : piNetIfIndex    ������������
** ��  ��  : ERROR_CODE
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
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
** ��������: vndIdGetByNetIfIndex
** ��������: ͨ�� NetIfIndex ��ȡ vnd ID
** ��  ��  : iNetIfIndex     ������������
** ��  ��  : piVndid         �������� ID
** ��  ��  : ERROR_CODE
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

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
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
** ��������: vndDelDBItem
** ��������: ɾ�����ݿ���ָ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           iVndID            ��������  ID
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: vndAddItemToDb
** ��������: �����ݿ������һ����Ŀ(����Ѿ����ڣ��������Ŀ����)
** ��  ��  : pVndInfo          ����������Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
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
** ��������: vndTableCreate
** ��������: ��ʼ�� vnd ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
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

