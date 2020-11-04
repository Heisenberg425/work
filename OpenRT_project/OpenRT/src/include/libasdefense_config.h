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
** 文   件   名: libasdefense_config.h
**
** 创   建   人: Wu.Pengcheng (吴鹏程)
**
** 文件创建日期: 2018 年 7 月 27 日
**
** 描        述: OpenRT ARP 防御信息头文件
*********************************************************************************************************/

#ifndef __LIBASDEFENSE_CONFIG_H
#define __LIBASDEFENSE_CONFIG_H

#include <netinet/in.h>
#include "libserver_type.h"
#include "netif/etharp.h"

/*********************************************************************************************************
  ARP 防御服务 ID (ACK 用于后期做有效性校验等)
*********************************************************************************************************/
#define OPENRT_GET_ASD_EN_CMD         (OPEN_RT_ARP_DEFENSE_BASE | 1)    /*  使能/失能 ARP 防御          */
#define OPENRT_GET_ASD_EN_ACK_CMD     (OPEN_RT_ARP_DEFENSE_BASE | 2)    /*  使能/失能 ARP 防御应答      */

#define OPENRT_GET_ASD_GET_CMD        (OPEN_RT_ARP_DEFENSE_BASE | 3)    /*  获取 ARP 防御信息           */
#define OPENRT_GET_ASD_GET_ACK_CMD    (OPEN_RT_ARP_DEFENSE_BASE | 4)    /*  获取 ARP 防御信息应答       */

#define OPENRT_GET_ASD_NUM_CMD        (OPEN_RT_ARP_DEFENSE_BASE | 5)    /*  获取 ARP 防御信息个数       */
#define OPENRT_GET_ASD_NUM_ACK_CMD    (OPEN_RT_ARP_DEFENSE_BASE | 6)    /*  获取 ARP 防御信息个数应答   */

#define OPENRT_GET_ASD_STATUS_CMD     (OPEN_RT_ARP_DEFENSE_BASE | 7)    /*  获取 ARP 防御开启状态       */
#define OPENRT_GET_ASD_STATUS_ACK_CMD (OPEN_RT_ARP_DEFENSE_BASE | 8)    /*  获取 ARP 防御开启状态应答   */
/*********************************************************************************************************
  ARP 标志位
*********************************************************************************************************/
#define ARP_NORMAL_FLAG               (0x0)                             /*  ARP 信息正常                */
#define ARP_NORMAL_STR                ("normal")                        /*  ARP 正常字符串              */
#define ARP_CONFLICT_FLAG             (0x01)                            /*  发生 MAC 地址冲突           */
#define ARP_CONFLICT_STR              ("conflict")                      /*  ARP 冲突字符串              */
#define ARP_ATTACK_FLAG               (0x02)                            /*  发生 MAC 欺骗               */
#define ARP_ATTACK_STR                ("attack")                        /*  ARP 攻击字符串              */
/*********************************************************************************************************
  防御使能标志宏
*********************************************************************************************************/
#define ARP_SPOOF_DEFENSE_ENABLE      (1)                               /*  使能防御                    */
#define ARP_SPOOF_DEFENSE_DISABLE     (0)                               /*  禁能防御                    */
/*********************************************************************************************************
  获取信息类型
*********************************************************************************************************/
#define ASDEFENSE_GET_ALL             (0)                               /*  获取所有信息内容            */
#define ASDEFENSE_GET_SPEC            (1)                               /*  获取有问题信息内容          */
/*********************************************************************************************************
  ARP 防御获取信息结构
*********************************************************************************************************/
struct asdefense_get {
    UINT8                          ucType;                              /*  获取信息的类型              */
    INT32                          iStartIndex;                         /*  开始条目数                  */
    INT32                          iEndIndex;                           /*  结束条目数                  */
};
typedef struct asdefense_get      __ASDEFENSE_GET;
typedef struct asdefense_get     *__PASDEFENSE_GET;
/*********************************************************************************************************
  ARP 防御返回信息结构
*********************************************************************************************************/
struct asdefense_info {
    ip4_addr_t                      uiIpAddr;                           /*  IP 地址                     */
    UCHAR                           ucTrueMacAddr[ETHARP_HWADDR_LEN];   /*  正确 MAC 地址               */
    UCHAR                           ucConMacAddr[ETHARP_HWADDR_LEN];    /*  冲突 MAC 地址               */
    UINT8                           ucFlag;                             /*  当前 ARP 信息状态标志       */
};
typedef struct asdefense_info      __ASDEFENSE_INFO;
typedef struct asdefense_info     *__PASDEFENSE_INFO;
/*********************************************************************************************************
  获取 ARP 防御信息全局符号表
*********************************************************************************************************/
LW_SYMBOL_EXPORT INT32  asDefenseStatus (VOID);
LW_SYMBOL_EXPORT UINT32 asDefenseGetSpecInfoNum (UINT8  ucInfoType);
LW_SYMBOL_EXPORT INT32  asDefenseGetSpecInfo (INT32  iStartIndex,
                                              INT32  iEndIndex,
                                              UINT8  ucInfoType,
                                              PCHAR  pcBuff,
                                              INT32  iBuffLen);

#endif                                                                       /* __LIBASDEFENSE_CONFIG_H */
/*********************************************************************************************************
  END
*********************************************************************************************************/
