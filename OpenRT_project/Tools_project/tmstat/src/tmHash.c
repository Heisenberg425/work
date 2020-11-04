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
** ��        ��: OpenRT ������ع��� hash �����
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include "tmHash.h"
#include "common.h"

/*********************************************************************************************************
  hash �����ȫ�ֱ���
*********************************************************************************************************/
PTM_STAT_ENTRY  __G_tmStatCbtable[TM_STAT_CB_SIZE];                     /*  �ն�����ͳ������ƿ� hash ��*/
PTM_STAT_ENTRY  __G_tmLineStatCbtable[TM_STAT_CB_SIZE];                 /*  ��·����ͳ������ƿ� hash ��*/
/*********************************************************************************************************
** ��������: tmStatCbTableInit
** ��������: ����ͳ������ƿ�hash���ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmStatCbTableInit (VOID)
{
    bzero(__G_tmStatCbtable, sizeof(__G_tmStatCbtable));
}

/*********************************************************************************************************
** ��������: tmStatCbFindByIp
** ��������: ���� IP ��ѯ����ͳ������ƿ����
** �䡡��  : pip4Addr     hash ���� key
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByIp (ip4_addr_t*  pip4Addr)
{
    return  __G_tmStatCbtable[TMCBHASH(pip4Addr)];
}

/*********************************************************************************************************
** ��������: tmStatCbFindByMac
** ��������: ���� MAC ��ѯ����ͳ������ƿ����
** �䡡��  : pucMac     hash ���� key
** �䡡��  : NONE
** ������  : ����ͳ�ƿ��ƿ�ָ��
*********************************************************************************************************/
PTM_STAT_ENTRY  tmStatCbFindByMac (UINT8  *pucMac)
{
    return  __G_tmLineStatCbtable[TMCB_MAC_HASH(*(pucMac + 5))];
}


/*********************************************************************************************************
** ��������: tmStatCbAdd
** ��������: ��������ͳ������ƿ�
** �䡡��  : pip4Addr  IP ��ַ��Ϊ hash key ֵ
**           ptmNode   �����ӵĿ��ƿ�ڵ�
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmStatCbAdd (ip4_addr_t  *pip4Addr, PTM_STAT_ENTRY  ptmNode)
{
    PLW_LIST_LINE   pline;
    PTM_STAT_ENTRY  pTmStatEntry = tmStatCbFindByIp(pip4Addr);

    if (NULL == pTmStatEntry){
        __G_tmStatCbtable[TMCBHASH(pip4Addr)] = ptmNode;
    } else {
        for (pline = (PLW_LIST_LINE)pTmStatEntry;
             pline != NULL;
             pline = _list_line_get_next(pline)) {
            ;
        }

        _List_Line_Add_Tail(&ptmNode->tmlist, (LW_LIST_LINE_HEADER  *)&pTmStatEntry);
    }
}

/*********************************************************************************************************
** ��������: tmStatCbAddByMac
** ��������: ��������ͳ������ƿ�
** �䡡��  : pucMac      MAC ��ַ��Ϊ hash key ֵ
**           ptmNode     �����ӵĿ��ƿ�ڵ�
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmStatCbAddByMac (UINT8  *pucMac, PTM_STAT_ENTRY  ptmNode)
{
    PLW_LIST_LINE   pline;
    PTM_STAT_ENTRY  pTmStatEntry = tmStatCbFindByMac(pucMac);

    if (NULL == pTmStatEntry){
        __G_tmLineStatCbtable[TMCB_MAC_HASH(*(pucMac + 5))] = ptmNode;
    } else {
        for (pline = (PLW_LIST_LINE)pTmStatEntry;
             pline != NULL;
             pline = _list_line_get_next(pline)) {
           ;
        }

        _List_Line_Add_Tail(&ptmNode->tmlist, (LW_LIST_LINE_HEADER  *)&pTmStatEntry);
    }
}
