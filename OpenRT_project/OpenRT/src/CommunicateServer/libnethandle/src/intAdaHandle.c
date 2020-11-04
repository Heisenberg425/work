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
** ��   ��   ��: intAdaHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: ��������Ӧ��λ����
*********************************************************************************************************/
#include "stdlib.h"
#include "intada_info.h"
#include "libcomm.h"
#include "string.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libIntAdaCheck
** ��������: ����Ƿ���ɶ�λ����
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ״̬
*********************************************************************************************************/
INT  libIntAdaCheck (VOID)
{
    return  (requestConfigMsg(OPENRT_ADA_GETS_CMD, LW_NULL, 0));
}
/*********************************************************************************************************
** ��������: libIntAdaGetNetif
** ��������: ��ȡ Web ��������Ӧ��ϵͳ��������
** �䡡��  : cpcWebNetifName    ǰ����������
**           iNetifLen          �������� buff ����
** �䡡��  : pcNetifName        ϵͳ��������
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaGetNetif (CPCHAR  cpcWebNetifName, PCHAR  pcNetifName, INT  iNetifLen)
{
    INT       iNum            = 0;
    PCHAR     pcNetifNameBuff = LW_NULL;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(cpcWebNetifName, "lo0")) {
        memcpy(pcNetifName, "lo0", strlen("lo0"));

        return  (ERROR_NONE);
    }

    pcNetifNameBuff = requestQueryMsg(OPENRT_NETIF_GET_CMD,
                                     (PVOID)cpcWebNetifName,
                                     strlen(cpcWebNetifName)+1,
                                     &iNum);
    if (LW_NULL == pcNetifNameBuff || 0 == iNum) {
        return  (PX_ERROR);
    }

    strncpy(pcNetifName, pcNetifNameBuff, iNetifLen);
    free(pcNetifNameBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: libIntAdaGetWebNetif
** ��������: ��ȡ netif ���ƻ�ȡ��Ӧ��ǰ����������
** �䡡��  : pcWebNetifName    ǰ��������
**           iNetifLen         �������� buff ����
** �䡡��  : cpcNetifName      ��������
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaGetWebNetif (CPCHAR  cpcNetifName, PCHAR  pcWebNetifName, INT  iNetifLen)
{
    INT       iNum            = 0;
    PCHAR     pcWebNetifNameBuff = LW_NULL;

    if (LW_NULL == pcWebNetifName) {
        return  (PX_ERROR);
    }

    if (0 == strcmp(cpcNetifName, "lo0")) {
        memcpy(pcWebNetifName, "lo0", strlen("lo0"));

        return  (ERROR_NONE);
    }

    pcWebNetifNameBuff = requestQueryMsg(OPENRT_WEBNETIF_GET_CMD,
                                        (PVOID)cpcNetifName,
                                        strlen(cpcNetifName)+1,
                                        &iNum);
    if (LW_NULL == pcWebNetifNameBuff || 0 == iNum) {
        return  (PX_ERROR);
    }

    strncpy(pcWebNetifName, pcWebNetifNameBuff, iNetifLen);
    free(pcWebNetifNameBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: libIntAdaSet
** ��������: ����λ���Զ�������
** �䡡��  : pIntAdaSetInfo     ��λ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaSet (__PINT_ADA_SET  pIntAdaSetInfo)
{
    if (LW_NULL == pIntAdaSetInfo ||
        (INT_ADA_FLAG_FIRST  != pIntAdaSetInfo->cFlag &&
         INT_ADA_FLAG_LAST   != pIntAdaSetInfo->cFlag &&
         INT_ADA_FLAG_MIDDLE != pIntAdaSetInfo->cFlag)) {
        return  (PX_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADA_SET_CMD, pIntAdaSetInfo, sizeof(__INT_ADA_SET)));
}
/*********************************************************************************************************
** ��������: libIntAdaClear
** ��������: ����ɾ������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libIntAdaClear (VOID)
{
    requestConfigMsg(OPENRT_ADA_CLEAR_CMD, NULL, 0);
    return  (ERROR_NONE);
}
