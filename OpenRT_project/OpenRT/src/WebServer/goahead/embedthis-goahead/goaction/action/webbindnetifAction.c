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
** ��   ��   ��: webbindnetifAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2019 �� 01 �� 22 ��
**
** ��        ��: Web �󶨵�ָ�����ں�̨�ӿ�
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
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
/*********************************************************************************************************
  Jason key:
*********************************************************************************************************/
#define WEB_NETIF             "webnetif"                                /*  Json�ֶΣ�ǰ��������        */
#define WEB_STATUS            "status"                                  /*  ��״̬                    */
#define STATUS_BIND           (1)                                       /*  �Ѱ�                      */
#define STATUS_UNBIND         (0)                                       /*  δ��                      */
/*********************************************************************************************************
** ��������: __webBindToNetifAction
** ��������: Web �󶨵�ָ�����ڴ����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
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

    extern int            listens[WEBS_MAX_LISTEN];                     /*  �����˿�                    */
    extern int            listenMax;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto  __error;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjWebNetif = cJSON_GetObjectItem(pRecvjRes, WEB_NETIF);
    if (LW_NULL == pjWebNetif) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __error;
    }

    pcWebNetif = pjWebNetif->valuestring;

    /*
     *  ��ȡ��Ӧ��ϵͳ��������
     */
    iRet = libIntAdaGetNetif(pcWebNetif, cNetIfName, sizeof(cNetIfName));
    if (PX_ERROR == iRet) {                                             /*  ����ʧ��                    */
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
         *  ��ָ�����豸
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
** ��������: __webBindToNetifCancelAction
** ��������: ȡ�� Web �󶨵�ָ�����ڴ����̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __webBindToNetifCancelAction (Webs *wp)
{
    cJSON                *pSendjRes                   = LW_NULL;
    INT                   iFd                         = 0;

    WebsSocket           *sp;
    INT                   i;
    struct ifreq          ifreq;

    extern int            listens[WEBS_MAX_LISTEN];                     /*  �����˿�                    */
    extern int            listenMax;

    /*
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
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
         *  ��ָ�����豸
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
** ��������: __webBindStatusGetAction
** ��������: ��ȡ Web ��״̬��̨����
** ��    ��: wp           ���������
** ��    ��: NONE
** ��    ��: NONE
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
     *  �û���Ȩ
     */
    if (!isUserLogin(wp)) {
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    /*
     *  ���� http ��ͷ
     */
    webHeadHandle(wp, 200);

    pjSend = cJSON_CreateObject();                                      /*  �������� Json ����          */
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
                 *  ��ȡ��Ӧ�� Web name
                 */
                iRet = libIntAdaGetWebNetif(cNetIfName, cWebName, sizeof(cWebName));
                if (PX_ERROR == iRet) {                                 /*  ����ʧ��                    */
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
** ��������: __webBindToNetifActionDefine
** ��������: Web �����ں�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __webBindToNetifActionDefine (VOID)
{
    websDefineAction("webBindToNetifAction",       __webBindToNetifAction);
    websDefineAction("webBindToNetifCancelAction", __webBindToNetifCancelAction);
    websDefineAction("webBindStatusGetAction",     __webBindStatusGetAction);
}

/*********************************************************************************************************
** ��������: webBindToNetifActionInit
** ��������: Web �����ں�̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  webBindToNetifActionInit (VOID)
{
    __webBindToNetifActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
