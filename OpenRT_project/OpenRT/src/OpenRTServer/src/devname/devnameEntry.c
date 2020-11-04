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
** ��   ��   ��: devnameEntry.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 7 �� 23 ��
**
** ��        ��: �豸�����������
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
#include "devname_common.h"
/*********************************************************************************************************
** ��������: devNameShowDeal
** ��������: �豸������ʾ������
** �䡡��  : pdevNameInfo  �豸���Ʋ�����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  devNameShowDeal (__PDEVNAME_HANDLE  pdevNameInfo)
{
    INT                iNum;
    __PDEVNAME_HANDLE  pdevNameHeader = NULL;
    __DEVNAME_HANDLE   devNameInfo;

    pdevNameHeader = devNameGetFromDB(&iNum);
    if (NULL != pdevNameHeader) {
        if ('\0' == pdevNameHeader->cdevName[0]) {
            lib_memcpy(pdevNameHeader->cdevName, DEFAULT_DEV_NAME, strlen(DEFAULT_DEV_NAME) + 1);
        }

        replyQueryMsg(OPENRT_GET_DEV_NAME_ACK_CMD, sizeof(__DEVNAME_HANDLE), 1, pdevNameHeader);

        free(pdevNameHeader);
    } else {
        bzero(&devNameInfo, sizeof(__DEVNAME_HANDLE));
        lib_memcpy(devNameInfo.cdevName, DEFAULT_DEV_NAME, strlen(DEFAULT_DEV_NAME) + 1);

        replyQueryMsg(OPENRT_GET_DEV_NAME_ACK_CMD, sizeof(__DEVNAME_HANDLE), 1, &devNameInfo);
    }
}
/*********************************************************************************************************
** ��������: devNameConfigDeal
** ��������: �豸�������ô�����
** �䡡��  : pdevNameInfo  �豸���Ʋ�����Ϣ
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  devNameConfigDeal (__PDEVNAME_HANDLE  pdevNameInfo)
{
    INT  iRet;

    iRet = devNameUpdateToDb(pdevNameInfo);

    replyConfigMsg(OPENRT_SET_DEV_NAME_ACK_CMD, iRet);
}
/*********************************************************************************************************
** ��������: devNameHandleEntry
** ��������: �豸�����������
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID   devNameHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    switch (iCommand) {
    case OPENRT_SET_DEV_NAME_CMD:
        devNameConfigDeal(pArgBuf);
        break;

    case OPENRT_GET_DEV_NAME_CMD:
        devNameShowDeal(pArgBuf);
        break;

    default:
        break;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
