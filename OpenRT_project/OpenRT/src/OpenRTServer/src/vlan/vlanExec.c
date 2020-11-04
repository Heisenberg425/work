/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: vlanExec.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 28 日
**
** 描        述: vlan 操作
*********************************************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "vlan_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "net/lwip/tools/vlan/lwip_vlan.h"

/*********************************************************************************************************
** 函数名称: vlanDoConfig
** 功能描述: 配置 vlan，增加、修改、删除网络接口
** 输　入  : pVlanInfo vlan 参数信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: vlanRecovery
** 功能描述: vlan 配置恢复
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
