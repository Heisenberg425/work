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
** ��   ��   ��: vndRecovery.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 27 ��
**
** ��        ��: ���������ָ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "interface/interface_common.h"
#include "vnd_common.h"
#include "vndDB.h"
#include "vndExec.h"
#include "libvnd_config.h"
#include "database/database_common.h"
#include "udial/udial_common.h"
/*********************************************************************************************************
** ��������: iVndID
** ��������: �ָ�����ǰ����������
** ��  ��  : pDb               ���ݿ���
**           iVndID            �������� ID
** ��  ��  : NONE
** ��  ��  : �Ƿ�ָ��ɹ�
*********************************************************************************************************/
INT  vndRecovery (INT  iVndID)
{
    INT                   iRet;
    INT                   iNetifIndex;
    INT                   iNetifMaxNum;
    __VND_HANDLE          vndHandleInfo;
    __IFPARAM_INFO        ifparamInfo;
    CHAR                  cCmdBuf[50]   = {0};
    sqlite3              *pDb           = LW_NULL;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

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

    bzero(&vndHandleInfo, sizeof(vndHandleInfo));
    iRet = vndGetInfoById(iVndID, &vndHandleInfo);
    if (PX_ERROR == iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get no vnd info.\n");

        return  (PX_ERROR);
    }

    /*
     *  �õ�������������Ϣ�����лָ�
     */
    bzero(&ifparamInfo, sizeof(ifparamInfo));
    ifparamInfo.cDefault = 0;
    ifparamInfo.cEnable  = 1;
    snprintf(ifparamInfo.cDevName, sizeof(ifparamInfo.cDevName), "vnd-%d", vndGetNextId());
    ifparamInfo.uiIpAddr.addr  = vndHandleInfo.uiIpAddr.addr;
    ifparamInfo.uiNetMask.addr = vndHandleInfo.uiNetMask.addr;
    ifparamInfo.uiGateWay.addr = vndHandleInfo.uiGateWay.addr;
    strncpy((PCHAR)ifparamInfo.ucMac, (const CHAR *)vndHandleInfo.ucHwAddr, sizeof(ifparamInfo.ucMac));
    ifparamInfo.uiUpdateFlag = INI_UPDATE_FLAG;
    ifparamInfo.cHandle      = INI_UPDATE_SEC;
    ifParamIniFileHandle(&ifparamInfo);                                 /*  ����Ϣ���� ifparam.ini �ļ� */

    if (0 == vndHandleInfo.ucHwAddr[0] &&
        0 == vndHandleInfo.ucHwAddr[1] &&
        0 == vndHandleInfo.ucHwAddr[2] &&
        0 == vndHandleInfo.ucHwAddr[3] &&
        0 == vndHandleInfo.ucHwAddr[4] &&
        0 == vndHandleInfo.ucHwAddr[5]) {
        sprintf(cCmdBuf, "vnd add %d", iVndID);
    } else {
        sprintf(cCmdBuf, "vnd add %d %02x:%02x:%02x:%02x:%02x:%02x",
                iVndID,
                vndHandleInfo.ucHwAddr[0],
                vndHandleInfo.ucHwAddr[1],
                vndHandleInfo.ucHwAddr[2],
                vndHandleInfo.ucHwAddr[3],
                vndHandleInfo.ucHwAddr[4],
                vndHandleInfo.ucHwAddr[5]);
    }

    iRet = system(cCmdBuf);                                             /*  ������������                */
    if (ERROR_NONE == iRet) {
        vndHandleInfo.iNetIfIndex = iNetifIndex;
        vndAddItemToDb(&vndHandleInfo);                                 /*  �������ݿ�                  */

        /*
         *  ����һ�� _G_iVndID ����������������ӻָ��� ID ����󴴽���������
         */
        vndSetNextId(++iVndID);
    }

    return  (iRet);
}
