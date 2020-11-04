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
** ��   ��   ��: route_common.h
**
** ��   ��   ��: Chen.HongBang (�º��)
**
** �ļ���������: 2018 �� 6 �� 22 ��
**
** ��        ��: route��Ϣͷ�ļ�
*********************************************************************************************************/

#ifndef __ROUTE_COMMON_H
#define __ROUTE_COMMON_H

#include "libroute_config.h"
/*********************************************************************************************************
  ��̬·����غ궨��
*********************************************************************************************************/
#define HOST_ROUTE            "-host"                                   /*����·��                      */
#define NET_ROUTE             "-net"                                    /*����·��                      */
#define GATEWAY_ROUTE         "-gateway"                                /*����·��                      */
#define DEFAULT_ROUTE         "default"                                 /*Ĭ������·��                  */
#define STATIC_ROUTE          "static"                                  /*��̬·��                      */
#define IPv4                  "IPv4"                                    /*IPv4��ַ��·��                */
#define IPv6                  "IPv6"                                    /*IPv6��ַ��·��                */
#define IPv4_ROUTE_TABLE_HEAD "IPv4 Route Table"                        /*IPv4��ַ���ͷ                */
#define IPv6_ROUTE_TABLE_HEAD "IPv6 Route Table"                        /*IPv6��ַ���ͷ                */
#define AODV_ROUTE_TABLE_HEAD "AODV Route Table"                        /*AODV��ͷ                      */
#define ROUTE_SHOW            "query"                                   /*��ѯ·�ɱ�                    */
#define ROUTE_ADD             "add"                                     /*���Ӿ�̬·��                  */
#define ROUTE_DEL             "del"                                     /*ɾ����̬·��                  */
#define ROUTE_CHG             "chg"                                     /*�޸ľ�̬·��                  */
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT    routeDoConfig(__PROUTE_HANDLE  pRouteInfo);
PVOID  routeDoShow(__PROUTE_HANDLE  pRouteHandle, INT  *piRecordNum);
PVOID  routeGetFromDB(INT  *piRecordNum);
INT    staticRouteCleanFromDbByIfName(CPCHAR  cpcIfName);
INT    routeUpdateToDb(CPCHAR  cpcOperType, __PROUTE_HANDLE  pRouteInfo);
VOID   routeHandleEntry(INT  iCommand, PVOID  pArgBuf);
INT    routeTableCreate(VOID);
INT    routeRecovery(VOID);

#endif                                                                  /*  __ROUTE_COMMON_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
