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
** ��   ��   ��: tmHash.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT ������ع��� hash ��ͷ�ļ�
*********************************************************************************************************/
#ifndef __TMHASH_H
#define __TMHASH_H

#include "common.h"

/*********************************************************************************************************
  hash ��غ궨��
*********************************************************************************************************/
#define TM_STAT_CB_SIZE           (256)                                 /*  hash Ͱ���                 */

#if (TM_STAT_CB_SIZE & (TM_STAT_CB_SIZE-1)) == 0
#define TMCBHASHMOD(h)        ((h) & (TM_STAT_CB_SIZE - 1))             /*  ��ģ                        */
#else
#define TMCBHASHMOD(h)        ((h) % TM_STAT_CB_SIZE)
#endif

#define TMCBHASH(ip)          TMCBHASHMOD(((ntohl(ip->addr)&0xFF) + \
                                           (ntohl(ip->addr)&0xFF)))     /*  ���� IP ���� hash ֵ        */
#define TMCB_MAC_HASH(mac)    TMCBHASHMOD((mac) & 0xFF)                 /*  ���� MAC ���� hash ֵ       */
/*********************************************************************************************************
  hash ȫ�ֱ�������
*********************************************************************************************************/
extern PTM_STAT_ENTRY __G_tmStatCbtable[TM_STAT_CB_SIZE];               /*  �ն�����ͳ������ƿ� hash ��*/
extern PTM_STAT_ENTRY __G_tmLineStatCbtable[TM_STAT_CB_SIZE];           /*  ��·����ͳ������ƿ� hash ��*/
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByIp (ip4_addr_t  *pip4Addr);
PTM_STAT_ENTRY  tmStatCbFindByMac (UINT8  *pucMac);
VOID            tmStatCbAdd (ip4_addr_t  *pip4Addr,  PTM_STAT_ENTRY  ptmNode);
VOID            tmStatCbAddByMac (UINT8  *pucMac, PTM_STAT_ENTRY  ptmNode);
VOID            tmStatCbTableInit (VOID);

#endif                                                                  /* __TMHASH_H                   */
