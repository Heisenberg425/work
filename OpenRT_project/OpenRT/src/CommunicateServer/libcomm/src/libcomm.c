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
** 文   件   名: libcomm.c
**
** 创   建   人: Li.xiaocheng (李孝成)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: socket 通信库接口
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <socket.h>
#include "unixSocket.h"
#include "../include/libcomm.h"
/*********************************************************************************************************
  UNIX socket 文件
*********************************************************************************************************/
#define COMM_DIR_PATH     "/tmp/OpenRT"                                 /*  socket 文件根目录           */
#define COMM_SER_ADDR     COMM_DIR_PATH"/Saddr"                         /*  服务端文件                  */
#define COMM_CLI_ADDR     COMM_DIR_PATH"/Caddr"                         /*  客户端文件                  */

/*********************************************************************************************************
  客户端通信句柄
*********************************************************************************************************/
static INT                 _G_iCliCommFd = -1;                          /*  客户端socket 描述           */
static INT                 _G_iSerCommFd = -1;                          /*  服务端socket 描述           */

/*********************************************************************************************************
  通讯结构体定义
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
  通讯结构体定义
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
  通讯iFlag定义
*********************************************************************************************************/
#define PACKAGE_HAVE_NEXT           1
#define PACKAGE_END                 0

/*********************************************************************************************************
  客户端和服务端的接收缓冲区
*********************************************************************************************************/
__PCOMM_INFO  _G_pComm_CBuff = NULL;
__PCOMM_INFO  _G_pComm_SBuff = NULL;

/*********************************************************************************************************
** 函数名称: initCliCommFd
** 功能描述: 初始化unix 客户端通讯通道
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
VOID  initCliCommFd (VOID)
{
    if (PX_ERROR == access(COMM_DIR_PATH, F_OK)) {
        system("mkdir "COMM_DIR_PATH);                                  /*  创建 UNIX Socket 目录       */
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
** 函数名称: initSerCommFd
** 功能描述: 初始化unix 客户端通讯通道
** 输　入  : NONE
** 输　出  : NONE
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  initSerCommFd (VOID)
{
    if (PX_ERROR == access(COMM_DIR_PATH, F_OK)) {
        system("mkdir "COMM_DIR_PATH);                                  /*  创建 UNIX Socket 目录       */
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
** 函数名称: commRecv
** 功能描述: 通用接收逻辑
** 输　入  : iRecvFd          接收句柄
**           pCommBuff        接收公共缓冲区
**           pvbuff           接收到命令的缓冲区
**           iBuffLen         缓冲区的长度
** 输　出  : ipCommand        接收到的命令字
**           piRecvLen        实际收到得长度
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
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

    if (iRecvFd == -1 && pCommBuff == NULL) {                           /*  未初始化返回失败            */
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
** 函数名称: clientRecv
** 功能描述: 客户端通讯接口，用来接收服务器送来的命令字
** 输　入  : pvbuff           接收到命令的缓冲区
**           iBuffLen         缓冲区的长度
** 输　出  : ipCommand        接收到的命令字
**           piRecvLen        实际收到得长度
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
*********************************************************************************************************/
INT  clientRecv (INT  *piCommand, void *pvBuff, INT *piRecvLen, INT iBuffLen)
{
   return commRecv(_G_iCliCommFd, _G_pComm_CBuff, piCommand, pvBuff, piRecvLen, iBuffLen);
}

/*********************************************************************************************************
** 函数名称: serverRecv
** 功能描述: 服务端通讯接口，用来接收客户端送来的命令字
** 输　入  : pvbuff           接收到命令的缓冲区
**           iBuffLen         缓冲区的长度
** 输　出  : ipCommand        接收到的命令字
**           piRecvLen        实际收到得长度
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
*********************************************************************************************************/
INT  serverRecv (INT  *piCommand, void *pvBuff, INT *piRecvLen, INT iBuffLen)
{
    return commRecv(_G_iSerCommFd, _G_pComm_SBuff, piCommand, pvBuff, piRecvLen, iBuffLen);
}

/*********************************************************************************************************
** 函数名称: CommSend
** 功能描述: 通用发送接口
** 输　入  : iSendFd          发送FD
**           pCommBuff        通用发送公用缓冲区
**           pPeerAddr        目标地址
**           iPeerAddrLen     目标地址长度
**           iCommand         发送命令字
**           pvSbuff          发送缓冲区
**           iSbuffLen        发送缓冲区长度
** 输　出  : NONE
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
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

    if (iSendFd == -1 && pCommBuff == NULL) {          	                /*  未初始化返回失败            */
        return COMM_ERROR;
    }

    /*
     *  通讯报文填充
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
** 函数名称: serverSend
** 功能描述: 服务端通讯接口，用来返回处理结果
** 输　入  : iCommand         发送命令字
**           pvSbuff          发送缓冲区
**           iSbuffLen        发送缓冲区长度
** 输　出  : NONE
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
*********************************************************************************************************/
INT  serverSend (INT iCommand, void *pvSbuff, INT iSbuffLen)
{
    static struct sockaddr_un  sPeerAddr;                               /*  对端 socket 地址            */
    static INT                 iPeerAddrLen = 0;                        /*  对端 socket 地址长度        */

    if (iPeerAddrLen == 0) {                                            /*  初始化对端地                */
       unixDgramSockAddr(COMM_CLI_ADDR, &sPeerAddr, &iPeerAddrLen);
    }

    return CommSend(_G_iSerCommFd, _G_pComm_SBuff, &sPeerAddr, iPeerAddrLen, iCommand, pvSbuff, iSbuffLen);
}

/*********************************************************************************************************
** 函数名称: clientSend
** 功能描述: 客户端发送接口
** 输　入  : iCommand         发送命令字
**           pvSbuff          发送缓冲区
**           iSbuffLen        发送缓冲区长度
** 输　出  : NONE
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
*********************************************************************************************************/
INT  clientSend (INT iCommand, void *pvSbuff, INT iSbuffLen)
{
    static struct sockaddr_un  sPeerAddr;                               /*  对端 socket 地址            */
    static INT                 iPeerAddrLen = 0;                        /*  对端 socket 地址长度        */

    if (iPeerAddrLen == 0) {                                            /*  初始化对端地                */
       unixDgramSockAddr(COMM_SER_ADDR, &sPeerAddr, &iPeerAddrLen);
    }

    return CommSend(_G_iCliCommFd, _G_pComm_CBuff, &sPeerAddr, iPeerAddrLen, iCommand, pvSbuff, iSbuffLen);
}

/*********************************************************************************************************
** 函数名称: cliSendAndRecv
** 功能描述: 客户端通讯接口，该接口发送完，等待接收数据
** 输　入  : iCommand         发送命令字
**           pvSbuff          发送缓冲区
**           iSbuffLen        发送缓冲区长度
** 输　出  : pvRbuff          接收缓冲区
**           iRbuffLen        接收缓冲区长度
** 返　回  : COMM_ERROR       发送失败
**           COMM_OK          发送成功
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
