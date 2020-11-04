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
** 文   件   名: libnat_config.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 23 日
**
** 描        述: NAT 信息配置头文件
*********************************************************************************************************/
#ifndef __LIBNAT_CONFIG_H
#define __LIBNAT_CONFIG_H

#include "SylixOS.h"
#include "libserver_type.h"
#include "lwip/netif.h"

/*********************************************************************************************************
  NAT 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_NAT_WANLAN_SET       (OPEN_RT_NAT_CONFIG_BASE | 1)       /*  设置 WAN LAN 口             */
#define OPENRT_NAT_WANLAN_SET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 2)       /*  设置 WAN LAN 口应答         */

#define OPENRT_NAT_IPFRAG_SET       (OPEN_RT_NAT_CONFIG_BASE | 3)       /*  设置 NAT IpFrag             */
#define OPENRT_NAT_IPFRAG_SET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 4)       /*  设置 NAT IpFrag 应答        */

#define OPENRT_NAT_IPFRAG_GET       (OPEN_RT_NAT_CONFIG_BASE | 5)       /*  获取 NAT IpFrag 状态        */
#define OPENRT_NAT_IPFRAG_GET_ACK   (OPEN_RT_NAT_CONFIG_BASE | 6)       /*  获取 NAT IpFrag 状态 应答   */

#define OPENRT_NAT_ASSNODE_GET      (OPEN_RT_NAT_CONFIG_BASE | 7)       /*  获取 NAT 端口使用情况       */
#define OPENRT_NAT_ASSNODE_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 8)       /*  获取 NAT 端口使用情况 应答  */

#define OPENRT_NAT_MAP_SET          (OPEN_RT_NAT_CONFIG_BASE | 9)      /*  设置 NAT 端口映射           */
#define OPENRT_NAT_MAP_SET_ACK      (OPEN_RT_NAT_CONFIG_BASE | 10)      /*  设置 NAT 端口映射 应答      */

#define OPENRT_NAT_MAP_GET          (OPEN_RT_NAT_CONFIG_BASE | 11)      /*  获取 NAT 端口映射           */
#define OPENRT_NAT_MAP_GET_ACK      (OPEN_RT_NAT_CONFIG_BASE | 12)      /*  获取 NAT 端口映射 应答      */

#define OPENRT_NAT_ALIAS_SET        (OPEN_RT_NAT_CONFIG_BASE | 13)      /*  设置 NAT 网络别名           */
#define OPENRT_NAT_ALIAS_SET_ACK    (OPEN_RT_NAT_CONFIG_BASE | 14)      /*  设置 NAT 网络别名 应答      */

#define OPENRT_NAT_ALIAS_GET        (OPEN_RT_NAT_CONFIG_BASE | 15)      /*  获取 NAT 网络别名           */
#define OPENRT_NAT_ALIAS_GET_ACK    (OPEN_RT_NAT_CONFIG_BASE | 16)      /*  获取 NAT 网络别名 应答      */

#define OPENRT_NAT_MAP_NUM_GET      (OPEN_RT_NAT_CONFIG_BASE | 17)      /*  获取 MAP 个数               */
#define OPENRT_NAT_MAP_NUM_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 18)      /*  获取 MAP 个数 应答          */

#define OPENRT_NAT_ALI_NUM_GET      (OPEN_RT_NAT_CONFIG_BASE | 19)      /*  获取 NAT 网络别名个数       */
#define OPENRT_NAT_ALI_NUM_GET_ACK  (OPEN_RT_NAT_CONFIG_BASE | 20)      /*  获取 NAT 网络别名 应答      */

/*********************************************************************************************************
  NAT 端口使用情况
*********************************************************************************************************/
#define NAT_FUNC_WAN                (0)                                 /*  NAT WAN 口标志              */
#define NAT_FUNC_LAN                (1)                                 /*  NAT LAN 口标志              */
#define NAT_WANLAN_SET              (1)                                 /*  NAT WAN LAN 设置            */
#define NAT_WANLAN_DEL              (0)                                 /*  NAT WAN LAN 删除            */
#define NAT_IPFRAG_ENABLE           (1)                                 /*  NAT 分片包使能              */
#define NAT_IPFRAG_DISABLE          (0)                                 /*  NAT 分片包禁能              */
/*********************************************************************************************************
  获取 NAT 信息参数
*********************************************************************************************************/
struct nat_info_get {
    INT32  iStartIndex;                                                 /*  开始条目数                  */
    INT32  iEndIndex;                                                   /*  结束条目数                  */
};
typedef struct nat_info_get     __NAT_INFO_GET;
typedef __NAT_INFO_GET         *__PNAT_INFO_GET;
/*********************************************************************************************************
  NAT 端口使用情况
*********************************************************************************************************/
struct nat_assnode {
    INT  iNatAssNodeTotal;                                              /*  当前总的端口个数            */
    INT  iNatAssNodeUse;                                                /*  当前已使用的端口个数        */
};
typedef struct nat_assnode      __NAT_ASSNODE_INFO;
typedef __NAT_ASSNODE_INFO     *__PNAT_ASSNODE_INFO;
/*********************************************************************************************************
  端口映射信息
*********************************************************************************************************/
struct natmap_info {
    INT   iIpCnt;                                                       /*  IP 个数                     */
    INT   iWanPort;                                                     /*  外网端口                    */
    INT   iLanPort;                                                     /*  内网端口                    */
    CHAR  cIpString[20];                                                /*  IP 字符串                   */
    CHAR  cProtoString[5];                                              /*  协议 字符串                 */
};
typedef struct natmap_info   __NATMAP_INFO;
typedef struct natmap_info  *__PNATMAP_INFO;
/*********************************************************************************************************
  用于配置端口映射的结构
*********************************************************************************************************/
struct natmap_handle {
    INT   iIpCnt;                                                       /*  IP 个数                     */
    INT   iWanPort;                                                     /*  外网端口                    */
    INT   iLanPort;                                                     /*  内网端口                    */
    CHAR  cProto;                                                       /*  协议                        */
    CHAR  cIpString[20];                                                /*  Ip 字符串                   */
    CHAR  cHandle;                                                      /*  处理方式                    */
};
typedef struct natmap_handle   __NAT_MAP_HANDLE;
typedef struct natmap_handle  *__PNAT_MAP_HANDLE;
/*********************************************************************************************************
  网络别名信息
*********************************************************************************************************/
struct natalias_info {
    CHAR  cAliasString[20];                                             /*  Alias 字符串                */
    CHAR  cLoaclSString[20];                                            /*  Local Start 字符串          */
    CHAR  cLoaclEString[20];                                            /*  Local End 字符串            */
};
typedef struct natalias_info   __NATALIAS_INFO;
typedef struct natalias_info  *__PNATALIAS_INFO;
/*********************************************************************************************************
  用于配置网络别名的结构
*********************************************************************************************************/
struct natalias_handle {
    CHAR  cAlias[20];                                                   /*  网络别名字符串              */
    CHAR  cLanStart[20];                                                /*  起始 IP 字符串              */
    CHAR  cLanEnd[20];                                                  /*  结束 IP 字符串              */
    CHAR  cHandle;                                                      /*  处理方式                    */
};
typedef struct natalias_handle   __NATALIAS_HANDLE;
typedef struct natalias_handle  *__PNATALIAS_HANDLE;
/*********************************************************************************************************
  分片包处理
*********************************************************************************************************/
struct nat_ipfrag_handle {
    CHAR  cHandle;                                                      /*  处理方式                    */
    CHAR  cIpfragType;                                                  /*  设置的分片包的类型          */
};
typedef struct nat_ipfrag_handle     __NAT_IPFRAG_HANDLE;
typedef __NAT_IPFRAG_HANDLE         *__PNAT_IPFRAG_HANDLE;
/*********************************************************************************************************
  WAN LAN 口消息
*********************************************************************************************************/
struct nat_wanlan_handle {
    CHAR  cpNetIfName[NETIF_NAMESIZE];                                  /*  网口名称                    */
    INT   iFunc;                                                        /*  网口功能                    */
    INT   iHandle;                                                      /*  处理类型                    */
};
typedef struct nat_wanlan_handle  __NAT_WANLAN_HANDLE;
typedef __NAT_WANLAN_HANDLE      *__PNAT_WANLAN_HANDLE;

#endif                                                                  /* __LIBNAT_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
