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
** ��   ��   ��: kidvpnConfAction.c
**
** ��   ��   ��: Bai.QiangQiang (��ǿǿ)
**
** �ļ���������: 2018 �� 10 �� 15 ��
**
** ��        ��: OpenRT VPN ��̨����ӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "SylixOS.h"
#include "cJSON.h"
#include "goahead.h"
#include "vnd_info.h"
#include "kidvpn_info.h"
#include "situationCommon.h"
#include "situation_info.h"
#include "../webHandle.h"
/*********************************************************************************************************
  json code����ֵ
*********************************************************************************************************/
#define WEB_CODE              "code"                                    /*  ���ص�״̬��                */
#define JSON_SUCCESS_CODE     (200)                                     /*  �ɹ�����Json codeֵ         */
#define JSON_FAILED_CODE      (500)                                     /*  ʧ�ܷ���Json codeֵ         */
/*********************************************************************************************************
  Jason key: kidvpn ����
*********************************************************************************************************/
#define WEB_VPN_VNDID         "vndid"                                   /*  Json�ֶΣ��������� id       */
#define WEB_VPN_IPADDR        "ipaddr"                                  /*  Json�ֶΣ�IP ��ַ           */
#define WEB_VPN_PASSWD        "passwd"                                  /*  Json�ֶΣ�vpn ������Կ      */
#define WEB_VPN_STATUS        "status"                                  /*  Json�ֶΣ�vpn ����״̬      */
#define WEB_PUBLIC_IP         "pub_ip"                                  /*  Json�ֶΣ����� ip           */
/*********************************************************************************************************
  Jason key: aes key ����
*********************************************************************************************************/
#define WEB_VPN_AES_KEY       "aeskey"                                  /*  Json�ֶΣ�key               */
/*********************************************************************************************************
  Jason key: VND ����
*********************************************************************************************************/
#define WEB_VND_IPADD         "vnd_ipaddr"                              /*  Json�ֶΣ�IP ��ַ           */
#define WEB_VND_NETMSK        "vnd_netmask"                             /*  Json�ֶΣ���������          */
#define WEB_VND_GATEWAY       "vnd_gateway"                             /*  Json�ֶΣ�����              */
/*********************************************************************************************************
  KidVPN ��غ궨��
*********************************************************************************************************/
#define KV_PORT_DEFAULT        (10088)                                  /*  kidvpn Ĭ�϶˿ں�           */
#define KV_MTU_DEFAULT         (1464)                                   /*  kidvpn Ĭ�� MTU             */
#define KV_AESKEY_DEFAULT      (192)                                    /*  kidvpn Ĭ�� AESKEY λ��     */
#define WEB_OPER_TYPE          "oper_type"                              /*  kidvpn ��������             */
/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define BUF_SIZE               (1024)                                   /*  ��������С                  */
#define PORT                   (80)                                     /*  �˿ں�                      */
#define CURL_STRING            "www.3322.org/dyndns/getip"              /*  CURL ���                   */
#define TRY_TIMES              (4)                                      /*  ���Դ���                    */
/*********************************************************************************************************
** ��������: __getPublicIp
** ��������: ��ȡ�������� ip
** �䡡��  : pcIpBuf     ��� ip ��Ϣ
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
static INT  __getPublicIp (PCHAR  pcIpBuf)
{
    struct sockaddr_in  sockIn;
    struct hostent     *pHostEnt;
    PCHAR               pTmp;
    PCHAR               pCur;
    INT                 isockFd = 0;
    INT                 iLen    = 0;

    CHAR                cBuf[BUF_SIZE] = {0};
    CHAR                cUrl[100]      = {0};
    CHAR                cHost[100]     = {0};
    CHAR                cGet[100]      = {0};
    CHAR                cHeader[240]   = {0};
    PCHAR               pHost          = NULL;
    INT                 iPos;

    /*
     *  �� url �л�ȡ����������صĵ�ַ��Ϣ
     */
    strcpy(cUrl, CURL_STRING);

    for (pHost = cUrl; *pHost != '/' && *pHost != '\0'; ++pHost) {
        ;
    }

    if ((INT)(pHost - cUrl) == strlen(cUrl)) {
        strcpy(cGet, "/");
    } else {
        strcpy(cGet, pHost);
    }

    *pHost = '\0';
    strcpy(cHost, cUrl);

    /*
     *  ���� socket ����
     */
    if (0 == (pHostEnt = lwip_gethostbyname(cHost))) {
        perror("error get host.\n");

        return  (PX_ERROR);
    }

    bzero(&sockIn, sizeof(sockIn));
    sockIn.sin_family = AF_INET;
    sockIn.sin_addr.s_addr = htonl(INADDR_ANY);
    sockIn.sin_addr.s_addr = ((struct in_addr *)(pHostEnt->h_addr))->s_addr;
    sockIn.sin_port = htons(PORT);

    if ((isockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error opening socket.\n");

        return  (PX_ERROR);
    }

    /*
     *  �ϳ�������Ϣ�����ڷ��͵� web ������
     */
    strcat(cHeader, "GET");
    strcat(cHeader, " ");
    strcat(cHeader, cGet);
    strcat(cHeader, " ");
    strcat(cHeader, "HTTP/1.1\r\n");
    strcat(cHeader, "HOST:");
    strcat(cHeader, cHost);
    strcat(cHeader, "\r\n");
    strcat(cHeader, "ACCEPT:*/*");
    strcat(cHeader, "\r\nConnection: close\r\n\r\n\r\n");

    /*
     *  ���� web ������������ͷ���ݣ������� web ���ص�����
     */
    if (connect(isockFd, (void *)&sockIn, sizeof(sockIn)) == -1) {
        perror("error connect to socket.\n");

        return  (PX_ERROR);
    }

    if (send(isockFd, cHeader, strlen(cHeader), 0) == -1) {
        perror("error in send.\n");

        return  (PX_ERROR);
    }

    iLen = recv(isockFd, cBuf, BUF_SIZE, 0);
    if (iLen < 0) {
        perror("receive data error.\n");

        return  (PX_ERROR);
    }

    if ('\0' == cBuf[0]) {
        return  (PX_ERROR);
    }

    /*
     *  ��ȡ���� ip ��Ϣ
     */
    pTmp = lib_strstr(cBuf, "Connection");
    if (NULL == pTmp) {
        return  (PX_ERROR);
    }

    pTmp = lib_index(pTmp, '.');
    if (NULL == pTmp) {
        return  (PX_ERROR);
    }

    pTmp = pTmp - 3;
    iPos = 3;
    while (iPos) {
        if ((*pTmp <= '9') && (*pTmp >= '0')) {
            break;
        }

        pTmp++;
        iPos--;
    }

    if (0 == iPos) {
        return  (PX_ERROR);
    }

    pCur = lib_index(pTmp, '\n');
    if (NULL == pCur) {
        return  (PX_ERROR);
    }

    lib_memcpy(pcIpBuf, pTmp, pCur - pTmp);

    close(isockFd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __kidvpnConfigAction
** ��������: ���� KidVPN ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnConfigAction (Webs *wp)
{
    INT                   iRet             = PX_ERROR;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;

    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcPassWd         = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjPassWd         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;

    CHAR                 *pcVndNetMask     = LW_NULL;
    CHAR                 *pcVndIpAddr      = LW_NULL;
    cJSON                *pjVndNetMask     = LW_NULL;
    cJSON                *pjVndIpAddr      = LW_NULL;
    cJSON                *pjPublicIp       = LW_NULL;

    INT                   iOperType        = -1;
    cJSON                *pjOperType       = LW_NULL;

    INT                   iVndID           = 0;
    INT                   iMtu             = KV_MTU_DEFAULT;
    INT                   iPort            = KV_PORT_DEFAULT;

    INT                   iTries           = TRY_TIMES;

    __VND_HANDLE          vndHandleInfo;
    CHAR                  cAesKey[70]      = {0};                       /*  ��� 64 �ֽڳ���            */
    CHAR                  cIpBuf[32]       = {0};

    __KIDVPN_HANDLE       kidvpnHandleInfo;
    __AES_KEY_HANDLE      aesKetInfo;
    __SITUATION_INFO      situationInfo;
    __KIDVPN_SERVER_INFO  kidVPNServerInfo;
    __KIDVPN_CLIENT_INFO  kidVPNClientInfo;

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

        goto  __finish;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjOperType   = cJSON_GetObjectItem(pRecvjRes, WEB_OPER_TYPE);
    pjVndIpAddr  = cJSON_GetObjectItem(pRecvjRes, WEB_VND_IPADD);
    pjVndNetMask = cJSON_GetObjectItem(pRecvjRes, WEB_VND_NETMSK);
    if (LW_NULL == pjOperType  ||
        LW_NULL == pjVndIpAddr ||
        LW_NULL == pjVndNetMask) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType    = pjOperType->valueint;
    pcVndIpAddr  = pjVndIpAddr->valuestring;
    pcVndNetMask = pjVndNetMask->valuestring;

    if (LW_NULL == pcVndIpAddr || LW_NULL == pcVndNetMask) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
        goto  __finish;
    }

    /*
     *  ������Ϣ
     */
    bzero(&vndHandleInfo, sizeof(vndHandleInfo));

    inet_aton(pcVndIpAddr,  (struct in_addr *)&(vndHandleInfo.uiIpAddr));
    inet_aton(pcVndNetMask, (struct in_addr *)&(vndHandleInfo.uiNetMask));
    inet_aton("0.0.0.0",    (struct in_addr *)&(vndHandleInfo.uiGateWay));

    vndHandleInfo.iMtu = KV_MTU_DEFAULT;

    bzero(vndHandleInfo.ucHwAddr, sizeof(vndHandleInfo.ucHwAddr));

    iVndID = libVndCreate(&vndHandleInfo);
    if (RET_ARG_ERROR == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
        goto  __finish;
    } else if (RET_HANDLE_FAIL == iVndID) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */
        goto  __finish;
    }

    switch (iOperType) {
    case TYPE_ADD_VPN_S:                                                /*  ���� KidVPN ������          */
        iRet = libKidVPNAesKeyGen(cAesKey, KV_AESKEY_DEFAULT, sizeof(cAesKey));
        if (PX_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish1;
        }

        system("ifconfig dns 0 8.8.8.8");

        pjPublicIp = cJSON_GetObjectItem(pRecvjRes, WEB_PUBLIC_IP);
        if (NULL != pjPublicIp && ('\0' != (pjPublicIp->valuestring)[0])) {
            strncpy(cIpBuf, pjPublicIp->valuestring, sizeof(cIpBuf));
        } else {
            while (iTries --) {
                iRet = __getPublicIp(cIpBuf);
                if (ERROR_NONE == iRet) {
                    break;
                } else {
                    continue;
                }
            }

            if (0 == iTries) {
                cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);      /*  ����ʧ��                    */
                goto  __finish1;
            }
        }

        pjPassWd = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PASSWD);
        if (LW_NULL == pjPassWd) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish1;
        }

        pcPassWd  = pjPassWd->valuestring;

        /*
         *  ���� KidVPN ��Ϣ
         */
        bzero(&kidvpnHandleInfo, sizeof(kidvpnHandleInfo));
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_S;
        kidvpnHandleInfo.iVndID      = iVndID;
        kidvpnHandleInfo.iMtu        = iMtu;
        kidvpnHandleInfo.iPort       = iPort;
        inet_aton(cIpBuf, (struct in_addr *)&(kidvpnHandleInfo.uiIpAddr));
        strncpy(kidvpnHandleInfo.cPassWd, pcPassWd, strlen(pcPassWd));

        /*
         *  ���� AES KEY ��Ϣ
         */
        bzero(&aesKetInfo, sizeof(aesKetInfo));
        aesKetInfo.iVndID = iVndID;
        strncpy(aesKetInfo.cAesKey, cAesKey, strlen(cAesKey));

        iRet = libKidVPNAesKeySave(&aesKetInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish1;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish1;
        }

        /*
         *  ���� Kidvpn
         */
        iRet = libKidVPNCreate(&kidvpnHandleInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish;
        }

        bzero(&kidVPNServerInfo, sizeof(kidVPNServerInfo));
        memcpy(kidVPNServerInfo.cPublicIp, cIpBuf, strlen(cIpBuf));
        memcpy(kidVPNServerInfo.cVndIpAddr, pcVndIpAddr, strlen(pcVndIpAddr));
        memcpy(kidVPNServerInfo.cVndNetMask, pcVndNetMask, strlen(pcVndNetMask));
        memcpy(kidVPNServerInfo.cVndGateway, "0.0.0.0", strlen("0.0.0.0"));
        memcpy(kidVPNServerInfo.cPasswd, pcPassWd, strlen(pcPassWd));
        memcpy(kidVPNServerInfo.cAesKey, cAesKey, strlen(cAesKey));
        kidVPNServerInfo.iVndId = iVndID;

        iRet = libKidVpnServerInfoSave(&kidVPNServerInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish;
        }

        bzero(&situationInfo, sizeof(situationInfo));
        memcpy(situationInfo.ucPosition, POS_SERVER_KIDVPN, strlen(POS_SERVER_KIDVPN));
        memcpy(situationInfo.cStatus, STATUS_CONFIGING, strlen(STATUS_CONFIGING));
        memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
        situationInfo.iFunc = -1;

        iRet = libSituationInfoSave(&situationInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */

            goto  __finish;
        }

        cJSON_AddStringToObject(pSendjRes, WEB_VPN_IPADDR, cIpBuf);
        cJSON_AddStringToObject(pSendjRes, WEB_VND_IPADD, pcVndIpAddr);
        cJSON_AddStringToObject(pSendjRes, WEB_VND_NETMSK, pcVndNetMask);
        cJSON_AddStringToObject(pSendjRes, WEB_VND_GATEWAY, "0.0.0.0");
        cJSON_AddStringToObject(pSendjRes, WEB_VPN_AES_KEY, cAesKey);
        cJSON_AddStringToObject(pSendjRes, WEB_VPN_PASSWD, pcPassWd);

        break;

    case TYPE_ADD_VPN_C:
        pjIpAddr = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_IPADDR);
        pjPassWd = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PASSWD);
        pjAesKey = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
        if (LW_NULL == pjIpAddr || LW_NULL == pjPassWd || LW_NULL == pjAesKey) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish1;
        }

        pcIpAddr  = pjIpAddr->valuestring;
        pcPassWd  = pjPassWd->valuestring;
        pcAesKey  = pjAesKey->valuestring;

        /*
         *  ���� KidVPN ��Ϣ
         */
        bzero(&kidvpnHandleInfo, sizeof(kidvpnHandleInfo));
        kidvpnHandleInfo.cHandleType = TYPE_ADD_VPN_C;
        kidvpnHandleInfo.iVndID      = iVndID;
        kidvpnHandleInfo.iMtu        = iMtu;
        kidvpnHandleInfo.iPort       = iPort;
        inet_aton(pcIpAddr, (struct in_addr *)&(kidvpnHandleInfo.uiIpAddr));
        strncpy(kidvpnHandleInfo.cPassWd, pcPassWd, strlen(pcPassWd));

        /*
         *  ���� AES KEY ��Ϣ
         */
        bzero(&aesKetInfo, sizeof(aesKetInfo));
        aesKetInfo.iVndID = iVndID;
        strncpy(aesKetInfo.cAesKey, pcAesKey, strlen(pcAesKey));
        iRet = libKidVPNAesKeySave(&aesKetInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish1;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish1;
        }

        /*
         *  ���� KidVPN
         */
        iRet = libKidVPNCreate(&kidvpnHandleInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);          /*  ��������                    */
            goto  __finish;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish;
        }

        bzero(&kidVPNClientInfo, sizeof(kidVPNClientInfo));
        memcpy(kidVPNClientInfo.cPublicIp, pcIpAddr, strlen(pcIpAddr));
        memcpy(kidVPNClientInfo.cVndIpAddr, pcVndIpAddr, strlen(pcVndIpAddr));
        memcpy(kidVPNClientInfo.cVndNetMask, pcVndNetMask, strlen(pcVndNetMask));
        memcpy(kidVPNClientInfo.cPasswd, pcPassWd, strlen(pcPassWd));
        memcpy(kidVPNClientInfo.cAesKey, pcAesKey, strlen(pcAesKey));
        kidVPNClientInfo.iVndId = iVndID;

        iRet = libKidVpnClientInfoSave(&kidVPNClientInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto  __finish;
        } else if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish;
        }

        /*
         *  kidvpn ��Ϣ��Ҫ���ݵ��龰ģʽ���ݿ���
         */
        bzero(&situationInfo, sizeof(situationInfo));
        memcpy(situationInfo.ucPosition, POS_CLIENT_KIDVPN, strlen(POS_CLIENT_KIDVPN));
        memcpy(situationInfo.cStatus,    STATUS_CONFIGING, strlen(STATUS_CONFIGING));
        memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
        situationInfo.iFunc = -1;

        iRet = libSituationInfoSave(&situationInfo);
        if (RET_ARG_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

            goto __finish;
        } else if (PX_ERROR == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

            goto __finish;
        }

        break;

    default:
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */
        goto  __finish;

        break;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_VPN_VNDID, iVndID);
    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);                  /*  �����ɹ�                    */

    goto  __finish;

__finish1:
    libVndDel(iVndID);
__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnCancelAction
** ��������: ���� KidVPN ��̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnCancelAction (Webs *wp)
{
    INT                   iRet             = PX_ERROR;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;

    INT                   iOperType        = -1;
    cJSON                *pjOperType       = LW_NULL;

    __SITUATION_INFO      situationInfo;

    if (!isUserLogin(wp)) {                                             /*  �û���Ȩ                    */
        webHeadHandle(wp, HTTP_CODE_UNAUTHORIZED);
        webBackHandle(wp, NULL);

        return;
    }

    webHeadHandle(wp, 200);                                             /*  ���� http ��ͷ              */

    pSendjRes = cJSON_CreateObject();                                   /*  �������� Json ����          */
    if (LW_NULL == pSendjRes) {
        webBackHandle(wp, &_G_ErrorJRes);

        return;
    }

    pRecvjRes = cJSON_Parse(wp->input.buf);                             /*  ����ǰ̨������ Json ��ʽ����*/
    if (LW_NULL == pRecvjRes) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    pjOperType = cJSON_GetObjectItem(pRecvjRes, WEB_OPER_TYPE);         /*  ��ȡ Json ��������          */
    if (LW_NULL == pjOperType) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    iOperType = pjOperType->valueint;

    iRet = libKidvpnCancel(iOperType);
    if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __finish;
    } else if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ����ʧ��                    */

        goto  __finish;
    }

#if 0
    if (TYPE_ADD_VPN_S == iOperType) {
        iRet = libKidVpnServerInfoClean();
        if (RET_HANDLE_FAIL == iRet) {
            cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);          /*  ����ʧ��                    */
            goto  __finish;
        }
    }
#endif

    /*
     *  ״̬��Ϣ���¼���龰ģʽ���ݿ���
     */
    bzero(&situationInfo, sizeof(situationInfo));

    if (TYPE_ADD_VPN_S == iOperType) {
        memcpy(situationInfo.ucPosition, POS_SERVPN_CONFIG, strlen(POS_SERVPN_CONFIG));
    } else if (TYPE_ADD_VPN_C == iOperType) {
        memcpy(situationInfo.ucPosition, POS_CLIVPN_CONFIG, strlen(POS_CLIVPN_CONFIG));
    }

    memcpy(situationInfo.cStatus,    STATUS_CONFIGING, strlen(STATUS_CONFIGING));
    memcpy(situationInfo.ucSituationMode, MODE_MIDDLE_ENT, strlen(MODE_MIDDLE_ENT));
    situationInfo.iFunc = -1;

    iRet = libSituationInfoSave(&situationInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto __finish;
    } else if (PX_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);

        goto __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);                  /*  �����ɹ�                    */

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnConfigSaveAction
** ��������: ���� KidVPN ������Ϣ��̨�ӿ�
** �䡡��  : wp          ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnConfigSaveAction (Webs *wp)
{
    INT                   iRet             = PX_ERROR;
    cJSON                *pRecvjRes        = LW_NULL;
    cJSON                *pSendjRes        = LW_NULL;

    CHAR                 *pcIpAddr         = LW_NULL;
    CHAR                 *pcPassWd         = LW_NULL;
    CHAR                 *pcAesKey         = LW_NULL;

    cJSON                *pjIpAddr         = LW_NULL;
    cJSON                *pjPassWd         = LW_NULL;
    cJSON                *pjAesKey         = LW_NULL;

    CHAR                 *pcVndIpAddr      = LW_NULL;
    CHAR                 *pcVndNetMask     = LW_NULL;
    CHAR                 *pcVndGateway     = LW_NULL;

    cJSON                *pjVndIpAddr      = LW_NULL;
    cJSON                *pjVndNetMask     = LW_NULL;
    cJSON                *pjVndGateway     = LW_NULL;

    __KIDVPN_SERVER_INFO  kidVPNServerInfo;

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

        goto  __finish;
    }

    /*
     *  ��ȡ Json ��������
     */
    pjVndIpAddr  = cJSON_GetObjectItem(pRecvjRes, WEB_VND_IPADD);
    pjVndNetMask = cJSON_GetObjectItem(pRecvjRes, WEB_VND_NETMSK);
    pjVndGateway = cJSON_GetObjectItem(pRecvjRes, WEB_VND_GATEWAY);
    pjIpAddr     = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_IPADDR);
    pjAesKey     = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_AES_KEY);
    pjPassWd     = cJSON_GetObjectItem(pRecvjRes, WEB_VPN_PASSWD);
    if (LW_NULL == pjVndIpAddr  ||
        LW_NULL == pjVndNetMask ||
        LW_NULL == pjVndGateway ||
        LW_NULL == pjIpAddr     ||
        LW_NULL == pjAesKey     ||
        LW_NULL == pjPassWd) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);

        goto  __finish;
    }

    pcVndIpAddr  = pjVndIpAddr->valuestring;
    pcVndNetMask = pjVndNetMask->valuestring;
    pcVndGateway = pjVndGateway->valuestring;
    pcIpAddr     = pjIpAddr->valuestring;
    pcPassWd     = pjPassWd->valuestring;
    pcAesKey     = pjAesKey->valuestring;
    if (LW_NULL == pcVndIpAddr  ||
        LW_NULL == pcVndNetMask ||
        LW_NULL == pcVndGateway ||
        LW_NULL == pcIpAddr     ||
        LW_NULL == pcAesKey     ||
        LW_NULL == pcPassWd) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    }

    bzero(&kidVPNServerInfo, sizeof(kidVPNServerInfo));
    memcpy(kidVPNServerInfo.cPublicIp, pcIpAddr, strlen(pcIpAddr));
    memcpy(kidVPNServerInfo.cVndIpAddr, pcVndIpAddr, strlen(pcVndIpAddr));
    memcpy(kidVPNServerInfo.cVndNetMask, pcVndNetMask, strlen(pcVndNetMask));
    memcpy(kidVPNServerInfo.cVndGateway, pcVndGateway, strlen(pcVndGateway));
    memcpy(kidVPNServerInfo.cPasswd, pcPassWd, strlen(pcPassWd));
    memcpy(kidVPNServerInfo.cAesKey, pcAesKey, strlen(pcAesKey));

    iRet = libVpnServConfInfoSave(&kidVPNServerInfo);
    if (RET_ARG_ERROR == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 100);              /*  ��������                    */

        goto  __finish;
    } else if (RET_HANDLE_FAIL == iRet) {
        cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 500);              /*  ����ʧ��                    */

        goto  __finish;
    }

    cJSON_AddNumberToObject(pSendjRes, WEB_CODE, 200);                  /*  �����ɹ�                    */

__finish:
    webBackHandle(wp, pSendjRes);
    cJSON_Delete(pSendjRes);
    cJSON_Delete(pRecvjRes);
}
/*********************************************************************************************************
** ��������: __kidvpnCliStatGetAction
** ��������: ��ȡ KidVPN �ͻ�������״̬��Ϣ��̨�ӿ�
** �䡡��  : wp                 ���������
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnCliStatGetAction (Webs *wp)
{
    cJSON*                pjSend          = LW_NULL;

    PCHAR                 pcKidVPNBuf     = LW_NULL;
    PCHAR                 pcKidVPNBufTmp  = LW_NULL;
    __PKIDVPN_DB_INFO     pcKidVPNInfo    = LW_NULL;
    INT                   iKidVpnNum      = 0;
    INT                   i;

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

    /*
     *  ��ȡ VPN ��Ϣ
     */
    pcKidVPNBuf = libKidVPNGetInfo(&iKidVpnNum);
    if (LW_NULL == pcKidVPNBuf) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    pcKidVPNBufTmp = pcKidVPNBuf;
    pcKidVPNInfo   = (__PKIDVPN_DB_INFO)pcKidVPNBuf;
    for (i = 0; i < iKidVpnNum; i++) {
        if (TYPE_ADD_VPN_C == pcKidVPNInfo->kidvpnHandle.cHandleType) {
            cJSON_AddStringToObject(pjSend,
                                    WEB_VPN_STATUS,
                                    pcKidVPNInfo->kidvpnHandle.cStatus ? "ON" : "OFF");

            break;
        }

        pcKidVPNBuf += sizeof(__KIDVPN_DB_INFO);
        pcKidVPNInfo = (__PKIDVPN_DB_INFO)(pcKidVPNBuf);
    }

    if (i == iKidVpnNum) {
        cJSON_AddNumberToObject(pjSend, WEB_CODE, 500);

        goto  __error;
    }

    cJSON_AddNumberToObject(pjSend, WEB_CODE, 200);

__error:
    if (NULL != pcKidVPNBufTmp) {
        free(pcKidVPNBufTmp);
    }

    webBackHandle(wp, pjSend);
    cJSON_Delete(pjSend);
}
/*********************************************************************************************************
** ��������: __kidvpnConfigActionDefine
** ��������: VPN ��̨�ӿڶ���
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
static VOID  __kidvpnConfigActionDefine (VOID)
{
    websDefineAction("kidvpnConfigAction",     __kidvpnConfigAction);
    websDefineAction("kidvpnCancelAction",     __kidvpnCancelAction);
    websDefineAction("kidvpnConfigSaveAction", __kidvpnConfigSaveAction);
    websDefineAction("kidvpnCliStatGetAction", __kidvpnCliStatGetAction);
}
/*********************************************************************************************************
** ��������: kidvpnConfigActionInit
** ��������: VPN ��̨�ӿڳ�ʼ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  kidvpnConfigActionInit (VOID)
{
    __kidvpnConfigActionDefine();
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
