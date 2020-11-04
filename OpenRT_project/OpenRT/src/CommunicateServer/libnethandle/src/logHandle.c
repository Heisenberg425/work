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
#include "liblog_config.h"
#include "msgcomm.h"

/*********************************************************************************************************
** ��������: libUdialLogNumGet
** ��������:  ��ȡ����������־����
** �䡡��  : pUdialLogInfo      ������־��Ϣ
** �䡡��  : NONE
** ������  : iNum               ��Ŀ����
*********************************************************************************************************/
INT32  libUdialLogNumGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle)
{
    return  (requestConfigMsg(OPENRT_UDIAL_LOG_NUM, (PVOID)pUdialLogHandle, sizeof(__UDIAL_LOG_HANDLE)));
}
/*********************************************************************************************************
** ��������: libUdialLogInfoGet
** ��������: ��ȡ����������־��Ϣ (��Ҫ���� free �ͷ���Դ)
** �䡡��  : pUdialLogInfo          ������־��Ϣ
** �䡡��  : piNum                  ����
** ������  : NONE
*********************************************************************************************************/
PVOID  libUdialLogInfoGet (__PUDIAL_LOG_HANDLE  pUdialLogHandle, INT  *piNum)
{
    if (LW_NULL == pUdialLogHandle) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_UDIAL_LOG_GET,
                             pUdialLogHandle,
                             sizeof(__UDIAL_LOG_HANDLE),
                             piNum));
}

/*********************************************************************************************************
** ��������: libVpnLogNumGet
** ��������:  ��ȡ VPN ��־����
** �䡡��  : pVpnLogHandle      VPN ��־��ȡ�ṹ
** �䡡��  : NONE
** ������  : iNum               ��Ŀ����
*********************************************************************************************************/
INT32  libVpnLogNumGet (__PVPN_LOG_HANDLE  pVpnLogHandle)
{
    return  (requestConfigMsg(OPENRT_VPN_LOG_NUM, (PVOID)pVpnLogHandle, sizeof(__VPN_LOG_HANDLE)));
}
/*********************************************************************************************************
** ��������: libVpnLogInfoGet
** ��������: ��ȡ VPN ��־��Ϣ (��Ҫ���� free �ͷ���Դ)
** �䡡��  : pVpnLogHandle      VPN ��־��ȡ�ṹ
** �䡡��  : piNum              ����
** ������  : ��־��Ϣ
*********************************************************************************************************/
PVOID  libVpnLogInfoGet (__PVPN_LOG_HANDLE  pVpnLogHandle, INT  *piNum)
{
    if (LW_NULL == pVpnLogHandle) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_VPN_LOG_GET,
                             pVpnLogHandle,
                             sizeof(__VPN_LOG_HANDLE),
                             piNum));
}


/*********************************************************************************************************
** ��������: writeOperLog
** ��������: д������־
** �䡡��  : pOperLog               ������־��Ϣ
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT32  writeOperLog (__POPER_LOG  pOperLog)
{
    return  (requestConfigMsg(OPENRT_OPER_LOG_PUT, (PVOID)pOperLog, sizeof(__OPER_LOG)));
}
/*********************************************************************************************************
** ��������: readOperLog
** ��������: ��������־
** �䡡��  : pOperLog               ������־��Ϣ
** �䡡��  : piNum                  ����
** ������  : NONE
*********************************************************************************************************/
PVOID  readOperLog (__POPER_LOG  pOperLog, INT  *piNum)
{
    if (LW_NULL == pOperLog) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_OPER_LOG_GET,
    		                 pOperLog,
                             sizeof(__OPER_LOG),
                             piNum));
}
/*********************************************************************************************************
** ��������: libUdialLogInfoDel
** ��������: ���������־��Ϣ
** �䡡��  : pUdialLogInfo    ������־��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libUdialLogInfoDel (__PUDIAL_LOG_HANDLE  pUdialLogHandle)
{
    if (LW_NULL == pUdialLogHandle) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_UDIAL_LOG_DEL, pUdialLogHandle, sizeof(__UDIAL_LOG_HANDLE));
}
/*********************************************************************************************************
** ��������: libOperLogInfoDel
** ��������: ���������־��Ϣ
** �䡡��  : pOperLog          ������־��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libOperLogInfoDel (__POPER_LOG  pOperLog)
{
    if (LW_NULL == pOperLog) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_OPER_LOG_DEL, pOperLog, sizeof(__OPER_LOG));
}
/*********************************************************************************************************
** ��������: libVpnLogInfoDel
** ��������: ��� VPN ��־��Ϣ
** �䡡��  : pVpnLogHandle      VPN ��־��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libVpnLogInfoDel (__PVPN_LOG_HANDLE  pVpnLogHandle)
{
    if (LW_NULL == pVpnLogHandle) {

        return  (PX_ERROR);
    }

    return  requestConfigMsg (OPENRT_VPN_LOG_DEL, pVpnLogHandle, sizeof(__VPN_LOG_HANDLE));
}
