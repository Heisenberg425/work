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
** ��   ��   ��: asdefenseRecovery.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 9 �� 12 ��
**
** ��        ��: ARP �����ָ�
*********************************************************************************************************/
#include <SylixOS.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "common.h"
extern INT32  asDefenseGetFromDB (BOOL  *pbStatus);
/*********************************************************************************************************
** ��������: asDefenseRecovry
** ��������: �ָ� ARP ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID asDefenseRecovry (VOID)
{
    BOOL    bStatus;
    INT     iRet;

    iRet = asDefenseGetFromDB(&bStatus);
    if (ERROR_NONE == iRet && LW_TRUE == bStatus) {
        system("asden 1");
    }
}
