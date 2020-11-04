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
** 文   件   名: udailRecovery.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 6 月 19 日
**
** 描        述: 拨号状态恢复
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <spawn.h>
#include "common.h"
#include "udialExec.h"
#include "udialDB.h"
#include <net/lwip/event/lwip_netevent.h>
#include "network/lwip/netif.h"
#include "libudial_config.h"
#include "database/database_common.h"
/*********************************************************************************************************
  拨号数据库配置信息
*********************************************************************************************************/
#define UDIAL_TABLE                "udial"                              /*  拨号表单名                  */
#define UDIAL_TABLE_HEAD           "(ID         INTEGER PRIMARY KEY AUTOINCREMENT,"  \
                                   "USERNAME    TEXT,"                               \
                                   "PASSWORD    TEXT,"                               \
                                   "NETIF       TEXT,"                               \
                                   "PPPNAME     TEXT,"                               \
                                   "PID         INT,"                                \
                                   "STATUS      TEXT)"                  /*  拨号表单格式                */
/*********************************************************************************************************
** 函数名称: udialRecovery
** 功能描述: 恢复重启前的拨号状态
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  udialRecovery (VOID)
{
    CHAR                  cSelectSql[200]  = {0};
    sqlite3               *pDb             = LW_NULL;
    CHAR                 *pcErrMsg         = LW_NULL;
    INT                   iRet;
    CHAR                **cDbResult;
    INT                   IRow, IColumn;

    INT                   i;

    __UDIAL_HANDLE        udialHandleInfo;

    OPENRT_DB_HANDLE(pDb);
    if(LW_NULL == pDb) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "data base error.\n");

        return  (PX_ERROR);
    }

    sqlite3_exec(pDb, "BEGIN;", 0, 0, NULL);
    snprintf(cSelectSql, sizeof(cSelectSql), "SELECT * FROM %s;", UDIAL_TABLE);

    iRet = sqlite3_get_table(pDb, cSelectSql, &cDbResult, &IRow, &IColumn, &pcErrMsg);
    if (SQLITE_OK == iRet) {
        for (i = 1; i <= IRow; i++) {                                   /*  按行获取(过滤第一行)        */
            bzero(&udialHandleInfo, sizeof(udialHandleInfo));
            strncpy(udialHandleInfo.cUsrname,
                    cDbResult[i * IColumn + 1],
                    sizeof(udialHandleInfo.cUsrname));
            strncpy(udialHandleInfo.cPassword,
                    cDbResult[i * IColumn + 2],
                    sizeof(udialHandleInfo.cPassword));
            strncpy(udialHandleInfo.cNetifname,
                    cDbResult[i * IColumn + 3],
                    sizeof(udialHandleInfo.cNetifname));

            /*
             *  之前已经连接成功的进行恢复
             */
            if (NET_EVENT_PPP_RUN == atoi(cDbResult[i * IColumn + 6])) {
                iRet = udialStart(&udialHandleInfo);                    /*  进行拨号操作                */
            } else {
                /*
                 *  对于其余那些本次恢复前没有成功拨号的网口，不进行拨号恢复
                 *  操作，这里将他们的 pid 、PP 网口、 状态名称恢复为默认值。
                 */
                udialHandleInfo.uiPid        = 0;
                udialHandleInfo.iUdialStatus = NET_EVENT_PPP_DEAD;
                udialUpdateToDb(&udialHandleInfo);
            }
        }
    } else {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "get data base lan info failed.\n");
    }

    sqlite3_free(pcErrMsg);
    sqlite3_free_table(cDbResult);
    sqlite3_exec(pDb, "COMMIT;", 0, 0, NULL);

    return  (ERROR_NONE);
}
