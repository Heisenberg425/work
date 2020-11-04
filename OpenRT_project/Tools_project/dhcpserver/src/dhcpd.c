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
** ��   ��   ��: dhcpd.c
**
** ��   ��   ��: Chen.Jian (�½�)
**
** �ļ���������: 2017 �� 02 �� 22 ��
**
** ��        ��: dhcp����Դ�ļ�
*********************************************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <socket.h>
#include <string.h>
#include <pthread.h>
#include <SylixOS.h>
#include <semaphore.h>

#include "configini.h"
#include "packet.h"
#include "sockets.h"
#include "leasesfix.h"
#include "log.h"

#define VERSION         "v1.2-beta"                                     /* �汾��                       */
#define MAXJOBS         255                                             /* �������                   */
#define WORKTHREADS     3                                               /* �����߳���                   */
#define RUNSTATE        1                                               /* �����߳�����                 */
#define STOPSTATE       0                                               /* �����߳�ֹͣ                 */
#define STIMEOUT        5                                               /* ���ݵȴ���ʱʱ�䣨�룩       */
#define USTIMEOUT       500                                             /* ���ݵȴ���ʱʱ�䣨΢�룩     */
#define DAEMON          1                                               /* ����Ϊ�ػ�����               */

#define CONFIGERROR     -1                                              /* ���ó�ʼ������               */
#define LEASEERROR      -2                                              /* ��Լ��ʼ������               */
#define SOCKETERROR     -3                                              /* �׽��ֳ�ʼ������             */
#define THREADERROR     -4                                              /* �̳߳�ʼ������               */
#define PARAMERROR      -5                                              /* �����������                 */

/*********************************************************************************************************
  �������нṹ��
*********************************************************************************************************/
struct jobqueues {
    DHCPMSG            dhcpmesgPacket;                                  /* dhcp���ݰ�                   */
    struct jobqueues  *pjobqueueNext;                                   /* ָ����һ��������ָ��         */
};
typedef struct jobqueues   JOBQUEUES;
/*********************************************************************************************************
   �̳߳ؽṹ��
*********************************************************************************************************/
struct threadpool {
     INT              iStatus;                                          /* �̳߳�״̬                   */
     UINT             uiJobs;                                           /* ��ǰ������                   */
     pthread_mutex_t  mutexLock;                                        /* �̳߳���                     */
     pthread_cond_t   condReady;                                        /* �̳߳���������               */
     JOBQUEUES       *pJobHeadr;                                        /* ��������ͷָ��               */
     JOBQUEUES       *pJobEnd;                                          /* ��������βָ��               */
     pthread_t       *pTid;                                             /* �̳߳��߳�Tidָ��            */
};
typedef struct threadpool   THRADPOOL;

static THRADPOOL       *Gthreadpool = NULL;                             /* ȫ���̳߳ؽṹ��ָ��         */
static PVOID            GpvMem;                                         /* �ڴ������ָ��               */
static LW_HANDLE        Gmempool;                                       /* ȫ���ڴ��������           */
static UINT             GuiLeaseFlag;                                   /* д��Լ���Ʊ�־λ             */
static sem_t            GsemLeaseSaveLock;

static pthread_t        GLeaseSaveTid       = 0;                        /* lease�ļ������߳�ID          */
extern pthread_t        GLogServerTid;
extern pthread_t        GFileSyncTid;
extern LOGFILE          GLogFile;

/*********************************************************************************************************
** ��������: memInit
** ��������: �ڴ��ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  memInit (VOID)
{
    GpvMem = malloc(MAXJOBS * sizeof(JOBQUEUES));                        /* ���ݹ���������������ڴ�ռ� */
    if (GpvMem == NULL) {
        LOG(LOG_ERR, "Mem malloc failed!\r\n");
        return (PX_ERROR);
    }

    Gmempool = Lw_Partition_Create("dhcpjobpool",                       /* �ڴ�ֿ����                 */
                                   GpvMem,
                                   MAXJOBS,
                                   sizeof(JOBQUEUES),
                                   LW_OPTION_DEFAULT,
                                   LW_NULL);
    if (Gmempool == LW_HANDLE_INVALID) {
        LOG(LOG_ERR, "Mem partition failed!\r\n");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: memGet
** ��������: ��ȡ�ڴ��
** ��    ��: NONE
** ��    ��: �ɹ������ڴ��ָ�룬ʧ�ܷ���LW_NULL
** ����ģ��: API
*********************************************************************************************************/
static PVOID  memGet ()
{
    PVOID  pMem;
    pMem = Lw_Partition_Get(Gmempool);                                  /* ��ȡ�ڴ��ָ��               */
    if (pMem == LW_NULL) {
        LOG(LOG_WARNING, "Memery Low!\r\n");
    }

    return pMem;
}

/*********************************************************************************************************
** ��������: memFree
** ��������: �ͷ��ڴ��
** ��    ��: PVOID pMem �ڴ��ָ��
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  memFree (PVOID  pvMem)
{
    if (pvMem == NULL) {                                                 /* �п�                        */
        return (PX_ERROR);
    }

    if (Lw_Partition_Put(Gmempool, pvMem) != LW_NULL) {                  /* �黹�ڴ��                  */
        LOG(LOG_WARNING, "Mem Free failed!\r\n");
        return (PX_ERROR);
    } else {
        return (ERROR_NONE);
    }
}

/*********************************************************************************************************
** ��������: jobGet
** ��������: ��ȡ�µĹ���
** ��    ��: jobָ��
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  jobGet (JOBQUEUES  **ppJob)
{
    pthread_mutex_lock(&(Gthreadpool->mutexLock));

    *ppJob = Gthreadpool->pJobHeadr;
    while (*ppJob == NULL) {                                            /* �ȴ���������                 */
        pthread_cond_wait(&(Gthreadpool->condReady), &(Gthreadpool->mutexLock));
        *ppJob = Gthreadpool->pJobHeadr;
    }

    Gthreadpool->pJobHeadr = Gthreadpool->pJobHeadr->pjobqueueNext;     /* ������ͷָ��ָ����һ������   */
    if (Gthreadpool->uiJobs > 0) {                                      /* ��������������               */
        Gthreadpool->uiJobs--;
    }

    pthread_mutex_unlock(&(Gthreadpool->mutexLock));
    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: jobPut
** ��������: ��ӹ�������������
** ��    ��: PDHCPMSG pPacket
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ����ģ��: API
*********************************************************************************************************/
static INT  jobPut (DHCPMSG  *pdhcpmsgPacket)
{
    JOBQUEUES    *jobqueuesEndjob;
    JOBQUEUES    *pjobqueuesNewjob;
    if (pdhcpmsgPacket == NULL) {                                       /* ����ָ����                 */
        LOG(LOG_WARNING, "DHCPMSG is null,workPut failed!\r\n");
        return (PX_ERROR);
    }

    pjobqueuesNewjob = (JOBQUEUES*) memGet();                           /* ��ȡ�ڴ�ռ�                 */
    if (pjobqueuesNewjob == LW_NULL) {
        LOG(LOG_WARNING, "Memery Low!\r\n");
        return (PX_ERROR);
    }
    memcpy(&(pjobqueuesNewjob->dhcpmesgPacket),
           pdhcpmsgPacket,
           sizeof(*pdhcpmsgPacket));                                    /* �������ݰ�����ȡ���ڴ�ռ�   */
    pjobqueuesNewjob->pjobqueueNext = NULL;

    pthread_mutex_lock(&(Gthreadpool->mutexLock));                      /* ���µĹ������빤�����ж�β   */
    if (Gthreadpool->pJobHeadr == NULL) {
        Gthreadpool->pJobHeadr = pjobqueuesNewjob;
        Gthreadpool->pJobEnd   = Gthreadpool->pJobHeadr;
    } else {
        jobqueuesEndjob                = Gthreadpool->pJobEnd;
        jobqueuesEndjob->pjobqueueNext = pjobqueuesNewjob;
        Gthreadpool->pJobEnd           = jobqueuesEndjob->pjobqueueNext;
    }
    Gthreadpool->uiJobs ++;
    LOG(LOG_DEBUG, "Now jobs :%d!\r\n",Gthreadpool->uiJobs);
    pthread_cond_signal(&(Gthreadpool->condReady));                     /* ������������                 */
    pthread_mutex_unlock(&(Gthreadpool->mutexLock));

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: packetHandle
** ��������: �����̺߳���
** ��    ��: PVOID arg
** ��    ��: NONE
** ����ģ��: API
*********************************************************************************************************/
PVOID  packetHandle (PVOID  pvArg)
{
    JOBQUEUES    *pJob =  NULL;
    INT           iRet;
    UCHAR         ucMsgtype;
    //INT           iPingSocket;

//    iPingSocket = icmpInit();                                           /* ��ʼ��ICMP�׽���             */
//    if (iPingSocket == PX_ERROR) {
//        LOG(LOG_ERR, "iPingSocket init failed!!!\r\n");
//        return NULL;
//    }

    while (Gthreadpool->iStatus) {
        iRet = jobGet(&pJob);                                           /* ��ȡ������Ҫ����Ĺ���       */
        if (iRet == ERROR_NONE) {                                       /* ��ȡ�����ɹ�                 */
            /*
             * ���ݰ�����
             */
            ucMsgtype = dhcpMsgtypeGet(&(pJob->dhcpmesgPacket));        /* ��ȡ����������               */
            switch (ucMsgtype) {                                        /* ��������ѡ��ͬ�����ݴ���   */

            case DHCPDISCOVER:                                          /* discover���ݴ���             */
                LOG(LOG_DEBUG, "DHCPDISCOVER PACKET!!!\r\n");
                if (discoverHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "discoverHandle failed\r\n");
                }
                break;

            case DHCPREQUEST:                                           /* request���ݴ���              */
                LOG(LOG_DEBUG, "DHCPREQUEST PACKET!!!\r\n");
                if (requestHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "requestHandle failed\r\n");
                }
                break;

            case DHCPDECLINE:                                           /* decline���ݴ���              */
                LOG(LOG_DEBUG, "DHCPDECLINE PACKET!!!\r\n");
                if (declienHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "declienHandle failed\r\n");
                }
                break;

            case DHCPRELEASE:                                           /* release���ݴ���              */
                LOG(LOG_DEBUG, "DHCPRELEASE PACKET!!!\r\n");
                if (releaseHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "releaseHandle failed\r\n");
                }
                break;

            case DHCPINFORM:                                            /* inform���ݴ���               */
                LOG(LOG_DEBUG, "DHCPINFORM PACKET!!!\r\n");
                if (informHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "informHandle failed\r\n");
                }
                break;

            default:
                LOG(LOG_DEBUG, "unsupported PACKET!!!\r\n");
            }
        }

        memFree((PVOID)pJob);                                           /* �ͷ���ع������ڴ�ռ�       */
    }

    return  (NULL);
}

/*********************************************************************************************************
** ��������: leaseSave
** ��������: lease�ļ������̺߳���
** ��    ��: PVOID arg
** ��    ��: NONE
** ����ģ��: API
*********************************************************************************************************/
PVOID  leaseSave (PVOID  pvArg)
{
    while (1) {
        sem_wait(&GsemLeaseSaveLock);                                   /* �ȴ������ź�                 */
        if (leaseWriteWithProtect() == PX_ERROR) {                      /* д��Լ                       */
             LOG(LOG_WARNING, "leasesWrite error !\r\n");
        }
        sleep(1);
    }
    return  (NULL);
}

/*********************************************************************************************************
** ��������: leaseSaveThreadInit
** ��������: lease�ļ������߳���������
** ��    ��: VOID
** ��    ��: NONE
** ����ģ��: API
*********************************************************************************************************/
static INT  leaseSaveThreadInit (VOID)
{
    INT             iRet;
    pthread_attr_t  attr;

    iRet = pthread_attr_init(&attr);                                    /* ��ʼ���߳�����              */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Pthread attr init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_attr_setname(&attr, "leaseSave");                    /* �����߳���                  */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Pthread attr init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_create(&GLeaseSaveTid, &attr, leaseSave, NULL);      /* ����lease�ļ������߳�       */
    if (iRet != LW_OBJECT_HANDLE_INVALID) {
        LOG(LOG_ERR, "Creat thread %s failed!\r\n", "leaseSave");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: threadpoolInit
** ��������: �����̳߳س�ʼ��
** ��    ��: NONE
** ��    ��: �ɹ�����ERROR_NONE ʧ�ܷ���PX_ERROR
** ����ģ��: NONE
*********************************************************************************************************/
static INT  threadpoolInit (VOID)
{
    INT             i;
    INT             iRet;
    pthread_attr_t  attr;
    CHAR            cName[32]= {0};

    if (memInit() != ERROR_NONE) {                                      /* ��ʼ���ڴ�ռ�               */
        return (PX_ERROR);
    }
    Gthreadpool = (THRADPOOL *)malloc(sizeof(THRADPOOL));
    if (Gthreadpool == NULL) {
        LOG(LOG_ERR, "Gthreadpool malloc failed!\r\n");
        return (PX_ERROR);
    }

    Gthreadpool->uiJobs = 0;
    iRet = pthread_mutex_init(&(Gthreadpool->mutexLock), NULL);         /* ��ʼ���̳߳���               */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Mutex init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_cond_init(&(Gthreadpool->condReady),NULL);           /* ��ʼ���̳߳���������         */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Cond init failed!\r\n");
        return (PX_ERROR);
    }

    Gthreadpool->iStatus    = RUNSTATE;                                 /* �����̳߳�״̬Ϊ����         */
    Gthreadpool->pJobHeadr  = NULL;
    Gthreadpool->pJobEnd    = NULL;
    Gthreadpool->pTid       = (pthread_t *)malloc(MAXJOBS * sizeof(pthread_t));
    if (Gthreadpool->pTid == NULL) {
        LOG(LOG_ERR, "pTid malloc failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_attr_init(&attr);
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Pthread attr init failed!\r\n");
        return (PX_ERROR);
    }

    for (i = 0; i < WORKTHREADS; i++) {                                 /* �����̳߳��߳�               */
        memset(cName, 0, sizeof(cName));
        sprintf(cName,"dhcpworker%d",i);
        iRet = pthread_attr_setname(&attr, cName);
        if (iRet != ERROR_NONE) {
            LOG(LOG_ERR, "Set work thread name failed!\r\n");
            return (PX_ERROR);
        }

        iRet = pthread_create(&(Gthreadpool->pTid[i]), &attr, packetHandle, NULL);
        if (iRet != LW_OBJECT_HANDLE_INVALID) {
            LOG(LOG_ERR, "Creat thread %d failed!\r\n", i);
            return (PX_ERROR);
        }
    }
    return (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: dhcpdExit
** ��������: �߳��˳�����
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static VOID  dhcpdExit (VOID)
{
    sleep(1);                                                           /* ��ʱȷ��log���              */

    logExit();                                                          /* log�˳�                      */

    if (&Gthreadpool->iStatus) {                                        /* �̳߳��߳��˳�               */
        Gthreadpool->iStatus = STOPSTATE;
    }
}

/*********************************************************************************************************
** ��������: __atexitFunction
** ��������: �����˳�����
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
static VOID __atexitFunction (void)
{
    INT i;

    if (Gthreadpool != LW_NULL && Gthreadpool->pTid != LW_NULL) {
        for (i = 0; i < WORKTHREADS; i++) {
            kill(Gthreadpool->pTid[i], 9);
        }
    }

    kill(GLeaseSaveTid, 9);
    kill(GLogServerTid, 9);
    kill(GFileSyncTid, 9);

    sem_destroy(&GsemLeaseSaveLock);
}

/*********************************************************************************************************
** ��������: main
** ��������: ������
** ��    ��: NONE
** ��    ��: NONE
** ����ģ��: NONE
*********************************************************************************************************/
INT  main (INT iArgc, CHAR *pcArgv[])
{
    fd_set              fdset;
    INT                 iRcvSocket;
    DHCPMSG             dhcpmsgPacket;
    INT                 iRet;
    struct timeval      tv;
    struct sockaddr_in  sockaddrFrom;
    CHAR                main_thread_name[32];
    CHAR               *pcconfig_file;


    /*
     *  ע������˳�hook
     */
    atexit(__atexitFunction);

    if(iArgc == 1) {
        printf("start dhcp server at one interface\r\n");
        printf("start cmd like following:\r\n");
        printf("dhcpserver -c /etc/dhcpdconfig.ini\r\n");
        return (ERROR_NONE);
    }


    if (iArgc < 3) {
        printf("argment too little ...\r\n");
        return (PARAMERROR);
    }

    pcconfig_file = pcArgv[2];

    if (configRead(pcconfig_file) == PX_ERROR) {                        /* ��ȡ�����ļ���Ϣ             */
        printf("CONFIG read  %s failed!\r\n", pcconfig_file);
        dhcpdExit();
        return (CONFIGERROR);
    }

#if DAEMON
    daemon(1,0);                                                        /* ����Ϊ�ػ�����               */
#endif

    LOG(LOG_INFO, "dhcp server %s starting ...\r\n",VERSION);
    LOG(LOG_INFO, "init server config ...\r\n");

    /* �������߳��������õ�������� */
    snprintf(main_thread_name, sizeof(main_thread_name), "dhcpserver-%s",GserverConfig.pcInteface);
    pthread_setname_np(pthread_self(),main_thread_name);

    LOG(LOG_INFO, "init leases ...\r\n");

    if (leaseMangerInit() == PX_ERROR) {                                /* ��ȡ��Լ�ļ�����ʼ����Լ���� */
        LOG(LOG_ERR, "lease Init failed!\r\n");
        dhcpdExit();
        return (LEASEERROR);
    }

    LOG(LOG_INFO, "init udp socket ...\r\n");

    iRet = socketInit(&iRcvSocket, &sockaddrFrom);                      /* ��ʼ���׽���                 */
    if (iRet == PX_ERROR) {
        dhcpdExit();
        return (SOCKETERROR);
    }

    LOG(LOG_INFO, "nydhcp server %s OK! \r\n",VERSION);

    iRet = threadpoolInit();                                            /* ��ʼ���̳߳�                 */
    if (iRet == PX_ERROR) {
        dhcpdExit();
        return (THREADERROR);
    }

    iRet = sem_init(&GsemLeaseSaveLock,1,1);
    if (iRet == PX_ERROR) {
        dhcpdExit();
        return (THREADERROR);
    }

    iRet = leaseSaveThreadInit();
    if (iRet == PX_ERROR) {
        dhcpdExit();
        return (THREADERROR);
    }

    tv.tv_sec  = STIMEOUT;
    tv.tv_usec = USTIMEOUT;

    while (Gthreadpool->iStatus) {
        FD_ZERO(&fdset);
        FD_SET(iRcvSocket, &fdset);
        iRet = select(iRcvSocket + 1, &fdset, NULL, NULL, &tv);         /* select�����׽���             */
        if (iRet == 0) {                                                /* �����ʱ                     */
            if (GuiLeaseFlag > 0) {
                GuiLeaseFlag = 0;
                sem_post(&GsemLeaseSaveLock);                           /* ���͸����ź�                 */
            }
            continue;
        } else if (iRet < 0) {                                          /* ���selectʧ��               */
            LOG(LOG_DEBUG, "Select error \r\n");
            continue;
        }

        if ((iRet = packetRecv(&dhcpmsgPacket,                          /* ����һ�����ݰ�               */
                               iRcvSocket,
                               (struct sockaddr_in *)&sockaddrFrom)) < 0) {
            continue;
        }

        if (dhcpMsgtypeGet(&dhcpmsgPacket) < 0) {                       /* �ж����ݰ�����               */
            continue;
        }

        iRet = jobPut(&dhcpmsgPacket);                                  /* ������ݰ�����������         */
        if (iRet == PX_ERROR) {
            LOG(LOG_DEBUG, "jobPut failed! \r\n");
        }
        GuiLeaseFlag ++;
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
