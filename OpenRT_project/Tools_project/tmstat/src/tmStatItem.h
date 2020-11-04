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
** 文   件   名: tmStatItem.h
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 11 日
**
** 描        述: OpenRT 流量监控管理统计项头文件
*********************************************************************************************************/

#ifndef __TMSTATITEM_H
#define __TMSTATITEM_H

/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
INT   tmStatTermUserAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatAppAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, INT  iProtocol, UINT16  usPort);
INT   tmStatLineAdd (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatItemAdd (INT  iArgC, PCHAR  *ppcArgV);
INT   tmStatItemDelete (INT  iArgC, PCHAR  *ppcArgV);
INT   tmStatAllLinesInstall (PCHAR  pcIfName);
VOID  tmStatLineAllAppsInstall (ip4_addr_t  *pip4Addr, UINT8  *pucMac, PCHAR  pcIfName);
INT   tmStatTermUserAutoInstall (VOID);
INT   tmStatItemAllTermRemove (VOID);
INT   tmStatAllLinesIpUpdate (PCHAR  pcIfName, ip4_addr_t  *pip4Addr);

#endif                                                                  /* __TMSTATITEM_H               */
