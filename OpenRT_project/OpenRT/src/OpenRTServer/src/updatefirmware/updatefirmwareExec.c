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
** 文   件   名: updatefirmwareExec.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 8 月 3 日
**
** 描        述: update firmware 操作
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "updatefirmware_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "iniparser/iniparser.h"

/*********************************************************************************************************
** 函数名称: __updateFirmwareCreateFlag
** 功能描述: 创建标志文件
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareCreateFlag (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};

    /*
     * 创建更新文件
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "touch %s/%s", UPDATE_FIRMWARE_LOCATION,
                                                        UPDATE_FIRMWARE_DATING_FLAG);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: create dating flag file failed!\n");
    }
}
/*********************************************************************************************************
** 函数名称: __updateFirmwareDeleteFlag
** 功能描述: 删除标志文件
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareDeleteFlag (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};

    /*
     * 删除更新文件
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "rm %s/%s", UPDATE_FIRMWARE_LOCATION,
                                                     UPDATE_FIRMWARE_DATING_FLAG);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: delete dating flag file failed!\n");
    }
}
/*********************************************************************************************************
** 函数名称: __updateFirmwareOper
** 功能描述: 更新固件
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __updateFirmwareOper (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};
    CHAR      cWorkDir[128] = {0};

    /*
     *  解压更新包
     */
    if (ERROR_NONE == access("/tmp/update", F_OK)) {
        system("rm_tool -r /tmp/update");
    }

    mkdir("/tmp/update", 666);

    snprintf(cCmdTemp, sizeof(cCmdTemp), "untar %s/%s %s", UPDATE_FIRMWARE_LOCATION,
                                                           UPDATE_FIRMWARE_TAR,
                                                           UPDATE_FIRMWARE_LOCATION);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: untar failed!\n");

        goto __error;
    }

    /*
     * 切换执行目录
     */
    snprintf(cWorkDir, sizeof(cWorkDir), "%s/%s", UPDATE_FIRMWARE_LOCATION,
                                                  UPDATE_FIRMWARE_DIRECTORY);
    iRet = chdir(cWorkDir);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: change work directory failed!\n");

        goto __error;
    }

    /*
     * 运行更新脚本
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "shfile %s/%s/%s",
                                 UPDATE_FIRMWARE_LOCATION,
                                 UPDATE_FIRMWARE_DIRECTORY,
                                 UPDATE_FIRMWARE_SHELL);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: run the update script failed!\n");

        goto __error;
    }

__error:
    /*
     * 删除系统更新包
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "rm_tool -r %s/%s",
                                 UPDATE_FIRMWARE_LOCATION,
                                 UPDATE_FIRMWARE_DIRECTORY);
    system(cCmdTemp);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: updateFirmwareDoConfig
** 功能描述: 更新固件
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  updateFirmwareDoConfig (VOID)
{
    INT iRet;

    __updateFirmwareCreateFlag();
    iRet = __updateFirmwareOper();
    __updateFirmwareDeleteFlag();

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: updateFirmwareRecovery
** 功能描述: update firmware 开机自检
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  updateFirmwareRecovery (VOID)
{
    INT   iRet           = ERROR_NONE;
    CHAR  cFileName[128] = {0};

    /*
     * 判断系统更新标志文件是否存在
     */
    snprintf(cFileName, sizeof(cFileName), "%s/%s", UPDATE_FIRMWARE_LOCATION,
                                                    UPDATE_FIRMWARE_DATING_FLAG);
    if (ERROR_NONE == access(cFileName, F_OK)) {
        iRet = __updateFirmwareOper();
        __updateFirmwareDeleteFlag();

        sync();

        system("reboot");
    }

    return  (iRet);
}
