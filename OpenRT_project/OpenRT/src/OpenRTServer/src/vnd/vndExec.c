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
** ��   ��   ��: vndExec.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: ������������
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <spawn.h>
#include "lwip/netif.h"
#include "common.h"
#include "vndExec.h"
#include "vndDB.h"
#include "../route/route_common.h"
#include "udial/udial_common.h"

/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static INT  _G_iVndID = VND_DEFAULT_ID;
/*********************************************************************************************************
** ��������: vndGetNextId
** ��������: ��ȡ������һ������������ ID
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : iVndID             �������� ID
*********************************************************************************************************/
INT  vndGetNextId (VOID)
{
    return  (_G_iVndID);
}
/*********************************************************************************************************
** ��������: vndSetNextId
** ��������: ���ô�����һ������������ ID
** �䡡��  : iVndID             �������� ID
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  vndSetNextId (INT  iVndID)
{
    _G_iVndID = (iVndID > _G_iVndID) ? iVndID : _G_iVndID;
}
/*********************************************************************************************************
** ��������: vndStart
** ��������: ����һ����������
** �䡡��  : pVndInfo           ����������Ϣ
** �䡡��  : piVndID            �������� ID
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  vndStart (INT  *piVndID, __PVND_HANDLE  pVndInfo)
{
    CHAR                  cCmdBuf[50] = {0};
    INT                   iNetifIndex;
    INT                   iNetifMaxNum;
    INT                   iRet;

    if (LW_NULL == pVndInfo) {
        return  (PX_ERROR);
    }

    /*
     *  �������ݿ���Ϣ
     */
    iNetifMaxNum = netIfMaxIndexGet();
    iNetifIndex  = netif_get_max_index();
    if (iNetifMaxNum > iNetifIndex) {
        iNetifIndex = iNetifMaxNum;
    } else {
        udialPppNumUpdate();
    }

    if (0 == pVndInfo->ucHwAddr[0] &&
        0 == pVndInfo->ucHwAddr[1] &&
        0 == pVndInfo->ucHwAddr[2] &&
        0 == pVndInfo->ucHwAddr[3] &&
        0 == pVndInfo->ucHwAddr[4] &&
        0 == pVndInfo->ucHwAddr[5]) {
        sprintf(cCmdBuf, "vnd add %d", _G_iVndID);
    } else {
        sprintf(cCmdBuf, "vnd add %d %02x:%02x:%02x:%02x:%02x:%02x",
                _G_iVndID,
                pVndInfo->ucHwAddr[0],
                pVndInfo->ucHwAddr[1],
                pVndInfo->ucHwAddr[2],
                pVndInfo->ucHwAddr[3],
                pVndInfo->ucHwAddr[4],
                pVndInfo->ucHwAddr[5]);
    }

    iRet = system(cCmdBuf);
    if (PX_ERROR == iRet) {
        return  (PX_ERROR);
    }

    *piVndID              = _G_iVndID;
    pVndInfo->iVndId      = _G_iVndID;
    pVndInfo->iNetIfIndex = iNetifIndex;

    iRet = vndAddItemToDb(pVndInfo);                                    /*  �������ݿ�                  */
    if (PX_ERROR == iRet) {
        bzero(cCmdBuf, sizeof(cCmdBuf));

        sprintf(cCmdBuf, "vnd del %d", *piVndID);

        iRet = system(cCmdBuf);
        if (PX_ERROR == iRet) {
            return  (PX_ERROR);
        }
    }

    _G_iVndID++;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: vndDel
** ��������: ɾ��һ����������
** �䡡��  : iVndId             �������� ID
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  vndDel (INT   iVndID)
{
    CHAR                  cCmdBuf[50]             = {0};
    CHAR                  cIfName[NETIF_NAMESIZE] = {0};
    INT                   iNetIfIndex;
    INT                   iRet;

    udialPppNumUpdate();                                                /*  ����һ�ε�ǰ����netif num */

    sprintf(cCmdBuf, "vnd del %d", iVndID);

    /*
     *  ��Ϊ���� vnd ʱ����� ifparam.ini ,�������ﲢû��ɾ�� ini �����Ϣ
     */
    iRet = system(cCmdBuf);
    if (ERROR_NONE == iRet) {
        iRet = netIfIndexGetByVndId(iVndID, &iNetIfIndex);
        if (ERROR_NONE == iRet) {
            sprintf(cIfName, "vn%d", iNetIfIndex);

            iRet = staticRouteCleanFromDbByIfName(cIfName);
            if (PX_ERROR == iRet) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete static router item from db error\n");

                return  (PX_ERROR);
            }

            iRet = vndDelDBItem(iVndID);
            if (PX_ERROR == iRet) {
                __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "delete vnd item from db error\n");

                return  (PX_ERROR);
            }
        }
    }

    return  (iRet);
}
