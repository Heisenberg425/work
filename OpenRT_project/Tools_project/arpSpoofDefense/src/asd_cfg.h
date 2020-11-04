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
** ��   ��   ��: asd_cfg.h
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2019 �� 1 �� 2 ��
**
** ��        ��: ��ȡ�����ļ�
*********************************************************************************************************/
#ifndef __ASD_CFG_H
#define __ASD_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
  INI �����ļ��ֶ��ַ���
*********************************************************************************************************/
#define CFG_SECTOR           "config"                                   /*  ���� sector �ֶ�            */
#define MAC_CHANGE_VAL       "mac_change_val"                           /*  MAC �����仯�жϱ�׼�ֶ�    */
#define ARP_SPOOF_VAL        "arp_spoof_val"                            /*  ARP ��ƭ�жϱ�׼�ֶ�        */
#define CON_TO_SPO_TIME      "conflict_to_spoof_time"                   /*  ��ͻת��ƭ���ʱ���ֶ�      */
#define SPOOF_FORBID_TIME    "spoof_forbid_time"                        /*  ����ʱ���ֶ�                */
#define NODE_DEL_TIME        "node_del_time"                            /*  ���ɾ��ʱ���ֶ�            */
#define NODE_CHECK_HZ        "node_check_hz"                            /*  �����Ƶ���ֶ�            */
#define LOG_SECTOR           "log"                                      /*  ��־ sector �ֶ�            */
#define LOG_FILE_INI         "logfile"                                  /*  ��־�����ֶ�                */
#define LOG_FILE_BK_INI      "logfilebk"                                /*  ������־�����ֶ�            */
#define LOG_FILE_LEN_INI     "logfile_len"                              /*  ��־�ļ���С�ֶ�            */
/*********************************************************************************************************
  ����������
*********************************************************************************************************/
#define LOG_FILE_NAME_LEN    (512)                                      /*  ��־�ļ����Ƶĳ���          */
struct asd_config {
    INT32    iMacChangeVal;                                             /*  MAC �����仯�жϱ�׼        */
    INT32    iArpSpoofVal;                                              /*  ARP ��ƭ�жϱ�׼            */
    INT32    iConToSpoTime;                                             /*  ��ͻת��ƭ���ʱ��          */
    INT32    iForbidTime;                                               /*  ����ʱ��                    */
    INT32    iNodeDelTime;                                              /*  ���ɾ��ʱ��                */
    INT32    iNodeCheckHz;                                              /*  �����Ƶ��                */

    PCHAR    pcLogFile;                                                 /*  ��־����                    */
    PCHAR    pcLogFileBk;                                               /*  ������־����                */
    INT32    iLogFilelen;                                               /*  ��־�ļ���С                */
};
typedef struct asd_config    __ASD_CONFIG;
typedef struct asd_config   *__PASD_CONFIG;
/*********************************************************************************************************
  ���������ļ�����
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
