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
** 文   件   名: npfDB.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 24 日
**
** 描        述: npf 数据库操作
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <net/if_type.h>
#include "netdev.h"
#include "lwip/netif.h"
#include "npf_common.h"
#include "common.h"
#include "database/database_common.h"

/*********************************************************************************************************
  npf 数据表信息，Type 字段意义 0: MAC 1: IP 2: UDP 3:TCP
*********************************************************************************************************/
#define NPF_TABLE_NAME                  "npf"                           /*  npf 表单名                  */
#define NPF_TABLE_HEAD                  "(ID     INTEGER PRIMARY KEY AUTOINCREMENT," \
                                        "IFNAME  TEXT," \
                                        "Type    INTEGER," \
                                        "MAC     TEXT," \
                                        "IPS     TEXT," \
                                        "IPE     TEXT," \
                                        "PortS   TEXT," \
                                        "PortE   TEXT," \
                                        "Enable  INTEGER," \
                                        "Direct  TEXT," \
                                        "Mangle  TEXT)"                 /*  npf 表字段                  */

static __PNPF_HANDLE pNpfHeader = LW_NULL;                              /*  返回的查询结果句柄          */
INT                  iTotal     = 0;
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define NPF_ARG_CHECK(p, express)       { \
                                        if (NULL != (p)) { \
                                            express; \
                                        } else { \
                                            do {} while (0); \
                                        }}
/*********************************************************************************************************
** 函数名称: __sqlGetRowsCallback
** 函数功能: 获取查询到的条目数
** 输    入: 请见sqlite3_exec回调函数参数说明
** 输    出: 请见sqlite3_exec回调函数参数说明
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __sqlGetRowsCallback (PVOID  pvData, INT  iArgc, CHAR  **ppArgv, CHAR  **ppColName)
{
    if (LW_NULL == pvData || LW_NULL == ppArgv[0]) {
        return (PX_ERROR);
    }

    *((INT *)(pvData)) = atoi((PCHAR)(ppArgv[0]));

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __sqlSelectCallback
** 函数功能: 处理查询结果
** 输    入: 请见sqlite3_exec回调函数参数说明
** 输    出: 请见sqlite3_exec回调函数参数说明
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __sqlSelectCallback (PVOID  pvData, INT  iArgc, CHAR  **ppArgv, CHAR  **ppColName)
{
    __PNPF_HANDLE pTmpNpfRow  = LW_NULL;

    pTmpNpfRow = (__PNPF_HANDLE)pNpfHeader;

    if (LW_NULL == pTmpNpfRow) {
        return (PX_ERROR);
    }

    /*
     *  赋值
     */
    NPF_ARG_CHECK(ppArgv[0], pTmpNpfRow->iRuleSeq = atoi(ppArgv[0]));
    NPF_ARG_CHECK(ppArgv[1], strncpy(pTmpNpfRow->cNetifName, ppArgv[1], sizeof(pTmpNpfRow->cNetifName)););
    NPF_ARG_CHECK(ppArgv[2], pTmpNpfRow->ucType = atoi(ppArgv[2]));
    NPF_ARG_CHECK(ppArgv[3], strncpy(pTmpNpfRow->cHwAddr,  ppArgv[3], sizeof(pTmpNpfRow->cHwAddr)));
    NPF_ARG_CHECK(ppArgv[4], strncpy(pTmpNpfRow->cIpAddrS, ppArgv[4], sizeof(pTmpNpfRow->cIpAddrS)));
    NPF_ARG_CHECK(ppArgv[5], strncpy(pTmpNpfRow->cIpAddrE, ppArgv[5], sizeof(pTmpNpfRow->cIpAddrE)));
    NPF_ARG_CHECK(ppArgv[6], pTmpNpfRow->usPortS = atoi(ppArgv[6]));
    NPF_ARG_CHECK(ppArgv[7], pTmpNpfRow->usPortE = atoi(ppArgv[7]));
    NPF_ARG_CHECK(ppArgv[8], pTmpNpfRow->bEnable = atoi(ppArgv[8]));
    NPF_ARG_CHECK(ppArgv[9], strncpy(pTmpNpfRow->cDirect, ppArgv[9],  sizeof(pTmpNpfRow->cDirect)));
    NPF_ARG_CHECK(ppArgv[10],strncpy(pTmpNpfRow->cMangle, ppArgv[10], sizeof(pTmpNpfRow->cMangle)));

    pTmpNpfRow->iTotal = iTotal;

    /*
     *  偏移值加 1
     */
    pNpfHeader++;

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __initNpfTableCallBack
** 函数功能: sqlite3_exec回调函数,插入NPF Attach规则
** 输    入: 请见sqlite3_exec回调函数参数说明
** 输    出: 请见sqlite3_exec回调函数参数说明
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __initNpfTableCallBack (PVOID  pvData, INT  iArgc, PCHAR  pcArgv[], PCHAR  pcCloumn[])
{
    PCHAR    pcNetifName     = LW_NULL;
    CHAR     cSelectSql[256] = {0};
    sqlite3 *pDb             = LW_NULL;
    CHAR    *pcErrMsg        = LW_NULL;
    INT      iRet            = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return (PX_ERROR);
    }

    if (LW_NULL == pcArgv[0] || LW_NULL == pvData) {
        return (PX_ERROR);
    }

    pcNetifName = (PCHAR)pvData;

    if (0 == atoi(pcArgv[0])) {
        snprintf(cSelectSql, sizeof(cSelectSql),
                "INSERT INTO %s (IFNAME, Type, Enable) VALUES ('%s', '%d', '%d');",
                NPF_TABLE_NAME,
                pcNetifName,
                NPFRULE_ATTACH,
                !(NPFRULE_ATTACH_ENABLE));

        iRet = sqlite3_exec(pDb, cSelectSql, NULL, NULL, &pcErrMsg);
        if (SQLITE_OK != iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                                   "error_code=%d.\n",cSelectSql, iRet);
            sqlite3_free(pcErrMsg);

            return (PX_ERROR);
        }
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: __initNpfTable
** 函数功能: 初始化 NPF 数据表
** 输    入: cNetifName  网口名称
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __initNpfTable (PCHAR  pcNetifName)
{
    CHAR          cSelectSql[256] = {0};
    sqlite3      *pDb             = LW_NULL;
    CHAR         *pcErrMsg        = LW_NULL;
    INT           iRet            = 0;

    if (NULL == pcNetifName) {
        return ;
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return ;
    }

    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT COUNT(*) FROM %s WHERE IFNAME='%s' AND Type='%d'",
            NPF_TABLE_NAME, pcNetifName, NPFRULE_ATTACH);

    iRet = sqlite3_exec(pDb, cSelectSql, __initNpfTableCallBack, (VOID *)pcNetifName, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                               "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);
    }
}

/*********************************************************************************************************
** 函数名称: __initNetIfNpfRules
** 函数功能: 初始化网口的 NPF 功能
** 输    入: NONE
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __initNetIfNpfRules (VOID)
{
    INT32           iSocketFd;
    struct  ifconf  iconf;
    INT32           iRet, i;
    INT32           iBuffSize;

    struct ifreq   *pIfreq     = LW_NULL;
    PCHAR           pcIconfBuf = LW_NULL;

    iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSocketFd < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "socket error.\n");

        return  (PX_ERROR);
    }

    /*
     *  获取需要多大的 buf 存放
     */
    iRet = ioctl(iSocketFd, SIOCGSIZIFCONF, &iBuffSize);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGSIZIFCONF error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    pcIconfBuf = (PCHAR)malloc(iBuffSize);
    if (LW_NULL == pcIconfBuf) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "malloc error.\n");
        close(iSocketFd);

        return  (PX_ERROR);
    }

    bzero(pcIconfBuf, iBuffSize);
    iconf.ifc_len = iBuffSize;
    iconf.ifc_buf = (PCHAR)pcIconfBuf;

    iRet = ioctl(iSocketFd, SIOCGIFCONF, (void *)&iconf);
    if (iRet < 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFCONF error.\n");
        close(iSocketFd);

        free(pcIconfBuf);

        return  (PX_ERROR);
    }

    /*
     *  遍历所有的网口信息
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  获取所有的网卡的类型        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");
            free(pcIconfBuf);

            close(iSocketFd);

            return  (PX_ERROR);
        }

        if (IFT_ETHER == pIfreq->ifr_type) {                            /*  依次初始化网口 NPF 功能    */
            __initNpfTable(pIfreq->ifr_name);
        }
    }

    free(pcIconfBuf);
    close(iSocketFd);

    return (ERROR_NONE);                                                /*  获取总的网卡个数            */
}

/*********************************************************************************************************
** 函数名称: npfTableCreate
** 函数功能: 创建 NPF 规则表
** 输    入: NONE
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
INT  npfTableCreate (VOID)
{
    sqlite3   *pDb = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (PX_ERROR);
    }

    INIT_CHECK(dataBaseTbCreate(pDb, NPF_TABLE_NAME, NPF_TABLE_HEAD));

    return  __initNetIfNpfRules();
}

/*********************************************************************************************************
** 函数名称: insertDBNpfRule
** 函数功能: 插入 NPF 规则
** 输    入: pNpfData  前端传入的参数
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
VOID  insertDBNpfRule (__PNPF_HANDLE  pNpfData)
{
    CHAR          cSelectSql[256] = {0};
    sqlite3      *pDb             = LW_NULL;
    CHAR         *pcErrMsg        = LW_NULL;
    INT           iRet            = 0;

    if (LW_NULL == pNpfData) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "insertDBNpfRule NPF Handle is NULL\n");
        return ;
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return ;
    }

    snprintf(cSelectSql, sizeof(cSelectSql),
            "INSERT INTO %s (IFNAME, Type, MAC, IPS, IPE, PortS, PortE, Direct, Mangle)"
            "VALUES ('%s', '%d', '%s', '%s', '%s', '%u', '%u', '%s', '%s');",
            NPF_TABLE_NAME,
            pNpfData->cNetifName,
            pNpfData->ucType,
            pNpfData->cHwAddr,
            pNpfData->cIpAddrS,
            pNpfData->cIpAddrE,
            pNpfData->usPortS,
            pNpfData->usPortE,
            pNpfData->cDirect,
            pNpfData->cMangle);

    iRet = sqlite3_exec(pDb, cSelectSql, NULL, NULL, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                               "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);
    }
}

/*********************************************************************************************************
** 函数名称: delDBNpfRule
** 函数功能: 删除 NPF 规则
** 输    入: pNpfData  前端传入的参数
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
VOID  delDBNpfRule (__PNPF_HANDLE  pNpfData)
{
    CHAR          cSelectSql[256] = {0};
    sqlite3      *pDb             = LW_NULL;
    CHAR         *pcErrMsg        = LW_NULL;
    INT           iRet            = 0;

    if (LW_NULL == pNpfData) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "delDBNpfRule NPF Handle is NULL\n");
        return ;
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return ;
    }

    snprintf(cSelectSql, sizeof(cSelectSql),
            "DELETE FROM %s WHERE ID=%d;",
            NPF_TABLE_NAME, pNpfData->iRuleSeq);

    iRet = sqlite3_exec(pDb, cSelectSql, NULL, NULL, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                               "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);

        return ;
    }

    return ;
}

/*********************************************************************************************************
** 函数名称: updateDBNpfRule
** 函数功能: 更新 NPF 规则
** 输    入: pNpfData  前端传入的数值
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
VOID  updateDBNpfRule (__PNPF_HANDLE  pNpfData)
{
    CHAR          cSelectSql[256] = {0};
    sqlite3      *pDb             = LW_NULL;
    CHAR         *pcErrMsg        = LW_NULL;
    INT           iRet;

    if (LW_NULL == pNpfData) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "updateDBNpfRule NPF Handle is NULL\n");

        return ;
    }

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return ;
    }

    snprintf(cSelectSql, sizeof(cSelectSql),
            "UPDATE %s SET MAC=%s, IPS=%s, IPE=%s, PortS=%u, "
            "PortE=%u, Direct=%s, Mangle=%s, WHERE INDEX=%d;",
            NPF_TABLE_NAME,
            pNpfData->cHwAddr,
            pNpfData->cIpAddrS,
            pNpfData->cIpAddrE,
            pNpfData->usPortS,
            pNpfData->usPortE,
            pNpfData->cDirect,
            pNpfData->cMangle,
            pNpfData->iRuleSeq);
    iRet = sqlite3_exec(pDb, cSelectSql, NULL, NULL, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                               "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);

        return ;
    }

    return ;
}

/*********************************************************************************************************
** 函数名称: selectDBNpfRule
** 函数功能: 获取 NPF 规则
** 输    入: pNpfData     前端传入的数据
**           piRecordNum  数据条目
**           cWhere       查询条件
** 输    出: piRecordNum  查询到的数据条目
** 返    回: 查询的结果数组
*********************************************************************************************************/
__PNPF_HANDLE  selectDBNpfRule (__PNPF_HANDLE  pNpfData, INT  *piRecordNum, PCHAR  cWhere)
{
    CHAR          cSelectSql[256] = {0};
    INT           iRet;
    sqlite3      *pDb;
    CHAR         *pcErrMsg        = LW_NULL;
    __PNPF_HANDLE pTmpHandle      = LW_NULL;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (LW_NULL);
    }

    snprintf(cSelectSql, sizeof(cSelectSql),
            "SELECT COUNT(*) FROM %s WHERE %s;",
            NPF_TABLE_NAME, ((LW_NULL == cWhere) ? "1=1" : cWhere));

    iRet = sqlite3_exec(pDb, cSelectSql, __sqlGetRowsCallback, (PVOID)piRecordNum, &pcErrMsg);

    /*
     *  piRecordNum 为 0 时，会在 reply 时处卡死
     */
    if ((SQLITE_OK != iRet) || (0 == (*piRecordNum))) {                 /*  未找到符合条件的数据        */
        goto __END;
    }

    /*
     * 申请 iRow 个 NPF 数据结构
     */
    pNpfHeader = (__PNPF_HANDLE)malloc(sizeof(__NPF_HANDLE) * (*piRecordNum));
    if (LW_NULL == pNpfHeader) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "selectDBNpfRule malloc failed\n");

        goto __END;
    }

    pTmpHandle = pNpfHeader;
    bzero(pNpfHeader, sizeof(__NPF_HANDLE) * (*piRecordNum));

    snprintf(cSelectSql, sizeof(cSelectSql),
            "SELECT * FROM %s WHERE %s;",
            NPF_TABLE_NAME, ((LW_NULL == cWhere) ? "1=1" : cWhere));

    iRet   = sqlite3_exec(pDb, cSelectSql, __sqlSelectCallback, LW_NULL, &pcErrMsg);
    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                                "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);

        *piRecordNum = 0;

        free(pNpfHeader);

        pNpfHeader = NULL;

        return  (LW_NULL);
    }

__END:
    return  (pTmpHandle);
}

/*********************************************************************************************************
** 函数名称: getDBNpfRule
** 函数功能: 获取 NPF 规则
** 输    入: pNpfData     前端传入的数据
**           piRecordNum  数据条目
**           cWhere       查询条件
** 输    出: piRecordNum  查询到的数据条目
** 返    回: 查询的结果数组
*********************************************************************************************************/
__PNPF_HANDLE  getDBNpfRule (__PNPF_HANDLE  pNpfData, INT  *piRecordNum, PCHAR  cWhere)
{
    CHAR          cSelectSql[256] = {0};
    INT           iRet;
    sqlite3      *pDb;
    CHAR         *pcErrMsg        = LW_NULL;
    __PNPF_HANDLE pTmpHandle      = LW_NULL;

    *piRecordNum = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL == pDb) {
        return  (LW_NULL);
    }

    snprintf(cSelectSql, sizeof(cSelectSql),
            "SELECT COUNT(*) FROM %s WHERE %s;",
            NPF_TABLE_NAME, ((LW_NULL == cWhere) ? "1=1" : cWhere));

    iRet = sqlite3_exec(pDb, cSelectSql, __sqlGetRowsCallback, (PVOID)piRecordNum, &pcErrMsg);
    if ((SQLITE_OK != iRet) || (0 == (*piRecordNum))) {                 /*  未找到符合条件的数据        */
        return  (LW_NULL);
    }

    iTotal = (*piRecordNum);

    /*
     * 申请 iRow 个 NPF 数据结构
     */
    if ((*piRecordNum) > pNpfData->iItemStart) {
        *piRecordNum = (((*piRecordNum) - pNpfData->iItemStart) > pNpfData->iPageSize) ?
                                pNpfData->iPageSize : ((*piRecordNum) - pNpfData->iItemStart);
    } else {
    	*piRecordNum = 0;

        return  (LW_NULL);
    }

    pNpfHeader = (__PNPF_HANDLE)malloc(sizeof(__NPF_HANDLE) * (*piRecordNum));
    if (LW_NULL == pNpfHeader) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "selectDBNpfRule malloc failed\n");

        sqlite3_free(pcErrMsg);
        *piRecordNum = 0;

        iTotal = 0;

        return  (LW_NULL);
    }

    pTmpHandle = pNpfHeader;
    bzero(pNpfHeader, sizeof(__NPF_HANDLE) * (*piRecordNum));

    snprintf(cSelectSql, sizeof(cSelectSql),
            "SELECT * FROM %s WHERE %s ORDER BY ID DESC LIMIT %d, %d;",
            NPF_TABLE_NAME,
            cWhere,
            pNpfData->iItemStart,
            pNpfData->iPageSize);

    iRet   = sqlite3_exec(pDb, cSelectSql, __sqlSelectCallback, LW_NULL, &pcErrMsg);
    iTotal = 0;

    if (SQLITE_OK != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "Failed to execute sql = %s, "
                                                "error_code=%d.\n",cSelectSql, iRet);
        sqlite3_free(pcErrMsg);
        *piRecordNum = 0;
        free(pNpfHeader);

        return  (LW_NULL);
    }

    return  (pTmpHandle);
}
