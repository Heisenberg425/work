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
** 文   件   名: dhcpd.c
**
** 创   建   人: Chen.Jian (陈健)
**
** 文件创建日期: 2017 年 02 月 22 日
**
** 描        述: dhcp服务源文件
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

#define VERSION         "v1.2-beta"                                     /* 版本号                       */
#define MAXJOBS         255                                             /* 最大工作数                   */
#define WORKTHREADS     3                                               /* 工作线程数                   */
#define RUNSTATE        1                                               /* 工作线程运行                 */
#define STOPSTATE       0                                               /* 工作线程停止                 */
#define STIMEOUT        5                                               /* 数据等待超时时间（秒）       */
#define USTIMEOUT       500                                             /* 数据等待超时时间（微秒）     */
#define DAEMON          1                                               /* 配置为守护进程               */

#define CONFIGERROR     -1                                              /* 配置初始化错误               */
#define LEASEERROR      -2                                              /* 租约初始化错误               */
#define SOCKETERROR     -3                                              /* 套接字初始化错误             */
#define THREADERROR     -4                                              /* 线程初始化错误               */
#define PARAMERROR      -5                                              /* 输入参数错误                 */

/*********************************************************************************************************
  工作队列结构体
*********************************************************************************************************/
struct jobqueues {
    DHCPMSG            dhcpmesgPacket;                                  /* dhcp数据包                   */
    struct jobqueues  *pjobqueueNext;                                   /* 指向下一个工作的指针         */
};
typedef struct jobqueues   JOBQUEUES;
/*********************************************************************************************************
   线程池结构体
*********************************************************************************************************/
struct threadpool {
     INT              iStatus;                                          /* 线程池状态                   */
     UINT             uiJobs;                                           /* 当前工作数                   */
     pthread_mutex_t  mutexLock;                                        /* 线程池锁                     */
     pthread_cond_t   condReady;                                        /* 线程池条件变量               */
     JOBQUEUES       *pJobHeadr;                                        /* 工作队列头指针               */
     JOBQUEUES       *pJobEnd;                                          /* 工作队列尾指针               */
     pthread_t       *pTid;                                             /* 线程池线程Tid指针            */
};
typedef struct threadpool   THRADPOOL;

static THRADPOOL       *Gthreadpool = NULL;                             /* 全局线程池结构体指针         */
static PVOID            GpvMem;                                         /* 内存池申请指针               */
static LW_HANDLE        Gmempool;                                       /* 全局内存块管理变量           */
static UINT             GuiLeaseFlag;                                   /* 写租约控制标志位             */
static sem_t            GsemLeaseSaveLock;

static pthread_t        GLeaseSaveTid       = 0;                        /* lease文件更新线程ID          */
extern pthread_t        GLogServerTid;
extern pthread_t        GFileSyncTid;
extern LOGFILE          GLogFile;

/*********************************************************************************************************
** 函数名称: memInit
** 功能描述: 内存初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  memInit (VOID)
{
    GpvMem = malloc(MAXJOBS * sizeof(JOBQUEUES));                        /* 根据工作最大数量开辟内存空间 */
    if (GpvMem == NULL) {
        LOG(LOG_ERR, "Mem malloc failed!\r\n");
        return (PX_ERROR);
    }

    Gmempool = Lw_Partition_Create("dhcpjobpool",                       /* 内存分块管理                 */
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
** 函数名称: memGet
** 功能描述: 获取内存块
** 输    入: NONE
** 输    出: 成功返回内存块指针，失败返回LW_NULL
** 调用模块: API
*********************************************************************************************************/
static PVOID  memGet ()
{
    PVOID  pMem;
    pMem = Lw_Partition_Get(Gmempool);                                  /* 获取内存快指针               */
    if (pMem == LW_NULL) {
        LOG(LOG_WARNING, "Memery Low!\r\n");
    }

    return pMem;
}

/*********************************************************************************************************
** 函数名称: memFree
** 功能描述: 释放内存块
** 输    入: PVOID pMem 内存块指针
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  memFree (PVOID  pvMem)
{
    if (pvMem == NULL) {                                                 /* 判空                        */
        return (PX_ERROR);
    }

    if (Lw_Partition_Put(Gmempool, pvMem) != LW_NULL) {                  /* 归还内存块                  */
        LOG(LOG_WARNING, "Mem Free failed!\r\n");
        return (PX_ERROR);
    } else {
        return (ERROR_NONE);
    }
}

/*********************************************************************************************************
** 函数名称: jobGet
** 功能描述: 获取新的工作
** 输    入: job指针
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  jobGet (JOBQUEUES  **ppJob)
{
    pthread_mutex_lock(&(Gthreadpool->mutexLock));

    *ppJob = Gthreadpool->pJobHeadr;
    while (*ppJob == NULL) {                                            /* 等待条件变量                 */
        pthread_cond_wait(&(Gthreadpool->condReady), &(Gthreadpool->mutexLock));
        *ppJob = Gthreadpool->pJobHeadr;
    }

    Gthreadpool->pJobHeadr = Gthreadpool->pJobHeadr->pjobqueueNext;     /* 将工作头指针指向下一个工作   */
    if (Gthreadpool->uiJobs > 0) {                                      /* 工作数计数处理               */
        Gthreadpool->uiJobs--;
    }

    pthread_mutex_unlock(&(Gthreadpool->mutexLock));
    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: jobPut
** 功能描述: 添加工作到工作队列
** 输    入: PDHCPMSG pPacket
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 调用模块: API
*********************************************************************************************************/
static INT  jobPut (DHCPMSG  *pdhcpmsgPacket)
{
    JOBQUEUES    *jobqueuesEndjob;
    JOBQUEUES    *pjobqueuesNewjob;
    if (pdhcpmsgPacket == NULL) {                                       /* 传入指针检查                 */
        LOG(LOG_WARNING, "DHCPMSG is null,workPut failed!\r\n");
        return (PX_ERROR);
    }

    pjobqueuesNewjob = (JOBQUEUES*) memGet();                           /* 获取内存空间                 */
    if (pjobqueuesNewjob == LW_NULL) {
        LOG(LOG_WARNING, "Memery Low!\r\n");
        return (PX_ERROR);
    }
    memcpy(&(pjobqueuesNewjob->dhcpmesgPacket),
           pdhcpmsgPacket,
           sizeof(*pdhcpmsgPacket));                                    /* 拷贝数据包到获取的内存空间   */
    pjobqueuesNewjob->pjobqueueNext = NULL;

    pthread_mutex_lock(&(Gthreadpool->mutexLock));                      /* 将新的工作插入工作队列队尾   */
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
    pthread_cond_signal(&(Gthreadpool->condReady));                     /* 发送条件变量                 */
    pthread_mutex_unlock(&(Gthreadpool->mutexLock));

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: packetHandle
** 功能描述: 工作线程函数
** 输    入: PVOID arg
** 输    出: NONE
** 调用模块: API
*********************************************************************************************************/
PVOID  packetHandle (PVOID  pvArg)
{
    JOBQUEUES    *pJob =  NULL;
    INT           iRet;
    UCHAR         ucMsgtype;
    //INT           iPingSocket;

//    iPingSocket = icmpInit();                                           /* 初始化ICMP套接字             */
//    if (iPingSocket == PX_ERROR) {
//        LOG(LOG_ERR, "iPingSocket init failed!!!\r\n");
//        return NULL;
//    }

    while (Gthreadpool->iStatus) {
        iRet = jobGet(&pJob);                                           /* 获取最新需要处理的工作       */
        if (iRet == ERROR_NONE) {                                       /* 获取工作成功                 */
            /*
             * 数据包处理
             */
            ucMsgtype = dhcpMsgtypeGet(&(pJob->dhcpmesgPacket));        /* 获取包数据类型               */
            switch (ucMsgtype) {                                        /* 根据类型选择不同的数据处理   */

            case DHCPDISCOVER:                                          /* discover数据处理             */
                LOG(LOG_DEBUG, "DHCPDISCOVER PACKET!!!\r\n");
                if (discoverHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "discoverHandle failed\r\n");
                }
                break;

            case DHCPREQUEST:                                           /* request数据处理              */
                LOG(LOG_DEBUG, "DHCPREQUEST PACKET!!!\r\n");
                if (requestHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "requestHandle failed\r\n");
                }
                break;

            case DHCPDECLINE:                                           /* decline数据处理              */
                LOG(LOG_DEBUG, "DHCPDECLINE PACKET!!!\r\n");
                if (declienHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "declienHandle failed\r\n");
                }
                break;

            case DHCPRELEASE:                                           /* release数据处理              */
                LOG(LOG_DEBUG, "DHCPRELEASE PACKET!!!\r\n");
                if (releaseHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "releaseHandle failed\r\n");
                }
                break;

            case DHCPINFORM:                                            /* inform数据处理               */
                LOG(LOG_DEBUG, "DHCPINFORM PACKET!!!\r\n");
                if (informHandle(&(pJob->dhcpmesgPacket)) < 0) {
                    LOG(LOG_DEBUG, "informHandle failed\r\n");
                }
                break;

            default:
                LOG(LOG_DEBUG, "unsupported PACKET!!!\r\n");
            }
        }

        memFree((PVOID)pJob);                                           /* 释放相关工作的内存空间       */
    }

    return  (NULL);
}

/*********************************************************************************************************
** 函数名称: leaseSave
** 功能描述: lease文件更新线程函数
** 输    入: PVOID arg
** 输    出: NONE
** 调用模块: API
*********************************************************************************************************/
PVOID  leaseSave (PVOID  pvArg)
{
    while (1) {
        sem_wait(&GsemLeaseSaveLock);                                   /* 等待更新信号                 */
        if (leaseWriteWithProtect() == PX_ERROR) {                      /* 写租约                       */
             LOG(LOG_WARNING, "leasesWrite error !\r\n");
        }
        sleep(1);
    }
    return  (NULL);
}

/*********************************************************************************************************
** 函数名称: leaseSaveThreadInit
** 功能描述: lease文件更新线程启动函数
** 输    入: VOID
** 输    出: NONE
** 调用模块: API
*********************************************************************************************************/
static INT  leaseSaveThreadInit (VOID)
{
    INT             iRet;
    pthread_attr_t  attr;

    iRet = pthread_attr_init(&attr);                                    /* 初始化线程属性              */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Pthread attr init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_attr_setname(&attr, "leaseSave");                    /* 设置线程名                  */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Pthread attr init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_create(&GLeaseSaveTid, &attr, leaseSave, NULL);      /* 启动lease文件更新线程       */
    if (iRet != LW_OBJECT_HANDLE_INVALID) {
        LOG(LOG_ERR, "Creat thread %s failed!\r\n", "leaseSave");
        return (PX_ERROR);
    }

    return (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: threadpoolInit
** 功能描述: 工作线程池初始化
** 输    入: NONE
** 输    出: 成功返回ERROR_NONE 失败返回PX_ERROR
** 调用模块: NONE
*********************************************************************************************************/
static INT  threadpoolInit (VOID)
{
    INT             i;
    INT             iRet;
    pthread_attr_t  attr;
    CHAR            cName[32]= {0};

    if (memInit() != ERROR_NONE) {                                      /* 初始化内存空间               */
        return (PX_ERROR);
    }
    Gthreadpool = (THRADPOOL *)malloc(sizeof(THRADPOOL));
    if (Gthreadpool == NULL) {
        LOG(LOG_ERR, "Gthreadpool malloc failed!\r\n");
        return (PX_ERROR);
    }

    Gthreadpool->uiJobs = 0;
    iRet = pthread_mutex_init(&(Gthreadpool->mutexLock), NULL);         /* 初始化线程池锁               */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Mutex init failed!\r\n");
        return (PX_ERROR);
    }

    iRet = pthread_cond_init(&(Gthreadpool->condReady),NULL);           /* 初始化线程池条件变量         */
    if (iRet != ERROR_NONE) {
        LOG(LOG_ERR, "Cond init failed!\r\n");
        return (PX_ERROR);
    }

    Gthreadpool->iStatus    = RUNSTATE;                                 /* 设置线程池状态为运行         */
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

    for (i = 0; i < WORKTHREADS; i++) {                                 /* 创建线程池线程               */
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
** 函数名称: dhcpdExit
** 功能描述: 线程退出函数
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
*********************************************************************************************************/
static VOID  dhcpdExit (VOID)
{
    sleep(1);                                                           /* 延时确保log输出              */

    logExit();                                                          /* log退出                      */

    if (&Gthreadpool->iStatus) {                                        /* 线程池线程退出               */
        Gthreadpool->iStatus = STOPSTATE;
    }
}

/*********************************************************************************************************
** 函数名称: __atexitFunction
** 功能描述: 进程退出函数
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
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
** 函数名称: main
** 功能描述: 主函数
** 输    入: NONE
** 输    出: NONE
** 调用模块: NONE
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
     *  注册进程退出hook
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

    if (configRead(pcconfig_file) == PX_ERROR) {                        /* 读取配置文件信息             */
        printf("CONFIG read  %s failed!\r\n", pcconfig_file);
        dhcpdExit();
        return (CONFIGERROR);
    }

#if DAEMON
    daemon(1,0);                                                        /* 启动为守护进程               */
#endif

    LOG(LOG_INFO, "dhcp server %s starting ...\r\n",VERSION);
    LOG(LOG_INFO, "init server config ...\r\n");

    /* 更改主线程名和配置的网口相关 */
    snprintf(main_thread_name, sizeof(main_thread_name), "dhcpserver-%s",GserverConfig.pcInteface);
    pthread_setname_np(pthread_self(),main_thread_name);

    LOG(LOG_INFO, "init leases ...\r\n");

    if (leaseMangerInit() == PX_ERROR) {                                /* 读取租约文件，初始化租约链表 */
        LOG(LOG_ERR, "lease Init failed!\r\n");
        dhcpdExit();
        return (LEASEERROR);
    }

    LOG(LOG_INFO, "init udp socket ...\r\n");

    iRet = socketInit(&iRcvSocket, &sockaddrFrom);                      /* 初始化套接字                 */
    if (iRet == PX_ERROR) {
        dhcpdExit();
        return (SOCKETERROR);
    }

    LOG(LOG_INFO, "nydhcp server %s OK! \r\n",VERSION);

    iRet = threadpoolInit();                                            /* 初始化线程池                 */
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
        iRet = select(iRcvSocket + 1, &fdset, NULL, NULL, &tv);         /* select接收套接字             */
        if (iRet == 0) {                                                /* 如果超时                     */
            if (GuiLeaseFlag > 0) {
                GuiLeaseFlag = 0;
                sem_post(&GsemLeaseSaveLock);                           /* 发送更新信号                 */
            }
            continue;
        } else if (iRet < 0) {                                          /* 如果select失败               */
            LOG(LOG_DEBUG, "Select error \r\n");
            continue;
        }

        if ((iRet = packetRecv(&dhcpmsgPacket,                          /* 接收一个数据包               */
                               iRcvSocket,
                               (struct sockaddr_in *)&sockaddrFrom)) < 0) {
            continue;
        }

        if (dhcpMsgtypeGet(&dhcpmsgPacket) < 0) {                       /* 判断数据包类型               */
            continue;
        }

        iRet = jobPut(&dhcpmsgPacket);                                  /* 添加数据包到工作队列         */
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
