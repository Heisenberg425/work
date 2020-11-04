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
** ��   ��   ��: libcomm.c
**
** ��   ��   ��: Li.xiaocheng (��Т��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: socket ͨ�ſ�ӿ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <socket.h>
#include "unixSocket.h"
#include "../include/libcomm.h"
/*********************************************************************************************************
  UNIX socket �ļ�
*********************************************************************************************************/
#define COMM_DIR_PATH     "/tmp/OpenRT"                                 /*  socket �ļ���Ŀ¼           */
#define COMM_SER_ADDR     COMM_DIR_PATH"/Saddr"                         /*  ������ļ�                  */
#define COMM_CLI_ADDR     COMM_DIR_PATH"/Caddr"                         /*  �ͻ����ļ�                  */

/*********************************************************************************************************
  �ͻ���ͨ�ž��
*********************************************************************************************************/
static INT                 _G_iCliCommFd = -1;                          /*  �ͻ���socket ����           */
static INT                 _G_iSerCommFd = -1;                          /*  �����socket ����           */

/*********************************************************************************************************
  ͨѶ�ṹ�嶨��
*********************************************************************************************************/

struct comm_info {
    INT    iCommand;
    INT    iLen;
    INT    iFlag;
    char   cBuff[4];
};
typedef struct comm_info   __COMM_INFO;
typedef struct comm_info  *__PCOMM_INFO;

#define COMM_INFO_LEN(len)  (sizeof(__COMM_INFO) - sizeof(char)*4 + len)

/*********************************************************************************************************
  ͨѶ�ṹ�嶨��
*********************************************************************************************************/
struct comm_list_info {
    INT    iCommand;
    INT    iLen;
    INT    iFlag;
    INT    iNum;
    char   cBuff[4];
};
typedef struct comm_list_info   __COMM_LIST_INFO;
typedef struct comm_list_info  *__PCOMM_LIST_INFO;

#define COMM_INFO_LEN(len)  (sizeof(__COMM_INFO) - sizeof(char)*4 + len)

/*********************************************************************************************************
  ͨѶiFlag����
*********************************************************************************************************/
#define PACKAGE_HAVE_NEXT           1
#define PACKAGE_END                 0

/*********************************************************************************************************
  �ͻ��˺ͷ���˵Ľ��ջ�����
*********************************************************************************************************/
__PCOMM_INFO  _G_pComm_CBuff = NULL;
__PCOMM_INFO  _G_pComm_SBuff = NULL;

/*********************************************************************************************************
** ��������: initCliCommFd
** ��������: ��ʼ��unix �ͻ���ͨѶͨ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : NONE
*********************************************************************************************************/
VOID  initCliCommFd (VOID)
{
    if (PX_ERROR == access(COMM_DIR_PATH, F_OK)) {
        system("mkdir "COMM_DIR_PATH);                                  /*  ���� UNIX Socket Ŀ¼       */
    }

    _G_iCliCommFd = unixDgramSocket(COMM_CLI_ADDR);
    if (PX_ERROR == _G_iCliCommFd) {
        printf("init _G_iCliCommFd fail\r\n");

        return;
    }

    _G_pComm_CBuff = malloc(COMM_INFO_LEN(COMM_BUFF_LEN));
    if (NULL == _G_pComm_CBuff) {
        printf("init _G_pComm_CBuff fail\r\n");

        return;
    }
}

/*********************************************************************************************************
** ��������: initSerCommFd
** ��������: ��ʼ��unix �ͻ���ͨѶͨ��
** �䡡��  : NONE
** �䡡��  : NONE
** ������  : ERROR_CODE
*********************************************************************************************************/
INT  initSerCommFd (VOID)
{
    if (PX_ERROR == access(COMM_DIR_PATH, F_OK)) {
        system("mkdir "COMM_DIR_PATH);                                  /*  ���� UNIX Socket Ŀ¼       */
    }

    _G_iSerCommFd = unixDgramSocket(COMM_SER_ADDR);
    if (PX_ERROR == _G_iSerCommFd) {
        printf("init _G_iSerCommFd fail\r\n");

        return  (PX_ERROR);
    }

    _G_pComm_SBuff = malloc(COMM_INFO_LEN(COMM_BUFF_LEN));
    if (NULL == _G_pComm_SBuff) {
        printf("init _G_pComm_SBuff fail\r\n");

        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: commRecv
** ��������: ͨ�ý����߼�
** �䡡��  : iRecvFd          ���վ��
**           pCommBuff        ���չ���������
**           pvbuff           ���յ�����Ļ�����
**           iBuffLen         �������ĳ���
** �䡡��  : ipCommand        ���յ���������
**           piRecvLen        ʵ���յ��ó���
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
static INT commRecv(INT   			iRecvFd,
					__PCOMM_INFO    pCommBuff,
					INT  		   *piCommand,
					void           *pvBuff,
					INT            *piRecvLen,
					INT             iBuffLen)
{
	INT                 iRecvLen     = 0;
	INT                 iRet         = COMM_OK;
	char               *pcinput      = (char *)pvBuff;

    if (iRecvFd == -1 && pCommBuff == NULL) {                           /*  δ��ʼ������ʧ��            */
        return COMM_ERROR;
    }

    if (piCommand == NULL || pvBuff == NULL || piRecvLen == NULL) {
        return COMM_ERROR;
    }

    *piRecvLen = 0;
    do {
        iRecvLen = recv(iRecvFd, pCommBuff, COMM_INFO_LEN(COMM_BUFF_LEN), 0);
        if (0 >= iRecvLen) {
            printf("recv error\n");

            return COMM_ERROR;
        }

        *piCommand  = pCommBuff->iCommand;
    	*piRecvLen += pCommBuff->iLen;

        if (pCommBuff->iFlag == PACKAGE_HAVE_NEXT) {
            if (iBuffLen < *piRecvLen) {
                iRet = COMM_ERROR;
            } else {
                memcpy(pcinput, pCommBuff->cBuff, pCommBuff->iLen);
                pcinput += pCommBuff->iLen;
            }
        } else {
            if (iBuffLen < *piRecvLen) {
                iRet = COMM_ERROR;
            } else {
                memcpy(pcinput, pCommBuff->cBuff, pCommBuff->iLen);
            }
        }
    } while (pCommBuff->iFlag == PACKAGE_HAVE_NEXT);

    return iRet;

}

/*********************************************************************************************************
** ��������: clientRecv
** ��������: �ͻ���ͨѶ�ӿڣ��������շ�����������������
** �䡡��  : pvbuff           ���յ�����Ļ�����
**           iBuffLen         �������ĳ���
** �䡡��  : ipCommand        ���յ���������
**           piRecvLen        ʵ���յ��ó���
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
INT  clientRecv (INT  *piCommand, void *pvBuff, INT *piRecvLen, INT iBuffLen)
{
   return commRecv(_G_iCliCommFd, _G_pComm_CBuff, piCommand, pvBuff, piRecvLen, iBuffLen);
}

/*********************************************************************************************************
** ��������: serverRecv
** ��������: �����ͨѶ�ӿڣ��������տͻ���������������
** �䡡��  : pvbuff           ���յ�����Ļ�����
**           iBuffLen         �������ĳ���
** �䡡��  : ipCommand        ���յ���������
**           piRecvLen        ʵ���յ��ó���
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
INT  serverRecv (INT  *piCommand, void *pvBuff, INT *piRecvLen, INT iBuffLen)
{
    return commRecv(_G_iSerCommFd, _G_pComm_SBuff, piCommand, pvBuff, piRecvLen, iBuffLen);
}

/*********************************************************************************************************
** ��������: CommSend
** ��������: ͨ�÷��ͽӿ�
** �䡡��  : iSendFd          ����FD
**           pCommBuff        ͨ�÷��͹��û�����
**           pPeerAddr        Ŀ���ַ
**           iPeerAddrLen     Ŀ���ַ����
**           iCommand         ����������
**           pvSbuff          ���ͻ�����
**           iSbuffLen        ���ͻ���������
** �䡡��  : NONE
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
static INT CommSend(INT             iSendFd,
					__PCOMM_INFO    pCommBuff,
					void           *pPeerAddr,
					INT             iPeerAddrLen,
					INT             iCommand,
					void           *pvSbuff,
					INT             iSbuffLen)
{
    char    *pcinput = (char *)pvSbuff;

    INT      iSbuffLenTmp = iSbuffLen;

    if (iSendFd == -1 && pCommBuff == NULL) {          	                /*  δ��ʼ������ʧ��            */
        return COMM_ERROR;
    }

    /*
     *  ͨѶ�������
     */
    pCommBuff->iCommand = iCommand;
    pCommBuff->iLen     = iSbuffLen;
    pCommBuff->iFlag    = PACKAGE_END;

    do {
        if (iSbuffLen > COMM_BUFF_LEN) {
            pCommBuff->iLen     = COMM_BUFF_LEN;
            pCommBuff->iFlag    = PACKAGE_HAVE_NEXT;
            memcpy((pCommBuff->cBuff), (char*)pcinput, COMM_BUFF_LEN);

            iSbuffLen               -= COMM_BUFF_LEN;
            pcinput                 += COMM_BUFF_LEN;
            iSbuffLenTmp             = COMM_BUFF_LEN;
        } else {
            pCommBuff->iLen     = iSbuffLen;
            pCommBuff->iFlag    = PACKAGE_END;
            memcpy((pCommBuff->cBuff), (char*)pcinput, iSbuffLen);

            iSbuffLenTmp             = iSbuffLen;
            iSbuffLen                = 0;
        }

        sendto(iSendFd,
               pCommBuff,
               COMM_INFO_LEN(iSbuffLenTmp),
               0,
               (struct sockaddr *)pPeerAddr,
               iPeerAddrLen);
    } while (iSbuffLen);

    return COMM_OK;
}

/*********************************************************************************************************
** ��������: serverSend
** ��������: �����ͨѶ�ӿڣ��������ش�����
** �䡡��  : iCommand         ����������
**           pvSbuff          ���ͻ�����
**           iSbuffLen        ���ͻ���������
** �䡡��  : NONE
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
INT  serverSend (INT iCommand, void *pvSbuff, INT iSbuffLen)
{
    static struct sockaddr_un  sPeerAddr;                               /*  �Զ� socket ��ַ            */
    static INT                 iPeerAddrLen = 0;                        /*  �Զ� socket ��ַ����        */

    if (iPeerAddrLen == 0) {                                            /*  ��ʼ���Զ˵�                */
       unixDgramSockAddr(COMM_CLI_ADDR, &sPeerAddr, &iPeerAddrLen);
    }

    return CommSend(_G_iSerCommFd, _G_pComm_SBuff, &sPeerAddr, iPeerAddrLen, iCommand, pvSbuff, iSbuffLen);
}

/*********************************************************************************************************
** ��������: clientSend
** ��������: �ͻ��˷��ͽӿ�
** �䡡��  : iCommand         ����������
**           pvSbuff          ���ͻ�����
**           iSbuffLen        ���ͻ���������
** �䡡��  : NONE
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
INT  clientSend (INT iCommand, void *pvSbuff, INT iSbuffLen)
{
    static struct sockaddr_un  sPeerAddr;                               /*  �Զ� socket ��ַ            */
    static INT                 iPeerAddrLen = 0;                        /*  �Զ� socket ��ַ����        */

    if (iPeerAddrLen == 0) {                                            /*  ��ʼ���Զ˵�                */
       unixDgramSockAddr(COMM_SER_ADDR, &sPeerAddr, &iPeerAddrLen);
    }

    return CommSend(_G_iCliCommFd, _G_pComm_CBuff, &sPeerAddr, iPeerAddrLen, iCommand, pvSbuff, iSbuffLen);
}

/*********************************************************************************************************
** ��������: cliSendAndRecv
** ��������: �ͻ���ͨѶ�ӿڣ��ýӿڷ����꣬�ȴ���������
** �䡡��  : iCommand         ����������
**           pvSbuff          ���ͻ�����
**           iSbuffLen        ���ͻ���������
** �䡡��  : pvRbuff          ���ջ�����
**           iRbuffLen        ���ջ���������
** ������  : COMM_ERROR       ����ʧ��
**           COMM_OK          ���ͳɹ�
*********************************************************************************************************/
INT  cliSendAndRecv (INT iCommand, void *pvSbuff, INT iSbuffLen, void *pvRbuff, INT iRbuffLen)
{
    INT iRecvCmd;
    INT iRecvLen;

    if (clientSend(iCommand, pvSbuff, iSbuffLen) == COMM_ERROR) {
        return COMM_ERROR;
    }

    return clientRecv(&iRecvCmd, pvRbuff, &iRecvLen, iRbuffLen);
}
