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
** ��   ��   ��: natDB.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 15 ��
**
** ��        ��:  NAT ״̬���ݿ����
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "natStatus.h"
#include "common.h"
#include "nat_common.h"
#include "database/database_common.h"
/*********************************************************************************************************
  IP ��Ƭ�����ݿ���Ϣ
*********************************************************************************************************/
#define NATIPFRAG_TABLE            "natipfrag"                          /*  ��Ƭ������                */
#define NATIPFRAG_TABLE_HEAD       "(TCP        INT,"                               \
                                   "UDP         INT,"                               \
                                   "ICMP        INT)"                   /*  ��Ƭ������ʽ              */
/*********************************************************************************************************
  �˿�ӳ�����ݿ���Ϣ
*********************************************************************************************************/
#define NATMAP_TABLE               "natmap"                             /*  �˿�ӳ�����              */
#define NATMAP_TABLE_HEAD          "(ID         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "ASSPORT     INT,"                               \
                                   "LOCALPORT   INT,"                               \
                                   "LOCALIP     TEXT,"                              \
                                   "IPCNT       INT,"                               \
                                   "PRTO        TEXT)"                  /*  �˿�ӳ�����ʽ            */
/*********************************************************************************************************
  ����������ݿ���Ϣ
*********************************************************************************************************/
#define NATALIAS_TABLE             "natalias"                           /*  �����������              */
#define NATALIAS_TABLE_HEAD        "(ID         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "ALIAS       TEXT,"                              \
                                   "LANSTART    TEXT,"                              \
                                   "LANEND      TEXT)"                  /*  �����������ʽ            */
/*********************************************************************************************************
  WAN ���������ݿ���Ϣ
*********************************************************************************************************/
#define NATWAN_TABLE               "natwan"                             /*  WAN �ڱ���                */
#define NATWAN_TABLE_HEAD          "(ID   INT,"  \
                                   "WAN  TEXT)"                         /*  WAN �ڱ�����ʽ            */
/*********************************************************************************************************
  LAN ���������ݿ���Ϣ
*********************************************************************************************************/
#define NATLAN_TABLE               "natlan"                             /*  LAN �ڱ���                */
#define NATLAN_TABLE_HEAD          "(ID   INT,"  \
                                   "LAN  TEXT)"                         /*  LAN �ڱ�����ʽ            */
#define MAP_INFO_ALL               (1000)                               /*  ���е�MAP��Ϣ               */
/*********************************************************************************************************
** ��������: __natMapGetFromDB
** ��������: �����ݿ��ȡ�˿�ӳ����Ϣ
** �䡡��  : pNatMapBuff        natmap buf
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __natMapGetFromDB (PCHAR  pcNatMapBuff)
{
    CHAR            cSelectSql[200]  = {0};

    INT             iRet             = PX_ERROR;
    sqlite3        *pDb              = LW_NULL;
    CHAR          **cDbResult        = LW_NULL;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             iRow;
    INT             iColumn;

    __PNATMAP_INFO  pNatMapInfo      =  LW_NULL;

    INT             i;

    if (LW_NULL == pcNatMapBuff) {
        return  (PX_ERROR);
    }

    pNatMapInfo = (__PNATMAP_INFO)pcNatMapBuff;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", NATMAP_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= iRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            pNatMapInfo->iIpCnt   = atoi(cDbResult[i * iColumn + 4]);
            pNatMapInfo->iWanPort = atoi(cDbResult[i * iColumn + 1]);
            pNatMapInfo->iLanPort = atoi(cDbResult[i * iColumn + 2]);

            if (!strcmp(cDbResult[i * iColumn + 5], "UDP")) {
                strncpy(pNatMapInfo->cProtoString, "udp", sizeof(pNatMapInfo->cProtoString));
            } else if (!strcmp(cDbResult[i * iColumn + 5], "TCP")) {
                strncpy(pNatMapInfo->cProtoString, "tcp", sizeof(pNatMapInfo->cProtoString));
            }

            strncpy(pNatMapInfo->cIpString,
                    cDbResult[i * iColumn + 3],
                    sizeof(pNatMapInfo->cIpString));

            pcNatMapBuff = pcNatMapBuff + sizeof(__NATMAP_INFO);
            pNatMapInfo = (__PNATMAP_INFO)(pcNatMapBuff);
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
** ��������: __natAliasGetFromDB
** ��������: �����ݿ��ȡ���������Ϣ
** �䡡��  : NONE
** �䡡��  : pNatAliasBuff      ������Ϣ
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __natAliasGetFromDB (PVOID  pNatAliasBuff)
{
    CHAR                  cSelectSql[200]  = {0};

    INT                   iRet             = PX_ERROR;
    sqlite3              *pDb              = LW_NULL;
    CHAR                **cDbResult        = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   IRow, IColumn;

    __PNATALIAS_INFO      pNatAliasInfo   =  LW_NULL;

    INT                   i;

    if (LW_NULL == pNatAliasBuff) {
        return  (PX_ERROR);
    }

    pNatAliasInfo = (__PNATALIAS_INFO)pNatAliasBuff;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", NATALIAS_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            strncpy(pNatAliasInfo->cAliasString,
                    cDbResult[i * IColumn + 1],
                    sizeof(pNatAliasInfo->cAliasString));
            strncpy(pNatAliasInfo->cLoaclSString,
                    cDbResult[i * IColumn + 2],
                    sizeof(pNatAliasInfo->cLoaclSString));
            strncpy(pNatAliasInfo->cLoaclEString,
                    cDbResult[i * IColumn + 3],
                    sizeof(pNatAliasInfo->cLoaclEString));

            pNatAliasBuff = (CHAR *)pNatAliasBuff + sizeof(__NATALIAS_INFO);
            pNatAliasInfo = (__PNATALIAS_INFO)(pNatAliasBuff);
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
** ��������: natLanGetFromDB
** ��������: �����ݿ��ȡ LAN ����Ϣ
** �䡡��  : pLanInfo          LAN ����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natLanGetFromDB (__PLAN_INFO   pLanInfo)
{
    CHAR            cSelectSql[200]  = {0};

    INT             iRet             = PX_ERROR;
    sqlite3        *pDb              = LW_NULL;
    CHAR          **cDbResult        = LW_NULL;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             IRow, IColumn;

    INT             i = 0;
    INT             j = 0;

    if (LW_NULL == pLanInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", NATLAN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */

            /*
             *  ������������
             */
            strncpy(pLanInfo->cLanName[j], cDbResult[i * IColumn + 1], sizeof(pLanInfo->cLanName[j]));

            j++;
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base lan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natLanUpdateToDB
** ��������: �������ݿ�LAN ��״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natLanUpdateToDB (VOID)
{
    CHAR          cTbInsertCmd[200]  = {0};
    sqlite3_stmt *pstmt              = LW_NULL;
    const CHAR   *pcTail             = LW_NULL;
    sqlite3      *pDb                = LW_NULL;
    INT           iId                = 0;
    INT           iCol               = 0;
    INT           iRet               = PX_ERROR;
    INT           i;

    __LAN_INFO    pLanInfo;

    bzero(&pLanInfo, sizeof(pLanInfo));
    iRet = natLanInfoGet(&pLanInfo);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get lan netif error\n");

        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    dataBaseTbInfoDelete(pDb, NATLAN_TABLE);                            /*  ɾ�����ݿ�����              */

    /*
     *  ���ݿ��ֶΣ�ID  WAN
     */
    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd), "INSERT INTO %s VALUES(?, ?);", NATLAN_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (PX_ERROR);
    }

    /*
     *  ����ȡ���ĵ�ǰ�� LAN ����Ϣ���������ݿ���
     */
    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  ��������                    */
         iCol = 1;

         if (!strlen(pLanInfo.cLanName[i])) {                           /*  �ַ���Ϊ��                  */
             break;
         }

         sqlite3_bind_int(pstmt, iCol++, ++iId);
         sqlite3_bind_text(pstmt, iCol++, pLanInfo.cLanName[i], strlen(pLanInfo.cLanName[i]), LW_NULL);

         sqlite3_step(pstmt);
         sqlite3_reset(pstmt);
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natWanGetFromDB
** ��������: �����ݿ��ȡ WAN ����Ϣ
** �䡡��  : pWanInfo          WAN ����Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  natWanGetFromDB (__PWAN_INFO   pWanInfo)
{
    CHAR            cSelectSql[200]  = {0};

    INT             iRet             = PX_ERROR;
    sqlite3        *pDb              = LW_NULL;
    CHAR          **cDbResult        = LW_NULL;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             IRow, IColumn;

    INT             i = 0;
    INT             j = 0;

    if (LW_NULL == pWanInfo) {
        return  (PX_ERROR);
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", NATWAN_TABLE);
    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */

            /*
             *  ������������
             */
            strncpy(pWanInfo->cWanName[j], cDbResult[i * IColumn + 1], sizeof(pWanInfo->cWanName[j]));

            j++;
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
** ��������: natWanUpdateToDB
** ��������: �������ݿ� WAN ��״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natWanUpdateToDB (VOID)
{
    CHAR             cTbInsertCmd[200]  = {0};
    sqlite3_stmt    *pstmt              = LW_NULL;
    sqlite3         *pDb                = LW_NULL;
    const CHAR      *pcTail             = LW_NULL;
    INT              iId                = 0;
    INT              iCol               = 0;
    INT              iRet;
    __WAN_INFO       pWanInfo;
    INT              i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    bzero(&pWanInfo, sizeof(pWanInfo));
    iRet = natWanInfoGet(&pWanInfo);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get wan netif error\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    dataBaseTbInfoDelete(pDb, NATWAN_TABLE);                            /*  ɾ�����ݿ�����              */

    /*
     *  ���ݿ��ֶΣ�ID  WAN
     */
    snprintf(cTbInsertCmd, sizeof(cTbInsertCmd), "INSERT INTO %s VALUES(?, ?);", NATWAN_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbInsertCmd, strlen(cTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ����ȡ���ĵ�ǰ�� WAN ����Ϣ���������ݿ���
     */
    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  ��������                    */
         iCol = 1;

         if (!strlen(pWanInfo.cWanName[i])) {                           /*  �ַ���Ϊ��                  */
             break;
         }

         sqlite3_bind_int(pstmt, iCol++, ++iId);
         sqlite3_bind_text(pstmt, iCol++, pWanInfo.cWanName[i], strlen(pWanInfo.cWanName[i]), LW_NULL);

         sqlite3_step(pstmt);
         sqlite3_reset(pstmt);
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natMapGetDBRow
** ��������: �����ݿ��ȡ map ��Ϣ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iRow              ��Ϣ����
*********************************************************************************************************/
INT  natMapGetDBRow (VOID)
{
    INT        iRow     = 0;
    INT        iColumn  = 0;
    INT        iRet     = PX_ERROR;
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iRet = dataBaseGetTbRowCol(pDb, NATMAP_TABLE, &iRow, &iColumn);

    if (PX_ERROR == iRet) {
        return (0);
    }

    return  (iRow);
}
/*********************************************************************************************************
** ��������: natMapGetDBInfo
** ��������: �����ݿ��ȡ�˿�ӳ����Ϣ (�����ڲ������ڴ�, ��Ҫ�������ͷ�)
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : pNatMapBuff        natmap buf
*********************************************************************************************************/
PVOID  natMapGetDBInfo (VOID)
{
    PCHAR                 pcNatMapInfoBuf  = LW_NULL;
    INT                   iNatMapNum       = 0;
    INT                   iBufLen          = 0;
    INT                   iRet             = PX_ERROR;

    iNatMapNum = natMapGetDBRow();
    if (0 == iNatMapNum) {

        return  (LW_NULL);
    }

    iBufLen         = iNatMapNum * sizeof(__NATMAP_INFO);
    pcNatMapInfoBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pcNatMapInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (LW_NULL);
    }

    bzero(pcNatMapInfoBuf, iBufLen);
    iRet = __natMapGetFromDB(pcNatMapInfoBuf);                          /*  ��ȡ ��Ϣ                   */
    if (PX_ERROR == iRet) {
        free(pcNatMapInfoBuf);

        return  (LW_NULL);
    }

    return  (pcNatMapInfoBuf);
}
/*********************************************************************************************************
** ��������: natMapUpdateToDB
** ��������: �������ݿ�˿�ӳ��״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT  natMapUpdateToDB (VOID)
{
    CHAR             cTbCmd[200]        = {0};
    sqlite3_stmt    *pstmt              = LW_NULL;
    const CHAR      *pcTail             = LW_NULL;
    CHAR            *pcErrMsg           = LW_NULL;
    sqlite3         *pDb                = LW_NULL;
    INT              iCol               = 0;
    INT              iRet;

    __PNATMAP_INFO   pNatMapInfo        = LW_NULL;
    __PNATMAP_INFO   pNatMapTmp         = LW_NULL;
    PCHAR            pcNatMapBuf        = LW_NULL;

    INT              iMapItemNum        = 0;
    INT              iBufLen            = 0;
    INT              i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    iMapItemNum = natMapItemNumGet();
    if (0 == iMapItemNum) {
        sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
        dataBaseTbInfoDelete(pDb, NATMAP_TABLE);                        /*  ɾ�����ݿ�����              */
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

        return  (ERROR_NONE);
    }

    iBufLen = iMapItemNum * sizeof(__NATMAP_INFO);
    pcNatMapBuf = malloc(iBufLen);
    if (LW_NULL == pcNatMapBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (PX_ERROR);
    }

    bzero(pcNatMapBuf, iBufLen);
    natMapInfoGet(pcNatMapBuf, 0, MAP_INFO_ALL);                        /*  ��ȡ ��Ϣ                   */

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    dataBaseTbInfoDelete(pDb, NATMAP_TABLE);                            /*  ɾ�����ݿ�����              */

    snprintf(cTbCmd,
            sizeof(cTbCmd),
            "UPDATE sqlite_sequence SET seq = 0 WHERE name='%s';",
            NATMAP_TABLE);

    iRet = sqlite3_exec(pDb, cTbCmd, LW_NULL, LW_NULL, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base sqlite3_exec failed.\n");
        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        sqlite3_close(pDb);
        free(pcNatMapBuf);

        return  (PX_ERROR);
    }

    bzero(cTbCmd, sizeof(cTbCmd));

    /*
     *  ���ݿ��ֶΣ�ID  WAN
     */
    snprintf(cTbCmd, sizeof(cTbCmd), "INSERT INTO %s VALUES(?, ?, ?, ?, ?, ?);", NATMAP_TABLE);

    iRet = sqlite3_prepare_v2(pDb, cTbCmd, strlen(cTbCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        free(pcNatMapBuf);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ����ȡ������Ϣ���������ݿ���
     */
    pNatMapTmp  = (__PNATMAP_INFO)pcNatMapBuf;
    pNatMapInfo = (__PNATMAP_INFO)pcNatMapBuf;
    for (i = 0; i < iMapItemNum; i++) {                                 /*  ��������                    */
         iCol = 2;

         sqlite3_bind_int(pstmt, iCol++, pNatMapInfo->iWanPort);
         sqlite3_bind_int(pstmt, iCol++, pNatMapInfo->iLanPort);
         sqlite3_bind_text(pstmt, iCol++, pNatMapInfo->cIpString,
                           strlen(pNatMapInfo->cIpString), LW_NULL);
         sqlite3_bind_int(pstmt, iCol++, pNatMapInfo->iIpCnt);
         sqlite3_bind_text(pstmt, iCol++, pNatMapInfo->cProtoString,
                           strlen(pNatMapInfo->cProtoString), LW_NULL);

         sqlite3_step(pstmt);
         sqlite3_reset(pstmt);

         pcNatMapBuf = (CHAR *)pcNatMapBuf + sizeof(__NATMAP_INFO);
         pNatMapInfo = (__PNATMAP_INFO)(pcNatMapBuf);
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    if (NULL != pNatMapTmp) {
        free(pNatMapTmp);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natAliasGetDBRow
** ��������: �����ݿ��ȡ���������Ϣ����
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iRow              ��Ϣ����
*********************************************************************************************************/
INT  natAliasGetDBRow (VOID)
{
    INT        iRow     = 0;
    INT        iColumn  = 0;
    INT        iRet     = PX_ERROR;
    sqlite3   *pDb      = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iRet = dataBaseGetTbRowCol(pDb, NATALIAS_TABLE, &iRow, &iColumn);

    if (PX_ERROR == iRet) {
        return (0);
    }

    return  (iRow);
}
/*********************************************************************************************************
** ��������: natAliasGetDBInfo
** ��������: �����ݿ��ȡ���������Ϣ (�����ڲ������ڴ�, ��Ҫ�������ͷ�)
** �䡡��  : NONE
** �䡡��  : NONE
** �䡡��  : pNatAliasBuff      ������Ϣ
*********************************************************************************************************/
PVOID  natAliasGetDBInfo (VOID)
{
    PCHAR                 pNatAliasInfoBuf   = LW_NULL;
    INT                   iNatAliasNum       = 0;
    INT                   iBufLen            = 0;

    iNatAliasNum = natAliasGetDBRow();
    if (0 == iNatAliasNum) {
        return  (LW_NULL);
    }

    iBufLen = iNatAliasNum * sizeof(__NATALIAS_INFO);
    pNatAliasInfoBuf = (PCHAR)malloc(iBufLen);
    if (LW_NULL == pNatAliasInfoBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (LW_NULL);
    }

    bzero(pNatAliasInfoBuf, iBufLen);
    __natAliasGetFromDB(pNatAliasInfoBuf);                              /*  ��ȡ ��Ϣ                   */

    return  (pNatAliasInfoBuf);
}
/*********************************************************************************************************
** ��������: natAliasUpdateToDB
** ��������: �������ݿ��������״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT natAliasUpdateToDB (VOID)
{
    PCHAR            pcTbInsertCmd      = "INSERT INTO "NATALIAS_TABLE" VALUES(?, ?, ?, ?);";

    sqlite3_stmt    *pstmt              = LW_NULL;
    const CHAR      *pcTail             = LW_NULL;
    INT              iRet               = PX_ERROR;
    sqlite3         *pDb                = LW_NULL;
    INT              iCol               = 0;

    __PNATALIAS_INFO pNatAliasInfo      = LW_NULL;
    __PNATALIAS_INFO pNatAliasTmp       = LW_NULL;
    PCHAR            pcNatAliasBuf      = LW_NULL;

    INT              iAliasItemNum      = 0;
    INT              iBufLen            = 0;
    INT              i;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    iAliasItemNum = natAliasItemNumGet();
    if (0 == iAliasItemNum) {
        dataBaseTbInfoDelete(pDb, NATALIAS_TABLE);                      /*  ɾ�����ݿ�����              */

        return  (ERROR_NONE);
    }

    iBufLen = iAliasItemNum * sizeof(__NATALIAS_INFO);
    pcNatAliasBuf = malloc(iBufLen);
    if (LW_NULL == pcNatAliasBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (PX_ERROR);
    }

    bzero(pcNatAliasBuf, iBufLen);
    natAliasInfoGet(pcNatAliasBuf, 0, MAP_INFO_ALL);                    /*  ��ȡ ��Ϣ                   */

    dataBaseTbInfoDelete(pDb, NATALIAS_TABLE);                          /*  ɾ�����ݿ�����              */

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    iRet = sqlite3_prepare_v2(pDb, pcTbInsertCmd, strlen(pcTbInsertCmd), &pstmt, &pcTail);
    if (SQLITE_OK != iRet) {
        if (pstmt) {
            sqlite3_finalize(pstmt);
        }

        sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);
        free(pcNatAliasBuf);

        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base option failed.\n");

        return  (PX_ERROR);
    }

    /*
     *  ����ȡ������Ϣ���������ݿ���
     */
    pNatAliasTmp  = (__PNATALIAS_INFO)pcNatAliasBuf;
    pNatAliasInfo = (__PNATALIAS_INFO)pcNatAliasBuf;
    for (i = 0; i < iAliasItemNum; i++) {                               /*  ��������                    */
         iCol = 2;

         sqlite3_bind_text(pstmt, iCol++, pNatAliasInfo->cAliasString,
                           strlen(pNatAliasInfo->cAliasString), LW_NULL);
         sqlite3_bind_text(pstmt, iCol++, pNatAliasInfo->cLoaclSString,
                           strlen(pNatAliasInfo->cLoaclSString), LW_NULL);
         sqlite3_bind_text(pstmt, iCol++, pNatAliasInfo->cLoaclEString,
                           strlen(pNatAliasInfo->cLoaclEString), LW_NULL);

         sqlite3_step(pstmt);
         sqlite3_reset(pstmt);

         pcNatAliasBuf = (CHAR *)pcNatAliasBuf + sizeof(__NATALIAS_INFO);
         pNatAliasInfo = (__PNATALIAS_INFO)(pcNatAliasBuf);
    }

    sqlite3_finalize(pstmt);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    if (NULL != pNatAliasTmp) {
        free(pNatAliasTmp);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: natIpFragUpdateToDB
** ��������: �������ݿ��Ƭ������״̬
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : PX_ERROR           ʧ��
**           ERROR_NONE         �ɹ�
*********************************************************************************************************/
INT  natIpFragUpdateToDB (VOID)
{
    CHAR          cTbInsertCmd[200]  = {0};
    INT           iNatIpFragInfo     = 0;
    INT           iRet;
    sqlite3      *pDb   = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    iNatIpFragInfo = natIpFragGet();

    dataBaseTbInfoDelete(pDb, NATIPFRAG_TABLE);                         /*  ɾ�����ݿ�����              */

    snprintf(cTbInsertCmd,
             sizeof(cTbInsertCmd),
             "INSERT INTO %s (TCP, UDP, ICMP) VALUES (%d, %d, %d);",
             NATIPFRAG_TABLE,
             (iNatIpFragInfo & 0x01) ? 1 : 0,
             (iNatIpFragInfo & 0x02) ? 1 : 0,
             (iNatIpFragInfo & 0x04) ? 1 : 0);

    iRet = dataBaseCmdExec(pDb, cTbInsertCmd);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: natIpFragGetFromDB
** ��������: �����ݿ��ȡ��Ƭ����Ϣ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : iNatIpFragInfo    ��Ƭ����Ϣ
*********************************************************************************************************/
INT  natIpFragGetFromDB (VOID)
{
    CHAR            cSelectSql[200]  = {0};
    INT             iRet             = PX_ERROR;
    sqlite3        *pDb              = LW_NULL;
    CHAR          **cDbResult        = LW_NULL;
    CHAR           *pcErrMsg         = LW_NULL;
    INT             IRow, IColumn;

    INT             iNatIpFragInfo   = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", NATIPFRAG_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        iNatIpFragInfo |= atoi(cDbResult[1 * IColumn + 0]) << 0;
        iNatIpFragInfo |= atoi(cDbResult[1 * IColumn + 1]) << 1;
        iNatIpFragInfo |= atoi(cDbResult[1 * IColumn + 2]) << 2;
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (iNatIpFragInfo);
}

/*********************************************************************************************************
** ��������: natTableCreate
** ��������:  NAT ��񴴽�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT natTableCreate (VOID)
{
    sqlite3           *pDb   = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, NATWAN_TABLE, NATWAN_TABLE_HEAD));
    INIT_CHECK(dataBaseTbCreate(pDb, NATLAN_TABLE, NATLAN_TABLE_HEAD));
    INIT_CHECK(dataBaseTbCreate(pDb, NATMAP_TABLE, NATMAP_TABLE_HEAD));
    INIT_CHECK(dataBaseTbCreate(pDb, NATALIAS_TABLE, NATALIAS_TABLE_HEAD));
    INIT_CHECK(dataBaseTbCreate(pDb, NATIPFRAG_TABLE, NATIPFRAG_TABLE_HEAD));

    return  (ERROR_NONE);
}

