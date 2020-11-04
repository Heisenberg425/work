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
** ��   ��   ��: libnat_config.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 23 ��
**
** ��        ��: NAT ��Ϣ����ͷ�ļ�
*********************************************************************************************************/
#ifndef __LIBNAT_CONFIG_H
#define __LIBNAT_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  NAT ���� ID (ACK ���ں�������Ч��У���)
*********************************************************************************************************/
#define OPENRT_NAT_WANLAN_SET       (OPEN_RT_NAT_CONFIG_BASE | 1)       /*  ���� WAN LAN ��             */
#define OPENRT_NAT_WANLAN_SET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 2)       /*  ���� WAN LAN ��Ӧ��         */

#define OPENRT_NAT_IPFRAG_SET       (OPEN_RT_NAT_CONFIG_BASE | 3)       /*  ���� NAT IpFrag             */
#define OPENRT_NAT_IPFRAG_SET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 4)       /*  ���� NAT IpFrag Ӧ��        */

#define OPENRT_NAT_IPFRAG_GET       (OPEN_RT_NAT_CONFIG_BASE | 5)       /*  ��ȡ NAT IpFrag ״̬        */
#define OPENRT_NAT_IPFRAG_GET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 6)       /*  ��ȡ NAT IpFrag ״̬ Ӧ��   */

#define OPENRT_NAT_ASSNODE_GET      (OPEN_RT_NAT_CONFIG_BASE | 7)       /*  ��ȡ NAT �˿�ʹ�����       */
#define OPENRT_NAT_ASSNODE_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 8)       /*  ��ȡ NAT �˿�ʹ����� Ӧ��  */

#define OPENRT_NAT_MAP_SET          (OPEN_RT_NAT_CONFIG_BASE | 9)      /*  ���� NAT �˿�ӳ��           */
#define OPENRT_NAT_MAP_SET_ACK      (OPEN_RT_NAT_CONFIG_BASE | 10)      /*  ���� NAT �˿�ӳ�� Ӧ��      */

#define OPENRT_NAT_MAP_GET          (OPEN_RT_NAT_CONFIG_BASE | 11)      /*  ��ȡ NAT �˿�ӳ��           */
#define OPENRT_NAT_MAP_GET_ACK      (OPEN_RT_NAT_CONFIG_BASE | 12)      /*  ��ȡ NAT �˿�ӳ�� Ӧ��      */

#define OPENRT_NAT_ALIAS_SET        (OPEN_RT_NAT_CONFIG_BASE | 13)      /*  ���� NAT �������           */
#define OPENRT_NAT_ALIAS_SET_ACK    (OPEN_RT_NAT_CONFIG_BASE | 14)      /*  ���� NAT ������� Ӧ��      */

#define OPENRT_NAT_ALIAS_GET        (OPEN_RT_NAT_CONFIG_BASE | 15)      /*  ��ȡ NAT �������           */
#define OPENRT_NAT_ALIAS_GET_ACK    (OPEN_RT_NAT_CONFIG_BASE | 16)      /*  ��ȡ NAT ������� Ӧ��      */

#define OPENRT_NAT_MAP_NUM_GET      (OPEN_RT_NAT_CONFIG_BASE | 17)      /*  ��ȡ MAP ����               */
#define OPENRT_NAT_MAP_NUM_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 18)      /*  ��ȡ MAP ���� Ӧ��          */

#define OPENRT_NAT_ALI_NUM_GET      (OPEN_RT_NAT_CONFIG_BASE | 19)      /*  ��ȡ NAT �����������       */
#define OPENRT_NAT_ALI_NUM_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 20)      /*  ��ȡ NAT ������� Ӧ��      */

/*********************************************************************************************************
  NAT �˿�ʹ�����
*********************************************************************************************************/
#define NAT_FUNC_WAN                (0)                                 /*  NAT WAN �ڱ�־              */
#define NAT_FUNC_LAN                (1)                                 /*  NAT LAN �ڱ�־              */
#define NAT_WANLAN_SET              (1)                                 /*  NAT WAN LAN ����            */
#define NAT_WANLAN_DEL              (0)                                 /*  NAT WAN LAN ɾ��            */
#define NAT_IPFRAG_ENABLE           (1)                                 /*  NAT ��Ƭ��ʹ��              */
#define NAT_IPFRAG_DISABLE          (0)                                 /*  NAT ��Ƭ������              */
/*********************************************************************************************************
  ��ȡ NAT ��Ϣ����
*********************************************************************************************************/
struct nat_info_get {
    INT32  iStartIndex;                                                 /*  ��ʼ��Ŀ��                  */
    INT32  iEndIndex;                                                   /*  ������Ŀ��                  */
};
typedef struct nat_info_get     __NAT_INFO_GET;
typedef __NAT_INFO_GET         *__PNAT_INFO_GET;
/*********************************************************************************************************
  NAT �˿�ʹ�����
*********************************************************************************************************/
struct nat_assnode {
    INT  iNatAssNodeTotal;                                              /*  ��ǰ�ܵĶ˿ڸ���            */
    INT  iNatAssNodeUse;                                                /*  ��ǰ��ʹ�õĶ˿ڸ���        */
};
typedef struct nat_assnode      __NAT_ASSNODE_INFO;
typedef __NAT_ASSNODE_INFO     *__PNAT_ASSNODE_INFO;
/*********************************************************************************************************
  �˿�ӳ����Ϣ
*********************************************************************************************************/
struct natmap_info {
    INT   iIpCnt;                                                       /*  IP ����                     */
    INT   iWanPort;                                                     /*  �����˿�                    */
    INT   iLanPort;                                                     /*  �����˿�                    */
    CHAR  cIpString[20];                                                /*  IP �ַ���                   */
    CHAR  cProtoString[5];                                              /*  Э�� �ַ���                 */
};
typedef struct natmap_info   __NATMAP_INFO;
typedef struct natmap_info  *__PNATMAP_INFO;
/*********************************************************************************************************
  �������ö˿�ӳ��Ľṹ
*********************************************************************************************************/
struct natmap_handle {
    INT   iIpCnt;                                                       /*  IP ����                     */
    INT   iWanPort;                                                     /*  �����˿�                    */
    INT   iLanPort;                                                     /*  �����˿�                    */
    CHAR  cProto;                                                       /*  Э��                        */
    CHAR  cIpString[20];                                                /*  Ip �ַ���                   */
    CHAR  cHandle;                                                      /*  ����ʽ                    */
};
typedef struct natmap_handle   __NAT_MAP_HANDLE;
typedef struct natmap_handle  *__PNAT_MAP_HANDLE;
/*********************************************************************************************************
  ���������Ϣ
*********************************************************************************************************/
struct natalias_info {
    CHAR  cAliasString[20];                                             /*  Alias �ַ���                */
    CHAR  cLoaclSString[20];                                            /*  Local Start �ַ���          */
    CHAR  cLoaclEString[20];                                            /*  Local End �ַ���            */
};
typedef struct natalias_info   __NATALIAS_INFO;
typedef struct natalias_info  *__PNATALIAS_INFO;
/*********************************************************************************************************
  ����������������Ľṹ
*********************************************************************************************************/
struct natalias_handle {
    CHAR  cAlias[20];                                                   /*  ��������ַ���              */
    CHAR  cLanStart[20];                                                /*  ��ʼ IP �ַ���              */
    CHAR  cLanEnd[20];                                                  /*  ���� IP �ַ���              */
    CHAR  cHandle;                                                      /*  ����ʽ                    */
};
typedef struct natalias_handle   __NATALIAS_HANDLE;
typedef struct natalias_handle  *__PNATALIAS_HANDLE;
/*********************************************************************************************************
  ��Ƭ������
*********************************************************************************************************/
struct nat_ipfrag_handle {
    CHAR  cHandle;                                                      /*  ����ʽ                    */
    CHAR  cIpfragType;                                                  /*  ���õķ�Ƭ��������          */
};
typedef struct nat_ipfrag_handle     __NAT_IPFRAG_HANDLE;
typedef __NAT_IPFRAG_HANDLE         *__PNAT_IPFRAG_HANDLE;
/*********************************************************************************************************
  WAN LAN ����Ϣ
*********************************************************************************************************/
struct nat_wanlan_handle {
    CHAR  cpNetIfName[NETIF_NAMESIZE];                                  /*  ��������                    */
    INT   iFunc;                                                        /*  ���ڹ���                    */
    INT   iHandle;                                                      /*  ��������                    */
};
typedef struct nat_wanlan_handle  __NAT_WANLAN_HANDLE;
typedef __NAT_WANLAN_HANDLE      *__PNAT_WANLAN_HANDLE;

#endif                                                                  /* __LIBNAT_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
