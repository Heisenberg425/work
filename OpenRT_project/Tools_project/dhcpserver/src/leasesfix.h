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
** 文   件   名: leasesfix.h
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017年3月8日
**
** 描        述: dhcp服务租约管理修改版头文件
*********************************************************************************************************/
#ifndef LEASESFIX_H_
#define LEASESFIX_H_
#include "packet.h"
#include "pthread.h"

#define MAXLEASES           254                                         /* 最大租约数                   */

/*********************************************************************************************************
  状态标志宏定义
*********************************************************************************************************/
#define LEASEFREE           0                                           /* 租约空闲状态                 */
#define LEASETACKED         1                                           /* 租约暂时提取状态             */
#define LEASEOFFER          2                                           /* 租约暂时提供状态             */
#define LEASEOCCUPY         3                                           /* 租赁成功状态                 */
#define LEASECONFLICT       4                                           /* 租约地址冲突                 */
#define ACKOK               1                                           /* 需要ACK应答                  */
#define TAKEERROR          -2                                           /* 租约暂时提取失败             */
#define OFFERERROR         -3                                           /* 租约OFFER提供失败            */
#define IPCHECKFAILE       -4                                           /* 地址检测已占用               */

/*********************************************************************************************************
  租约结构体
*********************************************************************************************************/
struct dhcplease {
    UINT8    ucChaddr[16];                                              /* 客户端物理地址               */
    UINT     uiYiaddr;                                                  /* 客户端IP地址                 */
    UINT     uiExpires;                                                 /* 客户端租约时间               */
    INT      iState;                                                    /* 租约状态                     */
};

struct leaseManger {
    struct dhcplease   *pdhcplease;                                     /* 租约结构体指针               */
    pthread_mutex_t     leasemutexLock;                                 /* 租约管理锁                   */
};

typedef struct leaseManger  LEASEMANGER;
/*********************************************************************************************************
  功能函数
*********************************************************************************************************/
INT   leaseMangerInit (VOID);
INT   leaseWriteWithProtect (VOID);
INT   leaseDecline(PDHCPMSG  pdhcpmsgPacket);
INT   leaseRelease(PDHCPMSG  pdhcpmsgPacket);
INT   discoverOfferGet(PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket);
INT   leaseAck (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket);
#endif /* LEASES_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
