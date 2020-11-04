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
** 文   件   名: asd_cfg.h
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2019 年 1 月 2 日
**
** 描        述: 读取配置文件
*********************************************************************************************************/
#ifndef __ASD_CFG_H
#define __ASD_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
  INI 配置文件字段字符串
*********************************************************************************************************/
#define CFG_SECTOR           "config"                                   /*  配置 sector 字段            */
#define MAC_CHANGE_VAL       "mac_change_val"                           /*  MAC 发生变化判断标准字段    */
#define ARP_SPOOF_VAL        "arp_spoof_val"                            /*  ARP 欺骗判断标准字段        */
#define CON_TO_SPO_TIME      "conflict_to_spoof_time"                   /*  冲突转欺骗间隔时间字段      */
#define SPOOF_FORBID_TIME    "spoof_forbid_time"                        /*  拉黑时间字段                */
#define NODE_DEL_TIME        "node_del_time"                            /*  结点删除时间字段            */
#define NODE_CHECK_HZ        "node_check_hz"                            /*  结点检测频率字段            */
#define LOG_SECTOR           "log"                                      /*  日志 sector 字段            */
#define LOG_FILE_INI         "logfile"                                  /*  日志名称字段                */
#define LOG_FILE_BK_INI      "logfilebk"                                /*  备份日志名称字段            */
#define LOG_FILE_LEN_INI     "logfile_len"                              /*  日志文件大小字段            */
/*********************************************************************************************************
  可配置内容
*********************************************************************************************************/
#define LOG_FILE_NAME_LEN    (512)                                      /*  日志文件名称的长度          */
struct asd_config {
    INT32    iMacChangeVal;                                             /*  MAC 发生变化判断标准        */
    INT32    iArpSpoofVal;                                              /*  ARP 欺骗判断标准            */
    INT32    iConToSpoTime;                                             /*  冲突转欺骗间隔时间          */
    INT32    iForbidTime;                                               /*  拉黑时间                    */
    INT32    iNodeDelTime;                                              /*  结点删除时间                */
    INT32    iNodeCheckHz;                                              /*  结点检测频率                */

    PCHAR    pcLogFile;                                                 /*  日志名称                    */
    PCHAR    pcLogFileBk;                                               /*  备份日志名称                */
    INT32    iLogFilelen;                                               /*  日志文件大小                */
};
typedef struct asd_config    __ASD_CONFIG;
typedef struct asd_config   *__PASD_CONFIG;
/*********************************************************************************************************
  加载配置文件函数
*********************************************************************************************************/
VOID    *asdCfgLoad (const CHAR *file, const CHAR *sector);
VOID     asdCfgUnload (VOID *loadret);
INT32    asdCfgGetInt (VOID *loadret, const CHAR  *keyword, int def);
CHAR    *asdCfgGetString (VOID  *loadret, const CHAR  *keyword, CHAR  *def);


#ifdef __cplusplus
}
#endif

#endif                                                                  /* __ASD_CFG_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
