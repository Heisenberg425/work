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
** ��   ��   ��: powerEntry.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: ϵͳ��Դ״̬�������
*********************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "common.h"
#include "msgcomm.h"
#include "power_common.h"
/*********************************************************************************************************
** ��������: powerOffDeal
** ��������: ϵͳ��Դ�رմ�����
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerOffDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerDoOff(pPowerInfo);

    /*
     *  ���ڹػ������Գ������в����ô�
     */
    replyConfigMsg(OPENRT_POWER_OFF_ACK_CMD, iRet);
}
/*********************************************************************************************************
** ��������: powerRestartDeal
** ��������: ϵͳ����������
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerRestartDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerDoRestart(pPowerInfo);

    /*
     *  �������������Գ������в����ô�
     */
    replyConfigMsg(OPENRT_POWER_RESTART_ACK_CMD, iRet);
}
/*********************************************************************************************************
** ��������: powerEventConfigDeal
** ��������: ��Դ�¼����ô�����
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerEventConfigDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT  iRet;

    iRet = powerEventDoConfig(pPowerInfo);

    replyConfigMsg(OPENRT_POWER_EVT_CFG_ACK_CMD, iRet);
}
/*********************************************************************************************************
** ��������: powerEventShowDeal
** ��������: ��Դ�¼���ʾ������
** �䡡��  : pPowerInfo  ϵͳ��Դ������Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerEventShowDeal (__PPOWER_HANDLE  pPowerInfo)
{
    INT              iRecordNum;
    __PPOWER_HANDLE  pPowerArrayHeader = LW_NULL;

    pPowerArrayHeader = powerEventGetFromDB(&iRecordNum);

    replyQueryMsg(OPENRT_POWER_EVT_SHOW_ACK_CMD, sizeof(__POWER_HANDLE), iRecordNum , pPowerArrayHeader);

    free(pPowerArrayHeader);
}
/*********************************************************************************************************
** ��������: powerHandleEntry
** ��������: ϵͳ��Դ�������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  powerHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_POWER_OFF_CMD:
        powerOffDeal(pArgBuf);
        break;

    case OPENRT_POWER_RESTART_CMD:
        powerRestartDeal(pArgBuf);
        break;

    case OPENRT_POWER_EVT_CFG_CMD:
        powerEventConfigDeal(pArgBuf);
        break;

    case OPENRT_POWER_EVT_SHOW_CMD:
        powerEventShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
