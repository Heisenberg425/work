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
** ��   ��   ��: hwInfoHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 8 �� 2 ��
**
** ��        ��: Ӳ����Ϣ��ȡ����
*********************************************************************************************************/
#include "SylixOS.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "libcomm.h"
#include "libhw_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: libHardWareInfoGet
** ��������: ��ȡӲ����Ϣ
** �䡡��  : pHwInfo            Ӳ����Ϣָ��
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
INT  libHardWareInfoGet (__PHW_INFO  pHwInfo)
{
    INT32    iNum;
    PVOID    pvBuff = LW_NULL;

    if (LW_NULL == pHwInfo) {
        return  (PX_ERROR);
    }

    pvBuff = requestQueryMsg(OPENRT_HW_INFO_GET_CMD, NULL, 0, &iNum);

    if (pvBuff) {
        memcpy(pHwInfo, pvBuff, sizeof(__HW_INFO));
        free(pvBuff);
    }

    return  (ERROR_NONE);
}
