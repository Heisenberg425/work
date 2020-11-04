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
** ��   ��   ��: vndHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: ������������
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include "libcomm.h"
#include "vnd_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libVndCreateHandle
** ��������: ����һ����������
** �䡡��  : pVndHandle         ����������Ϣ
** �䡡��  : NONE
** ������  : iVndID             �������������� ID
*********************************************************************************************************/
INT  libVndCreate (__PVND_HANDLE  pVndHandle)
{
    if (LW_NULL == pVndHandle || LW_NULL == pVndHandle->ucHwAddr) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADD_VND_CMD, pVndHandle, sizeof(__VND_HANDLE)));
}

/*********************************************************************************************************
** ��������: libVndGetInfoHandle
** ��������: ��ȡ VND ��Ϣ
** �䡡��  : NONE
** �䡡��  : piNum              ��Ŀ����
** ������  : ERROR_CODE
*********************************************************************************************************/
PVOID  libVndGetInfo (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_GET_VND_CMD, NULL, 0, piNum));
}

/*********************************************************************************************************
** ��������: libKidVPNDel
** ��������: ɾ�� VPN
** �䡡��  : pKidVPNHandle      VPN ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libVndDel (INT  iVndID)
{
    return  (requestConfigMsg(OPENRT_DEL_VND_CMD, &iVndID, sizeof(iVndID)));
}
/*********************************************************************************************************
** ��������: libGetVndidByNetIfIndex
** ��������: ���� vndid ��ȡ��������
** �䡡��  : iVndid             vndid
** �䡡��  : piNetIfIndex       ��������
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libGetNetIfIndexByVndid (INT  iVndid, INT  *piNetIfIndex)
{
    INT       iNum                = 0;
    PCHAR     pcNetifIndexBuff = LW_NULL;

    if (LW_NULL == piNetIfIndex) {
        return  (PX_ERROR);
    }

    pcNetifIndexBuff = requestQueryMsg(OPENRT_GET_IFINDEX_CMD,
                                      (PVOID)&iVndid,
                                       sizeof(iVndid),
                                       &iNum);
    if (LW_NULL == pcNetifIndexBuff) {
        return  (PX_ERROR);
    }

    *piNetIfIndex = *(INT *)pcNetifIndexBuff;

    free(pcNetifIndexBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: libGetVndidByNetIfIndex
** ��������: ��������������ȡ vndid
** �䡡��  : iNetIfIndex       ��������
** �䡡��  : piVndid           vndid
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libGetVndidByNetIfIndex (INT  iNetIfIndex, INT  *piVndid)
{
    INT       iNum                = 0;
    PCHAR     pcVndIdBuff = LW_NULL;

    if (LW_NULL == piVndid) {
        return  (PX_ERROR);
    }

    pcVndIdBuff = requestQueryMsg(OPENRT_GET_VNDID_CMD,
                                 (PVOID)&iNetIfIndex,
                                 sizeof(iNetIfIndex),
                                 &iNum);
    if (LW_NULL == pcVndIdBuff) {
        return  (PX_ERROR);
    }

    *piVndid = *(INT *)pcVndIdBuff;

    free(pcVndIdBuff);

    return  (ERROR_NONE);
}
