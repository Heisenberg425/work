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
** 文   件   名: webbindnetifAction.c
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2019 年 01 月 22 日
**
** 描        述: Web 绑定到指定网口后台接口
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
#include "net/if.h"
#include "../webHandle.h"
#include "intada_info.h"
/*********************************************************************************************************
  json code编码值
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  返回的状态码                */
/*********************************************************************************************************
  Jason key:
*********************************************************************************************************/
#define WEB_NETIF             "webnetif"                                /*  Json字段：前端网口名        */
#define WEB_STATUS            "status"                                  /*  绑定状态                    */
#define STATUS_BIND           (1)                                       /*  已绑定                      */
#define STATUS_UNBIND         (0)                                       /*  未绑定                      */
/*********************************************************************************************************
** 函数名称: __webBindToNetifAction
** 功能描述: Web 绑定到指定网口处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __webBindToNetifAction (Webs *wp)
{
    cJSON                *pRecvjRes                   = LW_NULL;
    cJSON                *pSendjRes                   = LW_NULL;
    CHAR                  cNetIfName[NETIF_NAMESIZE]  = {0};
    INT                   iFd                         = 0;
    size_t                sSize                       = 0;
    PCHAR                 pcWebNetif                  = LW_NULL;
    cJSON                *pjWebNetif                  = LW_NULL;
    INT                   iRet                        = PX_ERROR;
    WebsSocket           *sp;
    INT                   i;
    struct ifreq          ifreq;

    extern int            listens[WEBS_MAX_LISTEN];                     /*  监听端口                    */
    extern int            listenMax;

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
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_NETIF);
    if (LW_NULL == pjWebNetif) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcWebNetif = pjWebNetif->valuestring;

    /*
     *  获取对应的系统网口名称
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  设置失败                    */
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    iFd = open("/etc/webNetifConf.ini", O_RDWR | O_CREAT, 664);
    if (iFd <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    } else {
        sSize = write(iFd, cNetIfName, strlen(cNetIfName));
        if (sSize <= 0) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __error;
        }
    }

    bzero(&ifreq, sizeof(ifreq));
    lib_strlcpy(ifreq.ifr_name, cNetIfName, IFNAMSIZ);

    for (i = 0; i < listenMax; i++) {
        if ((sp = socketPtr(listens[i])) == NULL) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __error;
        }

        /*
         *  绑定指定的设备
         */
        setsockopt(sp->sock , SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq));
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    if (iFd > 0) {
        close(iFd);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** 函数名称: __webBindToNetifCancelAction
** 功能描述: 取消 Web 绑定到指定网口处理后台接口
** 输　入  : wp                 服务器句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __webBindToNetifCancelAction (Webs *wp)
{
    cJSON                *pSendjRes                   = LW_NULL;
    INT                   iFd                         = 0;

    WebsSocket           *sp;
    INT                   i;
    struct ifreq          ifreq;

    extern int            listens[WEBS_MAX_LISTEN];                     /*  监听端口                    */
    extern int            listenMax;

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

    iFd = open("/etc/webNetifConf.ini", O_RDWR | O_TRUNC | O_CREAT, 664);
    if (iFd <= 0) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    bzero(&ifreq, sizeof(ifreq));

    for (i = 0; i < listenMax; i++) {
        if ((sp = socketPtr(listens[i])) == NULL) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto  __error;
        }

        /*
         *  绑定指定的设备
         */
        setsockopt(sp->sock , SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq));
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);

__error:
    if (iFd > 0) {
        close(iFd);
    }

    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
}
/*********************************************************************************************************
** 函数名称: __webBindStatusGetAction
** 函数功能: 获取 Web 绑定状态后台定义
** 输    入: wp           服务器句柄
** 输    出: NONE
** 返    回: NONE
*********************************************************************************************************/
static VOID  __webBindStatusGetAction (Webs  *wp)
{
    cJSON         *pjSend                     = LW_NULL;
    CHAR           cNetIfName[NETIF_NAMESIZE] = {0};
    CHAR           cWebName[NETIF_NAMESIZE]   = {0};
    INT            iFd                        = 0;
    size_t         sSize                      = 0;
    INT            i                          = 0;
    INT            iRet;

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

    pjSend = cJSON_CreateObject();                                      /*  创建返回 Json 对象          */
    if(LW_NULL == pjSend) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    iFd = open("/etc/webNetifConf.ini", O_RDONLY);
    if (iFd <= 0) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);
        cJSON_AddStringToObject(pjSend, WEB_NETIF, "");
        cJSON_AddNumberToObject(pjSend, WEB_STATUS, STATUS_UNBIND);
    } else {
        sSize = read(iFd, cNetIfName, NETIF_NAMESIZE);
        if (sSize < 0) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

            goto  __error;
        } else if (0 == sSize) {
            cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);
            cJSON_AddStringToObject(pjSend, WEB_NETIF, "");
            cJSON_AddNumberToObject(pjSend, WEB_STATUS, STATUS_UNBIND);
        } else {
            if (strstr(cNetIfName, "en")) {
                for(i = 2; i < NETIF_NAMESIZE; i++) {
                    if (cNetIfName[i] < '0' || cNetIfName[i] > '9') {
                        cNetIfName[i] = '\0';

                        break;
                    }
                }

                /*
                 *  获取对应的 Web name
                 */
                iRet = libIntAdaGetWebNetif(cNetIfName, cWebName, sizeof(cWebName));
                if (PX_ERROR == iRet) {                                 /*  设置失败                    */
                    cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

                    goto  __error;
                } else {
                    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);
                    cJSON_AddStringToObject(pjSend, WEB_NETIF, cWebName);
                    cJSON_AddNumberToObject(pjSend, WEB_STATUS, STATUS_BIND);
                }
            } else {
                cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);
                cJSON_AddStringToObject(pjSend, WEB_NETIF, "");
                cJSON_AddNumberToObject(pjSend, WEB_STATUS, STATUS_UNBIND);
            }
        }
    }

__error:
    if (iFd > 0) {
        close(iFd);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** 函数名称: __webBindToNetifActionDefine
** 功能描述: Web 绑定网口后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID  __webBindToNetifActionDefine (VOID)
{
    websDefineAction("webBindToNetifAction",       __webBindToNetifAction);
    websDefineAction("webBindToNetifCancelAction", __webBindToNetifCancelAction);
    websDefineAction("webBindStatusGetAction",     __webBindStatusGetAction);
}

/*********************************************************************************************************
** 函数名称: webBindToNetifActionInit
** 功能描述: Web 绑定网口后台接口定义
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  webBindToNetifActionInit (VOID)
{
    __webBindToNetifActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
