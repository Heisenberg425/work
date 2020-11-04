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
** ��   ��   ��: kidvpnRecovery.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 27 ��
**
** ��        ��: VPN �ָ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "vnd/vnd_common.h"
#include "vnd/vndDB.h"
#include "kidvpn_common.h"
#include "kidvpnDB.h"
#include "route/route_common.h"
#include "database/database_common.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
/*********************************************************************************************************
  ��Ŀ���ڱ�־�궨��
*********************************************************************************************************/
#define ITEM_EXIST                    (1)                               /*  ��Ŀ����                    */
#define ITEM_NOT_EXIST                (0)                               /*  ��Ŀ������                  */
/*********************************************************************************************************
** ��������: __vpnRouteInfoDelFromDb
** ��������: �����ݿ���ɾ�����������ľ�̬·����Ϣ
** �䡡��  : iVndId             �������� ID
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __vpnRouteInfoDelFromDb (INT  iVndID)
{
    CHAR                  cIfName[NETIF_NAMESIZE] = {0};
    INT                   iNetIfIndex             = 0;
    INT                   iRet;

    /*
     *  ��ȡ VND �����豸����ֵ
     */
    iRet = netIfIndexGetByVndId(iVndID, &iNetIfIndex);
    if (ERROR_NONE == iRet) {
        sprintf(cIfName, "vn%d", iNetIfIndex);

        iRet = staticRouteCleanFromDbByIfName(cIfName);
        if (PX_ERROR == iRet) {
            __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete static router item from db error\n");

            return  (PX_ERROR);
        }
    }

    return  (iRet);
}
/*********************************************************************************************************
** ��������: __kidvpnItemExistFlagGet
** ��������: ���� VNDID ��ȡ����״̬
** ��  ��  : iVndId     VPN ID
** ��  ��  : NONE
** ��  ��  : ���ڱ�־�����ڷ���1�������ڷ���0
*********************************************************************************************************/
static INT  __kidvpnItemExistFlagGet (INT  iVndId)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iFlag         = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "select * from %s where VNDID = %d;", KIDVPN_TABLE, iVndId);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (PX_ERROR == iFlag) {
        return  (ITEM_NOT_EXIST);
    }

    return  (ITEM_EXIST);
}
/*********************************************************************************************************
** ��������: __invalidVndInfoDel
** ��������: �� VND ���ݿ���ɾȥ��Ч�� VND ��Ϣ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __invalidVndInfoDel (VOID)
{
    INT                   iRet;
    INT                   iItemNum         = 0;
    __PVND_HANDLE         pVndHandleInfo   = NULL;
    __PVND_HANDLE         pTmpInfo         = NULL;
    INT                   iFlag;
    INT                   i;

    iItemNum = vndGetDBRow();
    if (0 == iItemNum) {
        return  (ERROR_NONE);
    } else if (iItemNum < 0) {
        return  (PX_ERROR);
    }

    pVndHandleInfo = (__PVND_HANDLE)vndGetDBInfo();
    if (NULL == pVndHandleInfo) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iItemNum; i++) {
        pTmpInfo = pVndHandleInfo + i;

        iFlag = __kidvpnItemExistFlagGet(pTmpInfo->iVndId);
        if (ITEM_NOT_EXIST == iFlag) {
            iRet = __vpnRouteInfoDelFromDb(pTmpInfo->iVndId);
            if (PX_ERROR == iRet) {
                free(pVndHandleInfo);

                return  (PX_ERROR);
            }

            iRet = vndDelDBItem(pTmpInfo->iVndId);
            if (PX_ERROR == iRet) {
                free(pVndHandleInfo);

                return  (PX_ERROR);
            }
        }
    }

    free(pVndHandleInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vndItemExistFlagGet
** ��������: ���� vnd ��ȡ����״̬
** ��  ��  : iVndId     VPN ID
** ��  ��  : NONE
** ��  ��  : ���ڱ�־�����ڷ���1�������ڷ���0
*********************************************************************************************************/
static INT  __vndItemExistFlagGet (INT  iVndId)
{
    CHAR          cSqlCmd[200]  = {0};
    sqlite3      *pDb           = LW_NULL;
    INT           iFlag         = 0;

    OPENRT_DB_HANDLE(pDb);
    if (LW_NULL== pDb) {
        return  (PX_ERROR);
    }

    snprintf(cSqlCmd, sizeof(cSqlCmd), "select * from %s where ID = %d;", VNDINFO_TABLE, iVndId);

    iFlag = dataBaseItemCheck(pDb, cSqlCmd);
    if (PX_ERROR == iFlag) {
        return  (ITEM_NOT_EXIST);
    }

    return  (ITEM_EXIST);
}
/*********************************************************************************************************
** ��������: __invalidKidVpnInfoDel
** ��������: �� KIDVPN ���ݿ���ɾȥ��Ч�� KidVpn ��Ϣ
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
static INT  __invalidKidVpnInfoDel (VOID)
{
    INT                   iRet;
    INT                   iItemNum         = 0;
    __PKIDVPN_DB_INFO     pKidVndInfo      = NULL;
    __PKIDVPN_DB_INFO     pTmpInfo         = NULL;
    INT                   iFlag;
    INT                   i;

    iItemNum = kidvpnGetDBRow();
    if (0 == iItemNum) {
        return  (ERROR_NONE);
    } else if (iItemNum < 0) {
        return  (PX_ERROR);
    }

    pKidVndInfo = (__PKIDVPN_DB_INFO)kidvpnGetDBInfo();
    if (NULL == pKidVndInfo) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iItemNum; i++) {
        pTmpInfo = pKidVndInfo + i;

        iFlag = __vndItemExistFlagGet(pTmpInfo->kidvpnHandle.iVndID);
        if (ITEM_NOT_EXIST == iFlag) {
            iRet = kidvpnDelDBItem(pTmpInfo->kidvpnHandle.iVndID);
            if (PX_ERROR == iRet) {
                return  (PX_ERROR);
            }
        }
    }

    free(pKidVndInfo);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: kidvpnRecovery
** ��������: �ָ�����ǰ�� VPN ����(Ҫȷ�� VND �����߳��Ѿ�����)
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : �Ƿ�ָ��ɹ�
*********************************************************************************************************/
INT  kidvpnRecovery (VOID)
{
    CHAR                  cSelectSql[200]  = {0};
    INT                   iRet;
    CHAR                **cDbResult;
    INT                   IRow, IColumn;
    sqlite3              *pDb              = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;

    INT                   i;
    __KIDVPN_DB_INFO      kidvpnDBInfo;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

        return  (PX_ERROR);
    }

    iRet = __invalidKidVpnInfoDel();
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    iRet = __invalidVndInfoDel();
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    sprintf(cSelectSql, "SELECT * FROM %s;", KIDVPN_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet && IRow > 0) {
        for (i = 1; i <= IRow; i++) {                                   /*  ���л�ȡ(���˵�һ��)        */
            bzero(&kidvpnDBInfo, sizeof(kidvpnDBInfo));
            kidvpnDBInfo.kidvpnHandle.cHandleType = atoi(cDbResult[i * IColumn + 0]);
            kidvpnDBInfo.kidvpnHandle.iVndID      = atoi(cDbResult[i * IColumn + 1]);
            kidvpnDBInfo.kidvpnHandle.iMtu        = atoi(cDbResult[i * IColumn + 2]);
            kidvpnDBInfo.kidvpnHandle.iPort       = atoi(cDbResult[i * IColumn + 3]);

            inet_aton(cDbResult[i * IColumn + 5], (struct in_addr *)&(kidvpnDBInfo.kidvpnHandle.uiIpAddr));

            strncpy(kidvpnDBInfo.kidvpnHandle.cPassWd,
                    cDbResult[i * IColumn + 7],
                    sizeof(kidvpnDBInfo.kidvpnHandle.cPassWd));

            /*
             *  ���лָ�, �Ȼָ���������
             */
            iRet = vndRecovery(kidvpnDBInfo.kidvpnHandle.iVndID);
            if (ERROR_NONE == iRet) {
                if(PX_ERROR == kidVpnStart(&(kidvpnDBInfo.kidvpnHandle))) {
                    vndDel(kidvpnDBInfo.kidvpnHandle.iVndID);
                }
            } else {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "recovery vnd failed.\n");
            }
        }
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
