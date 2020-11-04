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
** ��   ��   ��: intAdaEntry.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: ������λ�������
*********************************************************************************************************/
#include <string.h>
#include "lwip/netif.h"
#include "intada_common.h"
#include "intAdaptionExec.h"
#include "msgcomm.h"
#include "common.h"
/*********************************************************************************************************
** ��������: intAdaHandleEntry
** ��������: ��������Ӧ��λ���
** �䡡��  : iCommand           ��������
**           pArgBuf            ��������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  intAdaHandleEntry (INT  iCommand, PVOID  pArgBuf)
{
    __PINT_ADA_SET  pIntAdaSetInfo                = LW_NULL;
    CHAR            cWebNetifName[NETIF_NAMESIZE] = {0};
    CHAR            cNetIfName[NETIF_NAMESIZE]    = {0};
    INT             iRet;

    switch (iCommand) {
    case OPENRT_ADA_GETS_CMD:
        replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, intAdaptionCheck());

        break;

    case OPENRT_NETIF_GET_CMD:
        if (LW_NULL == pArgBuf) {
            goto  __get_netif_err;
        }

        if (strlen(pArgBuf) >=  NETIF_NAMESIZE) {
            goto  __get_netif_err;
        }

        strncpy(cWebNetifName, pArgBuf, sizeof(cWebNetifName));

        iRet = intAdaptionGetNetif(cWebNetifName, cNetIfName);          /*  ��ȡ������Ϣ����            */
        if (ERROR_NONE != iRet) {
            goto  __get_netif_err;
        }

        replyQueryMsg(OPENRT_NETIF_GET_ACK_CMD, sizeof(cNetIfName), 1, cNetIfName);

        break;

    case OPENRT_WEBNETIF_GET_CMD:
        if (LW_NULL == pArgBuf) {
            goto  __get_webname_err;
        }

        if (strlen(pArgBuf) >=  NETIF_NAMESIZE) {
            goto  __get_webname_err;
        }

        strncpy(cNetIfName, pArgBuf, sizeof(cNetIfName));

        iRet = intAdaptionGetWebNetif(cNetIfName, cWebNetifName);          /*  ��ȡ web ������Ϣ����    */
        if (ERROR_NONE != iRet) {
            goto  __get_webname_err;
        }

        replyQueryMsg(OPENRT_WEBNETIF_GET_ACK_CMD, sizeof(cWebNetifName), 1, cWebNetifName);

        break;

    case OPENRT_ADA_SET_CMD:
        if (LW_NULL == pArgBuf) {
            replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, PX_ERROR);

            break;
        }

        pIntAdaSetInfo = (__PINT_ADA_SET)pArgBuf;

        /*
         *  ����λ����
         */
        replyConfigMsg(OPENRT_ADA_GETS_ACK_CMD, intAdaptionHandle(pIntAdaSetInfo));

        break;

    case OPENRT_ADA_CLEAR_CMD:
        intAdaptionClear();

        replyConfigMsg(OPENRT_ADA_CLEAR_ACK_CMD, 0);

        break;

    default:
        break;
    }

    return;

__get_netif_err:
    replyQueryMsg(OPENRT_NETIF_GET_ACK_CMD, sizeof(cNetIfName), 0, NULL);

    return;
__get_webname_err:
    replyQueryMsg(OPENRT_WEBNETIF_GET_ACK_CMD, sizeof(cWebNetifName), 0, NULL);

    return;
}
