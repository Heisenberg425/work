#include <stdio.h>
#include "libcomm.h"
#include "webHandle.h"
#include "goahead.h"
#include "net/if.h"

/*********************************************************************************************************
** ��������: goActionInterfaceInit
** ��������: Web goaction �ӿ�ʹ����Ҫ����һЩ��ʼ������
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID goActionInterfaceInit (VOID)
{
    FILE          *pFp;
    size_t         sSize;

    WebsSocket    *sp;
    INT            i;
    struct ifreq   ifreq;
    CHAR           cNetif[IFNAMSIZ] = {0};

    extern int     listens[WEBS_MAX_LISTEN];
    extern int     listenMax;

    extern VOID interfaceActionInit (VOID);
    extern VOID intAdaActionInit (VOID);
    extern VOID natActionInit (VOID);
    extern VOID vndActionInit (VOID);
    extern VOID kidvpnActionInit (VOID);
    extern VOID udialActionInit (VOID);
    extern VOID routeActionInit (VOID);
    extern VOID vlanActionInit (VOID);
    extern VOID flowctlActionInit (VOID);
    extern VOID arpActionInit (VOID);
    extern VOID dhcpActionInit (VOID);
    extern VOID npfActionInit (VOID);
    extern VOID dnsActionInit (VOID);
    extern VOID logActionInit (VOID);
    extern VOID monitorActionInit (VOID);
    extern VOID loginActionInit (VOID);
    extern VOID timeActionInit (VOID);
    extern VOID powerActionInit (VOID);
    extern VOID devNameActionInit (VOID);
    extern VOID pingActionInit (VOID);
    extern VOID hwActionInit (VOID);
    extern VOID userAuthActionInit (VOID);
    extern VOID dhcpServerActionInit (VOID);
    extern VOID updateFirmwareActionInit (VOID);
    extern VOID versionActionInit(VOID);
    extern VOID qosActionInit (VOID);
    extern VOID publicNetActionInit(VOID);
    extern VOID localNetActionInit(VOID);
    extern VOID kidvpnConfigActionInit(VOID);
    extern VOID arpConfigActionInit(VOID);
    extern VOID configureCompletActionInit(VOID);
    extern VOID webBindToNetifActionInit(VOID);
    extern VOID usrScriptActionInit(VOID);


    initCliCommFd();
    cJsonErrorReturnInit();

    /*
     *  ��ģ�����ݳ�ʼ��
     */
    natActionInit();
    intAdaActionInit();
    interfaceActionInit();
    udialActionInit();
    vndActionInit();
    kidvpnActionInit();
    routeActionInit();
    vlanActionInit();
    flowctlActionInit();
    arpActionInit();
    dhcpActionInit();
    npfActionInit();
    dnsActionInit();
    logActionInit();
    monitorActionInit();
    loginActionInit();
    timeActionInit();
    powerActionInit();
    devNameActionInit();
    pingActionInit();
    hwActionInit();
    dhcpServerActionInit();
    updateFirmwareActionInit();
    versionActionInit();
    qosActionInit();
    userAuthActionInit();
    usrScriptActionInit();
    publicNetActionInit();
    localNetActionInit();
    kidvpnConfigActionInit();
    arpConfigActionInit();
    configureCompletActionInit();
    webBindToNetifActionInit();

    pFp = fopen("/etc/webNetifConf.ini", "r");
    if (NULL != pFp) {
        sSize = fread(cNetif, 1, IFNAMSIZ, pFp);
        if (sSize <= 0) {
            fclose(pFp);

            return;
        }
    } else {
        return;
    }

    lib_strlcpy(ifreq.ifr_name, cNetif, IFNAMSIZ);                      /*  ��ָ�����豸              */

    for (i = 0; i < listenMax; i++) {
        if ((sp = socketPtr(listens[i])) == NULL) {
            return;
        }

        setsockopt(sp->sock , SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq));
    }

    if (NULL != pFp) {
        fclose(pFp);
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
