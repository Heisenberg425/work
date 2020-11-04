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
** ��   ��   ��: routehandle.c
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 22 ��
**
** ��        ��:  ·�����ô���
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "route_info.h"
#include "msgcomm.h"
#include "libroute_config.h"

/*********************************************************************************************************
** ��������: routeInfoQuery
** ��������: ��ȡ·����Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : pRouteInfo        ·����Ϣ����
*********************************************************************************************************/
PVOID  routeInfoQuery (__PROUTE_HANDLE  pRouteHandle, INT* piRecordTotal)
{
    return  requestQueryMsg(OPENRT_GET_ROUTE_CMD, pRouteHandle, sizeof(__ROUTE_HANDLE), piRecordTotal);
}

/*********************************************************************************************************
** ��������: routeConfig
** ��������: ·������
** �䡡��  : PROUTE_HANDLE pRouteHandle
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  routeConfig (__PROUTE_HANDLE pRouteHandle)
{
    return  requestConfigMsg (OPENRT_CFG_ROUTE_CMD, pRouteHandle, sizeof(__ROUTE_HANDLE));
}

/*********************************************************************************************************
** ��������: routeArrayFree
** ��������: �ͷ�·�������ڴ�
** �䡡��  : PROUTE_HANDLE pRouteHandle
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
VOID routeArrayFree(PVOID  pRouteHandle)
{
    if (pRouteHandle) {
        free(pRouteHandle);
    }
    return;
}
