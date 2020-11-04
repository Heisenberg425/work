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
** 文   件   名: monitoraction.c
**
** 创   建   人: Chen.HongBang (陈洪邦)
**
** 文件创建日期: 2018 年 7 月 16 日
**
** 描        述: monitor 配置后台接口
*********************************************************************************************************/
#include <stdio.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "lwip/netif.h"
#include "monitor_info.h"
#include "../webHandle.h"
#include "intada_info.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  返回的状态码                */
/*********************************************************************************************************
  Jason key: monitor 配置及流控信息显示
*********************************************************************************************************/
#define WEB_MONITOR_TYPE                "type"                          /*  ip (终端) ，APP(应用)，
                                                                            net interface(线路)，
                                                                            net interface(应用)         */
#define WEB_MONITOR_ENABLE              "enable"                        /*  1: 使能 0: 去使能           */
#define WEB_MONITOR_PROTO               "protocol"                      /*  协议类型tcp/udp/all         */
#define WEB_MONITOR_PORT                "port"                          /*  app的知名端口号             */
#define WEB_MONITOR_IPADDR              "ip"                            /*  监控对象IP地址              */
#define WEB_MONITOR_MAC                 "mac"                           /*  监控对象mac地址             */
#define WEB_MONITOR_ALIAS               "alias"                         /*  监控对象别名                */
#define WEB_MONITOR_IFNAME              "ifname"                        /*  网口名称                    */
#define WEB_MONITOR_WEBNAME             "webname"                       /*  前端网口名称                */
#define WEB_MONITOR_DURATION            "duration"                      /*  持续时间                    */
#define WEB_MONITOR_SEND_RATE           "send_rate"                     /*  下行速率（KB/s）            */
#define WEB_MONITOR_RECV_RATE           "recv_rate"                     /*  上行速率（KB/s）            */
#define WEB_MONITOR_SEND_TOTAL_LEN      "send_total_length"             /*  下行总流量                  */
#define WEB_MONITOR_RECV_TOTAL_LEN      "recv_total_length"             /*  上行总流量                  */
#define WEB_MONITOR_SEND_PACKETS        "send_total_packets"            /*  下行总报文数                */
#define WEB_MONITOR_RECV_PACKETS        "recv_total_packets"            /*  上行总报文数                */
#define WEB_MONITOR_CPU_USAGE           "cpu_usage"                     /*  CPU占用率                   */
#define WEB_MONITOR_MEM_USAGE           "memory_usage"                  /*  内存占用率                  */
#define WEB_MONITOR_ONLINE_USERS        "online_users"                  /*  在线用户数                  */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                        "page"                          /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                       "total"                         /*  Json字段：总共的条目数      */
#define ARRAY_NAME                      "data"                          /*  Json字段：内网端口          */
#define WEB_PAGE_SIZE                   (10)                            /*  网页每页显示条数            */
/*********************************************************************************************************
  Jason key: 互联网连接状态信息
*********************************************************************************************************/
#define WEB_INTERNET_STATUS             "status"                        /*  Json字段：互联网连接状态    */
#define WEB_INTERNET_TIMES              "time"                          /*  Json字段：互联网连接时间    */
#define DAY_SEC                         (24 * 60 * 60)                  /*  一天(秒制)                  */
#define HOUR_SEC                        (60 * 60)                       /*  一时(秒制)                  */
#define MIN_SEC                         (60)                            /*  一分(秒制)                  */
#define SEC_SEC                         (1)                             /*  一秒(秒制)                  */
/*********************************************************************************************************
** 函数名称: __monitorShowAction
** 功能描述: 获取监控信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __monitorShowAction (Webs *wp)
{
    PTM_STATS          pTmStatsHeader           = LW_NULL;
    PTM_STATS          pTmpNode                 = LW_NULL;
    TM_STATS           monitorHandle            = {0};
    INT                iPage                    = 1;                 /*  待显示的页码                   */
    INT                iTotal                   = 0;                 /*  总条目                         */
    cJSON*             pJsonMonitorArry         = LW_NULL;
    cJSON*             pjSend                   = LW_NULL;
    cJSON             *pRecvjRes                = LW_NULL;
    INT                iRecordTotal             = 0;
    INT                i;
    CHAR               cWebName[NETIF_NAMESIZE] = {0};

    /*
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pjSend           = cJSON_CreateObject();                            /*  创建返回 Json 对象          */
    pJsonMonitorArry = cJSON_CreateArray();                             /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || LW_NULL == pJsonMonitorArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_PAGE)){
        iPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE)->valueint;
        if (iPage <= 0) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

            goto  __error;
        }
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE)) {
        monitorHandle.cType = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE)->valueint;
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR)) {
        strncpy(monitorHandle.pIpAddr,
                cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR)->valuestring,
                sizeof(monitorHandle.pIpAddr));
    }

    if (LW_NULL != cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_MAC) ) {
        strncpy(monitorHandle.pMac,
                cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_MAC)->valuestring,
                sizeof(monitorHandle.pMac));
    }

    /*
     *  向 server 发送查询流控请求
     */
    if (TMT_CPU_MEM_USER == monitorHandle.cType) {
        PTM_CPU_MEM  ptmCpuMem = cpuMemShow(&monitorHandle, &iRecordTotal);
        if (NULL == ptmCpuMem) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto  __error;
        }

        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_CPU_USAGE,  ptmCpuMem->dCpuUsage);
        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_MEM_USAGE,  ptmCpuMem->dMemoryUsage);
        cJSON_AddNumberToObject(pjSend, WEB_MONITOR_ONLINE_USERS,  ptmCpuMem->uiOnlineUsers);
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

        if (NULL != ptmCpuMem) {
            free(ptmCpuMem);
        }

        goto  __finish;
    }

    pTmStatsHeader = monitorShow(&monitorHandle, &iRecordTotal);
    if (LW_NULL != pTmStatsHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  遍历数组                    */
            pTmpNode = pTmStatsHeader + i;

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示5条）
             */
            if ((iTotal >= (iPage - 1) * WEB_PAGE_SIZE) && (iTotal < iPage * WEB_PAGE_SIZE)) {
                cJSON*  pjMonitor = cJSON_CreateObject();               /*  创建一条映射 Json 对象      */
                if(LW_NULL == pjMonitor) {
                    break;
                }

                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_TYPE,   pTmpNode->cType);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_ENABLE, pTmpNode->cEnable);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_PROTO,  pTmpNode->cProtocol);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_PORT,   pTmpNode->usPort);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_IPADDR, pTmpNode->pIpAddr);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_MAC,    pTmpNode->pMac);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_ALIAS,  pTmpNode->pAlias);
                cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_IFNAME, pTmpNode->pIfname);

                if (strstr(pTmpNode->pIfname, "en")) {
                    libIntAdaGetWebNetif(pTmpNode->pIfname, cWebName, sizeof(cWebName));

                    cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_WEBNAME, cWebName);
                } else {
                    cJSON_AddStringToObject(pjMonitor, WEB_MONITOR_WEBNAME, pTmpNode->pIfname);
                }

                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_DURATION,  pTmpNode->iDuration);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_RATE, pTmpNode->iSendRate);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_RATE, pTmpNode->iRecvRate);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_TOTAL_LEN, pTmpNode->iSendTotalLen);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_TOTAL_LEN, pTmpNode->iRecvTotalLen);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_SEND_PACKETS, pTmpNode->iSendTotalPackets);
                cJSON_AddNumberToObject(pjMonitor, WEB_MONITOR_RECV_PACKETS, pTmpNode->iRecvTotalPackets);

                cJSON_AddItemToArray(pJsonMonitorArry, pjMonitor);      /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    monitorInfoFree(pTmStatsHeader);

    cJSON_AddItemToObject(pjSend, ARRAY_NAME, pJsonMonitorArry);
    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,  iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,  200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__finish:
__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pJsonMonitorArry);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __monitorItemDisableAction
** 功能描述: 流控配置处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __monitorItemDisableAction (Webs *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;

    cJSON                *pjType        = LW_NULL;
    cJSON                *pjEnable      = LW_NULL;
    cJSON                *pjIpAddr      = LW_NULL;
    cJSON                *pjProto       = LW_NULL;
    cJSON                *pjPort        = LW_NULL;

    TM_STATS              monitorHandle = {0};
    INT                   iRet;

    /*
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjType   = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_TYPE);
    pjEnable = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_ENABLE);
    pjIpAddr = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_IPADDR);
    if (NULL == pjType || NULL == pjEnable || NULL == pjIpAddr) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    monitorHandle.cType   = pjType->valueint;
    monitorHandle.cEnable = pjEnable->valueint;

    strncpy(monitorHandle.pIpAddr, pjIpAddr->valuestring, sizeof(monitorHandle.pIpAddr));

    if (monitorHandle.cType == TMT_APP) {
        pjProto = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_PROTO);
        pjPort  = cJSON_GetObjectItem(pRecvjRes, WEB_MONITOR_PORT);
        if (NULL == pjProto || NULL == pjPort) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __error;
        }

        monitorHandle.cProtocol = pjProto->valueint;
        monitorHandle.usPort    = pjPort->valueint;
    }

    /*
     *  向 server 发送配置流控请求
     */
    iRet = monitorConfig(&monitorHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __internetStatusAction
** 功能描述: 获取互联网连接状态后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __internetStatusAction (Webs *wp)
{
    cJSON               *pSendjRes      = LW_NULL;
    INT                  iRet;
    __INTERNET_STATUS    internetStatus = {0};
    CHAR                 cTimeBuff[20]  = {0};
    INT32                iDay           = 0;
    INT32                iHour          = 0;
    INT32                iMin           = 0;
    INT32                iSec           = 0;

    /*
     *  用户鉴权
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  设置 http 表头
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  创建返回 Json 对象          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iRet = libInternetStatusMonitor(&internetStatus);
    if (ERROR_NONE == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

        iDay  = internetStatus.llTime / DAY_SEC;
        iHour = internetStatus.llTime % DAY_SEC / HOUR_SEC;
        iMin  = internetStatus.llTime % HOUR_SEC / MIN_SEC;
        iSec  = internetStatus.llTime % MIN_SEC;

        snprintf(cTimeBuff, sizeof(cTimeBuff), "%d:%d:%d:%d", iDay, iHour, iMin, iSec);

        cJSON_AddStringToObject(pSendjRes, WEB_INTERNET_TIMES, cTimeBuff);
        cJSON_AddStringToObject(pSendjRes, WEB_INTERNET_STATUS,
                                (internetStatus.ucStatus == INTERNET_OK) ? "ok" : "failed");
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __monitorActionDefine
** 功能描述: 监控后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __monitorActionDefine (VOID)
{
    websDefineAction("monitorInfoGetAction",     __monitorShowAction);
    websDefineAction("monitorItemDisableAction", __monitorItemDisableAction);
    websDefineAction("internetStatusAction",     __internetStatusAction);
}

/*********************************************************************************************************
** 函数名称: monitorActionInit
** 功能描述: 监控后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  monitorActionInit (VOID)
{
    __monitorActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
