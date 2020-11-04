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
** ��   ��   ��: updatefirmwareExec.c
**
** ��   ��   ��: Wang.JingShi (����ʯ)
**
** �ļ���������: 2018 �� 8 �� 3 ��
**
** ��        ��: update firmware ����
*********************************************************************************************************/
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "updatefirmware_common.h"
#include "common.h"
#include "fileparser/fileparser.h"
#include "iniparser/iniparser.h"

/*********************************************************************************************************
** ��������: __updateFirmwareCreateFlag
** ��������: ������־�ļ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareCreateFlag (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};

    /*
     * ���������ļ�
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "touch %s/%s", UPDATE_FIRMWARE_LOCATION,
                                                        UPDATE_FIRMWARE_DATING_FLAG);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: create dating flag file failed!\n");
    }
}
/*********************************************************************************************************
** ��������: __updateFirmwareDeleteFlag
** ��������: ɾ����־�ļ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __updateFirmwareDeleteFlag (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};

    /*
     * ɾ�������ļ�
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "rm %s/%s", UPDATE_FIRMWARE_LOCATION,
                                                     UPDATE_FIRMWARE_DATING_FLAG);
    iRet = system(cCmdTemp);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: delete dating flag file failed!\n");
    }
}
/*********************************************************************************************************
** ��������: __updateFirmwareOper
** ��������: ���¹̼�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __updateFirmwareOper (VOID)
{
    INT       iRet;
    CHAR      cCmdTemp[128] = {0};
    CHAR      cWorkDir[128] = {0};

    /*
     *  ��ѹ���°�
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
     * �л�ִ��Ŀ¼
     */
    snprintf(cWorkDir, sizeof(cWorkDir), "%s/%s", UPDATE_FIRMWARE_LOCATION,
                                                  UPDATE_FIRMWARE_DIRECTORY);
    iRet = chdir(cWorkDir);
    if (ERROR_NONE != iRet) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "update firmware: change work directory failed!\n");

        goto __error;
    }

    /*
     * ���и��½ű�
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
     * ɾ��ϵͳ���°�
     */
    snprintf(cCmdTemp, sizeof(cCmdTemp), "rm_tool -r %s/%s",
                                 UPDATE_FIRMWARE_LOCATION,
                                 UPDATE_FIRMWARE_DIRECTORY);
    system(cCmdTemp);

    return  (iRet);
}
/*********************************************************************************************************
** ��������: updateFirmwareDoConfig
** ��������: ���¹̼�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
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
** ��������: updateFirmwareRecovery
** ��������: update firmware �����Լ�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  updateFirmwareRecovery (VOID)
{
    INT   iRet           = ERROR_NONE;
    CHAR  cFileName[128] = {0};

    /*
     * �ж�ϵͳ���±�־�ļ��Ƿ����
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
