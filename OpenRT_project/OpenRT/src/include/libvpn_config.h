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
** 文   件   名: libvpn_config.h
**
** 创   建   人: Li.xiaocheng (李孝成)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: OpenRT 配置工具和前端服务定义文件
*********************************************************************************************************/
#ifndef __LIBVPN_CONFIG_H
#define __LIBVPN_CONFIG_H

#include "lwip/ip4_addr.h"
#include "libserver_type.h"

/*********************************************************************************************************
  VPN 服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_ADD_VPN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 1)        /*  添加 VPN                    */
#define OPENRT_ADD_VPN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 2)        /*  添加 VPN   应答             */

#define OPENRT_DEL_VPN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 3)        /*  删除 VPN                    */
#define OPENRT_DEL_VPN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 4)        /*  删除 VPN 应答               */

#define OPENRT_KEY_SAVE_CMD        (OPEN_RT_VPN_CONFIG_BASE | 5)        /*  保存 AES KEY 到本地         */
#define OPENRT_KEY_SAVE_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 6)        /*  保存 AES KEY 到本地应答     */

#define OPENRT_KEY_GEN_CMD         (OPEN_RT_VPN_CONFIG_BASE | 7)        /*  生成 AES KEY                */
#define OPENRT_KEY_GEN_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 8)        /*  生成 AES KEY 应答           */

#define OPENRT_KEY_GET_CMD         (OPEN_RT_VPN_CONFIG_BASE | 9)        /*  获取本地 AES KEY            */
#define OPENRT_KEY_GET_ACK_CMD     (OPEN_RT_VPN_CONFIG_BASE | 10)       /*  获取本地 AES KEY 应答       */

#define OPENRT_INFO_GET_CMD        (OPEN_RT_VPN_CONFIG_BASE | 11)       /*  获取 VPN 信息               */
#define OPENRT_INFO_GET_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 12)       /*  获取 VPN 信息 应答          */

#define OPENRT_LINK_NUM_CMD        (OPEN_RT_VPN_CONFIG_BASE | 13)       /*  获取 VPN 连接信息数目       */
#define OPENRT_LINK_NUM_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 14)       /*  获取 VPN 连接信息数目应答   */

#define OPENRT_LINK_GET_CMD        (OPEN_RT_VPN_CONFIG_BASE | 15)       /*  获取 VPN 连接信息           */
#define OPENRT_LINK_GET_ACK_CMD    (OPEN_RT_VPN_CONFIG_BASE | 16)       /*  获取 VPN 连接信息应答       */

#define OPENRT_POSIT_GET_CMD       (OPEN_RT_VPN_CONFIG_BASE | 17)       /*  获取 VPN 地理信息           */
#define OPENRT_POSIT_GET_ACK_CMD   (OPEN_RT_VPN_CONFIG_BASE | 18)       /*  获取 VPN 地理信息应答       */
/*********************************************************************************************************
  KidVPN 信息 cHandleType 字段
*********************************************************************************************************/
#define TYPE_ADD_VPN_S             (0)                                  /*  添加 VPN 服务器             */
#define TYPE_ADD_VPN_C             (1)                                  /*  添加 VPN 客户端             */
#define TYPE_DEL_VPN               (2)                                  /*  删除 VPN                    */
/*********************************************************************************************************
  KidVPN 信息的结构
*********************************************************************************************************/
struct kidvpn_handle {
    ip4_addr_t         uiIpAddr;                                        /*  IP 地址                     */
    INT                iVndID;                                          /*  虚拟网卡 ID                 */
    INT                iMtu;                                            /*  MTU 值                      */
    INT                iPort;                                           /*  端口信息                    */
    CHAR               cHandleType;                                     /*  处理类型                    */
    CHAR               cStatus;                                         /*  VPN 状态                    */
    CHAR               cPassWd[32];                                     /*  密码                        */
    CHAR               cPos[128];                                       /*  服务器端地理位置信息        */
};
typedef struct kidvpn_handle   __KIDVPN_HANDLE;
typedef struct kidvpn_handle  *__PKIDVPN_HANDLE;
/*********************************************************************************************************
  KidVPN 数据库信息的结构
*********************************************************************************************************/
struct kidvpn_db {
    __KIDVPN_HANDLE    kidvpnHandle;                                    /*  VPN 信息                    */
    INT                iPid;                                            /*  VPN 进程 ID                 */
    CHAR               cStatus;                                         /*  VPN 状态                    */
};
typedef struct kidvpn_db   __KIDVPN_DB_INFO;
typedef struct kidvpn_db  *__PKIDVPN_DB_INFO;
/********************************************************************************************************
  获取 KidVPN 信息结构
*********************************************************************************************************/
struct vpn_info_handle {
    INT64       llStartIndex;                                           /*  开始序号                    */
    INT64       llEndIndex;                                             /*  结束序号                    */
};
typedef struct vpn_info_handle   __VPN_INFO_HANDLE;
typedef struct vpn_info_handle  *__PVPN_INFO_HANDLE;
/*********************************************************************************************************
  KidVPN 数据库信息的结构
*********************************************************************************************************/
struct kidvpn_link {
    INT                iVndId;                                          /*  VND ID                      */
    CHAR               cIp[32];                                         /*  连接客户端 IP               */
    CHAR               cMac[32];                                        /*  连接客户端 MAC              */
    CHAR               cPos[128];                                       /*  客户端地理位置              */
};
typedef struct kidvpn_link   __KIDVPN_LINK;
typedef struct kidvpn_link  *__PKIDVPN_LINK;
/*********************************************************************************************************
  Aes Key 信息的结构
*********************************************************************************************************/
struct aeskey_handle {
    INT                iVndID;                                          /*  虚拟网卡 ID                 */
    CHAR               cAesKey[70];                                     /*  AES KEY                     */
};
typedef struct aeskey_handle   __AES_KEY_HANDLE;
typedef struct aeskey_handle  *__PAES_KEY_HANDLE;
/*********************************************************************************************************
 用于 文件句柄信息的结构
*********************************************************************************************************/
struct fd_handle {
    INT  iVndId;                                                        /*  VND ID                      */
    INT  iPipeFd[2];                                                    /*  管道文件句柄                */
};
typedef struct fd_handle  __FD_HANDLE;
typedef struct fd_handle  *__PFD_HANDLE;
/*********************************************************************************************************
  文件句柄链表节点
*********************************************************************************************************/
struct fd_node{
    LW_LIST_LINE          fdListLine;
    __FD_HANDLE           tNodeData;                                    /*  节点数据                    */
};
typedef struct fd_node   __FD_NODE;
typedef struct fd_node  *__PFD_NODE;
/*********************************************************************************************************
  kidvpn ip 地理位置匹配信息结构体
*********************************************************************************************************/
struct kidvpn_ipPos {
    CHAR            cIp[32];                                            /*  KidVPN IP 信息              */
    CHAR            cPos[128];                                          /*  KidVPN 位置信息             */
};
typedef struct kidvpn_ipPos   __KIDVPN_IPPOS;
typedef struct kidvpn_ipPos  *__PKIDVPN_IPPOS;
/*********************************************************************************************************
  kidvpn 客户端、服务端地理位置结构体
*********************************************************************************************************/
struct kidvpn_pos {
    __KIDVPN_IPPOS  vpnSrc;                                             /*  KidVPN 源地理位置           */
    __KIDVPN_IPPOS  vpnDst;                                             /*  KidVPN 目的地理位置         */
};
typedef struct kidvpn_pos   __KIDVPN_POS;
typedef struct kidvpn_pos  *__PKIDVPN_POS;

#endif                                                                  /* __LIBVPN_CONFIG_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
