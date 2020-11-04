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
** 文   件   名: leasesfix.c
**
** 创   建   人: Administrator
**
** 文件创建日期: 2017年3月8日
**
** 描        述: 修改过的租约管理模块，解决重复分配和死锁问题
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <pthread.h>

#include "log.h"
#include "configini.h"
#include "packet.h"
#include "sockets.h"
#include "leasesfix.h"

#define HAVESRCFILE                     1
#define HAVEBAKFILE                     2

static LEASEMANGER  GleaseManger;                                       /* 租约管理全局结构体           */

/*********************************************************************************************************
** 函数名称: leaseLock
** 功能描述: 获得租约管理锁
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static VOID  leaseLock (VOID)
{
    pthread_mutex_lock(&(GleaseManger.leasemutexLock));
}

/*********************************************************************************************************
** 函数名称: leaseUnlock
** 功能描述: 释放租约管理锁
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static VOID  leaseUnlock (VOID)
{
    pthread_mutex_unlock(&(GleaseManger.leasemutexLock));
}

/*********************************************************************************************************
** 函数名称: leaseNotExpired
** 功能描述: 判断当前租约是否超期
** 输    入: INT  iIndex
** 输    出: 超期返回PX_ERROR，否则返回iIndex(iIndex在租约数组下标范围之内)
** 调用模块: API
*********************************************************************************************************/
static INT  leaseNotExpired (INT  iIndex)
{
    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 有效性检测                   */
        if (GleaseManger.pdhcplease[iIndex].uiExpires < time(0)) {      /* 租约时间小于当前时间即为超期 */
            iIndex = PX_ERROR;
        }
    } else {
        iIndex = PX_ERROR;
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: leaseOfferSet
** 功能描述: 将租约置为暂时提供状态
** 输    入: pcChaddr  硬件地址
**           iIndex    租约索引
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseOfferSet (PUCHAR  pcChaddr, INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 有效性检测                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 清空硬件地址                 */
                   0,
                   16);
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 配置硬件地址                 */
                   pcChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState = LEASEOFFER;        /* 配置临时供状态和等待时间     */
            GleaseManger.pdhcplease[iIndex].uiExpires = GserverConfig.ulOfferWaite + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseOccupySet
** 功能描述: 将租约置为已提供状态
** 输    入: pcChaddr  硬件地址
**           iIndex    租约索引
**           uiLease   租约时长
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseOccupySet (PUCHAR  pcChaddr, INT iIndex, UINT uiLease)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 有效性检测                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 清空硬件地址                 */
                   0,
                   16);
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 配置硬件地址                 */
                   pcChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = LEASEOCCUPY;    /* 配置已提供状态和租约时间     */
            GleaseManger.pdhcplease[iIndex].uiExpires = uiLease + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseConflictSet
** 功能描述: 将租约置为地址冲突状态
** 输    入: iIndex  租约索引
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseConflictSet (INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 判断有效性                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 清空硬件地址                 */
                   0,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = LEASECONFLICT;  /* 设置冲突状态配置冲突检查时间 */
            GleaseManger.pdhcplease[iIndex].uiExpires = GserverConfig.ulLease + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseTackSet
** 功能描述: 将租约置为暂时提取状态
** 输    入: pcChaddr   硬件地址
**           iIndex     租约索引
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseTackSet (PUCHAR  pcChaddr, INT  iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 判断有效性                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            if (GleaseManger.pdhcplease[iIndex].iState == LEASEFREE) {
                memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,        /* 赋值硬件地址                 */
                       pcChaddr,
                       16);
                GleaseManger.pdhcplease[iIndex].iState = LEASETACKED;   /* 设置为临时提取状态           */
                iRet = ERROR_NONE;
            } else if (GleaseManger.pdhcplease[iIndex].iState == LEASETACKED ||
                       GleaseManger.pdhcplease[iIndex].iState == LEASEOFFER  ||
                       GleaseManger.pdhcplease[iIndex].iState == LEASEOCCUPY) {
                iRet = ERROR_NONE;                                      /* 已经是提取状态的不作重复操作 */
            }
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseFreeSet
** 功能描述: 释放租约
** 输    入: iIndex  租约索引
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseFreeSet (INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* 判断有效性                  */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            GleaseManger.pdhcplease[iIndex].iState    = LEASEFREE;      /* 配置为空闲状态              */
            GleaseManger.pdhcplease[iIndex].uiExpires = time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: freeLeaseFind
** 功能描述: 查找空闲租约
** 输    入: NONE
** 输    出: 成功返回相关租约数组下标, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  freeLeaseFind (VOID)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    for (i = 0; i < MAXLEASES; i++) {                                   /* 遍历并查找第一个空闲地址     */
        if (leaseNotExpired(i) == PX_ERROR &&                           /* 判断超期,超期置为空闲        */
            GleaseManger.pdhcplease[i].uiYiaddr != 0 &&
            GleaseManger.pdhcplease[i].iState != LEASETACKED) {
            GleaseManger.pdhcplease[i].iState = LEASEFREE;
            iIndex = i;
        }

        if (GleaseManger.pdhcplease[i].iState   == LEASEFREE &&
            GleaseManger.pdhcplease[i].uiYiaddr != 0) {
            iIndex = i;
            break;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: leaseFindbyYiaddr
** 功能描述: 通过IP地址查找相关租约
** 输    入: uiYiaddr   ip地址
** 输    出: 成功返回相关租约数组下标, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  leaseFindbyYiaddr (UINT  uiYiaddr)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    /*
     * 遍历租约表查找ip地址匹配的租约，返回其Index
     */
    for (i = 0; i < MAXLEASES; i++) {
        if (GleaseManger.pdhcplease[i].uiYiaddr == uiYiaddr &&
            GleaseManger.pdhcplease[i].uiYiaddr != 0) {
            iIndex = i;
            break;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: leaseFindbyChaddr
** 功能描述: 通过硬件地址查找相关租约
** 输    入: pcChaddr  硬件地址
** 输    出: 成功返回相关租约数组下标, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  leaseFindbyChaddr (PUCHAR  pcChaddr)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    /*
     * 遍历租约表查找硬件地址匹配的租约，返回其Index
     */
    if (pcChaddr) {
        for (i = 0; i < MAXLEASES; i++) {
            if (memcmp(GleaseManger.pdhcplease[i].ucChaddr, pcChaddr, 16) == 0 &&
                GleaseManger.pdhcplease[i].uiYiaddr != 0) {
                iIndex = i;
                break;
            }
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: clinetInLease
** 功能描述: 判断是否是已租赁客户端
** 输    入: PDHCPMSG  pdhcpmsgPacket
** 输    出: 成功返回相关租约数组下标, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  clinetInLease (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* 查找硬件地址是否存在租约表中 */
    if (iIndex >= 0) {                                                  /* 如果存在                     */
        if (GleaseManger.pdhcplease[iIndex].iState == LEASECONFLICT) {  /* 判断是否是冲突地址           */
            iIndex = PX_ERROR;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: addisFree
** 功能描述: 判读地址是否是空闲的
** 输    入: uiYiaddr  对应的ip地址
** 输    出: 成功返回相关租约数组下标, 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  addisFree (INT  iIndex)
{
    leaseLock();

    if (iIndex >= 0) {                                                  /* 如果存在                     */
        if ((GleaseManger.pdhcplease[iIndex].iState == LEASEFREE ||
            leaseNotExpired(iIndex) == PX_ERROR) &&
            GleaseManger.pdhcplease[iIndex].uiYiaddr != 0 &&
            GleaseManger.pdhcplease[iIndex].iState != LEASETACKED) {
            GleaseManger.pdhcplease[iIndex].iState = LEASEFREE;
        } else {
            iIndex = PX_ERROR;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** 函数名称: leasaFileRead
** 功能描述: 从租约文件读取已保存租约
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leasaFileRead (FILE  *pLeasefile)
{
    INT                i = 0;
    INT                iIndex;
    struct  dhcplease  dhcpleaseread;

    memset(&dhcpleaseread, 0, sizeof(dhcpleaseread));
    fseek(pLeasefile, sizeof(INT64), SEEK_SET);
    while (i < MAXLEASES &&                                             /* 读取租约内容到结构体         */
           (fread(&dhcpleaseread, sizeof(dhcpleaseread), 1, pLeasefile) == 1)) {
        iIndex = leaseFindbyYiaddr(dhcpleaseread.uiYiaddr);             /* 查找读取IP地址是否在租约表中 */
        if (iIndex >= 0) {                                              /* 如果在租约表中               */
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* 将读取的数据赋值到对应租约中 */
                   dhcpleaseread.ucChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = dhcpleaseread.iState;
            GleaseManger.pdhcplease[iIndex].uiExpires = dhcpleaseread.uiExpires;
            memset(&dhcpleaseread, 0, sizeof(dhcpleaseread));
        } else {                                                        /* 如果读取的地址不在租约表中   */
            for (i = 0;i < MAXLEASES; i++) {                            /* 初始化租约表内容             */
                memset(GleaseManger.pdhcplease[i].ucChaddr, 0, 16);
                GleaseManger.pdhcplease[i].iState   = LEASEFREE;        /* 全部租约置为空闲状态         */
             }
            break;
        }
        i++;
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: leaseFileWrite
** 功能描述: 将当前租约写入租约文件
** 输    入: pLeasefile   租约文件描述符
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseFileWrite (FILE  *pLeasefile, struct dhcplease  *pdhcplease)
{
    INT     i;
    INT64   iFileMark = time(0);

    fwrite(&iFileMark, 8, 1, pLeasefile);                               /* 写入开头校验值               */

    for (i = 0; i < MAXLEASES; i++) {                                   /* 将租约数组写入租约文件       */
        if (pdhcplease[i].uiYiaddr != 0) {                              /* 判断租约有效性               */
            fwrite(pdhcplease[i].ucChaddr,                              /* 将租约硬件地址写入文件       */
                   16,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].uiYiaddr,                             /* 将租约ip地址写入文件         */
                   4,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].uiExpires,                            /* 将租约时间写入文件           */
                   4,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].iState,                               /* 将租约状态写入文件           */
                   4,
                   1,
                   pLeasefile);
        } else {
            break;
        }
    }

    fwrite(&iFileMark, 8, 1, pLeasefile);                               /* 写入末尾校验值               */

    sync();

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: fileCheck
** 功能描述: 检查文件有效性
** 输    入: pLeasefile   租约文件描述符
**           pLen         文件大小
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  fileCheck (FILE  *pLeasefile, LONG  *pLen)
{
    INT    iRet;
    INT64  iFileMarkStart = 0;
    INT64  iFileMarkEnd   = 0;

    fseek(pLeasefile, 0, SEEK_END);
    *pLen = ftell(pLeasefile);                                          /* 获取文件当前大小             */
    fseek(pLeasefile, 0, SEEK_SET);                                     /* 文件读取指针定位到文件头     */
    if (*pLen == 0) {
        return (ERROR_NONE);
    }

    iRet = fread(&iFileMarkStart,                                       /* 读取首校验值                 */
                 8,
                 1,
                 pLeasefile);
    if (iRet == 1) {
        fseek(pLeasefile, 0 - sizeof(iFileMarkEnd), SEEK_END);          /* 移到末尾校验值的首地址       */
        iRet = fread(&iFileMarkEnd,                                     /* 读取尾校验值                 */
                     sizeof(iFileMarkEnd),
                     1,
                     pLeasefile);
        if (iRet == 1) {                                                /* 确认读取正确                 */
            if (iFileMarkStart == iFileMarkEnd) {                       /* 校验文件完整性               */
                return (ERROR_NONE);
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: precentFile
** 功能描述: 打开最新的有效文件
** 输    入: pLen         文件大小
** 输    出: 成功返回对应文件描述符, 失败返回NULL
** 调用模块: NONE
*********************************************************************************************************/
static FILE  *precentFile (LONG  *pLen)
{
    FILE   *pLeasefilesrc     = NULL;
    FILE   *pLeasefilebak     = NULL;
    INT64   iFileMarkStartSrc = 0;
    INT64   iFileMarkStartBak = 0;

    pLeasefilesrc = fopen(GserverConfig.pcLeasefile, "r");              /* 打开租约源文件               */
    pLeasefilebak = fopen(GserverConfig.pcLeasefilebak, "r");           /* 打卡租约备份文件             */

    if (pLeasefilesrc && pLeasefilebak) {                               /* 如果两个文件都能打开         */
        if (fileCheck(pLeasefilesrc, pLen) == ERROR_NONE &&             /* 如果两个文件都是未损坏的     */
            fileCheck(pLeasefilebak, pLen) == ERROR_NONE) {
            fread(&iFileMarkStartSrc, sizeof(iFileMarkStartSrc), 1, pLeasefilesrc);
            fread(&iFileMarkStartBak, sizeof(iFileMarkStartBak), 1, pLeasefilebak);
            if (iFileMarkStartBak > iFileMarkStartSrc) {                /* 比较校验值，选择最新的文件   */
                fclose(pLeasefilesrc);                                  /* 关闭另一个文件描述符         */
                return (pLeasefilebak);                                 /* 返回最新的文件的描述符       */
            } else {
                fclose(pLeasefilebak);                                  /* 关闭另一个文件描述符         */
                return (pLeasefilesrc);                                 /* 返回最新的文件的描述符       */
            }
        } else if (fileCheck(pLeasefilesrc, pLen) == ERROR_NONE &&      /* 如果只有源文件完整           */
                   fileCheck(pLeasefilebak, pLen) != ERROR_NONE) {
            fclose(pLeasefilebak);                                      /* 关闭备份文件描述符           */
            return (pLeasefilesrc);                                     /* 返回源文件描述符             */
        } else if (fileCheck(pLeasefilesrc, pLen) != ERROR_NONE &&      /* 如果只有备份文件完整         */
                   fileCheck(pLeasefilebak, pLen) == ERROR_NONE) {
            fclose(pLeasefilesrc);                                      /* 关闭源文件描述符             */
            return (pLeasefilebak);                                     /* 返回备份文件描述符           */
        }
    } else if (!pLeasefilebak && pLeasefilesrc ) {                      /* 如果只有源文件能打开         */
        if (fileCheck(pLeasefilesrc, pLen)) {                           /* 检查源文件完整性             */
            return (pLeasefilesrc);                                     /* 如果完整返回其描述符         */
        }
    } else if (!pLeasefilesrc && pLeasefilebak) {                       /* 如果只有备份文件能打开       */
        if (fileCheck(pLeasefilebak, pLen)) {                           /* 检查备份文件完整性           */
            return (pLeasefilebak);                                     /* 如果完整返回其描述符         */
        }
    }

    return (NULL);
}
/*********************************************************************************************************
** 函数名称: leaseReadWithProtect
** 功能描述: 从租约文件读取已保存租约(带掉电保护机制)
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  leaseReadWithProtect (VOID)
{
    FILE   *pLeasefile = NULL;
    INT     iState     = 0;
    INT     iRet;
    LONG    lLen;

    iRet = access(GserverConfig.pcLeasefile, F_OK);                     /* 检查源文件是否存在           */
    if (iRet >=  0) {
        iState = iState + HAVESRCFILE;
    }

    iRet = access(GserverConfig.pcLeasefilebak, F_OK);                  /* 检查备份文件是否存在         */
    if (iRet >=  0) {
        iState = iState + HAVEBAKFILE;
    }

    switch (iState) {                                                   /* 判断文件状态                 */

    case 0:
        fopen(GserverConfig.pcLeasefile, "w");                          /* 租约文件不存在，则创建一个   */
        return (ERROR_NONE);
        break;

    case HAVESRCFILE:                                                   /* 只存在源文件                 */
        pLeasefile = fopen(GserverConfig.pcLeasefile, "r");             /* 打开租约源文件               */
        if (!pLeasefile) {                                              /* 如果打开失败则返回失败       */
            LOG(LOG_ERR, "Unable to open %s for reading\r\n", GserverConfig.pcLeasefile);
            return (PX_ERROR);
        }

        if (fileCheck(pLeasefile, &lLen) == ERROR_NONE) {               /* 检查源文件完整性             */
            if (lLen) {
                if (leasaFileRead(pLeasefile) == ERROR_NONE) {          /* 若源文件不为空，读取其内容   */
                    fclose(pLeasefile);
                    return (ERROR_NONE);
                }

            } else {
                fclose(pLeasefile);
                return (ERROR_NONE);                                    /* 为空则直接返回ERROR_NONE     */
            }

        } else {
            fclose(pLeasefile);
            return (PX_ERROR);
        }
        break;

    case HAVEBAKFILE:                                                   /* 如果只存在备份文件           */
        pLeasefile = fopen(GserverConfig.pcLeasefilebak, "r");          /* 打开租约备份文件             */
        if (!pLeasefile) {                                              /* 如果打开失败则返回失败       */
            LOG(LOG_ERR, "Unable to open %s for reading\r\n", GserverConfig.pcLeasefilebak);
            return (PX_ERROR);
        }

        if (fileCheck(pLeasefile, &lLen) == ERROR_NONE) {               /* 检查备份文件完整性           */
            if (lLen) {
                if (leasaFileRead(pLeasefile) == ERROR_NONE) {          /* 若文件不为空则读取其内容     */
                    fclose(pLeasefile);
                    return (ERROR_NONE);
                }

            } else {
                fclose(pLeasefile);
                return (ERROR_NONE);                                    /* 为空则直接返回ERROR_NONE     */
            }

        } else {
            fclose(pLeasefile);
            return (PX_ERROR);
        }
        break;

    case HAVESRCFILE + HAVEBAKFILE:                                     /* 如果两个文件都存在           */
        pLeasefile = precentFile(&lLen);                                /* 获取最新的文件描述符         */
        if (!pLeasefile) {                                              /* 获取失败返回PX_ERROR         */
            LOG(LOG_ERR, "Unable to open leasefile for reading\r\n");
            return (PX_ERROR);
        }

        if (lLen) {                                                     /* 判断文件是否为空             */
            if (leasaFileRead(pLeasefile) == ERROR_NONE) {              /* 不为空则读取其内容           */
                fclose(pLeasefile);
                return (ERROR_NONE);
            }

        } else {
            fclose(pLeasefile);
            return (ERROR_NONE);                                        /* 为空则直接返回               */
        }
        break;

    default:
        break;
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** 函数名称: leaseWriteWithProtect
** 功能描述: 将当前租约写入租约文件(带掉电保护机制)
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
INT  leaseWriteWithProtect (VOID)
{
    FILE   *pLeasefile    = NULL;
    static struct dhcplease  pdhcplease[MAXLEASES];

    /*
     *  申请临时空间，拷贝lease数据
     */
//    pdhcplease = (struct dhcplease *)malloc(sizeof(struct dhcplease) * MAXLEASES);
    memset(pdhcplease, 0 , sizeof(struct dhcplease)* MAXLEASES);
    leaseLock();
    memcpy(pdhcplease, GleaseManger.pdhcplease, sizeof(struct dhcplease)* MAXLEASES);
    leaseUnlock();

    pLeasefile = fopen(GserverConfig.pcLeasefile, "w");                 /* 清空覆盖，打开文件           */
    if (!pLeasefile) {
        LOG(LOG_ERR, "Unable to open %s\r\n", GserverConfig.pcLeasefile);
        return (PX_ERROR);
    }

    if (leaseFileWrite(pLeasefile, pdhcplease) == PX_ERROR) {           /* 租约写入源文件               */
        fclose(pLeasefile);
        return (PX_ERROR);
    }

    fclose(pLeasefile);

    pLeasefile = fopen(GserverConfig.pcLeasefilebak, "w");              /* 清空覆盖，打开文件           */
    if (!pLeasefile) {
        LOG(LOG_ERR, "Unable to open %s\r\n", GserverConfig.pcLeasefile);
        return (PX_ERROR);
    }

    if (leaseFileWrite(pLeasefile, pdhcplease) == PX_ERROR) {           /* 租约写入备份文件             */
        fclose(pLeasefile);
        return (PX_ERROR);
    }

    fclose(pLeasefile);                                                 /* 写完关闭文件描述符           */

    //free(pdhcplease);                                                   /* 释放临时空间                 */

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: leaseMangerInit
** 功能描述: 租约管理初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
INT  leaseMangerInit (VOID)
{
    UINT  uiAddr = ntohl(GserverConfig.uiStart);
    INT   i      = 0;
    INT   iRet;

    pthread_mutexattr_t  mutexattr;

    GleaseManger.pdhcplease = (struct dhcplease *)malloc(sizeof(struct dhcplease) * MAXLEASES);
    if (NULL == GleaseManger.pdhcplease) {
        LOG(LOG_ERR, "leaseMangerInit malloc fail\r\n");
        return (PX_ERROR);
    }

    memset(GleaseManger.pdhcplease, 0 , sizeof(struct dhcplease)* MAXLEASES);

    for (;uiAddr <= ntohl(GserverConfig.uiEnd); uiAddr++) {             /* 初始化租约表内容             */
        if (!(uiAddr & 0xFF) || (uiAddr & 0xFF) == 0xFF) {              /* 过滤0和255地址               */
            continue;
        }

        GleaseManger.pdhcplease[i].uiYiaddr = htonl(uiAddr);            /* 预填充地址到全局结构体中     */
        GleaseManger.pdhcplease[i].iState   = LEASEFREE;                /* 全部租约置为空闲状态         */
        i++;
     }

    pthread_mutexattr_init(&mutexattr);                                 /* 初始化可重入锁               */
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    iRet = pthread_mutex_init(&(GleaseManger.leasemutexLock), &mutexattr);
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Mutex init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = leaseReadWithProtect();                                      /* 读取租约文件内容到系统       */

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: freeOfferGet
** 功能描述: 获取一个空闲租约
** 输    入: pdhcpmsgPacket       dhcp数据包
**           pdhcpmsgSendPacket   要发送的数据包
** 输    出: 成功返回ERROR_NONE, 临时提取失败返回TAKEERROR，租约临时发放失败返回OFFERERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  freeOfferGet (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    INT                iIndex           = PX_ERROR;
    INT                iRet             = PX_ERROR;
    PUCHAR             pcRequestLeases  = NULL;
    PUCHAR             pcRequestIp      = NULL;
    struct optionset  *poptionset       = NULL;
    UINT               uiLeaseAgain     = (UINT)GserverConfig.ulLease;

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* 获取客户端申请的ip           */
    if (pcRequestIp) {                                                  /* 如果有申请ip                 */
        memcpy(&(pdhcpmsgSendPacket->uiYiaddr), pcRequestIp, 4);        /* 赋值                         */
    }

    leaseLock();                                                        /* 加锁                         */

    iIndex = clinetInLease(pdhcpmsgPacket);                             /* 判断是否曾经分配过           */
    if (iIndex < 0) {
        if (pcRequestIp) {
            iIndex = leaseFindbyYiaddr(pdhcpmsgSendPacket->uiYiaddr);   /* 判断是否有对应IP             */
            if (iIndex >= 0) {
                iIndex = addisFree(iIndex);                             /* 判读对已IP租约是否空闲       */
                if (iIndex < 0) {
                    iIndex = freeLeaseFind();                           /* 查找空闲租约                 */
                }
            } else {
                iIndex = freeLeaseFind();                               /* 查找空闲租约                 */
            }
        } else {
            iIndex = freeLeaseFind();
        }
    }

    if (iIndex < 0) {                                                   /* 如果没有地址可以分配         */
        iRet = PX_ERROR;
        LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned\r\n");
    } else {
        iRet = leaseTackSet(pdhcpmsgPacket->ucChaddr, iIndex);          /* 如果已找到租约，临时提取     */
    }

    leaseUnlock();

    if (iRet >= 0) {                                                    /* 检查ip是否被占用             */
        if (ipConflictCheck(GleaseManger.pdhcplease[iIndex].uiYiaddr) == ERROR_NONE) {
            leaseConflictSet(iIndex);                                   /* 若被占用则添加租约冲突       */
            iRet = IPCHECKFAILE;                                        /* 置返回值                     */
        } else {                                                        /* 将租约置为临时提取           */
            if (leaseOfferSet(pdhcpmsgSendPacket->ucChaddr, iIndex) == ERROR_NONE) {
                pdhcpmsgSendPacket->uiYiaddr = GleaseManger.pdhcplease[iIndex].uiYiaddr;

                pcRequestLeases = poptionGet(pdhcpmsgPacket, DHCP_LEASE_TIME);
                if (pcRequestLeases) {                                  /* 配置租约时间                 */
                    memcpy(&uiLeaseAgain, pcRequestLeases, 4);
                    uiLeaseAgain = (UINT)ntohl(uiLeaseAgain);
                    if (uiLeaseAgain > GserverConfig.ulLease) {
                        uiLeaseAgain = GserverConfig.ulLease;
                    }
                }
                optionsmpAdd(pdhcpmsgSendPacket->ucOptions,             /* 添加租约时间选项             */
                             DHCP_LEASE_TIME,
                             htonl(uiLeaseAgain));

                poptionset = GserverConfig.poptionsets;
                while (poptionset) {                                    /* 将添加全部配置选项到发送包   */
                    if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
                        optionstrAdd((PUCHAR)(pdhcpmsgSendPacket->ucOptions),
                                     (PUCHAR)(poptionset->pcData));
                    }
                    poptionset = poptionset->optionsetNext;
                }
                iRet = ERROR_NONE;                                      /* 临时租赁成功                 */
            } else {
                iRet = OFFERERROR;                                      /* 临时租赁失败                 */
            }

        }
    } else {
        iRet = TAKEERROR;                                               /* 临时提取租约失败             */
    }

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: discoverOfferGet
** 功能描述: discover流程获取租约Offer
** 输    入: pdhcpmsgPacket      dhcp数据包
**           pdhcpmsgSendPacket  要发送的数据包
**           iSocket             ICMP用的套接字
** 输    出: 返回租约操作状态
** 调用模块: API
*********************************************************************************************************/
INT  discoverOfferGet (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    INT  iRet;

    do {
        iRet = freeOfferGet(pdhcpmsgPacket, pdhcpmsgSendPacket);/* 获取可用租约                */
    }while(iRet == IPCHECKFAILE);

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseAck
** 功能描述: reques流程响应
** 输    入: pdhcpmsgPacket       dhcp数据包
**           pdhcpmsgSendPacket   要发送的数据包
** 输    出: 返回租约操作状态
** 调用模块: API
*********************************************************************************************************/
INT  leaseAck (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    PUCHAR             pcRequestIp      = NULL;
    PUCHAR             pcServerIp       = NULL;
    struct optionset  *poptionset       = NULL;
    INT                iIndex           = PX_ERROR;
    UINT               uiServerIpAgain  = 0;
    UINT               uiLeaseAgain     = GserverConfig.ulLease;
    PUCHAR             pcRequestLeases  = NULL;
    INT                iRet             = ERROR_NONE;

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* 获取客户端申请的ip           */
    pcServerIp       = poptionGet(pdhcpmsgPacket, DHCP_SERVER_ID);      /* 获取服务器ip                 */

    if (pcRequestIp) {                                                  /* 如果有申请ip                 */
        memcpy(&(pdhcpmsgSendPacket->uiYiaddr), pcRequestIp, 4);        /* 赋值                         */
    }

    if (pcServerIp) {                                                   /* 如果数据包里有服务器ip地址   */
        memcpy(&uiServerIpAgain, pcServerIp, 4);                        /* 赋值                         */
    }

    leaseLock();

    iIndex = clinetInLease(pdhcpmsgPacket);                             /* 判断是否曾经给该客户端分配过 */
    if (iIndex >= 0) {                                                  /* 如果分配过地址给该客户端     */
        pdhcpmsgSendPacket->uiYiaddr = GleaseManger.pdhcplease[iIndex].uiYiaddr;
        if (pcServerIp) {                                               /* 判断是否是向本服务器申请     */
            if (uiServerIpAgain != GserverConfig.uiServer) {            /* 如果不是向本服务器申请则跳出 */
                LOG(LOG_DEBUG, "Requeset server is not me!\r\n");
            } else {
                iRet  = ACKOK;                                          /* 否则需要发送ACK包            */
            }
        } else {
            iRet = ACKOK;                                               /* 如果没有服务器地址则直接ACK  */
        }
    } else {                                                            /* 如果没有给该客户端分配过     */
        if (pcRequestIp) {                                              /* 判断是否有申请IP             */
            iIndex = leaseFindbyYiaddr(pdhcpmsgSendPacket->uiYiaddr);   /* 查找该IP是否在租约表中       */
            if (iIndex >= 0) {                                          /* 若该IP在租约表中             */
                if (leaseNotExpired(iIndex) == PX_ERROR &&              /* 判断该IP是否超期             */
                    GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
                    memset(GleaseManger.pdhcplease[iIndex].ucChaddr,    /* 若超期则释放该租约           */
                           0,
                           16);
                    GleaseManger.pdhcplease[iIndex].iState = LEASEFREE;
                } else if (leaseNotExpired(iIndex) >= 0 &&              /* 若没超期需要发送NCK          */
                           GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
                    iRet = PX_ERROR;
                }
            } else {
                iRet = PX_ERROR;                                        /* 若该地址不在租约中发送NCK    */
            }
        }
    }

    if (iRet == ACKOK) {                                                /* 如果需要发送ACK              */
        pcRequestLeases = poptionGet(pdhcpmsgPacket, DHCP_LEASE_TIME);  /* 配置租约时间                 */
        if (pcRequestLeases) {
            memcpy(&uiLeaseAgain, pcRequestLeases, 4);
            if (uiLeaseAgain > GserverConfig.ulLease) {
                uiLeaseAgain = GserverConfig.ulLease;
            }
        }
                                                                        /* 将租约置为分配状态           */
        if (leaseOccupySet(pdhcpmsgSendPacket->ucChaddr, iIndex, uiLeaseAgain) == ERROR_NONE) {
            optionsmpAdd(pdhcpmsgSendPacket->ucOptions, DHCP_LEASE_TIME, htonl(uiLeaseAgain));
            poptionset = GserverConfig.poptionsets;                     /* 处理选项字段                 */
            while (poptionset) {
                if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
                    optionstrAdd((PUCHAR)(pdhcpmsgSendPacket->ucOptions), (PUCHAR)(poptionset->pcData));
                }
                poptionset = poptionset->optionsetNext;
            }
        } else {
            iRet = PX_ERROR;
        }

    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseDecline
** 功能描述: decline流程响应
** 输    入: pdhcpmsgPacket  dhcp数据包
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
INT  leaseDecline (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;
    INT  iRet = PX_ERROR;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* 通过MAC找到对应租约          */
    if (iIndex >= 0) {                                                  /* 如果租约存在将租约设为冲突   */
        iRet = leaseConflictSet(iIndex);
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** 函数名称: leaseRelease
** 功能描述: Release流程响应
** 输    入: pdhcpmsgPacket  dhcp数据包
** 输    出: 成功返回ERROR_NONE, 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
INT  leaseRelease (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;
    INT  iRet = PX_ERROR;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* 通过MAC找到对应租约          */
    if (iIndex >= 0) {                                                  /* 如果租约存在则释放租约       */
        iRet = leaseFreeSet(iIndex);
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
  END
*********************************************************************************************************/

