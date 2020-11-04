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
** ��   ��   ��: vlanExec.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 28 ��
**
** ��        ��: vlan ����
*********************************************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "vlan_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "net/lwip/tools/vlan/lwip_vlan.h"

/*********************************************************************************************************
** ��������: vlanDoConfig
** ��������: ���� vlan�����ӡ��޸ġ�ɾ������ӿ�
** �䡡��  : pVlanInfo vlan ������Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  vlanDoConfig (__PVLAN_HANDLE  pVlanInfo)
{
    INT iRet = ERROR_NONE;

    if (LW_NULL == pVlanInfo) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "vlan data is null.\n");

       return  (PX_ERROR);
    }

    if (0 == strcmp(pVlanInfo->cOperType, VLAN_DELETE)){
        iRet = inetVlanSet(pVlanInfo->cIface, -1);
    } else {
        iRet = inetVlanSet(pVlanInfo->cIface, pVlanInfo->iVlanID);
    }

    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to config vlan interface name is %s, "
                                                "error_code=%d.\n",pVlanInfo->cIface, iRet);
        return  (PX_ERROR);
    }

    return  vlanUpdateToDb(pVlanInfo->cOperType, pVlanInfo);
}
/*********************************************************************************************************
** ��������: vlanRecovery
** ��������: vlan ���ûָ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  vlanRecovery (VOID)
{
    __PVLAN_HANDLE pVlanHeader  = LW_NULL;
    __PVLAN_HANDLE pTmpNode     = LW_NULL;
    CHAR           cCmdTmp[128] = {0};
    INT            iRet         = 0;
    INT            iRecordNum   = 0;
    INT            i            = 0 ;

    pVlanHeader = vlanGetFromDB(&iRecordNum);
    if (LW_NULL == pVlanHeader && 0 == iRecordNum) {
        return  (ERROR_NONE);
    }

    for (i = 0; i < iRecordNum; i++) {
        pTmpNode = pVlanHeader + i;
        bzero(cCmdTmp, sizeof(cCmdTmp));
        snprintf(cCmdTmp, sizeof(cCmdTmp), "vlan set %s %d", pTmpNode->cIface, pTmpNode->iVlanID);

        iRet = system(cCmdTmp);
        if (ERROR_NONE != iRet) {
             __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "Failed to execute recovery command = %s, "
                                                   "error_code=%d.\n",cCmdTmp, iRet);
        }
    }

    if (NULL != pVlanHeader) {
        free(pVlanHeader);
    }

    return  (iRet);
}
