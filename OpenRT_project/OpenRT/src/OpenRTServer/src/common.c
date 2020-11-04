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
** 文   件   名: common.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 16 日
**
** 描        述: 通用函数接口
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include <wait.h>
#include "libcomm.h"
#include <pthread.h>
/*********************************************************************************************************
  信号处理结点
*********************************************************************************************************/
struct recliam_handle_func {
    VOID                       (*pvServerFunc)(int);
    struct recliam_handle_func  *pNextFunc;
};
typedef struct recliam_handle_func   __RECLIAM_HANDLE_FUNC;
typedef struct recliam_handle_func  *__PRECLIAM_HANDLE_FUNC;
/*********************************************************************************************************
  信号处理链表全局变量
*********************************************************************************************************/
__PRECLIAM_HANDLE_FUNC  _G_pRecliamHead = LW_NULL;

/*********************************************************************************************************
** 函数名称: __subProRecliamThread
** 功能描述: 子进程回收线程
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : none
*********************************************************************************************************/
static PVOID __subProRecliamThread (void *arg)
{
    INT                     iPid            = 0;
    INT                     iStat;
    __PRECLIAM_HANDLE_FUNC  pRecliamNode    = _G_pRecliamHead;

    while (1) {
        iPid = waitpid(-1, &iStat, WNOHANG);
        if(iPid > 0) {
            for (; LW_NULL != pRecliamNode; pRecliamNode = pRecliamNode->pNextFunc) {
                pRecliamNode->pvServerFunc(iPid);
            }
        }

        sleep(1);
    }

    return  (LW_NULL);
}

/*********************************************************************************************************
** 函数名称: subProRecliamInit
** 功能描述: 子进程回收初始化
** 输  入  : NONE
** 输  出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT  subProRecliamInit (VOID)
{
    pthread_t             tid;
    pthread_attr_t        attr;
    INT                   iRet;

    /*
     *  创建回收线程
     */
    iRet = pthread_attr_init(&attr);
    if (iRet != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pthread attr init failed.\n");
        return  (PX_ERROR);
    }

    pthread_attr_setname(&attr, "t_subProRecliam");
    iRet = pthread_create(&tid, &attr, __subProRecliamThread, NULL);
    if (iRet != 0) {
        __OPENRT_DEBUGF(OPENRT_LOG_ERR_LEVEL, "pthread create failed.\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** 函数名称: subRecliamHandleAdd
** 功能描述: 添加子进程退出处理
** 输　入  : vServerFunc        处理函数
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  subRecliamHandleAdd (VOID   (*pvHandleFunc)(int))
{
    __PRECLIAM_HANDLE_FUNC pRecliamNode    = _G_pRecliamHead;
    __PRECLIAM_HANDLE_FUNC pRecliamNewNode = LW_NULL;
    __PRECLIAM_HANDLE_FUNC pRecliamNext    = LW_NULL;

    if (LW_NULL == pvHandleFunc) {
        return  (PX_ERROR);
    }

    pRecliamNewNode = (__PRECLIAM_HANDLE_FUNC)malloc(sizeof(__RECLIAM_HANDLE_FUNC));
    if (LW_NULL == pRecliamNewNode) {
        return  (PX_ERROR);
    }

    pRecliamNewNode->pvServerFunc = pvHandleFunc;
    pRecliamNewNode->pNextFunc    = LW_NULL;

    if (LW_NULL == _G_pRecliamHead) {
        _G_pRecliamHead = pRecliamNewNode;

        return  (ERROR_NONE);
    }

    for (pRecliamNext = pRecliamNode->pNextFunc;
         LW_NULL     != pRecliamNext; ) {
        pRecliamNode  = pRecliamNext;
        pRecliamNext  = pRecliamNode->pNextFunc;
    }

    pRecliamNode->pNextFunc = pRecliamNewNode;

    return  (ERROR_NONE);
}
