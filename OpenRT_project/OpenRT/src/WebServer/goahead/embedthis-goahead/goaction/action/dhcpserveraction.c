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
** 文   件   名: dhcpserveraction.c
**
** 创   建   人: Wang.JingShi (王京石)
**
** 文件创建日期: 2018 年 7 月 23 日
**
** 描        述: DHCP SERVER 配置后台接口
** 2018.09.20    DHCP SERVER 支持在多网口设备运行多个服务器
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
#include "intada_info.h"
#include "dhcpserver_info.h"
#include "../webHandle.h"

/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                   "code"                               /*  返回的状态码                */
#define DEFAULT_FAILED_CJSON       "{\n\t\"code\":\t500\n}"             /*  json结构创建失败时使用      */
#define JSON_ARGERR_CODE           (100)                                /*  参数错误返回Json code值     */
/*********************************************************************************************************
  Jason key: DHCP SERVER 配置信息显示
*********************************************************************************************************/
#define WEB_DHCPSERVER_ENABLE     "enable"                              /*  json字段：使能              */
#define WEB_DHCPSERVER_START      "start"                               /*  json字段：地址池起始地址    */
#define WEB_DHCPSERVER_END        "end"                                 /*  json字段：地址池结束地址    */
#define WEB_DHCPSERVER_INTERFACE  "interface"                           /*  json字段：绑定网卡          */
#define WEB_DHCPSERVER_DNS1       "dns1"                                /*  json字段：dns1              */
#define WEB_DHCPSERVER_DNS2       "dns2"                                /*  json字段：dns2              */
#define WEB_DHCPSERVER_MASK       "mask"                                /*  json字段：mask              */
#define WEB_DHCPSERVER_GATEWAY    "gateway"                             /*  json字段：网关              */
#define WEB_DHCPSERVER_LEASE      "lease"                               /*  json字段：租约时间          */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                  "page"                                /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                 "total"                               /*  Json字段：总共的条目数      */
#define ARRAY_NAME                "data"                                /*  Json字段：内网端口          */
/*********************************************************************************************************
  DHCP 数据相关宏定义
*********************************************************************************************************/
#define IPSTART                   "start"                               /*  地址池起始地址键值匹配字符串*/
#define IPEND                     "end"                                 /*  地址池结束地址键值匹配字符串*/
#define DNSOPT1                   "dns1"                                /*  dns1 键值匹配字符串         */
#define DNSOPT2                   "dns2"                                /*  dns2 键值匹配字符串         */
#define SUBNETOPT                 "subnet"                              /*  子网掩码键值匹配字符串      */
#define ROUTEROPT                 "router"                              /*  默认网关键值匹配字符串      */
#define LEASEOPT                  "lease"                               /*  租约时间键值匹配字符串      */
#define DNS_DEFAULT               "8.8.8.8"                             /*  默认 DNS                    */

#define ISNOTIP                   (0)                                   /*  非 IP                       */
#define IPVAILD                   (1)                                   /*  IP 值错误                   */
#define IPOK                      (2)                                   /*  IP 值正确                   */

#define addr_cmp(addr1, addr2, mask) \
                                  ((addr1 & mask) == (addr2 & mask))    /*  判断地址是否在同一网段      */
#define SIGMASK                   (0xffffff00)                          /*  255.255.255.0 掩码          */
/*********************************************************************************************************
** 函数名称: __isIpValid
** 功能描述: 判断 IP 地址有效性
** 输    入: cpcIp  ip字符串指针
** 输    出: NONE
** 返    回: 不是 IP 地址返回ISNOTIP
                  IP 地址无效返回 IPVAILD
                  IP 地址正常返回 IPOK
*********************************************************************************************************/
static INT  __isIpValid (CPCHAR  cpcIp)
{
    INT  i         = 0;
    INT  j         = 0;
    INT  iIpSeg[4] = {0, 0, 0, 0};
    INT  iLen      = strlen(cpcIp);
    INT  iTmp      = 0;

    for (i = 0; i < iLen + 1; i++) {
        if (cpcIp[i] == '.' || i == iLen) {                             /*  处理 IP 地址分配符          */
            if (iIpSeg[j] >= 0 && iIpSeg[j] <= 255) {                   /*  处理 IP 地址范围            */
                j ++;
            } else {
                return  (ISNOTIP);                                      /*  不在 IP 数值范围返回 ISNOTIP*/
            }
        } else {
            iTmp = cpcIp[i] - '0';                                      /*  IP 字符串转为整形           */
            if (iTmp > 9 || iTmp < 0) {
                return  (ISNOTIP);                                      /*  非数值字符串返回 ISNOTIP    */
            } else {
                iIpSeg[j] = iIpSeg[j] * 10 + iTmp;                      /*  整形值处理                  */
            }
        }
    }

    if (4 != j) {                                                       /*  判断 IP 地址段数是否正常    */
        return  (ISNOTIP);
    }

    if (iIpSeg[0] > 0 && iIpSeg[0] < 255 &&                             /*  判断 IP 有效性              */
        iIpSeg[3] > 0 && iIpSeg[3] < 255 ) {
        return  (IPOK);                                                 /*  有效返回 IPOK               */
    } else if (0 == cpcIp[0]) {
        return  (ISNOTIP);                                              /*  非 IP 返回 ISNOTIP          */
    }

    return  (IPVAILD);
}
/*********************************************************************************************************
** 函数名称: __isMaskValid
** 功能描述: 判断子网掩码有效性
** 输    入: cpcMask   子网掩码字符串指针
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __isMaskValid (CPCHAR  cpcMask)
{
    UINT  uiIpVal  = 0;
    UINT  uiTmp[4] = {0, 0, 0, 0};
    INT   i        = 0;

    if (IPVAILD == __isIpValid(cpcMask)) {
        sscanf(cpcMask,
               "%u.%u.%u.%u",
               &uiTmp[3],
               &uiTmp[2],
               &uiTmp[1],
               &uiTmp[0]);

        for (i = 0; i < 4; i++) {                                       /*  子网掩码-->32 位无符号整型  */
            uiIpVal += uiTmp[i] << (i * 8);
        }

        uiIpVal = ~uiIpVal + 1;                                         /*  判断掩码是否为 2 的整数幂   */
        if ((uiIpVal & (uiIpVal - 1)) == 0) {
            return  (ERROR_NONE);
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: __dhcpOptArgCheck
** 功能描述: 选项参数检查
** 输    入: cpcType               选项配置类型
**           cpcStrValue           字符串值
**           uiIntValue            整型值
** 输    出: NONE
** 返    回: ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpOptArgCheck (CPCHAR  cpcType, CPCHAR  cpcStrValue, UINT  uiIntValue)
{
    struct in_addr   inaddr;

    if (0 == strcmp(cpcType, DNSOPT1)) {                                /*  判断是否是 dns1 选项        */
        if (!inet_aton(cpcStrValue, &inaddr)) {                         /*  转换字符串为 in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, DNSOPT2)) {                         /*  判断是否是 dns2 选项        */
        if (!inet_aton(cpcStrValue, &inaddr)) {                         /*  转换字符串为 in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, SUBNETOPT)) {                       /*  判断是否是子网掩码选项      */
        if (PX_ERROR == __isMaskValid(cpcStrValue) ||
           !inet_aton(cpcStrValue, &inaddr)) {                          /*  转换字符串为 in_addr        */
            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, ROUTEROPT)) {                       /*  判断是否是默认网关选项      */
        if (__isIpValid(cpcStrValue) != IPOK ||
           !inet_aton(cpcStrValue, &inaddr)) {                          /*  转换字符串为 in_addr        */

            return  (PX_ERROR);
        }
    } else if (0 == strcmp(cpcType, LEASEOPT)) {                        /*  判断是否是租约时间选项      */
        if (uiIntValue <= 0) {                                          /*  判断数值有效性              */
            return  (PX_ERROR);
        }
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __dhcpIpValueCheck
** 功能描述: 检查 DHCP IP 地址的有效性
** 输    入: cpcIpStart   起始 IP
**           cpcIpEnd     结束 IP
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
static INT  __dhcpIpValueCheck (CPCHAR  cpcIpStart, CPCHAR  cpcIpEnd)
{
    struct in_addr  inaddr;
    UINT            uiStart = 0;
    UINT            uiEnd   = 0;

    if (IPOK == __isIpValid(cpcIpStart) &&
        inet_aton(cpcIpStart, &inaddr)) {                               /*  转换字符串为整形地址        */
        uiStart = inaddr.s_addr;                                        /*  赋值全局配置结构体          */

        if (IPOK == __isIpValid(cpcIpEnd) &&
            inet_aton(cpcIpEnd, &inaddr)) {                             /*  转换字符串为整形地址        */
            uiEnd = inaddr.s_addr;                                      /*  赋值全局配置结构体          */

            /*
             *  判断是否在同一网段
             */
            if (addr_cmp(ntohl(uiStart), ntohl(uiEnd), SIGMASK) && ntohl(uiStart) < ntohl(uiEnd)) {
                return  (ERROR_NONE);                                   /*  地址池配置成功              */
            }
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: __dhcpServerShowAction
** 功能描述: dhcp server 信息的后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerShowAction (Webs  *wp)
{
    __PDHCPSERVER_HANDLE   pDhcpServerHeader         = LW_NULL;
    __PDHCPSERVER_HANDLE   pDhcpTmpNode              = LW_NULL;
    __DHCPSERVER_HANDLE    DhcpServerHandle          = {0};
    cJSON                 *pJsonDhcpArry             = LW_NULL;
    cJSON                 *pjSend                    = LW_NULL;
    cJSON                 *pRecvjRes                 = LW_NULL;
    cJSON                 *pjPage                    = LW_NULL;
    INT                    iPage                     = 0;
    INT                    iTotal                    = 0;               /*  总条目                      */
    INT                    iRecordTotal              = 0;
    CHAR                   cWebfName[NETIF_NAMESIZE] = {0};
    INT                    i;

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

    pjSend        = cJSON_CreateObject();                               /*  创建返回 Json 对象          */
    pJsonDhcpArry = cJSON_CreateArray();                                /*  创建 Json 映射信息数组      */
    if (LW_NULL == pjSend || NULL == pJsonDhcpArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    pjPage = cJSON_GetObjectItem(pRecvjRes, WEB_PAGE);
    if (NULL == pjPage) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPage = pjPage->valueint;
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    /*
     *  调用查询请求接口
     */
    pDhcpServerHeader = dhcpServerShow(&DhcpServerHandle, &iRecordTotal);
    if (LW_NULL != pDhcpServerHeader) {
        for (i = 0; i < iRecordTotal; i++) {                            /*  循环设置返回的 Json 内容    */
            pDhcpTmpNode = pDhcpServerHeader + i;

            /*
             *  把需要显示的内容构造成 Json 格式 （一页显示 10 条）
             */
            if ((iTotal >= (iPage - 1) * 10) && (iTotal < iPage * 10)) {

                cJSON  *pjDhcp = cJSON_CreateObject();                  /*  创建一条映射 Json 对象      */
                if(NULL == pjDhcp) {
                     break;
                }

                cJSON_AddNumberToObject(pjDhcp,
                        WEB_DHCPSERVER_ENABLE,
                        pDhcpTmpNode->iEnable);
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_START,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpStart));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_END,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpEnd));

                bzero(cWebfName, sizeof(cWebfName));
                libIntAdaGetWebNetif(pDhcpTmpNode->cInterface, cWebfName, sizeof(cWebfName));

                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_INTERFACE,
                        cWebfName);
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_DNS1,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpDns1));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_DNS2,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpDns2));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_MASK,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpMask));
                cJSON_AddStringToObject(pjDhcp,
                        WEB_DHCPSERVER_GATEWAY,
                        inet_ntoa(*(struct in_addr *)&pDhcpTmpNode->uiIpGateway));
                cJSON_AddNumberToObject(pjDhcp,
                        WEB_DHCPSERVER_LEASE,
                        pDhcpTmpNode->uiLease);

                cJSON_AddItemToArray(pJsonDhcpArry, pjDhcp);            /*  将此条目加入数组            */
            }

            iTotal++;
        }
    }

    if (pDhcpServerHeader) {
        free(pDhcpServerHeader);
    }

    cJSON_AddItemToObject(pjSend,   ARRAY_NAME, pJsonDhcpArry);

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL,  iTotal);
    cJSON_AddNumberToObject(pjSend, WEB_PAGE,   iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE,   200);

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);

    return;

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pJsonDhcpArry);
}
/*********************************************************************************************************
** 函数名称: __dhcpServerConfigAction
** 功能描述: dhcp server 处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerdcConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes           = LW_NULL;
    cJSON                *pSendjRes           = LW_NULL;
    cJSON                *pjEnable            = LW_NULL;
    cJSON                *pjIpStart           = LW_NULL;
    cJSON                *pjIpEnd             = LW_NULL;
    cJSON                *pjInterface         = LW_NULL;
    cJSON                *pjIpDns1            = LW_NULL;
    cJSON                *pjIpDns2            = LW_NULL;
    cJSON                *pjIpMask            = LW_NULL;
    cJSON                *pjIpGateway         = LW_NULL;
    cJSON                *pjLease             = LW_NULL;
    __DHCPSERVER_HANDLE   dhcpServerHandle    = {0};
    CHAR                  cDns2[16]           = {0};
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

    websSetFormVars(wp);                                                /*  获取 post 的主体内容        */

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjEnable    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_ENABLE);
    pjIpStart   = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_START);
    pjIpEnd     = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_END);
    pjInterface = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_INTERFACE);
    pjIpDns1    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_DNS1);
    pjIpMask    = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_MASK);
    pjIpGateway = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_GATEWAY);
    pjLease     = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_LEASE);
    if (pjEnable     == LW_NULL ||
        pjIpStart    == LW_NULL ||
        pjIpEnd      == LW_NULL ||
        pjInterface  == LW_NULL ||
        pjIpDns1     == LW_NULL ||
        pjIpMask     == LW_NULL ||
        pjIpGateway  == LW_NULL ||
        pjLease      == LW_NULL) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto __error;
    }

    pjIpDns2 = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_DNS2);
    if (NULL == pjIpDns2 || '\0' == (pjIpDns2->valuestring)[0]) {
        strncpy(cDns2, DNS_DEFAULT, sizeof(cDns2));
    } else {
        strncpy(cDns2, pjIpDns2->valuestring, sizeof(cDns2));
    }

    iRet = __dhcpIpValueCheck(pjIpStart->valuestring, pjIpEnd->valuestring);
    if (ERROR_NONE != iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto __error;
    }

    if (__dhcpOptArgCheck(DNSOPT1,   pjIpDns1->valuestring,    0) ||
        __dhcpOptArgCheck(DNSOPT2,   cDns2,                    0) ||
        __dhcpOptArgCheck(SUBNETOPT, pjIpMask->valuestring,    0) ||
        __dhcpOptArgCheck(ROUTEROPT, pjIpGateway->valuestring, 0) ||
        __dhcpOptArgCheck(LEASEOPT,  NULL, pjLease->valueint)) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  参数出错                    */

        goto __error;
    }

    dhcpServerHandle.iEnable     = pjEnable->valueint;
    dhcpServerHandle.uiIpStart   = inet_addr(pjIpStart->valuestring);
    dhcpServerHandle.uiIpEnd     = inet_addr(pjIpEnd->valuestring);
    dhcpServerHandle.uiIpDns1    = inet_addr(pjIpDns1->valuestring);
    dhcpServerHandle.uiIpDns2    = inet_addr(cDns2);
    dhcpServerHandle.uiIpMask    = inet_addr(pjIpMask->valuestring);
    dhcpServerHandle.uiIpGateway = inet_addr(pjIpGateway->valuestring);
    dhcpServerHandle.uiLease     = pjLease->valueint;

    iRet = libIntAdaGetNetif(pjInterface->valuestring,                  /*  获取对应的系统网口名称      */
                             dhcpServerHandle.cInterface,
                             sizeof(dhcpServerHandle.cInterface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  调用配置请求接口
     */
    iRet = dhcpServerConfig(&dhcpServerHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __dhcpServerDelAction
** 功能描述: 删除 dhcp Server 后台接口
** 输　入  : wp               服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerDelAction (Webs *wp)
{
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;
    cJSON                *pjInterface      = LW_NULL;
    __DHCPSERVER_HANDLE   dhcpServerHandle = {0};
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

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 json 格式数据*/
    if (LW_NULL== pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    pjInterface = cJSON_GetObjectItem(pRecvjRes, WEB_DHCPSERVER_INTERFACE);
    if (LW_NULL == pjInterface) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    iRet = libIntAdaGetNetif(pjInterface->valuestring,                  /*  获取对应的系统网口名称      */
                             dhcpServerHandle.cInterface,
                             sizeof(dhcpServerHandle.cInterface));
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    iRet = dhcpServerDel(&dhcpServerHandle);
    if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  操作出错                    */
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);              /*  操作成功                    */
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __dhcpServerActionDefine
** 功能描述: dhcp server 后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __dhcpServerActionDefine (VOID)
{
    websDefineAction("dhcpServerShowAction",   __dhcpServerShowAction);
    websDefineAction("dhcpServerConfigAction", __dhcpServerdcConfigAction);
    websDefineAction("dhcpServerDelAction",    __dhcpServerDelAction);
}
/*********************************************************************************************************
** 函数名称: dhcpServerActionInit
** 功能描述: dhcpServer后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  dhcpServerActionInit (VOID)
{
    __dhcpServerActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
