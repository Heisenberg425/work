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
** ��   ��   ��: kidVPNHandle.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 6 �� 26 ��
**
** ��        ��: KidVPN ����
*********************************************************************************************************/
#include <stdlib.h>
#include "kidvpn_info.h"
#include "libcomm.h"
#include "msgcomm.h"
/*********************************************************************************************************
** ��������: libKidVPNCreate
** ��������: ���� VPN
** �䡡��  : pKidVPNHandle      VPN ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNCreate (__PKIDVPN_HANDLE  pKidVPNHandle)
{
    CHAR                  cType;

    if (LW_NULL == pKidVPNHandle) {
        return  (RET_ARG_ERROR);
    }

    /*
     *  ����� VPN �����ͽ����ж�
     */
    cType = pKidVPNHandle->cHandleType;
    if (TYPE_ADD_VPN_S != cType && TYPE_ADD_VPN_C != cType) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADD_VPN_CMD, pKidVPNHandle, sizeof(__KIDVPN_HANDLE)));
}

/*********************************************************************************************************
** ��������: libKidVPNDel
** ��������: ɾ�� VPN
** �䡡��  : pKidVPNHandle      VPN ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNDel (INT  iVndID)
{
    return  (requestConfigMsg(OPENRT_DEL_VPN_CMD, &iVndID, sizeof(iVndID)));
}

/*********************************************************************************************************
** ��������: libKidVPNGetInfo
** ��������: ��ȡ VPN ��Ϣ (�������ͷ��ڴ�)
** �䡡��  : NONE
** �䡡��  : piNum              ��Ŀ����
** ������  : ERROR_CODE
*********************************************************************************************************/
PVOID  libKidVPNGetInfo (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_INFO_GET_CMD, NULL, 0, piNum));
}
/*********************************************************************************************************
** ��������: libVpnLinkNumGet
** ��������: ��ȡ VPN ������Ϣ����Ŀ
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ��Ŀ����
*********************************************************************************************************/
INT    libVpnLinkNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_LINK_NUM_CMD, NULL, 0));
}
/*********************************************************************************************************
** ��������: libKidVPNLinkInfoGet
** ��������: ��ȡ VPN ������Ϣ (�������ͷ��ڴ�)
** �䡡��  : pVpnInfoHandle     ��ȡ��Ϣ����
** �䡡��  : piNum              ��Ŀ����
** ������  : ERROR_CODE
*********************************************************************************************************/
PVOID  libKidVPNLinkInfoGet (__PVPN_INFO_HANDLE  pVpnInfoHandle, INT  *piNum)
{
    if (LW_NULL == pVpnInfoHandle) {

        return  (LW_NULL);
    }

    return  (requestQueryMsg(OPENRT_LINK_GET_CMD,
                             pVpnInfoHandle,
                             sizeof(__VPN_INFO_HANDLE),
                             piNum));
}
/*********************************************************************************************************
** ��������: libKidVPNPosInfoGet
** ��������: ��ȡ VPN ���ӵ���λ����Ϣ (�������ͷ��ڴ�)
** �䡡��  : NONE
** �䡡��  : piNum              ��Ŀ����
** ������  : ERROR_CODE
*********************************************************************************************************/
PVOID  libKidVPNPosInfoGet (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_POSIT_GET_CMD, NULL, 0, piNum));
}
/*********************************************************************************************************
** ��������: libKidVPNAesKeySave
** ��������: ���� Aes Key ������
** �䡡��  : pKidVPNHandle      VPN ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNAesKeySave (__PAES_KEY_HANDLE  pKidVPNAesKey)
{
    if (LW_NULL == pKidVPNAesKey) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_KEY_SAVE_CMD, pKidVPNAesKey, sizeof(__AES_KEY_HANDLE)));
}

/*********************************************************************************************************
** ��������: libKidVPNAesKeyGen
** ��������: ���� Aes Key
** �䡡��  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNAesKeyGen (PCHAR  pcKey, INT  iBit, INT  iLen)
{
    INT                   iNum      = 0;
    PCHAR                 pcKeyBuff = LW_NULL;

    if (LW_NULL == pcKey) {
        return  (RET_ARG_ERROR);
    }

    pcKeyBuff = requestQueryMsg(OPENRT_KEY_GEN_CMD, &iBit, sizeof(iBit), &iNum);
    if (LW_NULL == pcKeyBuff) {
        return  (RET_HANDLE_FAIL);
    }

    strncpy(pcKey, pcKeyBuff, iLen);
    free(pcKeyBuff);

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: libKidVPNAesKeyGet
** ��������: ��ȡ���� Aes Key
** �䡡��  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY ����
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNAesKeyGet (PCHAR  pcKey, INT  iVndID, INT  iLen)
{
    INT                   iNum      = 0;
    PCHAR                 pcKeyBuff = LW_NULL;

    if (LW_NULL == pcKey) {
        return  (RET_ARG_ERROR);
    }

    pcKeyBuff = requestQueryMsg(OPENRT_KEY_GET_CMD, &iVndID, sizeof(iVndID), &iNum);
    if (LW_NULL == pcKeyBuff) {
        return  (RET_HANDLE_FAIL);
    }

    strncpy(pcKey, pcKeyBuff, iLen);
    free(pcKeyBuff);

    return  (RET_HANDLE_OK);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
