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
** 文   件   名: situation.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 10 月 15 日
**
** 描        述:情景模式数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "situationDB.h"
#include "situation_common.h"
#include "../database/database_common.h"
/*********************************************************************************************************
** 函数名称: __situationInfoGet
** 功能描述: 获取情景模式信息
** 输  入  : pSituationInfo    情景模式信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  situationInfoGet (__PSITUATION_INFO  pSituationInfo)
{
    CHAR                  cSelectSql[200]  = {0};

    INT                   iRet             = PX_ERROR;
    sqlite3              *pDb              = LW_NULL;
    PCHAR                *ppcRes           = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pSituationInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", SITUATION_INFO_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &ppcRes, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            pSituationInfo->iFunc = atoi(ppcRes[iColumn + 2]);

            strncpy(pSituationInfo->ucSituationMode,
                    ppcRes[iColumn + 0],
                    sizeof(pSituationInfo->ucSituationMode));
            strncpy(pSituationInfo->ucPosition,
                    ppcRes[iColumn + 1],
                    sizeof(pSituationInfo->ucPosition));
            strncpy(pSituationInfo->cStatus,
                    ppcRes[iColumn + 3],
                    sizeof(pSituationInfo->cStatus));
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base wan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(ppcRes);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __situationInfoUpdate
** 功能描述: 更新情景模式信息
** 输  入  : pcTableHead       表单头字段
**           pcInfo            字符型信息
**           iInfo             数值型信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT  __situationInfoUpdate (PCHAR  pcTableHead, PCHAR  pcInfo, INT  iInfo)
{
    CHAR          cSqlCmd[300]  = {0};
    sqlite3      *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pcTableHead) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    if (LW_NULL != pcInfo) {
        bzero(cSqlCmd, sizeof(cSqlCmd));
        snprintf(cSqlCmd, sizeof(cSqlCmd),
                 "UPDATE %s  SET %s = '%s';",
                 SITUATION_INFO_TABLE, pcTableHead, pcInfo);
    } else if (-1 != iInfo) {
        snprintf(cSqlCmd, sizeof(cSqlCmd),
                "UPDATE %s  SET %s = %d;",
                 SITUATION_INFO_TABLE, pcTableHead, iInfo);
    } else {
        return  (PX_ERROR);
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: situationUpdateMode
** 功能描述: 更新情景模式当前配置的模式信息
** 输  入  : pcMode            模式
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  situationUpdateMode (PCHAR   pcMode)
{
    if (LW_NULL == pcMode) {
        return  (PX_ERROR);
    }

    return  (__situationInfoUpdate("MODE", pcMode, -1));
}
/*********************************************************************************************************
** 函数名称: wanPageGetFromDb
** 功能描述: 从数据库中获取信息
** 输  入  : NONE
** 输  出  : pWanPageInfo      WAN 页的信息
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  wanPageGetFromDb (__PWANPAGE_INFO  pWanPageInfo)
{
    CHAR                  cSelectSql[200]  = {0};

    INT                   iRet             = PX_ERROR;
    sqlite3              *pDb              = LW_NULL;
    PCHAR                *ppcRes           = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pWanPageInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", WAN_PAGE_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &ppcRes, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            pWanPageInfo->ucMode = atoi(ppcRes[iColumn + 1]);

            strncpy(pWanPageInfo->cNetifName, ppcRes[iColumn + 0], sizeof(pWanPageInfo->cNetifName));
            strncpy(pWanPageInfo->cPppoeUsr,  ppcRes[iColumn + 2], sizeof(pWanPageInfo->cPppoeUsr));
            strncpy(pWanPageInfo->cPppoePwd,  ppcRes[iColumn + 3], sizeof(pWanPageInfo->cPppoePwd));
            strncpy(pWanPageInfo->cIpAddr,    ppcRes[iColumn + 4], sizeof(pWanPageInfo->cIpAddr));
            strncpy(pWanPageInfo->cNetMask,   ppcRes[iColumn + 5], sizeof(pWanPageInfo->cNetMask));
            strncpy(pWanPageInfo->cGateWay,   ppcRes[iColumn + 6], sizeof(pWanPageInfo->cGateWay));
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base wan info failed.\n");

        return  (PX_ERROR);
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(ppcRes);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: kidVpnServPageGetFromDb
** 功能描述: 从数据库中获取信息
** 输  入  : NONE
** 输  出  : pKidVPNServerInfo      KIDVPN SERVER 信息
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServPageGetFromDb (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    CHAR                  cSelectSql[200]  = {0};

    INT                   iRet             = PX_ERROR;
    sqlite3              *pDb              = LW_NULL;
    PCHAR                *ppcRes           = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pKidVPNServerInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", KIDVPN_SERV_PAGE_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &ppcRes, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            strncpy(pKidVPNServerInfo->cPublicIp,
                    ppcRes[iColumn + 0],
                    sizeof(pKidVPNServerInfo->cPublicIp));
            strncpy(pKidVPNServerInfo->cVndIpAddr,
                    ppcRes[iColumn + 1],
                    sizeof(pKidVPNServerInfo->cVndIpAddr));
            strncpy(pKidVPNServerInfo->cVndNetMask,
                    ppcRes[iColumn + 2],
                    sizeof(pKidVPNServerInfo->cVndNetMask));
            strncpy(pKidVPNServerInfo->cVndGateway,
                    ppcRes[iColumn + 3],
                    sizeof(pKidVPNServerInfo->cVndGateway));
            strncpy(pKidVPNServerInfo->cPasswd,
                    ppcRes[iColumn + 4],
                    sizeof(pKidVPNServerInfo->cPasswd));
            strncpy(pKidVPNServerInfo->cAesKey,
                    ppcRes[iColumn + 5],
                    sizeof(pKidVPNServerInfo->cAesKey));
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base kidvpn server info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(ppcRes);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: kidVpnCliPageGetFromDb
** 功能描述: 从数据库中获取信息 KidVPN 客户端信息
** 输  入  : NONE
** 输  出  : pKidVPNClientInfo      KIDVPN Client 信息
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnCliPageGetFromDb (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    CHAR                  cSelectSql[200]  = {0};

    INT                   iRet             = PX_ERROR;
    sqlite3              *pDb              = LW_NULL;
    PCHAR                *ppcRes           = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pKidVPNClientInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "arguement error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", KIDVPN_CLI_PAGE_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &ppcRes, &iRow, &iColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        if (1 == iRow) {
            strncpy(pKidVPNClientInfo->cPublicIp,
                    ppcRes[iColumn + 0],
                    sizeof(pKidVPNClientInfo->cPublicIp));
            strncpy(pKidVPNClientInfo->cVndIpAddr,
                    ppcRes[iColumn + 1],
                    sizeof(pKidVPNClientInfo->cVndIpAddr));
            strncpy(pKidVPNClientInfo->cVndNetMask,
                    ppcRes[iColumn + 2],
                    sizeof(pKidVPNClientInfo->cVndNetMask));
            strncpy(pKidVPNClientInfo->cPasswd,
                    ppcRes[iColumn + 3],
                    sizeof(pKidVPNClientInfo->cPasswd));
            strncpy(pKidVPNClientInfo->cAesKey,
                    ppcRes[iColumn + 4],
                    sizeof(pKidVPNClientInfo->cAesKey));
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base kidvpn client info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(ppcRes);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: situationAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pSituationInfo      情景模式信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  situationAddItemToDb (__PSITUATION_INFO  pSituationInfo)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT32         iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pSituationInfo) {
        return  (PX_ERROR);
    }

    dataBaseGetTbRowCol(pDb, SITUATION_INFO_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "INSERT INTO %s (MODE, POSITION, FUNC, STATUS) "
                "VALUES ('%s', '%s', %d, '%s');",
                SITUATION_INFO_TABLE,
                pSituationInfo->ucSituationMode,
                pSituationInfo->ucPosition,
                pSituationInfo->iFunc,
                pSituationInfo->cStatus);
    } else {
        if (-1 == pSituationInfo->iFunc) {
            snprintf(cSqlCmd,
                     sizeof(cSqlCmd),
                     "UPDATE %s  SET MODE = '%s', POSITION = '%s', STATUS = '%s';",
                     SITUATION_INFO_TABLE,
                     pSituationInfo->ucSituationMode,
                     pSituationInfo->ucPosition,
                     pSituationInfo->cStatus);
        } else {
            snprintf(cSqlCmd,
                     sizeof(cSqlCmd),
                     "UPDATE %s  SET MODE = '%s', POSITION = '%s', FUNC = %d, STATUS = '%s';",
                     SITUATION_INFO_TABLE,
                     pSituationInfo->ucSituationMode,
                     pSituationInfo->ucPosition,
                     pSituationInfo->iFunc,
                     pSituationInfo->cStatus);
        }
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}

/*********************************************************************************************************
** 函数名称: wanPageAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pWanPageInfo      WAN 口信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  wanPageAddItemToDb (__PWANPAGE_INFO  pWanPageInfo)
{
    CHAR          cSqlCmd[512]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT32         iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pWanPageInfo) {
        return  (PX_ERROR);
    }

    dataBaseGetTbRowCol(pDb, WAN_PAGE_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "INSERT INTO %s (WAN_NAME, MODE, USER, PWD, IP, NETMASK, GATEWAY) "
                "VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s');",
                WAN_PAGE_TABLE,
                pWanPageInfo->cNetifName,
                pWanPageInfo->ucMode,
                pWanPageInfo->cPppoeUsr,
                pWanPageInfo->cPppoePwd,
                pWanPageInfo->cIpAddr,
                pWanPageInfo->cNetMask,
                pWanPageInfo->cGateWay);
    } else {
        snprintf(cSqlCmd,
                 sizeof(cSqlCmd),
                 "UPDATE %s  SET WAN_NAME = '%s', "
                 "MODE = %d , USER = '%s' , PWD = '%s', IP = '%s', NETMASK = '%s', GATEWAY = '%s';",
                 WAN_PAGE_TABLE,
                 pWanPageInfo->cNetifName,
                 pWanPageInfo->ucMode,
                 pWanPageInfo->cPppoeUsr,
                 pWanPageInfo->cPppoePwd,
                 pWanPageInfo->cIpAddr,
                 pWanPageInfo->cNetMask,
                 pWanPageInfo->cGateWay);
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: kidVpnServerPageAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pKidVPNServerInfo      KidVPN server 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerPageAddItemToDb (__PKIDVPN_SERVER_INFO  pKidVPNServerInfo)
{
    CHAR          cSqlCmd[384]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT32         iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pKidVPNServerInfo) {
        return  (PX_ERROR);
    }

    dataBaseGetTbRowCol(pDb, KIDVPN_SERV_PAGE_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "INSERT INTO %s (PUBLIC_IP, VND_IP, VND_MSK, VND_GW, PASSWD, AESKEY, VNDID) "
                "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', %d);",
                KIDVPN_SERV_PAGE_TABLE,
                pKidVPNServerInfo->cPublicIp,
                pKidVPNServerInfo->cVndIpAddr,
                pKidVPNServerInfo->cVndNetMask,
                pKidVPNServerInfo->cVndGateway,
                pKidVPNServerInfo->cPasswd,
                pKidVPNServerInfo->cAesKey,
                pKidVPNServerInfo->iVndId);
    } else {
        snprintf(cSqlCmd,
                 sizeof(cSqlCmd),
                 "UPDATE %s  SET PUBLIC_IP = '%s', VND_IP = '%s', "
                 "VND_MSK = '%s', VND_GW = '%s', PASSWD = '%s', AESKEY = '%s', VNDID = %d;",
                 KIDVPN_SERV_PAGE_TABLE,
                 pKidVPNServerInfo->cPublicIp,
                 pKidVPNServerInfo->cVndIpAddr,
                 pKidVPNServerInfo->cVndNetMask,
                 pKidVPNServerInfo->cVndGateway,
                 pKidVPNServerInfo->cPasswd,
                 pKidVPNServerInfo->cAesKey,
                 pKidVPNServerInfo->iVndId);
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: kidVpnClientPageAddItemToDb
** 功能描述: 向数据库中添加一个条目(如果已经存在，则更新条目内容)
** 输  入  : pKidVPNClientInfo      KidVPN Client 信息
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnClientPageAddItemToDb (__PKIDVPN_CLIENT_INFO  pKidVPNClientInfo)
{
    CHAR          cSqlCmd[384]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT32         iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb || LW_NULL == pKidVPNClientInfo) {
        return  (PX_ERROR);
    }

    dataBaseGetTbRowCol(pDb, KIDVPN_CLI_PAGE_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        snprintf(cSqlCmd,
                sizeof(cSqlCmd),
                "INSERT INTO %s (PUBLIC_IP, VND_IP, VND_MSK, PASSWD, AESKEY, VNDID) "
                "VALUES ('%s', '%s', '%s', '%s', '%s', %d);",
                KIDVPN_CLI_PAGE_TABLE,
                pKidVPNClientInfo->cPublicIp,
                pKidVPNClientInfo->cVndIpAddr,
                pKidVPNClientInfo->cVndNetMask,
                pKidVPNClientInfo->cPasswd,
                pKidVPNClientInfo->cAesKey,
                pKidVPNClientInfo->iVndId);
    } else {
        snprintf(cSqlCmd,
                 sizeof(cSqlCmd),
                 "UPDATE %s  SET PUBLIC_IP = '%s', VND_IP = '%s', "
                 "VND_MSK = '%s', PASSWD = '%s', AESKEY = '%s', VNDID = %d;",
                 KIDVPN_CLI_PAGE_TABLE,
                 pKidVPNClientInfo->cPublicIp,
                 pKidVPNClientInfo->cVndIpAddr,
                 pKidVPNClientInfo->cVndNetMask,
                 pKidVPNClientInfo->cPasswd,
                 pKidVPNClientInfo->cAesKey,
                 pKidVPNClientInfo->iVndId);
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: kidVpnServerPageDeleteFromDb
** 功能描述: 删除数据库中的条目
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServerPageDeleteFromDb (VOID)
{
    CHAR          cSqlCmd[100]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT32         iRow, iColumn;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    dataBaseGetTbRowCol(pDb, KIDVPN_SERV_PAGE_TABLE, &iRow, &iColumn);
    if (0 == iRow) {                                                    /*  只有字段行                  */
        return  (ERROR_NONE);
    } else {
        snprintf(cSqlCmd, sizeof(cSqlCmd), "DELETE FROM %s;", KIDVPN_SERV_PAGE_TABLE);
    }

    return  (dataBaseCmdExec(pDb, cSqlCmd));
}
/*********************************************************************************************************
** 函数名称: kidVpnServInfoDeleteFromDbByVndid
** 功能描述: 根据 VND ID 删除 KidVPN Server 数据库中的条目
** 输  入  : iVndId      VND ID
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnServInfoDeleteFromDbByVndid (INT  iVndId)
{
    CHAR          cCmd[100]  = {0};
    sqlite3      *pDb        = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cCmd, sizeof(cCmd), "DELETE FROM %s WHERE VNDID = %d;", KIDVPN_SERV_PAGE_TABLE, iVndId);

    return  (dataBaseCmdExec(pDb, cCmd));
}
/*********************************************************************************************************
** 函数名称: kidVpnCliInfoDeleteFromDbByVndid
** 功能描述: 根据 VND ID 删除 KidVPN Client 数据库中的条目
** 输  入  : iVndId      VND ID
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  kidVpnCliInfoDeleteFromDbByVndid (INT  iVndId)
{
    CHAR          cCmd[100]  = {0};
    sqlite3      *pDb        = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    snprintf(cCmd, sizeof(cCmd), "DELETE FROM %s WHERE VNDID = %d;", KIDVPN_CLI_PAGE_TABLE, iVndId);

    return  (dataBaseCmdExec(pDb, cCmd));
}
/*********************************************************************************************************
** 函数名称: situationTableCreate
** 功能描述: 情景模式表格创建
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT situationTableCreate (VOID)
{
    sqlite3   *pDb   = LW_NULL;
    INT32      iRow, iColumn;
    CHAR       cSituationInserSql[200] = {0};

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    sprintf(cSituationInserSql,
            "INSERT INTO %s VALUES ('%s', '%s', %d, '%s');",
            SITUATION_INFO_TABLE, MODE_MIDDLE_ENT, POS_STARTING_SET, 0, STATUS_UNCONFIG);

    /*
     *  创建情景模式状态表格
     */
    INIT_CHECK(dataBaseTbCreate(pDb, SITUATION_INFO_TABLE, SITUATION_INFO_TABLE_HEAD));

    INIT_CHECK(dataBaseGetTbRowCol(pDb, SITUATION_INFO_TABLE, &iRow, &iColumn));

    if (0 == iRow) {                                                    /*  只有字段行                  */
        INIT_CHECK(dataBaseCmdExec(pDb, cSituationInserSql));           /*  初始化数据库数据            */
    }

    /*
     *  创建 WAN 表格
     */
    INIT_CHECK(dataBaseTbCreate(pDb, WAN_PAGE_TABLE, WAN_PAGE_TABLE_HEAD));

    /*
     *  创建 KIDVPN SERVER 表格
     */
    INIT_CHECK(dataBaseTbCreate(pDb, KIDVPN_SERV_PAGE_TABLE, KIDVPN_SERV_PAGE_TABLE_HEAD));

    /*
     *  创建 KIDVPN CLIENT 表格
     */
    INIT_CHECK(dataBaseTbCreate(pDb, KIDVPN_CLI_PAGE_TABLE, KIDVPN_CLI_PAGE_TABLE_HEAD));

    return  (ERROR_NONE);
}

