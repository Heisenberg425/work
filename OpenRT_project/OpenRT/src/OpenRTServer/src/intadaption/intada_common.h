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
** 文   件   名: intada_common.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 17 日
**
** 描        述: 网卡顺序自适应公用头文件
*********************************************************************************************************/
#ifndef __INTADA_COMMON_H
#define __INTADA_COMMON_H

/*********************************************************************************************************
  网口自定义配置信息
*********************************************************************************************************/
struct intadaption_handle_info {
    INT                iWebIndex;                                       /*  Web 对应的索引              */
    CHAR               cWebName[NETIF_NAMESIZE];                        /*  Web 设置的网口名称          */
    CHAR               cNetIfName[NETIF_NAMESIZE];                      /*  系统网口名称                */
    CHAR               cStatus[12];                                     /*  状态标志                    */
};
typedef struct intadaption_handle_info   __INTADAPTION_HANDLE_INFO;
typedef struct intadaption_handle_info  *__PINTADAPTION_HANDLE_INFO;
/*********************************************************************************************************
  状态标志
*********************************************************************************************************/
#define NET_ADAPTING               "adapting"                           /*  正在进行自适应              */
#define NET_FINISH                 "finished"                           /*  完成自适应                  */
#define NET_UNUSED                 "unused"                             /*  未使用                      */
/*********************************************************************************************************
  EXPORT 操作函数声明
*********************************************************************************************************/
VOID intAdaHandleEntry (INT iCommand, PVOID  pArgBuf);
INT  intAdaptionGetWebName (PCHAR  pcNetifName, PCHAR  pcWebName, INT  *piWebIndex);

/*********************************************************************************************************
  EXPORT 数据库函数
*********************************************************************************************************/
INT  intAdaptionTableCreate (VOID);
/*********************************************************************************************************
  数据库函数声明
*********************************************************************************************************/
VOID intAdaptionDBClear (VOID);
INT  intAdaptionDBFindFinish (VOID);
INT  intAdaptionDBGetInfo (__PINTADAPTION_HANDLE_INFO   pNetInfo, CHAR  cFlag);
INT  intadaStatusGetFromDbByName (CPCHAR  cpcNetIfName, INT  *piStatus);
INT  intAdaptionDBSetFinish (VOID);
INT  intAdaptionDBUpdate (__PINTADAPTION_HANDLE_INFO  pIntAdaptionInfo);
INT  intAdaptionDBInit (VOID);

#endif                                                                  /*  __INTADA_COMMON_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
