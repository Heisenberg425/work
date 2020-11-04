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
** ��   ��   ��: udialHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: ���Ŵ���
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "libudial_config.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libUdialStartHandle
** ��������: ��������
** �䡡��  : pcAccount          �����˺�
**           pcPassWd           ��������
**           pcNetifName        ����ϵͳ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  libUdialStartHandle (CHAR   *pcAccount, CHAR   *pcPassWd, CHAR  *pcNetifName)
{
    __UDIAL_HANDLE        udialHandle;

    if (LW_NULL == pcNetifName || LW_NULL == pcAccount || LW_NULL == pcPassWd) {

        return  (PX_ERROR);
    }

    /*
     *  ����Ϣ����ȥ
     */
    bzero(&udialHandle, sizeof(udialHandle));
    strncpy(udialHandle.cNetifname, pcNetifName, sizeof(udialHandle.cNetifname));
    strncpy(udialHandle.cUsrname,   pcAccount, sizeof(udialHandle.cUsrname));
    strncpy(udialHandle.cPassword,  pcPassWd, sizeof(udialHandle.cPassword));

    return  (requestConfigMsg(OPENRT_UDIAL_START, (PVOID)&udialHandle, sizeof(udialHandle)));
}

/*********************************************************************************************************
** ��������: libUdialDelHandle
** ��������: ɾ������
** �䡡��  : pcNetifName        ����ϵͳ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  libUdialDelHandle (PCHAR  pcNetifName)
{
    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_UDIAL_DEL, (PVOID)pcNetifName, NETIF_NAMESIZE));
}

/*********************************************************************************************************
** ��������: libUdialInfoGet
** ��������: ��ȡ����������Ϣ
** �䡡��  : pUdialInfo            �洢������Ϣ�Ľṹ(��Ҫָ��netifname)
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  libUdialInfoGet (__PUDIAL_HANDLE  pUdialInfo)
{
    INT              iNum = 0;
    __PUDIAL_HANDLE  pUdialHandle = LW_NULL;

    if (LW_NULL == pUdialInfo || LW_NULL == pUdialInfo->cNetifname) {

        return  (PX_ERROR);
    }

    pUdialHandle = requestQueryMsg(OPENRT_UDIAL_INFO,
                                   pUdialInfo->cNetifname,
                                   sizeof(pUdialInfo->cNetifname),
                                   &iNum);
    if (LW_NULL == pUdialHandle) {

        return  (PX_ERROR);
    }

    memcpy(pUdialInfo, pUdialHandle, sizeof(__UDIAL_HANDLE));

    free(pUdialHandle);

    return  (ERROR_NONE);
}
