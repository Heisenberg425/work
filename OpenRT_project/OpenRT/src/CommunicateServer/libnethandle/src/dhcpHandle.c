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
** ��   ��   ��: dhcpHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: DHCP ����
*********************************************************************************************************/
#include <stdlib.h>
#include "dhcp_info.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libDhcpClientHandle
** ��������: ʹ�� / �������� DHCP �ͻ���
** �䡡��  : pcNetifName        ��������
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libDhcpClientHandle (PCHAR  pcNetifName, CHAR  cHandle)
{
    __DHCP_CLIENT_SET     dhcpClientHandle;

    if (LW_NULL == pcNetifName) {
        return  (PX_ERROR);
    }

    bzero(&dhcpClientHandle, sizeof(dhcpClientHandle));
    dhcpClientHandle.cHandle = cHandle;
    strncpy(dhcpClientHandle.cNetifName, pcNetifName, sizeof(dhcpClientHandle.cNetifName));

    return  (requestConfigMsg(OPENRT_DHCPC_HANDLE, &dhcpClientHandle, sizeof(dhcpClientHandle)));
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
