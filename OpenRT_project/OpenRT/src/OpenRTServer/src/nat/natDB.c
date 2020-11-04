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
** 文   件   名: natDB.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 15 日
**
** 描        述:  NAT 状态数据库操作
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
  IP 分片包数据库信息
*********************************************************************************************************/
#define NATIPFRAG_TABLE            "natipfrag"                          /*  分片包表单名                */
#define NATIPFRAG_TABLE_HEAD       "(TCP        INT,"                               \
                                   "UDP         INT,"                               \
                                   "ICMP        INT)"                   /*  分片包表单格式              */
/*********************************************************************************************************
  端口映射数据库信息
*********************************************************************************************************/
#define NATMAP_TABLE               "natmap"                             /*  端口映射表单名              */
#define NATMAP_TABLE_HEAD          "(ID         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "ASSPORT     INT,"                               \
                                   "LOCALPORT   INT,"                               \
                                   "LOCALIP     TEXT,"                              \
                                   "IPCNT       INT,"                               \
                                   "PRTO        TEXT)"                  /*  端口映射表单格式            */
/*********************************************************************************************************
  网络别名数据库信息
*********************************************************************************************************/
#define NATALIAS_TABLE             "natalias"                           /*  网络别名表单名              */
#define NATALIAS_TABLE_HEAD        "(ID         INTEGER PRIMARY KEY AUTOINCREMENT," \
                                   "ALIAS       TEXT,"                              \
                                   "LANSTART    TEXT,"                              \
                                   "LANEND      TEXT)"                  /*  网络别名表单格式            */
/*********************************************************************************************************
  WAN 口配置数据库信息
*********************************************************************************************************/
#define NATWAN_TABLE               "natwan"                             /*  WAN 口表单名                */
#define NATWAN_TABLE_HEAD          "(ID   INT,"  \
                                   "WAN  TEXT)"                         /*  WAN 口表单单格式            */
/*********************************************************************************************************
  LAN 口配置数据库信息
*********************************************************************************************************/
#define NATLAN_TABLE               "natlan"                             /*  LAN 口表单名                */
#define NATLAN_TABLE_HEAD          "(ID   INT,"  \
                                   "LAN  TEXT)"                         /*  LAN 口表单单格式            */
#define MAP_INFO_ALL               (1000)                               /*  所有的MAP信息               */
/*********************************************************************************************************
** 函数名称: __natMapGetFromDB
** 功能描述: 从数据库获取端口映射信息
** 输　入  : pNatMapBuff        natmap buf
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        for (i = 1; i <= iRow; i++) {                                   /*  按行获取(过滤第一行)        */
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
** 函数名称: __natAliasGetFromDB
** 功能描述: 从数据库获取网络别名信息
** 输　入  : NONE
** 输　出  : pNatAliasBuff      别名信息
** 返　回  : ERROR_CODE
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
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
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
** 函数名称: natLanGetFromDB
** 功能描述: 从数据库获取 LAN 口信息
** 输　入  : pLanInfo          LAN 口信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */

            /*
             *  拷贝网卡名称
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
** 函数名称: natLanUpdateToDB
** 功能描述: 更新数据库LAN 口状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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

    dataBaseTbInfoDelete(pDb, NATLAN_TABLE);                            /*  删除数据库内容              */

    /*
     *  数据库字段：ID  WAN
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
     *  将获取到的当前的 LAN 口信息保存在数据库中
     */
    for (i = 0; i < LW_CFG_NET_NAT_MAX_LOCAL_IF; i++) {                 /*  遍历链表                    */
         iCol = 1;

         if (!strlen(pLanInfo.cLanName[i])) {                           /*  字符串为空                  */
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
** 函数名称: natWanGetFromDB
** 功能描述: 从数据库获取 WAN 口信息
** 输　入  : pWanInfo          WAN 口信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */

            /*
             *  拷贝网卡名称
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
** 函数名称: natWanUpdateToDB
** 功能描述: 更新数据库 WAN 口状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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

    dataBaseTbInfoDelete(pDb, NATWAN_TABLE);                            /*  删除数据库内容              */

    /*
     *  数据库字段：ID  WAN
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
     *  将获取到的当前的 WAN 口信息保存在数据库中
     */
    for (i = 0; i < LW_CFG_NET_NAT_MAX_AP_IF; i++) {                    /*  遍历链表                    */
         iCol = 1;

         if (!strlen(pWanInfo.cWanName[i])) {                           /*  字符串为空                  */
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
** 函数名称: natMapGetDBRow
** 功能描述: 从数据库获取 map 信息个数
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : iRow              信息个数
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
** 函数名称: natMapGetDBInfo
** 功能描述: 从数据库获取端口映射信息 (函数内部申请内存, 需要调用者释放)
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : pNatMapBuff        natmap buf
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
    iRet = __natMapGetFromDB(pcNatMapInfoBuf);                          /*  获取 信息                   */
    if (PX_ERROR == iRet) {
        free(pcNatMapInfoBuf);

        return  (LW_NULL);
    }

    return  (pcNatMapInfoBuf);
}
/*********************************************************************************************************
** 函数名称: natMapUpdateToDB
** 功能描述: 更新数据库端口映射状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
        dataBaseTbInfoDelete(pDb, NATMAP_TABLE);                        /*  删除数据库内容              */
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
    natMapInfoGet(pcNatMapBuf, 0, MAP_INFO_ALL);                        /*  获取 信息                   */

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    dataBaseTbInfoDelete(pDb, NATMAP_TABLE);                            /*  删除数据库内容              */

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
     *  数据库字段：ID  WAN
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
     *  将获取到的信息保存在数据库中
     */
    pNatMapTmp  = (__PNATMAP_INFO)pcNatMapBuf;
    pNatMapInfo = (__PNATMAP_INFO)pcNatMapBuf;
    for (i = 0; i < iMapItemNum; i++) {                                 /*  遍历链表                    */
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
** 函数名称: natAliasGetDBRow
** 功能描述: 从数据库获取网络别名信息个数
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : iRow              信息个数
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
** 函数名称: natAliasGetDBInfo
** 功能描述: 从数据库获取网络别名信息 (函数内部申请内存, 需要调用者释放)
** 输　入  : NONE
** 输　出  : NONE
** 输　出  : pNatAliasBuff      别名信息
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
    __natAliasGetFromDB(pNatAliasInfoBuf);                              /*  获取 信息                   */

    return  (pNatAliasInfoBuf);
}
/*********************************************************************************************************
** 函数名称: natAliasUpdateToDB
** 功能描述: 更新数据库网络别名状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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
        dataBaseTbInfoDelete(pDb, NATALIAS_TABLE);                      /*  删除数据库内容              */

        return  (ERROR_NONE);
    }

    iBufLen = iAliasItemNum * sizeof(__NATALIAS_INFO);
    pcNatAliasBuf = malloc(iBufLen);
    if (LW_NULL == pcNatAliasBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error\n");

        return  (PX_ERROR);
    }

    bzero(pcNatAliasBuf, iBufLen);
    natAliasInfoGet(pcNatAliasBuf, 0, MAP_INFO_ALL);                    /*  获取 信息                   */

    dataBaseTbInfoDelete(pDb, NATALIAS_TABLE);                          /*  删除数据库内容              */

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
     *  将获取到的信息保存在数据库中
     */
    pNatAliasTmp  = (__PNATALIAS_INFO)pcNatAliasBuf;
    pNatAliasInfo = (__PNATALIAS_INFO)pcNatAliasBuf;
    for (i = 0; i < iAliasItemNum; i++) {                               /*  遍历链表                    */
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
** 函数名称: natIpFragUpdateToDB
** 功能描述: 更新数据库分片包接收状态
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : PX_ERROR           失败
**           ERROR_NONE         成功
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

    dataBaseTbInfoDelete(pDb, NATIPFRAG_TABLE);                         /*  删除数据库内容              */

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
** 函数名称: natIpFragGetFromDB
** 功能描述: 从数据库获取分片包信息
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : iNatIpFragInfo    分片包信息
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
** 函数名称: natTableCreate
** 功能描述:  NAT 表格创建
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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

