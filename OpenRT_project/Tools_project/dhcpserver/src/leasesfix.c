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
** ��   ��   ��: leasesfix.c
**
** ��   ��   ��: Administrator
**
** �ļ���������: 2017��3��8��
**
** ��        ��: �޸Ĺ�����Լ����ģ�飬����ظ��������������
*********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <pthread.h>

#include "log.h"
#include "configini.h"
#include "packet.h"
#include "sockets.h"
#include "leasesfix.h"

#define HAVESRCFILE                     1
#define HAVEBAKFILE                     2

static LEASEMANGER  GleaseManger;                                       /* ��Լ����ȫ�ֽṹ��           */

/*********************************************************************************************************
** ��������: leaseLock
** ��������: �����Լ������
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static VOID  leaseLock (VOID)
{
    pthread_mutex_lock(&(GleaseManger.leasemutexLock));
}

/*********************************************************************************************************
** ��������: leaseUnlock
** ��������: �ͷ���Լ������
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static VOID  leaseUnlock (VOID)
{
    pthread_mutex_unlock(&(GleaseManger.leasemutexLock));
}

/*********************************************************************************************************
** ��������: leaseNotExpired
** ��������: �жϵ�ǰ��Լ�Ƿ���
** ��    ��: INT  iIndex
** ��    ��: ���ڷ���PX_ERROR�����򷵻�iIndex(iIndex����Լ�����±귶Χ֮��)
** ����ģ��: API
*********************************************************************************************************/
static INT  leaseNotExpired (INT  iIndex)
{
    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* ��Ч�Լ��                   */
        if (GleaseManger.pdhcplease[iIndex].uiExpires < time(0)) {      /* ��Լʱ��С�ڵ�ǰʱ�伴Ϊ���� */
            iIndex = PX_ERROR;
        }
    } else {
        iIndex = PX_ERROR;
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: leaseOfferSet
** ��������: ����Լ��Ϊ��ʱ�ṩ״̬
** ��    ��: pcChaddr  Ӳ����ַ
**           iIndex    ��Լ����
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseOfferSet (PUCHAR  pcChaddr, INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* ��Ч�Լ��                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ���Ӳ����ַ                 */
                   0,
                   16);
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ����Ӳ����ַ                 */
                   pcChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState = LEASEOFFER;        /* ������ʱ��״̬�͵ȴ�ʱ��     */
            GleaseManger.pdhcplease[iIndex].uiExpires = GserverConfig.ulOfferWaite + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseOccupySet
** ��������: ����Լ��Ϊ���ṩ״̬
** ��    ��: pcChaddr  Ӳ����ַ
**           iIndex    ��Լ����
**           uiLease   ��Լʱ��
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseOccupySet (PUCHAR  pcChaddr, INT iIndex, UINT uiLease)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* ��Ч�Լ��                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ���Ӳ����ַ                 */
                   0,
                   16);
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ����Ӳ����ַ                 */
                   pcChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = LEASEOCCUPY;    /* �������ṩ״̬����Լʱ��     */
            GleaseManger.pdhcplease[iIndex].uiExpires = uiLease + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseConflictSet
** ��������: ����Լ��Ϊ��ַ��ͻ״̬
** ��    ��: iIndex  ��Լ����
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseConflictSet (INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* �ж���Ч��                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            memset(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ���Ӳ����ַ                 */
                   0,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = LEASECONFLICT;  /* ���ó�ͻ״̬���ó�ͻ���ʱ�� */
            GleaseManger.pdhcplease[iIndex].uiExpires = GserverConfig.ulLease + time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseTackSet
** ��������: ����Լ��Ϊ��ʱ��ȡ״̬
** ��    ��: pcChaddr   Ӳ����ַ
**           iIndex     ��Լ����
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseTackSet (PUCHAR  pcChaddr, INT  iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* �ж���Ч��                   */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            if (GleaseManger.pdhcplease[iIndex].iState == LEASEFREE) {
                memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,        /* ��ֵӲ����ַ                 */
                       pcChaddr,
                       16);
                GleaseManger.pdhcplease[iIndex].iState = LEASETACKED;   /* ����Ϊ��ʱ��ȡ״̬           */
                iRet = ERROR_NONE;
            } else if (GleaseManger.pdhcplease[iIndex].iState == LEASETACKED ||
                       GleaseManger.pdhcplease[iIndex].iState == LEASEOFFER  ||
                       GleaseManger.pdhcplease[iIndex].iState == LEASEOCCUPY) {
                iRet = ERROR_NONE;                                      /* �Ѿ�����ȡ״̬�Ĳ����ظ����� */
            }
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseFreeSet
** ��������: �ͷ���Լ
** ��    ��: iIndex  ��Լ����
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseFreeSet (INT iIndex)
{
    INT  iRet = PX_ERROR;

    leaseLock();

    if (iIndex >= 0 && iIndex < MAXLEASES) {                            /* �ж���Ч��                  */
        if (GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
            GleaseManger.pdhcplease[iIndex].iState    = LEASEFREE;      /* ����Ϊ����״̬              */
            GleaseManger.pdhcplease[iIndex].uiExpires = time(0);
            iRet = ERROR_NONE;
        }
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: freeLeaseFind
** ��������: ���ҿ�����Լ
** ��    ��: NONE
** ��    ��: �ɹ����������Լ�����±�, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  freeLeaseFind (VOID)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    for (i = 0; i < MAXLEASES; i++) {                                   /* ���������ҵ�һ�����е�ַ     */
        if (leaseNotExpired(i) == PX_ERROR &&                           /* �жϳ���,������Ϊ����        */
            GleaseManger.pdhcplease[i].uiYiaddr != 0 &&
            GleaseManger.pdhcplease[i].iState != LEASETACKED) {
            GleaseManger.pdhcplease[i].iState = LEASEFREE;
            iIndex = i;
        }

        if (GleaseManger.pdhcplease[i].iState   == LEASEFREE &&
            GleaseManger.pdhcplease[i].uiYiaddr != 0) {
            iIndex = i;
            break;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: leaseFindbyYiaddr
** ��������: ͨ��IP��ַ���������Լ
** ��    ��: uiYiaddr   ip��ַ
** ��    ��: �ɹ����������Լ�����±�, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  leaseFindbyYiaddr (UINT  uiYiaddr)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    /*
     * ������Լ�����ip��ַƥ�����Լ��������Index
     */
    for (i = 0; i < MAXLEASES; i++) {
        if (GleaseManger.pdhcplease[i].uiYiaddr == uiYiaddr &&
            GleaseManger.pdhcplease[i].uiYiaddr != 0) {
            iIndex = i;
            break;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: leaseFindbyChaddr
** ��������: ͨ��Ӳ����ַ���������Լ
** ��    ��: pcChaddr  Ӳ����ַ
** ��    ��: �ɹ����������Լ�����±�, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  leaseFindbyChaddr (PUCHAR  pcChaddr)
{
    INT  i;
    INT  iIndex = PX_ERROR;

    leaseLock();

    /*
     * ������Լ�����Ӳ����ַƥ�����Լ��������Index
     */
    if (pcChaddr) {
        for (i = 0; i < MAXLEASES; i++) {
            if (memcmp(GleaseManger.pdhcplease[i].ucChaddr, pcChaddr, 16) == 0 &&
                GleaseManger.pdhcplease[i].uiYiaddr != 0) {
                iIndex = i;
                break;
            }
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: clinetInLease
** ��������: �ж��Ƿ��������޿ͻ���
** ��    ��: PDHCPMSG  pdhcpmsgPacket
** ��    ��: �ɹ����������Լ�����±�, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  clinetInLease (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* ����Ӳ����ַ�Ƿ������Լ���� */
    if (iIndex >= 0) {                                                  /* �������                     */
        if (GleaseManger.pdhcplease[iIndex].iState == LEASECONFLICT) {  /* �ж��Ƿ��ǳ�ͻ��ַ           */
            iIndex = PX_ERROR;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: addisFree
** ��������: �ж���ַ�Ƿ��ǿ��е�
** ��    ��: uiYiaddr  ��Ӧ��ip��ַ
** ��    ��: �ɹ����������Լ�����±�, ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  addisFree (INT  iIndex)
{
    leaseLock();

    if (iIndex >= 0) {                                                  /* �������                     */
        if ((GleaseManger.pdhcplease[iIndex].iState == LEASEFREE ||
            leaseNotExpired(iIndex) == PX_ERROR) &&
            GleaseManger.pdhcplease[iIndex].uiYiaddr != 0 &&
            GleaseManger.pdhcplease[iIndex].iState != LEASETACKED) {
            GleaseManger.pdhcplease[iIndex].iState = LEASEFREE;
        } else {
            iIndex = PX_ERROR;
        }
    }

    leaseUnlock();

    return (iIndex);
}

/*********************************************************************************************************
** ��������: leasaFileRead
** ��������: ����Լ�ļ���ȡ�ѱ�����Լ
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leasaFileRead (FILE  *pLeasefile)
{
    INT                i = 0;
    INT                iIndex;
    struct  dhcplease  dhcpleaseread;

    memset(&dhcpleaseread, 0, sizeof(dhcpleaseread));
    fseek(pLeasefile, sizeof(INT64), SEEK_SET);
    while (i < MAXLEASES &&                                             /* ��ȡ��Լ���ݵ��ṹ��         */
           (fread(&dhcpleaseread, sizeof(dhcpleaseread), 1, pLeasefile) == 1)) {
        iIndex = leaseFindbyYiaddr(dhcpleaseread.uiYiaddr);             /* ���Ҷ�ȡIP��ַ�Ƿ�����Լ���� */
        if (iIndex >= 0) {                                              /* �������Լ����               */
            memcpy(GleaseManger.pdhcplease[iIndex].ucChaddr,            /* ����ȡ�����ݸ�ֵ����Ӧ��Լ�� */
                   dhcpleaseread.ucChaddr,
                   16);
            GleaseManger.pdhcplease[iIndex].iState    = dhcpleaseread.iState;
            GleaseManger.pdhcplease[iIndex].uiExpires = dhcpleaseread.uiExpires;
            memset(&dhcpleaseread, 0, sizeof(dhcpleaseread));
        } else {                                                        /* �����ȡ�ĵ�ַ������Լ����   */
            for (i = 0;i < MAXLEASES; i++) {                            /* ��ʼ����Լ������             */
                memset(GleaseManger.pdhcplease[i].ucChaddr, 0, 16);
                GleaseManger.pdhcplease[i].iState   = LEASEFREE;        /* ȫ����Լ��Ϊ����״̬         */
             }
            break;
        }
        i++;
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: leaseFileWrite
** ��������: ����ǰ��Լд����Լ�ļ�
** ��    ��: pLeasefile   ��Լ�ļ�������
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseFileWrite (FILE  *pLeasefile, struct dhcplease  *pdhcplease)
{
    INT     i;
    INT64   iFileMark = time(0);

    fwrite(&iFileMark, 8, 1, pLeasefile);                               /* д�뿪ͷУ��ֵ               */

    for (i = 0; i < MAXLEASES; i++) {                                   /* ����Լ����д����Լ�ļ�       */
        if (pdhcplease[i].uiYiaddr != 0) {                              /* �ж���Լ��Ч��               */
            fwrite(pdhcplease[i].ucChaddr,                              /* ����ԼӲ����ַд���ļ�       */
                   16,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].uiYiaddr,                             /* ����Լip��ַд���ļ�         */
                   4,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].uiExpires,                            /* ����Լʱ��д���ļ�           */
                   4,
                   1,
                   pLeasefile);
            fwrite(&pdhcplease[i].iState,                               /* ����Լ״̬д���ļ�           */
                   4,
                   1,
                   pLeasefile);
        } else {
            break;
        }
    }

    fwrite(&iFileMark, 8, 1, pLeasefile);                               /* д��ĩβУ��ֵ               */

    sync();

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: fileCheck
** ��������: ����ļ���Ч��
** ��    ��: pLeasefile   ��Լ�ļ�������
**           pLen         �ļ���С
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  fileCheck (FILE  *pLeasefile, LONG  *pLen)
{
    INT    iRet;
    INT64  iFileMarkStart = 0;
    INT64  iFileMarkEnd   = 0;

    fseek(pLeasefile, 0, SEEK_END);
    *pLen = ftell(pLeasefile);                                          /* ��ȡ�ļ���ǰ��С             */
    fseek(pLeasefile, 0, SEEK_SET);                                     /* �ļ���ȡָ�붨λ���ļ�ͷ     */
    if (*pLen == 0) {
        return (ERROR_NONE);
    }

    iRet = fread(&iFileMarkStart,                                       /* ��ȡ��У��ֵ                 */
                 8,
                 1,
                 pLeasefile);
    if (iRet == 1) {
        fseek(pLeasefile, 0 - sizeof(iFileMarkEnd), SEEK_END);          /* �Ƶ�ĩβУ��ֵ���׵�ַ       */
        iRet = fread(&iFileMarkEnd,                                     /* ��ȡβУ��ֵ                 */
                     sizeof(iFileMarkEnd),
                     1,
                     pLeasefile);
        if (iRet == 1) {                                                /* ȷ�϶�ȡ��ȷ                 */
            if (iFileMarkStart == iFileMarkEnd) {                       /* У���ļ�������               */
                return (ERROR_NONE);
            }
        }
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: precentFile
** ��������: �����µ���Ч�ļ�
** ��    ��: pLen         �ļ���С
** ��    ��: �ɹ����ض�Ӧ�ļ�������, ʧ�ܷ���NULL
** ����ģ��: NONE
*********************************************************************************************************/
static FILE  *precentFile (LONG  *pLen)
{
    FILE   *pLeasefilesrc     = NULL;
    FILE   *pLeasefilebak     = NULL;
    INT64   iFileMarkStartSrc = 0;
    INT64   iFileMarkStartBak = 0;

    pLeasefilesrc = fopen(GserverConfig.pcLeasefile, "r");              /* ����ԼԴ�ļ�               */
    pLeasefilebak = fopen(GserverConfig.pcLeasefilebak, "r");           /* ����Լ�����ļ�             */

    if (pLeasefilesrc && pLeasefilebak) {                               /* ��������ļ����ܴ�         */
        if (fileCheck(pLeasefilesrc, pLen) == ERROR_NONE &&             /* ��������ļ�����δ�𻵵�     */
            fileCheck(pLeasefilebak, pLen) == ERROR_NONE) {
            fread(&iFileMarkStartSrc, sizeof(iFileMarkStartSrc), 1, pLeasefilesrc);
            fread(&iFileMarkStartBak, sizeof(iFileMarkStartBak), 1, pLeasefilebak);
            if (iFileMarkStartBak > iFileMarkStartSrc) {                /* �Ƚ�У��ֵ��ѡ�����µ��ļ�   */
                fclose(pLeasefilesrc);                                  /* �ر���һ���ļ�������         */
                return (pLeasefilebak);                                 /* �������µ��ļ���������       */
            } else {
                fclose(pLeasefilebak);                                  /* �ر���һ���ļ�������         */
                return (pLeasefilesrc);                                 /* �������µ��ļ���������       */
            }
        } else if (fileCheck(pLeasefilesrc, pLen) == ERROR_NONE &&      /* ���ֻ��Դ�ļ�����           */
                   fileCheck(pLeasefilebak, pLen) != ERROR_NONE) {
            fclose(pLeasefilebak);                                      /* �رձ����ļ�������           */
            return (pLeasefilesrc);                                     /* ����Դ�ļ�������             */
        } else if (fileCheck(pLeasefilesrc, pLen) != ERROR_NONE &&      /* ���ֻ�б����ļ�����         */
                   fileCheck(pLeasefilebak, pLen) == ERROR_NONE) {
            fclose(pLeasefilesrc);                                      /* �ر�Դ�ļ�������             */
            return (pLeasefilebak);                                     /* ���ر����ļ�������           */
        }
    } else if (!pLeasefilebak && pLeasefilesrc ) {                      /* ���ֻ��Դ�ļ��ܴ�         */
        if (fileCheck(pLeasefilesrc, pLen)) {                           /* ���Դ�ļ�������             */
            return (pLeasefilesrc);                                     /* �������������������         */
        }
    } else if (!pLeasefilesrc && pLeasefilebak) {                       /* ���ֻ�б����ļ��ܴ�       */
        if (fileCheck(pLeasefilebak, pLen)) {                           /* ��鱸���ļ�������           */
            return (pLeasefilebak);                                     /* �������������������         */
        }
    }

    return (NULL);
}
/*********************************************************************************************************
** ��������: leaseReadWithProtect
** ��������: ����Լ�ļ���ȡ�ѱ�����Լ(�����籣������)
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  leaseReadWithProtect (VOID)
{
    FILE   *pLeasefile = NULL;
    INT     iState     = 0;
    INT     iRet;
    LONG    lLen;

    iRet = access(GserverConfig.pcLeasefile, F_OK);                     /* ���Դ�ļ��Ƿ����           */
    if (iRet >=  0) {
        iState = iState + HAVESRCFILE;
    }

    iRet = access(GserverConfig.pcLeasefilebak, F_OK);                  /* ��鱸���ļ��Ƿ����         */
    if (iRet >=  0) {
        iState = iState + HAVEBAKFILE;
    }

    switch (iState) {                                                   /* �ж��ļ�״̬                 */

    case 0:
        fopen(GserverConfig.pcLeasefile, "w");                          /* ��Լ�ļ������ڣ��򴴽�һ��   */
        return (ERROR_NONE);
        break;

    case HAVESRCFILE:                                                   /* ֻ����Դ�ļ�                 */
        pLeasefile = fopen(GserverConfig.pcLeasefile, "r");             /* ����ԼԴ�ļ�               */
        if (!pLeasefile) {                                              /* �����ʧ���򷵻�ʧ��       */
            LOG(LOG_ERR, "Unable to open %s for reading\r\n", GserverConfig.pcLeasefile);
            return (PX_ERROR);
        }

        if (fileCheck(pLeasefile, &lLen) == ERROR_NONE) {               /* ���Դ�ļ�������             */
            if (lLen) {
                if (leasaFileRead(pLeasefile) == ERROR_NONE) {          /* ��Դ�ļ���Ϊ�գ���ȡ������   */
                    fclose(pLeasefile);
                    return (ERROR_NONE);
                }

            } else {
                fclose(pLeasefile);
                return (ERROR_NONE);                                    /* Ϊ����ֱ�ӷ���ERROR_NONE     */
            }

        } else {
            fclose(pLeasefile);
            return (PX_ERROR);
        }
        break;

    case HAVEBAKFILE:                                                   /* ���ֻ���ڱ����ļ�           */
        pLeasefile = fopen(GserverConfig.pcLeasefilebak, "r");          /* ����Լ�����ļ�             */
        if (!pLeasefile) {                                              /* �����ʧ���򷵻�ʧ��       */
            LOG(LOG_ERR, "Unable to open %s for reading\r\n", GserverConfig.pcLeasefilebak);
            return (PX_ERROR);
        }

        if (fileCheck(pLeasefile, &lLen) == ERROR_NONE) {               /* ��鱸���ļ�������           */
            if (lLen) {
                if (leasaFileRead(pLeasefile) == ERROR_NONE) {          /* ���ļ���Ϊ�����ȡ������     */
                    fclose(pLeasefile);
                    return (ERROR_NONE);
                }

            } else {
                fclose(pLeasefile);
                return (ERROR_NONE);                                    /* Ϊ����ֱ�ӷ���ERROR_NONE     */
            }

        } else {
            fclose(pLeasefile);
            return (PX_ERROR);
        }
        break;

    case HAVESRCFILE + HAVEBAKFILE:                                     /* ��������ļ�������           */
        pLeasefile = precentFile(&lLen);                                /* ��ȡ���µ��ļ�������         */
        if (!pLeasefile) {                                              /* ��ȡʧ�ܷ���PX_ERROR         */
            LOG(LOG_ERR, "Unable to open leasefile for reading\r\n");
            return (PX_ERROR);
        }

        if (lLen) {                                                     /* �ж��ļ��Ƿ�Ϊ��             */
            if (leasaFileRead(pLeasefile) == ERROR_NONE) {              /* ��Ϊ�����ȡ������           */
                fclose(pLeasefile);
                return (ERROR_NONE);
            }

        } else {
            fclose(pLeasefile);
            return (ERROR_NONE);                                        /* Ϊ����ֱ�ӷ���               */
        }
        break;

    default:
        break;
    }

    return (PX_ERROR);
}

/*********************************************************************************************************
** ��������: leaseWriteWithProtect
** ��������: ����ǰ��Լд����Լ�ļ�(�����籣������)
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  leaseWriteWithProtect (VOID)
{
    FILE   *pLeasefile    = NULL;
    static struct dhcplease  pdhcplease[MAXLEASES];

    /*
     *  ������ʱ�ռ䣬����lease����
     */
//    pdhcplease = (struct dhcplease *)malloc(sizeof(struct dhcplease) * MAXLEASES);
    memset(pdhcplease, 0 , sizeof(struct dhcplease)* MAXLEASES);
    leaseLock();
    memcpy(pdhcplease, GleaseManger.pdhcplease, sizeof(struct dhcplease)* MAXLEASES);
    leaseUnlock();

    pLeasefile = fopen(GserverConfig.pcLeasefile, "w");                 /* ��ո��ǣ����ļ�           */
    if (!pLeasefile) {
        LOG(LOG_ERR, "Unable to open %s\r\n", GserverConfig.pcLeasefile);
        return (PX_ERROR);
    }

    if (leaseFileWrite(pLeasefile, pdhcplease) == PX_ERROR) {           /* ��Լд��Դ�ļ�               */
        fclose(pLeasefile);
        return (PX_ERROR);
    }

    fclose(pLeasefile);

    pLeasefile = fopen(GserverConfig.pcLeasefilebak, "w");              /* ��ո��ǣ����ļ�           */
    if (!pLeasefile) {
        LOG(LOG_ERR, "Unable to open %s\r\n", GserverConfig.pcLeasefile);
        return (PX_ERROR);
    }

    if (leaseFileWrite(pLeasefile, pdhcplease) == PX_ERROR) {           /* ��Լд�뱸���ļ�             */
        fclose(pLeasefile);
        return (PX_ERROR);
    }

    fclose(pLeasefile);                                                 /* д��ر��ļ�������           */

    //free(pdhcplease);                                                   /* �ͷ���ʱ�ռ�                 */

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: leaseMangerInit
** ��������: ��Լ�����ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  leaseMangerInit (VOID)
{
    UINT  uiAddr = ntohl(GserverConfig.uiStart);
    INT   i      = 0;
    INT   iRet;

    pthread_mutexattr_t  mutexattr;

    GleaseManger.pdhcplease = (struct dhcplease *)malloc(sizeof(struct dhcplease) * MAXLEASES);
    if (NULL == GleaseManger.pdhcplease) {
        LOG(LOG_ERR, "leaseMangerInit malloc fail\r\n");
        return (PX_ERROR);
    }

    memset(GleaseManger.pdhcplease, 0 , sizeof(struct dhcplease)* MAXLEASES);

    for (;uiAddr <= ntohl(GserverConfig.uiEnd); uiAddr++) {             /* ��ʼ����Լ������             */
        if (!(uiAddr & 0xFF) || (uiAddr & 0xFF) == 0xFF) {              /* ����0��255��ַ               */
            continue;
        }

        GleaseManger.pdhcplease[i].uiYiaddr = htonl(uiAddr);            /* Ԥ����ַ��ȫ�ֽṹ����     */
        GleaseManger.pdhcplease[i].iState   = LEASEFREE;                /* ȫ����Լ��Ϊ����״̬         */
        i++;
     }

    pthread_mutexattr_init(&mutexattr);                                 /* ��ʼ����������               */
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    iRet = pthread_mutex_init(&(GleaseManger.leasemutexLock), &mutexattr);
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Mutex init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = leaseReadWithProtect();                                      /* ��ȡ��Լ�ļ����ݵ�ϵͳ       */

    return (iRet);
}

/*********************************************************************************************************
** ��������: freeOfferGet
** ��������: ��ȡһ��������Լ
** ��    ��: pdhcpmsgPacket       dhcp���ݰ�
**           pdhcpmsgSendPacket   Ҫ���͵����ݰ�
** ��    ��: �ɹ�����ERROR_NONE, ��ʱ��ȡʧ�ܷ���TAKEERROR����Լ��ʱ����ʧ�ܷ���OFFERERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  freeOfferGet (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    INT                iIndex           = PX_ERROR;
    INT                iRet             = PX_ERROR;
    PUCHAR             pcRequestLeases  = NULL;
    PUCHAR             pcRequestIp      = NULL;
    struct optionset  *poptionset       = NULL;
    UINT               uiLeaseAgain     = (UINT)GserverConfig.ulLease;

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* ��ȡ�ͻ��������ip           */
    if (pcRequestIp) {                                                  /* ���������ip                 */
        memcpy(&(pdhcpmsgSendPacket->uiYiaddr), pcRequestIp, 4);        /* ��ֵ                         */
    }

    leaseLock();                                                        /* ����                         */

    iIndex = clinetInLease(pdhcpmsgPacket);                             /* �ж��Ƿ����������           */
    if (iIndex < 0) {
        if (pcRequestIp) {
            iIndex = leaseFindbyYiaddr(pdhcpmsgSendPacket->uiYiaddr);   /* �ж��Ƿ��ж�ӦIP             */
            if (iIndex >= 0) {
                iIndex = addisFree(iIndex);                             /* �ж�����IP��Լ�Ƿ����       */
                if (iIndex < 0) {
                    iIndex = freeLeaseFind();                           /* ���ҿ�����Լ                 */
                }
            } else {
                iIndex = freeLeaseFind();                               /* ���ҿ�����Լ                 */
            }
        } else {
            iIndex = freeLeaseFind();
        }
    }

    if (iIndex < 0) {                                                   /* ���û�е�ַ���Է���         */
        iRet = PX_ERROR;
        LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned\r\n");
    } else {
        iRet = leaseTackSet(pdhcpmsgPacket->ucChaddr, iIndex);          /* ������ҵ���Լ����ʱ��ȡ     */
    }

    leaseUnlock();

    if (iRet >= 0) {                                                    /* ���ip�Ƿ�ռ��             */
        if (ipConflictCheck(GleaseManger.pdhcplease[iIndex].uiYiaddr) == ERROR_NONE) {
            leaseConflictSet(iIndex);                                   /* ����ռ���������Լ��ͻ       */
            iRet = IPCHECKFAILE;                                        /* �÷���ֵ                     */
        } else {                                                        /* ����Լ��Ϊ��ʱ��ȡ           */
            if (leaseOfferSet(pdhcpmsgSendPacket->ucChaddr, iIndex) == ERROR_NONE) {
                pdhcpmsgSendPacket->uiYiaddr = GleaseManger.pdhcplease[iIndex].uiYiaddr;

                pcRequestLeases = poptionGet(pdhcpmsgPacket, DHCP_LEASE_TIME);
                if (pcRequestLeases) {                                  /* ������Լʱ��                 */
                    memcpy(&uiLeaseAgain, pcRequestLeases, 4);
                    uiLeaseAgain = (UINT)ntohl(uiLeaseAgain);
                    if (uiLeaseAgain > GserverConfig.ulLease) {
                        uiLeaseAgain = GserverConfig.ulLease;
                    }
                }
                optionsmpAdd(pdhcpmsgSendPacket->ucOptions,             /* �����Լʱ��ѡ��             */
                             DHCP_LEASE_TIME,
                             htonl(uiLeaseAgain));

                poptionset = GserverConfig.poptionsets;
                while (poptionset) {                                    /* �����ȫ������ѡ����Ͱ�   */
                    if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
                        optionstrAdd((PUCHAR)(pdhcpmsgSendPacket->ucOptions),
                                     (PUCHAR)(poptionset->pcData));
                    }
                    poptionset = poptionset->optionsetNext;
                }
                iRet = ERROR_NONE;                                      /* ��ʱ���޳ɹ�                 */
            } else {
                iRet = OFFERERROR;                                      /* ��ʱ����ʧ��                 */
            }

        }
    } else {
        iRet = TAKEERROR;                                               /* ��ʱ��ȡ��Լʧ��             */
    }

    return (iRet);
}

/*********************************************************************************************************
** ��������: discoverOfferGet
** ��������: discover���̻�ȡ��ԼOffer
** ��    ��: pdhcpmsgPacket      dhcp���ݰ�
**           pdhcpmsgSendPacket  Ҫ���͵����ݰ�
**           iSocket             ICMP�õ��׽���
** ��    ��: ������Լ����״̬
** ����ģ��: API
*********************************************************************************************************/
INT  discoverOfferGet (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    INT  iRet;

    do {
        iRet = freeOfferGet(pdhcpmsgPacket, pdhcpmsgSendPacket);/* ��ȡ������Լ                */
    }while(iRet == IPCHECKFAILE);

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseAck
** ��������: reques������Ӧ
** ��    ��: pdhcpmsgPacket       dhcp���ݰ�
**           pdhcpmsgSendPacket   Ҫ���͵����ݰ�
** ��    ��: ������Լ����״̬
** ����ģ��: API
*********************************************************************************************************/
INT  leaseAck (PDHCPMSG  pdhcpmsgPacket, PDHCPMSG  pdhcpmsgSendPacket)
{
    PUCHAR             pcRequestIp      = NULL;
    PUCHAR             pcServerIp       = NULL;
    struct optionset  *poptionset       = NULL;
    INT                iIndex           = PX_ERROR;
    UINT               uiServerIpAgain  = 0;
    UINT               uiLeaseAgain     = GserverConfig.ulLease;
    PUCHAR             pcRequestLeases  = NULL;
    INT                iRet             = ERROR_NONE;

    pcRequestIp      = poptionGet(pdhcpmsgPacket, DHCP_REQUESTED_IP);   /* ��ȡ�ͻ��������ip           */
    pcServerIp       = poptionGet(pdhcpmsgPacket, DHCP_SERVER_ID);      /* ��ȡ������ip                 */

    if (pcRequestIp) {                                                  /* ���������ip                 */
        memcpy(&(pdhcpmsgSendPacket->uiYiaddr), pcRequestIp, 4);        /* ��ֵ                         */
    }

    if (pcServerIp) {                                                   /* ������ݰ����з�����ip��ַ   */
        memcpy(&uiServerIpAgain, pcServerIp, 4);                        /* ��ֵ                         */
    }

    leaseLock();

    iIndex = clinetInLease(pdhcpmsgPacket);                             /* �ж��Ƿ��������ÿͻ��˷���� */
    if (iIndex >= 0) {                                                  /* ����������ַ���ÿͻ���     */
        pdhcpmsgSendPacket->uiYiaddr = GleaseManger.pdhcplease[iIndex].uiYiaddr;
        if (pcServerIp) {                                               /* �ж��Ƿ����򱾷���������     */
            if (uiServerIpAgain != GserverConfig.uiServer) {            /* ��������򱾷��������������� */
                LOG(LOG_DEBUG, "Requeset server is not me!\r\n");
            } else {
                iRet  = ACKOK;                                          /* ������Ҫ����ACK��            */
            }
        } else {
            iRet = ACKOK;                                               /* ���û�з�������ַ��ֱ��ACK  */
        }
    } else {                                                            /* ���û�и��ÿͻ��˷����     */
        if (pcRequestIp) {                                              /* �ж��Ƿ�������IP             */
            iIndex = leaseFindbyYiaddr(pdhcpmsgSendPacket->uiYiaddr);   /* ���Ҹ�IP�Ƿ�����Լ����       */
            if (iIndex >= 0) {                                          /* ����IP����Լ����             */
                if (leaseNotExpired(iIndex) == PX_ERROR &&              /* �жϸ�IP�Ƿ���             */
                    GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
                    memset(GleaseManger.pdhcplease[iIndex].ucChaddr,    /* ���������ͷŸ���Լ           */
                           0,
                           16);
                    GleaseManger.pdhcplease[iIndex].iState = LEASEFREE;
                } else if (leaseNotExpired(iIndex) >= 0 &&              /* ��û������Ҫ����NCK          */
                           GleaseManger.pdhcplease[iIndex].uiYiaddr != 0) {
                    iRet = PX_ERROR;
                }
            } else {
                iRet = PX_ERROR;                                        /* ���õ�ַ������Լ�з���NCK    */
            }
        }
    }

    if (iRet == ACKOK) {                                                /* �����Ҫ����ACK              */
        pcRequestLeases = poptionGet(pdhcpmsgPacket, DHCP_LEASE_TIME);  /* ������Լʱ��                 */
        if (pcRequestLeases) {
            memcpy(&uiLeaseAgain, pcRequestLeases, 4);
            if (uiLeaseAgain > GserverConfig.ulLease) {
                uiLeaseAgain = GserverConfig.ulLease;
            }
        }
                                                                        /* ����Լ��Ϊ����״̬           */
        if (leaseOccupySet(pdhcpmsgSendPacket->ucChaddr, iIndex, uiLeaseAgain) == ERROR_NONE) {
            optionsmpAdd(pdhcpmsgSendPacket->ucOptions, DHCP_LEASE_TIME, htonl(uiLeaseAgain));
            poptionset = GserverConfig.poptionsets;                     /* ����ѡ���ֶ�                 */
            while (poptionset) {
                if ((poptionset->pcData[OPT_CODE]) != (UINT8)DHCP_LEASE_TIME) {
                    optionstrAdd((PUCHAR)(pdhcpmsgSendPacket->ucOptions), (PUCHAR)(poptionset->pcData));
                }
                poptionset = poptionset->optionsetNext;
            }
        } else {
            iRet = PX_ERROR;
        }

    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseDecline
** ��������: decline������Ӧ
** ��    ��: pdhcpmsgPacket  dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  leaseDecline (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;
    INT  iRet = PX_ERROR;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* ͨ��MAC�ҵ���Ӧ��Լ          */
    if (iIndex >= 0) {                                                  /* �����Լ���ڽ���Լ��Ϊ��ͻ   */
        iRet = leaseConflictSet(iIndex);
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
** ��������: leaseRelease
** ��������: Release������Ӧ
** ��    ��: pdhcpmsgPacket  dhcp���ݰ�
** ��    ��: �ɹ�����ERROR_NONE, ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
INT  leaseRelease (PDHCPMSG  pdhcpmsgPacket)
{
    INT  iIndex;
    INT  iRet = PX_ERROR;

    leaseLock();

    iIndex = leaseFindbyChaddr(pdhcpmsgPacket->ucChaddr);               /* ͨ��MAC�ҵ���Ӧ��Լ          */
    if (iIndex >= 0) {                                                  /* �����Լ�������ͷ���Լ       */
        iRet = leaseFreeSet(iIndex);
    }

    leaseUnlock();

    return (iRet);
}

/*********************************************************************************************************
  END
*********************************************************************************************************/

