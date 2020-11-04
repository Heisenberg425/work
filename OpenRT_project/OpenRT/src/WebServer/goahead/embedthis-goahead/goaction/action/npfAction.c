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
** 文   件   名: npfAction.c
**
** 创   建   人: Shen.WenJun (沈文君)
**
** 文件创建日期: 2018 年 7 月 25 日
**
** 描        述: NPF配置后台接口
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
#include "npf_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json
*********************************************************************************************************/
#define WEB_CODE                        "code"                          /*  返回的状态码                */
#define DEFAULT_FAILED_CJSON            "{\n\t\"code\":\t500\n}"        /*  json结构创建失败时使用      */
/*********************************************************************************************************
 Jason Key: npf配置参数
*********************************************************************************************************/
#define WEB_NPF_RULE_INDEX              "id"                            /*  RULE规则索引号              */
#define WEB_NPF_HANDLE_TYPE             "optype"                        /*  操作类型描述                */
#define WEB_NPF_WEBNAME                 "webname"                       /*  网络接口描述                */
#define WEB_NPF_IFACE                   "iface"                         /*  网络接口描述                */
#define WEB_NPF_IFACESTR                "ifacestr"                      /*  网络接口字符串              */
#define WEB_NPF_RULE                    "rules"                         /*  NPF规则描述                 */
#define WEB_NPF_RULE_TYPE               "ruletype"                      /*  NPF规则类型描述             */
#define WEB_NPF_MACS                    "macs"                          /*  MAC地址描述                 */
#define WEB_NPF_IP                      "ip"                            /*  IP地址描述                  */
#define WEB_NPF_UDP                     "udp"                           /*  UDP描述                     */
#define WEB_NPF_TCP                     "tcp"                           /*  TCP描述                     */
#define WEB_NPF_ENABLE                  "enable"                        /*  NPF状态                     */

#define WEB_NPF_MAC                     "mac"                           /*  MAC地址描述                 */
#define WEB_NPF_IPS                     "ips"                           /*  IP起始地址描述              */
#define WEB_NPF_IPE                     "ipe"                           /*  IP结束地址描述              */
#define WEB_NPF_PORTS                   "ports"                         /*  起始端口描述                */
#define WEB_NPF_PORTE                   "porte"                         /*  结束端口描述                */
#define WEB_NPF_DIRECT                  "direct"                        /*  数据方向                    */
#define WEB_NPF_MANGLE                  "mangle"                        /*  数据处理方式                */
/*********************************************************************************************************
  Jason key: 分页显示信息
*********************************************************************************************************/
#define WEB_PAGE                        "page"                          /*  Json字段：当前待显示的页码  */
#define WEB_TOTAL                       "total"                         /*  Json字段：总共的条目数      */
#define ARRAY_NAME                      "data"                          /*  Json字段：内网端口          */
#define WEB_PAGE_SIZE                   "pagesize"                      /*  每页显示条目                */

#define NPF_GETOBJSTRING(p, key, defVal) \
                    (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                     defVal : cJSON_GetObjectItem(p, key)->valuestring  /*  获取Json对象中字符串内容    */
#define NPF_GETOBJINTER(p, key, defVal) \
                    (LW_NULL == cJSON_GetObjectItem(p, key)) ? \
                     defVal : cJSON_GetObjectItem(p, key)->valueint     /*  获取Json对象中整型内容      */
/*********************************************************************************************************
** 函数名称: __parseNpfData2
** 函数功能: 解析 NPF 规则
** 输    入: pDataHandle  NPF 规则数组头
**           iRules       NPF 规则条目数
** 输    出: NONE
** 返    回: 处理后的 JSON 数据
*********************************************************************************************************/
static cJSON  *__parseNpfData2 (__PNPF_HANDLE  pDataHandle, INT  iRules)
{
    __PNPF_HANDLE pTmpHandle        = LW_NULL;
    INT           i                 = 0;
    CHAR          cWebNetifName[32] = {0};

    if (LW_NULL == pDataHandle) {
        return  (LW_NULL);
    }

    cJSON *pNetifObj     = cJSON_CreateObject();
    cJSON *pJsonNpfAarry = cJSON_CreateArray();

    for (i = 0; i < iRules; i++) {
       pTmpHandle = pDataHandle + i;
       cJSON  *pNpfRule = cJSON_CreateObject();

       bzero(cWebNetifName, sizeof(cWebNetifName));

       libIntAdaGetWebNetif(pTmpHandle->cNetifName, cWebNetifName, sizeof(cWebNetifName));

       cJSON_AddStringToObject(pNpfRule, WEB_NPF_IFACE, cWebNetifName);
       cJSON_AddNumberToObject(pNpfRule, WEB_NPF_ENABLE, pTmpHandle->bEnable);
       cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_TYPE, pTmpHandle->ucType);

       switch (pTmpHandle->ucType) {
       case NPFRULE_MAC:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MAC, pTmpHandle->cHwAddr);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_IP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_UDP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTS, pTmpHandle->usPortS);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTE, pTmpHandle->usPortE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       case NPFRULE_TCP:
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_RULE_INDEX, pTmpHandle->iRuleSeq);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPS, pTmpHandle->cIpAddrS);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_IPE, pTmpHandle->cIpAddrE);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTS, pTmpHandle->usPortS);
            cJSON_AddNumberToObject(pNpfRule, WEB_NPF_PORTE, pTmpHandle->usPortE);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_DIRECT, pTmpHandle->cDirect);
            cJSON_AddStringToObject(pNpfRule, WEB_NPF_MANGLE, pTmpHandle->cMangle);
            break;

       default:
           break;
       }

       cJSON_AddItemToArray(pJsonNpfAarry, pNpfRule);
   }

    cJSON_AddItemToObject(pNetifObj, WEB_NPF_RULE, pJsonNpfAarry);

    return (pNetifObj);
}
/*********************************************************************************************************
** 函数名称: __npfShowAction
** 函数功能: 请求显示NPF规则
** 输    入: wp  webs句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID __npfShowAction (Webs  *wp)
{
    __PNPF_HANDLE  pNpfHeader              = LW_NULL;
    __NPF_HANDLE   npfHandle               = {0};

    cJSON         *pjSend                  = LW_NULL;
    cJSON         *pRecvjRes               = LW_NULL;
    cJSON         *pjArry                  = LW_NULL;
    cJSON         *pjWebName               = LW_NULL;
    PCHAR          pcWebName               = LW_NULL;
    CHAR           cIfName[NETIF_NAMESIZE] = {0};
    INT            iTotal                  = 0;
    INT            iPage                   = 0;
    INT            iPageSize               = 0;

    if (!isUserLogin(wp)) {                                             /*  用户鉴权                    */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    webHeadHandle(wp, 200);                                             /*  设置 http 表头              */

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    pjArry = cJSON_CreateArray();                                       /*  创建 Json 映射信息数组      */
    if(LW_NULL == pjSend || NULL == pjArry) {
        webBackHandle(wp, &_G_ErrorJRes);

        return ;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "show npf, parser request data failed!\n", 0);

        goto  __error;
    }

    iPage = NPF_GETOBJINTER(pRecvjRes, WEB_PAGE, 1);
    if (iPage <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    iPageSize = NPF_GETOBJINTER(pRecvjRes, WEB_PAGE_SIZE, 10);

    npfHandle.iItemStart = (iPage -1) * iPageSize;
    npfHandle.iPageSize  = iPageSize;

    /*
     *  获取 Json 数据内容
     */
    pjWebName = cJSON_GetObjectItem(pRecvjRes, WEB_NPF_WEBNAME);
    if (NULL == pjWebName) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 100);

        goto  __error;
    }

    pcWebName = pjWebName->valuestring;
    if ('\0' == pcWebName[0]) {
        snprintf(npfHandle.cWhereLimit, sizeof(npfHandle.cWhereLimit), "(Type != '255')");
    } else {
        libIntAdaGetNetif(pcWebName, cIfName, sizeof(cIfName));

        snprintf(npfHandle.cWhereLimit,
                 sizeof(npfHandle.cWhereLimit),
                 "(IFNAME IN ('%s') AND Type != '255')", cIfName);
    }

    pNpfHeader = npfShow(&npfHandle, &iTotal);
    if (LW_NULL != pNpfHeader && 0 != iTotal) {
        cJSON_AddItemToObject(pjSend, ARRAY_NAME, __parseNpfData2(pNpfHeader, iTotal));
    } else {
        cJSON_AddItemToObject(pjSend, ARRAY_NAME, pjArry);
    }

    cJSON_AddNumberToObject(pjSend, WEB_TOTAL, (NULL == pNpfHeader) ? iTotal : (pNpfHeader->iTotal));
    cJSON_AddNumberToObject(pjSend, WEB_PAGE, iPage);
    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

    if (NULL != pNpfHeader) {
        npfInfoFree(pNpfHeader);
    }

    operLogInsert(wp, OPEN_RT_NPF, "show npf rules success!\n", 0);

__error:
    webBackHandle(wp, pjSend);
    cJSON_Delete(pRecvjRes);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __npfConfigAction
** 功能描述: npf处理后台接口
** 输　入  : wp  服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID __npfConfigAction (Webs  *wp)
{
    cJSON                *pRecvjRes     = LW_NULL;
    cJSON                *pSendjRes     = LW_NULL;
    cJSON                *ifaceArray    = LW_NULL;
    cJSON                *pArrayItem    = LW_NULL;

    cJSON                *pjWebName     = LW_NULL;
    PCHAR                 pcWebName     = LW_NULL;
    CHAR                  cDefVal[64]   = {0};
    __NPF_HANDLE          npfHandle     = {0};
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

        return ;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  解析前台传来的 Json 格式数据*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf, parser request data failed!\n", 0);

        goto  __error;
    }

    ifaceArray = cJSON_GetObjectItem(pRecvjRes, ARRAY_NAME);
    if (LW_NULL == ifaceArray) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules failed!\n", 0);

        goto  __error;
    }

    pArrayItem = cJSON_GetArrayItem(ifaceArray, 0);
    if (LW_NULL == pArrayItem) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules failed!\n", 0);

        goto  __error;
    }

    /*
     *  获取 Json 数据内容
     */
    bzero(&npfHandle, sizeof(npfHandle));

    pjWebName = cJSON_GetObjectItem(pArrayItem, WEB_NPF_IFACE);
    if (LW_NULL == pjWebName) {
        strncpy(npfHandle.cNetifName, "lo0", sizeof(npfHandle.cNetifName));
    } else {
        pcWebName = pjWebName->valuestring;
        if ('\0' == pcWebName[0]) {
            strncpy(npfHandle.cNetifName, "lo0", sizeof(npfHandle.cNetifName));
        } else {
            libIntAdaGetNetif(pcWebName, npfHandle.cNetifName, sizeof(npfHandle.cNetifName));
        }
    }

    strncpy(npfHandle.cHandle,                                            /*  获取操作动作类型           */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_HANDLE_TYPE, cDefVal),
           sizeof(npfHandle.cHandle));

    strncpy(npfHandle.cHwAddr,                                            /*  获取 MAC 地址              */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_MAC, cDefVal),
           sizeof(npfHandle.cHwAddr));

    strncpy(npfHandle.cIpAddrS,                                           /*  获取起始IP地址             */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_IPS, cDefVal),
           sizeof(npfHandle.cIpAddrS));

    strncpy(npfHandle.cIpAddrE,                                           /*  获取终止IP地址             */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_IPE, cDefVal),
           sizeof(npfHandle.cIpAddrE));

    strncpy(npfHandle.cDirect,                                            /*  获取传输方向               */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_DIRECT, cDefVal),
           sizeof(npfHandle.cDirect));

    strncpy(npfHandle.cMangle,                                            /*  获取允许或拒绝标识         */
           NPF_GETOBJSTRING(pArrayItem, WEB_NPF_MANGLE, cDefVal),
           sizeof(npfHandle.cMangle));

    npfHandle.ucType   = NPF_GETOBJINTER(pArrayItem, WEB_NPF_RULE, 0);
    npfHandle.usPortS  = NPF_GETOBJINTER(pArrayItem, WEB_NPF_PORTS, 0);
    npfHandle.usPortE  = NPF_GETOBJINTER(pArrayItem, WEB_NPF_PORTE, 0);
    npfHandle.iRuleSeq = NPF_GETOBJINTER(pArrayItem, WEB_NPF_RULE_INDEX, 0);

    /*
     *  调用配置请求接口
     */
    iRet = npfConfig(&npfHandle);
    if (ERROR_NONE != iRet) {                                           /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __error;
    } else {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);
        operLogInsert(wp, OPEN_RT_NPF, "config npf rules success!\n", 0);
    }

__error:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __npfActionDefine
** 功能描述: npf后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __npfActionDefine (VOID)
{
    websDefineAction("npfShowAction",   __npfShowAction);
    websDefineAction("npfConfigAction", __npfConfigAction);
}
/*********************************************************************************************************
** 函数名称: npfActionInit
** 功能描述: npf后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  npfActionInit (VOID)
{
    __npfActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
