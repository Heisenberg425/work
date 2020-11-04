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
** ��   ��   ��: routeEntry.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: OpenRT ·�����ô������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "libroute_config.h"
#include "route_common.h"
/*********************************************************************************************************
** ��������: routeConfigDeal
** ��������: ·�����ô�����
** �䡡��  : pRouteHandle           ǰ�˴����·����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  routeConfigDeal (__PROUTE_HANDLE  pRouteHandle)
{
    INT iRet = 0;

    iRet = routeDoConfig(pRouteHandle);

    replyConfigMsg(OPENRT_CFG_ROUTE_ACK_CMD, iRet);
}
/*********************************************************************************************************
** ��������: routeShowDeal
** ��������: ·�ɲ�ѯ������
** �䡡��  : pRouteHandle           ǰ�˴����·����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  routeShowDeal (__PROUTE_HANDLE  pRouteHandle)
{
    __PROUTE_HANDLE pRouteArrayHeader = LW_NULL;
    INT             iRecordNum        = 0;

    pRouteArrayHeader = routeDoShow(pRouteHandle, &iRecordNum);

    replyQueryMsg(OPENRT_GET_ROUTE_ACK_CMD, sizeof(__ROUTE_HANDLE), iRecordNum , pRouteArrayHeader);

    free(pRouteArrayHeader);
}
/*********************************************************************************************************
** ��������: routeHandleEntry
** ��������: ·���������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  routeHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_CFG_ROUTE_CMD:
        routeConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_ROUTE_CMD:
        routeShowDeal(pArgBuf);
        break;
    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
