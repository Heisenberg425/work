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
** ��   ��   ��: tmCmd.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 7 �� 12 ��
**
** ��        ��: OpenRT ��ع���������
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <SylixOS.h>
#include <stdio.h>
#include "common.h"
#include "tmHash.h"
#include "tmStat.h"
#include "tmStatItem.h"
#include "net/if_iphook.h"

/*********************************************************************************************************
** ��������: tmEnable
** ��������: ʹ��ȥʹ������ͳ�ƹ���
** �䡡��  : bIsEnable   �Ƿ�ʹ��ͳ�ƹ���
** �䡡��  : NONE
** ������  : �������
*********************************************************************************************************/
INT  tmEnable (BOOL  bIsEnable)
{
    INT   iRet = 0;
    BOOL  bIsAdd;

    if (bIsEnable) {
        net_ip_hook_isadd(tmStatsStart, &bIsAdd);

        if (bIsAdd) {
            fprintf(stdout, "tm statistics has been started.\n");
        } else {
            iRet = net_ip_hook_add("tm_stats_start", tmStatsStart);
        }
    } else {
        iRet = net_ip_hook_delete(tmStatsStart);
    }

    return  (iRet);
}

/*********************************************************************************************************
** ��������: __tshellTmStat
** ��������: ���������������д�����
** �䡡��  : iArgC     �����в�������
**           ppcArgV   �����в�������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static INT  __tshellTmStat (INT  iArgC, PCHAR  *ppcArgV)
{
    INT                 iType = 0;
    struct sockaddr_in  sockAddr;
    ip4_addr_t          ip4Addr;

    if (1 == iArgC) {
        tmShowStats(iType, LW_NULL);

        return  (ERROR_NONE);
    }

    if (!lib_strcmp(ppcArgV[1], "enable")) {
        tmEnable(LW_TRUE);
    } else if (!lib_strcmp(ppcArgV[1], "disable")) {
        tmEnable(LW_FALSE);
    } else if (!lib_strcmp(ppcArgV[1], "add")) {
        tmStatItemAdd(iArgC, ppcArgV);
    } else if (!lib_strcmp(ppcArgV[1], "del")) {
        tmStatItemDelete(iArgC, ppcArgV);
    } else if (!lib_strcmp(ppcArgV[1], "show")) {
        if (iArgC >= 3) {
            if (!lib_strcmp(ppcArgV[2], "term")) {
                iType = TMT_TERM;
            } else if (!lib_strcmp(ppcArgV[2], "line")) {
                iType = TMT_LINE;
            } else if (!lib_strcmp(ppcArgV[2], "app")) {
                iType = TMT_APP;
            } else if (!lib_strcmp(ppcArgV[2], "lineapp")) {
                iType = TMT_LINE_APP;
            }
        }

        if (iArgC < 4) {
            tmShowStats(iType, LW_NULL);
        } else if (4 == iArgC) {
            if (!inet_aton(ppcArgV[3], &((struct sockaddr_in *)&sockAddr)->sin_addr)) {
                goto  __arg_error;
            }

            sockAddr.sin_len    = sizeof(struct sockaddr_in);
            sockAddr.sin_family = AF_INET;

            inet_addr_to_ip4addr(&ip4Addr, &((struct sockaddr_in *)&sockAddr)->sin_addr);

            tmShowStats(iType, &ip4Addr);
        } else {
            goto  __arg_error;
        }
    } else {
        goto  __arg_error;
    }

    return  (ERROR_NONE);

__arg_error:
    fprintf(stderr, "arguments error!\n");

    return  (-ERROR_TSHELL_EPARAM);
}

/*********************************************************************************************************
** ��������: tmInstallCmd
** ��������: ��װ������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  tmInstallCmd (VOID)
{
    API_TShellKeywordAdd("tm", __tshellTmStat);
    API_TShellFormatAdd("tm", " add | del tm_type ip [mac] [ifname] [tcp | udp] [port]. \n"
                              "tm show [tm_type]. \n"
                              "tm disable | enable. \n");
    API_TShellHelpAdd("tm", "add, delete statistics item(tm_type should be one of term,line,app),\n"
                             " and show traffic statistics.\n"
                             "eg: tm add term 192.168.1.100 48:4d:7e:9f:bc:11\n"
                             "    tm add line en4 \n"
                             "    tm del line en4 \n"
                             "    tm del term 192.168.1.100 \n"
                             "    tm show term \n"
                             "    tm show line \n"
                             "    tm show app \n"
                             "    tm show lineapp \n"
                             "    tm disable \n");
}
