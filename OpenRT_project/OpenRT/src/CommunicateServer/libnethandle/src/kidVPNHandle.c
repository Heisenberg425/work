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
** 文   件   名: kidVPNHandle.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 26 日
**
** 描        述: KidVPN 处理
*********************************************************************************************************/
#include <stdlib.h>
#include "kidvpn_info.h"
#include "libcomm.h"
#include "msgcomm.h"
/*********************************************************************************************************
** 函数名称: libKidVPNCreate
** 功能描述: 创建 VPN
** 输　入  : pKidVPNHandle      VPN 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNCreate (__PKIDVPN_HANDLE  pKidVPNHandle)
{
    CHAR                  cType;

    if (LW_NULL == pKidVPNHandle) {
        return  (RET_ARG_ERROR);
    }

    /*
     *  对添加 VPN 的类型进行判断
     */
    cType = pKidVPNHandle->cHandleType;
    if (TYPE_ADD_VPN_S != cType && TYPE_ADD_VPN_C != cType) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_ADD_VPN_CMD, pKidVPNHandle, sizeof(__KIDVPN_HANDLE)));
}

/*********************************************************************************************************
** 函数名称: libKidVPNDel
** 功能描述: 删除 VPN
** 输　入  : pKidVPNHandle      VPN 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNDel (INT  iVndID)
{
    return  (requestConfigMsg(OPENRT_DEL_VPN_CMD, &iVndID, sizeof(iVndID)));
}

/*********************************************************************************************************
** 函数名称: libKidVPNGetInfo
** 功能描述: 获取 VPN 信息 (调用者释放内存)
** 输　入  : NONE
** 输　出  : piNum              条目个数
** 返　回  : ERROR_CODE
*********************************************************************************************************/
PVOID  libKidVPNGetInfo (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_INFO_GET_CMD, NULL, 0, piNum));
}
/*********************************************************************************************************
** 函数名称: libVpnLinkNumGet
** 功能描述: 获取 VPN 连接信息总数目
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : 条目个数
*********************************************************************************************************/
INT    libVpnLinkNumGet (VOID)
{
    return  (requestConfigMsg(OPENRT_LINK_NUM_CMD, NULL, 0));
}
/*********************************************************************************************************
** 函数名称: libKidVPNLinkInfoGet
** 功能描述: 获取 VPN 连接信息 (调用者释放内存)
** 输　入  : pVpnInfoHandle     获取信息条件
** 输　出  : piNum              条目个数
** 返　回  : ERROR_CODE
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
** 函数名称: libKidVPNPosInfoGet
** 功能描述: 获取 VPN 连接地理位置信息 (调用者释放内存)
** 输　入  : NONE
** 输　出  : piNum              条目个数
** 返　回  : ERROR_CODE
*********************************************************************************************************/
PVOID  libKidVPNPosInfoGet (INT  *piNum)
{
    return  (requestQueryMsg(OPENRT_POSIT_GET_CMD, NULL, 0, piNum));
}
/*********************************************************************************************************
** 函数名称: libKidVPNAesKeySave
** 功能描述: 保存 Aes Key 到本地
** 输　入  : pKidVPNHandle      VPN 信息
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  libKidVPNAesKeySave (__PAES_KEY_HANDLE  pKidVPNAesKey)
{
    if (LW_NULL == pKidVPNAesKey) {
        return  (RET_ARG_ERROR);
    }

    return  (requestConfigMsg(OPENRT_KEY_SAVE_CMD, pKidVPNAesKey, sizeof(__AES_KEY_HANDLE)));
}

/*********************************************************************************************************
** 函数名称: libKidVPNAesKeyGen
** 功能描述: 生成 Aes Key
** 输　入  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY 长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
** 函数名称: libKidVPNAesKeyGet
** 功能描述: 获取本地 Aes Key
** 输　入  : cpcKey             AES KEY
**           iBit               KEY
**           iLen               KEY 长度
** 输　出  : NONE
** 返　回  : ERROR_CODE
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
