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
** ��   ��   ��: common.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 16 ��
**
** ��        ��: ͨ�ú����ӿ�
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
  �źŴ�����
*********************************************************************************************************/
struct recliam_handle_func {
    VOID                       (*pvServerFunc)(int);
    struct recliam_handle_func  *pNextFunc;
};
typedef struct recliam_handle_func   __RECLIAM_HANDLE_FUNC;
typedef struct recliam_handle_func  *__PRECLIAM_HANDLE_FUNC;
/*********************************************************************************************************
  �źŴ�������ȫ�ֱ���
*********************************************************************************************************/
__PRECLIAM_HANDLE_FUNC  _G_pRecliamHead = LW_NULL;

/*********************************************************************************************************
** ��������: __subProRecliamThread
** ��������: �ӽ��̻����߳�
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : none
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
** ��������: subProRecliamInit
** ��������: �ӽ��̻��ճ�ʼ��
** ��  ��  : NONE
** ��  ��  : NONE
** ��  ��  : ERROR_CODE
*********************************************************************************************************/
INT  subProRecliamInit (VOID)
{
    pthread_t             tid;
    pthread_attr_t        attr;
    INT                   iRet;

    /*
     *  ���������߳�
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
** ��������: subRecliamHandleAdd
** ��������: ����ӽ����˳�����
** �䡡��  : vServerFunc        ������
** �䡡��  : NONE
** ������  : ERROR_CODE
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
