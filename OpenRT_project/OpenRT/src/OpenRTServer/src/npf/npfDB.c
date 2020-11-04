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
** ��   ��   ��: npfDB.c
**
** ��   ��   ��: Shen.WenJun (���ľ�)
**
** �ļ���������: 2018 �� 7 �� 24 ��
**
** ��        ��: npf ���ݿ����
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
  npf ���ݱ���Ϣ��Type �ֶ����� 0: MAC 1: IP 2: UDP 3:TCP
*********************************************************************************************************/
#define NPF_TABLE_NAME                  "npf"                           /*  npf ����                  */
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
                                        "Mangle  TEXT)"                 /*  npf ���ֶ�                  */

static __PNPF_HANDLE pNpfHeader = LW_NULL;                              /*  ���صĲ�ѯ������          */
INT                  iTotal     = 0;
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define NPF_ARG_CHECK(p, express)       { \
                                        if (NULL != (p)) { \
                                            express; \
                                        } else { \
                                            do {} while (0); \
                                        }}
/*********************************************************************************************************
** ��������: __sqlGetRowsCallback
** ��������: ��ȡ��ѯ������Ŀ��
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ERROR_CODE
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
** ��������: __sqlSelectCallback
** ��������: �����ѯ���
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ERROR_CODE
*********************************************************************************************************/
static INT  __sqlSelectCallback (PVOID  pvData, INT  iArgc, CHAR  **ppArgv, CHAR  **ppColName)
{
    __PNPF_HANDLE pTmpNpfRow  = LW_NULL;

    pTmpNpfRow = (__PNPF_HANDLE)pNpfHeader;

    if (LW_NULL == pTmpNpfRow) {
        return (PX_ERROR);
    }

    /*
     *  ��ֵ
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
     *  ƫ��ֵ�� 1
     */
    pNpfHeader++;

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __initNpfTableCallBack
** ��������: sqlite3_exec�ص�����,����NPF Attach����
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ���sqlite3_exec�ص���������˵��
** ��    ��: ERROR_CODE
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
** ��������: __initNpfTable
** ��������: ��ʼ�� NPF ���ݱ�
** ��    ��: cNetifName  ��������
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: __initNetIfNpfRules
** ��������: ��ʼ�����ڵ� NPF ����
** ��    ��: NONE
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
     *  ��ȡ��Ҫ���� buf ���
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
     *  �������е�������Ϣ
     */
    for (i = 0; i < iconf.ifc_len / sizeof(struct ifreq); i++) {
        pIfreq = &iconf.ifc_req[i];
        iRet = ioctl(iSocketFd, SIOCGIFTYPE, pIfreq);                   /*  ��ȡ���е�����������        */
        if (iRet < 0) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL,  "ioctl SIOCGIFTYPE error.\n");
            free(pcIconfBuf);

            close(iSocketFd);

            return  (PX_ERROR);
        }

        if (IFT_ETHER == pIfreq->ifr_type) {                            /*  ���γ�ʼ������ NPF ����    */
            __initNpfTable(pIfreq->ifr_name);
        }
    }

    free(pcIconfBuf);
    close(iSocketFd);

    return (ERROR_NONE);                                                /*  ��ȡ�ܵ���������            */
}

/*********************************************************************************************************
** ��������: npfTableCreate
** ��������: ���� NPF �����
** ��    ��: NONE
** ��    ��: NONE
** ��    ��: ERROR_CODE
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
** ��������: insertDBNpfRule
** ��������: ���� NPF ����
** ��    ��: pNpfData  ǰ�˴���Ĳ���
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: delDBNpfRule
** ��������: ɾ�� NPF ����
** ��    ��: pNpfData  ǰ�˴���Ĳ���
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: updateDBNpfRule
** ��������: ���� NPF ����
** ��    ��: pNpfData  ǰ�˴������ֵ
** ��    ��: NONE
** ��    ��: NONE
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
** ��������: selectDBNpfRule
** ��������: ��ȡ NPF ����
** ��    ��: pNpfData     ǰ�˴��������
**           piRecordNum  ������Ŀ
**           cWhere       ��ѯ����
** ��    ��: piRecordNum  ��ѯ����������Ŀ
** ��    ��: ��ѯ�Ľ������
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
     *  piRecordNum Ϊ 0 ʱ������ reply ʱ������
     */
    if ((SQLITE_OK != iRet) || (0 == (*piRecordNum))) {                 /*  δ�ҵ���������������        */
        goto __END;
    }

    /*
     * ���� iRow �� NPF ���ݽṹ
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
** ��������: getDBNpfRule
** ��������: ��ȡ NPF ����
** ��    ��: pNpfData     ǰ�˴��������
**           piRecordNum  ������Ŀ
**           cWhere       ��ѯ����
** ��    ��: piRecordNum  ��ѯ����������Ŀ
** ��    ��: ��ѯ�Ľ������
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
    if ((SQLITE_OK != iRet) || (0 == (*piRecordNum))) {                 /*  δ�ҵ���������������        */
        return  (LW_NULL);
    }

    iTotal = (*piRecordNum);

    /*
     * ���� iRow �� NPF ���ݽṹ
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
