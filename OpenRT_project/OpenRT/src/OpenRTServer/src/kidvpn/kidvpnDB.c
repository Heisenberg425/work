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
** ��   ��   ��: kidvpnDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��: VPN ���ݿ����
** 2018.08.16  �������ݿ��еĳ�Ա PASSWD
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "kidvpn_common.h"
#include "network/lwip/inet.h"
#include "kidvpnDB.h"
#include "database/database_common.h"
/*********************************************************************************************************
** ��������: __kidvpnUpdateItem
** ��������: ���� vnd �����е�һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pKidVpnDBInfo     VPN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnUpdateItem (sqlite3  *pDb, __PKIDVPN_DB_INFO  pKidVpnDBInfo)
{
    CHAR          cSqlCmd[300]  = {0};

    INT           iRet;

    if (LW_NULL == pKidVpnDBInfo) {
        return  (PX_ERROR);
    }

    bzero(cSqlCmd, sizeof(cSqlCmd));
    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "UPDATE %s  SET TYPE = %d , "
             "MTU = %d , PORT = %d , PID = %d , IP = '%s', STATUS = '%s', PASSWD = '%s'"
             "WHERE VNDID = %d;" ,
             KIDVPN_TABLE,
             pKidVpnDBInfo->kidvpnHandle.cHandleType,
             pKidVpnDBInfo->kidvpnHandle.iMtu,
             pKidVpnDBInfo->kidvpnHandle.iPort,
             pKidVpnDBInfo->iPid,
             inet_ntoa(*(struct in_addr *)&(pKidVpnDBInfo->kidvpnHandle.uiIpAddr)),
             "0",
             pKidVpnDBInfo->kidvpnHandle.cPassWd,
             pKidVpnDBInfo->kidvpnHandle.iVndID);

    iRet = dataBaseCmdExec(pDb, cSqlCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: __kidvpnInsertItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pKidVpnDBInfo     VPN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnInsertItem (sqlite3 *pDb, __PKIDVPN_DB_INFO  pKidVpnDBInfo)
{
    CHAR               cTbInsertCmd[200]  = {0};
    INT                iRet;

    if (LW_NULL == pKidVpnDBInfo) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));
    snprintf(cTbInsertCmd,
            sizeof(cTbInsertCmd),
            "INSERT INTO %s (TYPE, VNDID, MTU, PORT, PID, IP, STATUS, PASSWD) "
            "VALUES (%d, %d, %d, %d, %d, '%s', '%s', '%s');",
            KIDVPN_TABLE,
            pKidVpnDBInfo->kidvpnHandle.cHandleType,
            pKidVpnDBInfo->kidvpnHandle.iVndID,
            pKidVpnDBInfo->kidvpnHandle.iMtu,
            pKidVpnDBInfo->kidvpnHandle.iPort,
            pKidVpnDBInfo->iPid,
            inet_ntoa(*(struct in_addr *)&(pKidVpnDBInfo->kidvpnHandle.uiIpAddr)),
            "0",
            pKidVpnDBInfo->kidvpnHandle.cPassWd);

    iRet = dataBaseCmdExec(pDb, cTbInsertCmd);
    return  (iRet);
}
/*********************************************************************************************************
** ��������: __kidvpnGetFromDB
** ��������: �����ݿ��ȡ�������ݿ����Ϣ
** ��  ��  : pcKidVPNInfoBuf   VPN ��Ϣ�洢��ַ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static  INT  __kidvpnGetFromDB (PCHAR   pcKidVPNInfoBuf)
{
    CHAR            cSelectSql[200]  = {0};
    INT             iRet;
    CHAR          **cDbResult;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             IRow, IColumn;
    CHAR            cIpPos[128] = {0};

    __PKIDVPN_DB_INFO     pKidVPNInfo       = LW_NULL;
    sqlite3              *pDb               = LW_NULL;

    INT             i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pcKidVPNInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    pKidVPNInfo = (__PKIDVPN_DB_INFO)pcKidVPNInfoBuf;

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", KIDVPN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            pKidVPNInfo->kidvpnHandle.cHandleType = atoi(cDbResult[i * IColumn + 0]);
            pKidVPNInfo->kidvpnHandle.iVndID      = atoi(cDbResult[i * IColumn + 1]);
            pKidVPNInfo->kidvpnHandle.iMtu        = atoi(cDbResult[i * IColumn + 2]);
            pKidVPNInfo->kidvpnHandle.iPort       = atoi(cDbResult[i * IColumn + 3]);

            inet_aton(cDbResult[i * IColumn + 5],
                      (struct in_addr *)&(pKidVPNInfo->kidvpnHandle.uiIpAddr));

            /*
             *  ��ȡ IP ����Ӧ�ĵ���λ����Ϣ
             */
            getPositionByIpAddr(cDbResult[i * IColumn + 5], cIpPos);
            strncpy(pKidVPNInfo->kidvpnHandle.cPos, cIpPos, sizeof(pKidVPNInfo->kidvpnHandle.cPos));

            pKidVPNInfo->kidvpnHandle.cStatus = atoi(cDbResult[i * IColumn + 6]);

            strncpy(pKidVPNInfo->kidvpnHandle.cPassWd,
                    cDbResult[i * IColumn + 7],
                    sizeof(pKidVPNInfo->kidvpnHandle.cPassWd));

            pcKidVPNInfoBuf += sizeof(__KIDVPN_DB_INFO);
            pKidVPNInfo = (__PKIDVPN_DB_INFO)(pcKidVPNInfoBuf);

            bzero(cIpPos, sizeof(cIpPos));
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: kidvpnClientStatusGetFromDB
** ��������: �����ݿ��ȡ�ͻ��˵�״̬��Ϣ���龰ģʽ�£�ֻ����һ���ͻ��˵������ʹ�øýӿڣ�
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnClientStatusGetFromDB (VOID)
{
    INT                   iRet;
    CHAR                  cSelectSql[200]  = {0};
    CHAR                **cDbResult        = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;
    INT                   iLinkStat        = 0;
    sqlite3              *pDb              = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql,
             sizeof(cSelectSql),
             "SELECT * FROM %s WHERE TYPE = %d;",
             KIDVPN_TABLE,
             TYPE_ADD_VPN_C);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            iLinkStat = atoi(cDbResult[iColumn + 6]);
        } else {
            iLinkStat = 0;
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iLinkStat);
}
/*********************************************************************************************************
** ��������: kidvpnGetLinkedCliInfoFromDB
** ��������: �����ݿ��ȡ�����ӿͻ��˵���Ϣ
** ��  ��  : NONE
** ��  ��  : piRow   VPN ��Ϣ����
** ��  ��  : �ɹ����������ӿͻ��˵���Ϣָ�룬ʧ�ܷ���NULL
*********************************************************************************************************/
PCHAR  kidvpnGetLinkedCliInfoFromDB (INT  *piRow)
{
    CHAR                 cSelectSql[200]  = {0};
    INT                  iRet;
    CHAR               **cDbResult;
    CHAR                *pcErrMsg         = LW_NULL;
    INT                  IRow, IColumn;
    __PKIDVPN_DB_INFO    pKidVPNInfo      = LW_NULL;
    PCHAR                pcKidVPNInfoBuf  = LW_NULL;
    sqlite3             *pDb              = LW_NULL;
    INT                  i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (NULL);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql,
             sizeof(cSelectSql),
             "SELECT * FROM %s WHERE TYPE=1 AND STATUS='1';",
             KIDVPN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        *piRow = IRow;
        pcKidVPNInfoBuf = (PCHAR)malloc(IRow * sizeof(__KIDVPN_DB_INFO));
        if (NULL == pcKidVPNInfoBuf) {
            return  (NULL);
        }

        pKidVPNInfo = (__PKIDVPN_DB_INFO)pcKidVPNInfoBuf;

        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            pKidVPNInfo->kidvpnHandle.cHandleType = atoi(cDbResult[i * IColumn + 0]);
            pKidVPNInfo->kidvpnHandle.iVndID      = atoi(cDbResult[i * IColumn + 1]);
            pKidVPNInfo->kidvpnHandle.iMtu        = atoi(cDbResult[i * IColumn + 2]);
            pKidVPNInfo->kidvpnHandle.iPort       = atoi(cDbResult[i * IColumn + 3]);

            inet_aton(cDbResult[i * IColumn + 5],
                      (struct in_addr *)&(pKidVPNInfo->kidvpnHandle.uiIpAddr));

            pKidVPNInfo->kidvpnHandle.cStatus = atoi(cDbResult[i * IColumn + 6]);

            strncpy(pKidVPNInfo->kidvpnHandle.cPassWd,
                    cDbResult[i * IColumn + 7],
                    sizeof(pKidVPNInfo->kidvpnHandle.cPassWd));

            pKidVPNInfo++;
        }
    } else {
        *piRow = 0;
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (pcKidVPNInfoBuf);
}
/*********************************************************************************************************
** ��������: kidvpnGetDBRow
** ��������: �����ݿ��ȡ��Ϣ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iRow              ��Ϣ����
*********************************************************************************************************/
INT  kidvpnGetDBRow (VOID)
{
    INT       iRow     = 0;
    INT       iColumn  = 0;
    INT       iRet     = PX_ERROR;

    sqlite3  *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iRet = dataBaseGetTbRowCol(pDb, KIDVPN_TABLE, &iRow, &iColumn);

    if (PX_ERROR == iRet) {
        return (0);
    }

    return  (iRow);
}
/*********************************************************************************************************
** ��������: kidvpnGetDBInfo
** ��������: �����ݿ��ȡ�������ݿ����Ϣ (��Ҫ�������Լ��ͷ��ڴ�)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : pcKidVPNInfoBuf      vpn ��Ϣ
*********************************************************************************************************/
PVOID  kidvpnGetDBInfo (VOID)
{
    PCHAR                 pcKidVPNInfoBuf  = LW_NULL;
    INT                   iItemNum         = 0;
    INT                   iBufLen          = 0;
    INT                   iRet             = PX_ERROR;

    iItemNum = kidvpnGetDBRow();
    if (0 == iItemNum) {

        return  (LW_NULL);
    }

    iBufLen         = iItemNum * sizeof(__KIDVPN_DB_INFO);
    pcKidVPNInfoBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcKidVPNInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (LW_NULL);
    }

    bzero(pcKidVPNInfoBuf, iBufLen);
    iRet = __kidvpnGetFromDB(pcKidVPNInfoBuf);                          /*  ��ȡ��Ϣ                    */
    if (PX_ERROR == iRet) {
        free(pcKidVPNInfoBuf);

        return  (LW_NULL);
    }

    return  (pcKidVPNInfoBuf);
}
/*********************************************************************************************************
** ��������: kidvpnGetSpecInfo
** ��������: ��ȡ���ݿ�ָ����Ϣ
** ��  ��  : cpcField          �ֶ�
**           pcArg             �ַ���ֵ
**           piArg             ����ֵ
** ��  ��  : pKidVpnDBInfo     VPN ��Ϣ
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnGetSpecInfo (__PKIDVPN_DB_INFO  pKidVpnDBInfo,
                        CPCHAR             cpcField,
                        PCHAR              pcArg,
                        INT               *piArg)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3_stmt *pstmt         = LW_NULL;
    const CHAR   *pcTail        = LW_NULL;
    sqlite3      *pDb           = LW_NULL;
    CHAR         *cpIp          = LW_NULL;

    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== cpcField || LW_NULL == pKidVpnDBInfo || LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    if (LW_NULL == pcArg && LW_NULL == piArg) {
        return  (PX_ERROR);
    }

    if (LW_NULL != pcArg) {
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "select * from %s where %s = '%s';",
                KIDVPN_TABLE, cpcField, pcArg);
    } else {
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "select * from %s where %s = %d;",
                KIDVPN_TABLE, cpcField, *piArg);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    iRet = sqlite3_prepare_v2(pDb, cSqlCmd, strlen(cSqlCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    if (SQLITE_ROW == sqlite3_step(pstmt)) {                            /*  ��Ŀ�Ѿ�����                */
        pKidVpnDBInfo->kidvpnHandle.cHandleType = sqlite3_column_int(pstmt, 0);
        pKidVpnDBInfo->kidvpnHandle.iVndID      = sqlite3_column_int(pstmt, 1);
        pKidVpnDBInfo->kidvpnHandle.iMtu        = sqlite3_column_int(pstmt, 2);
        pKidVpnDBInfo->kidvpnHandle.iPort       = sqlite3_column_int(pstmt, 3);
        pKidVpnDBInfo->iPid                     = sqlite3_column_int(pstmt, 4);

        cpIp = (PCHAR)sqlite3_column_text(pstmt, 5);
        inet_aton(cpIp, (struct in_addr *)&(pKidVpnDBInfo->kidvpnHandle.uiIpAddr));

        strncpy(pKidVpnDBInfo->kidvpnHandle.cPassWd,
               (PCHAR)sqlite3_column_text(pstmt, 7),
                sizeof(pKidVpnDBInfo->kidvpnHandle.cPassWd));

        /*
         *  todo: ��ȡstatus
         */

        iRet = ERROR_NONE;
    } else {
        iRet = PX_ERROR;
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnDelDBItem
** ��������: ɾ�����ݿ���ָ����Ŀ
** ��  ��  : iVndID            ��������  ID
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnDelDBItem (INT  iVndID)
{
    CHAR               cDelCmd[200]       = {0};
    sqlite3           *pDb                = LW_NULL;
    INT                iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s WHERE VNDID = %d;", KIDVPN_TABLE, iVndID);
    iRet = dataBaseCmdExec(pDb, cDelCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnAddItemToDb
** ��������: �����ݿ������һ����Ŀ(����Ѿ����ڣ��������Ŀ����)
** ��  ��  : pKidVpnDBInfo     VPN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnAddItemToDb (__PKIDVPN_DB_INFO  pKidVpnDBInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    INT           iFlag = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pKidVpnDBInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where VNDID = %d;",
             KIDVPN_TABLE,
             pKidVpnDBInfo->kidvpnHandle.iVndID);
    iFlag = dataBaseItemCheck(pDb, cSqlCmd);

    if (ERROR_NONE == iFlag) {
        iRet = __kidvpnUpdateItem(pDb, pKidVpnDBInfo);
    } else {
        iRet = __kidvpnInsertItem(pDb, pKidVpnDBInfo);
    }

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnServIpExistFlagGet
** ��������: ��ȡ������ ip ��ַ�Ƿ��Ѵ���
** ��  ��  : __PKIDVPN_HANDLE     VPN ��Ϣ
** ��  ��  : NONE
** ��  ��  : ���ڷ��أ�ERROR_CODE�������ڷ��أ�PX_ERROR
*********************************************************************************************************/
INT  kidvpnServIpExistFlagGet (__PKIDVPN_HANDLE  pKidVpnInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pKidVpnInfo) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd,
             sizeof(cSqlCmd),
             "select * from %s where TYPE = %d and IP = '%s' and PORT = %d;",
             KIDVPN_TABLE,
             TYPE_ADD_VPN_S,
             inet_ntoa(*(struct in_addr *)&(pKidVpnInfo->uiIpAddr)),
             pKidVpnInfo->iPort);

    return  dataBaseItemCheck(pDb, cSqlCmd);
}
/*********************************************************************************************************
** ��������: kidvpnUpdateVpnStatToDb
** ��������: ���� VPN ����״̬
** ��  ��  : iVndId       VND ID
**           pcStatus     VPN ״̬
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnUpdateVpnStatToDb (INT  iVndId, PCHAR  pcStatus)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd),
            "UPDATE %s  SET STATUS = '%s' WHERE VNDID = %d;" ,
             KIDVPN_TABLE, pcStatus, iVndId);

    iRet = dataBaseCmdExec(pDb, cSqlCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnTableCreate
** ��������: ��ʼ�� vpn ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  kidvpnTableCreate (VOID)
{
    sqlite3  *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    return  dataBaseTbCreate(pDb, KIDVPN_TABLE, KIDVPN_TABLE_HEAD);
}

/*********************************************************************************************************
** ��������: __kidvpnLinkInsertItem
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pDb               ���ݿ���
**           pKidVpnLogInfo    VPN Log ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __kidvpnLinkInsertItem (sqlite3  *pDb, __PKIDVPN_LINK  pKidVpnLinkInfo)
{
    CHAR  cTbInsertCmd[300] = {0};

    if (LW_NULL == pKidVpnLinkInfo) {
        return  (PX_ERROR);
    }

    bzero(cTbInsertCmd, sizeof(cTbInsertCmd));
    snprintf(cTbInsertCmd,
            sizeof(cTbInsertCmd),
            "INSERT INTO %s (VNDID, IP, MAC) VALUES (%d, '%s', '%s');",
            KIDVPN_LINK_TABLE,
            pKidVpnLinkInfo->iVndId,
            pKidVpnLinkInfo->cIp,
            pKidVpnLinkInfo->cMac);

    return  dataBaseCmdExec(pDb, cTbInsertCmd);
}
/*********************************************************************************************************
** ��������: __kidvpnLinkGetFromDB
** ��������: �����ݿ��ȡ������Ϣ
** ��  ��  : NONE
** ��  ��  : pcKidVPNInfoBuf   VPN ��Ϣ�洢��ַ
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static  INT  __kidvpnLinkGetFromDB (PCHAR   pcKidVPNLinkBuf)
{
    INT             iRet;
    CHAR            cSelectSql[200]  = {0};
    CHAR          **cDbResult;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             iRow;
    INT             iColumn;
    CHAR            cIpPos[128]      = {0};
    __PKIDVPN_LINK  pKidVPNInfo     = LW_NULL;
    sqlite3        *pDb              = LW_NULL;

    INT             i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pcKidVPNLinkBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    pKidVPNInfo = (__PKIDVPN_LINK)pcKidVPNLinkBuf;

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", KIDVPN_LINK_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && iRow > 0) {
        for (i = 1; i <= iRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            pKidVPNInfo->iVndId = atoi(cDbResult[i * iColumn + 0]);
            strncpy(pKidVPNInfo->cIp,  cDbResult[i * iColumn + 1], sizeof(pKidVPNInfo->cIp));
            strncpy(pKidVPNInfo->cMac, cDbResult[i * iColumn + 2], sizeof(pKidVPNInfo->cMac));

            /*
             *  ��ȡ IP ����Ӧ�ĵ���λ����Ϣ
             */
            getPositionByIpAddr(pKidVPNInfo->cIp, cIpPos);
            strncpy(pKidVPNInfo->cPos, cIpPos, sizeof(pKidVPNInfo->cPos));

            pcKidVPNLinkBuf += sizeof(__KIDVPN_LINK);
            pKidVPNInfo      = (__PKIDVPN_LINK)(pcKidVPNLinkBuf);

            bzero(cIpPos, sizeof(cIpPos));
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: kidvpnLinkGetDBRow
** ��������: �����ݿ��ȡ��Ϣ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iRow              ��Ϣ����
*********************************************************************************************************/
INT  kidvpnLinkGetDBRow (VOID)
{
    INT       iRow     = 0;
    INT       iColumn  = 0;
    INT       iRet     = PX_ERROR;

    sqlite3  *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iRet = dataBaseGetTbRowCol(pDb, KIDVPN_LINK_TABLE, &iRow, &iColumn);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    return  (iRow);
}
/*********************************************************************************************************
** ��������: kidvpnLinkGetSpecInfo
** ��������: ��ȡ VPN ָ��������Ϣ
** �䡡��  : llStartIndex       ��ʼ���
**           llEndIndex         �������
**           iBuffLen           ��Ϣ����
** �䡡��  : pvLinkBuff         ������Ϣ
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnLinkGetSpecInfo (INT64  llStartIndex,
                            INT64  llEndIndex,
                            PVOID  pvLinkBuff,
                            INT32  iBuffLen)
{
    sqlite3            *pDb              = LW_NULL;
    CHAR              **cDbResult        = LW_NULL;
    CHAR               *pcErrMsg         = LW_NULL;
    CHAR                cSelectSql[200]  = {0};
    CHAR                cIpPos[128]      = {0};
    INT32               iRet;
    INT32               IRow, iColumn;
    INT32               i;

    __PKIDVPN_LINK      pVpnLinkNode     = LW_NULL;
    PVOID               pBuffTmp         = pvLinkBuff;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pvLinkBuff) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", KIDVPN_LINK_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            if (i < llStartIndex) {
                continue;
            }

            if (i > llEndIndex) {
                break;
            }

            pVpnLinkNode = (__PKIDVPN_LINK)pvLinkBuff;

            pVpnLinkNode->iVndId = atoi(cDbResult[i * iColumn + 0]);
            strncpy(pVpnLinkNode->cIp,  cDbResult[i * iColumn + 1], sizeof(pVpnLinkNode->cIp));
            strncpy(pVpnLinkNode->cMac, cDbResult[i * iColumn + 2], sizeof(pVpnLinkNode->cMac));

            /*
             *  ��ȡ IP ����Ӧ�ĵ���λ����Ϣ
             */
            getPositionByIpAddr(pVpnLinkNode->cIp, cIpPos);
            strncpy(pVpnLinkNode->cPos, cIpPos, sizeof(pVpnLinkNode->cPos));

            bzero(cIpPos, sizeof(cIpPos));

            pvLinkBuff = (PCHAR)pvLinkBuff + sizeof(__KIDVPN_LINK);
            if (pvLinkBuff - pBuffTmp > iBuffLen) {                     /*  �ڴ治��                    */
                break;
            }
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get vpn link info failed.\n");

        return  (PX_ERROR);
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: kidvpnLinkGetDBInfo
** ��������: �����ݿ��ȡ���������ӵ���Ϣ (��Ҫ�������Լ��ͷ��ڴ�)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : pcKidVPNInfoBuf      vpn ��Ϣ
*********************************************************************************************************/
PVOID  kidvpnLinkGetDBInfo (VOID)
{
    PCHAR                 pcKidvpnLinkBuf  = LW_NULL;
    INT                   iItemNum         = 0;
    INT                   iBufLen          = 0;
    INT                   iRet             = PX_ERROR;

    iItemNum = kidvpnLinkGetDBRow();
    if (0 == iItemNum) {

        return  (LW_NULL);
    }

    iBufLen         = iItemNum * sizeof(__KIDVPN_LINK);
    pcKidvpnLinkBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcKidvpnLinkBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (LW_NULL);
    }

    bzero(pcKidvpnLinkBuf, iBufLen);
    iRet = __kidvpnLinkGetFromDB(pcKidvpnLinkBuf);                      /*  ��ȡ������Ϣ                */
    if (PX_ERROR == iRet) {
        free(pcKidvpnLinkBuf);

        return  (LW_NULL);
    }

    return  (pcKidvpnLinkBuf);
}
/*********************************************************************************************************
** ��������: kidvpnLinkDelDBItemById
** ��������: ɾ�����ݿ���ָ����Ŀ
** ��  ��  : iVndID            ��������  ID
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnLinkDelDBItemById (INT  iVndID)
{
    CHAR               cDelCmd[200] = {0};
    sqlite3           *pDb          = LW_NULL;
    INT                iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s WHERE VNDID = %d;", KIDVPN_LINK_TABLE, iVndID);
    iRet = dataBaseCmdExec(pDb, cDelCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnLinkDelDBItem
** ��������: ɾ�����ݿ���ָ����Ŀ
** ��  ��  : iVndID            ��������  ID
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnLinkDelDBItem (__PKIDVPN_LINK  pKidVpnLinkInfo)
{
    CHAR               cDelCmd[200] = {0};
    sqlite3           *pDb          = LW_NULL;
    INT                iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s WHERE VNDID = %d AND IP='%s' AND MAC='%s';",
                                        KIDVPN_LINK_TABLE,
                                        pKidVpnLinkInfo->iVndId,
                                        pKidVpnLinkInfo->cIp,
                                        pKidVpnLinkInfo->cMac);
    iRet = dataBaseCmdExec(pDb, cDelCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnLinkDelDBAllItem
** ��������: ɾ�����ݿ���ȫ����Ŀ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnLinkDelDBAllItem (VOID)
{
    CHAR               cDelCmd[200] = {0};
    sqlite3           *pDb          = LW_NULL;
    INT                iRet;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cDelCmd, sizeof(cDelCmd), "DELETE FROM %s;", KIDVPN_LINK_TABLE);
    iRet = dataBaseCmdExec(pDb, cDelCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: kidvpnLinkAddItemToDb
** ��������: �����ݿ������һ����Ŀ
** ��  ��  : pKidVpnLogInfo     VPN Log ��Ϣ
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  kidvpnLinkAddItemToDb (__PKIDVPN_LINK  pKidVpnLinkInfo)
{
    sqlite3      *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb || LW_NULL == pKidVpnLinkInfo) {
        return  (PX_ERROR);
    }

    return  __kidvpnLinkInsertItem(pDb, pKidVpnLinkInfo);
}
/*********************************************************************************************************
** ��������: kidvpnLinkTableCreate
** ��������: ����������Ϣ���ݿ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  kidvpnLinkTableCreate (VOID)
{
    sqlite3  *pDb;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, KIDVPN_LINK_TABLE, KIDVPN_LINK_TABLE_HEAD));

    return  kidvpnLinkDelDBAllItem();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
