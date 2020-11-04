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
** 文   件   名: msgcomm.c
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 业务通信库接口
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/libcomm.h"
/*********************************************************************************************************
  消息头
*********************************************************************************************************/
struct msg_header {
    INT  iMsgCmdId;                                                       /*  处理结果                  */
    INT  iMsgTotalLen;                                                    /*  消息总长度                */
    INT  iNodeTotalNum;                                                   /*  节点总个数                */
    INT  iNodeLength;                                                     /*  单个节点长度(字节）       */
    INT  iResult;                                                         /*  处理结果                  */
};
typedef struct msg_header MSG_HEADER;
/*********************************************************************************************************
** 函数名称: requestMsgFree
** 功能描述: 释放资源接口
** 输　入  : pvSource           内存首地址
** 输　出  : none
** 返　回  : none
*********************************************************************************************************/
VOID requestMsgFree (PVOID  pvReSource)
{
    if (pvReSource) {
        free(pvReSource);
    }
}

/*********************************************************************************************************
** 函数名称: requestQueryMsg
** 功能描述: 向服务器端发送查询消息，并获取查询结果和查询条数。
** 输　入  : iCmdId         查询命令字
**           pArg           查询参数
**           iArgLen        查询参数长度
** 输　出  : iTotalNum      查询结果条数
** 返　回  : (PVOID)(pcInfoBuf) 查询信息结果指针
*********************************************************************************************************/
PVOID  requestQueryMsg (INT iCmdId, PVOID  pArg, INT iArgLen, INT* iTotalNum)
{
    INT         iRet;
    PCHAR       pcInfoBuf      = LW_NULL;
    INT         iRecvLen       = 0;
    INT         iCommand       = 0;
    MSG_HEADER  msgHeader      = {0};

    *iTotalNum = 0;

    iRet = cliSendAndRecv(iCmdId, pArg, iArgLen, &msgHeader, sizeof(msgHeader));
    if (0 > iRet) {
        return  (LW_NULL);
    }

    if (msgHeader.iMsgTotalLen <= 0) {
        return  (LW_NULL);
    }

    pcInfoBuf = (PCHAR)malloc(msgHeader.iMsgTotalLen);
    if (LW_NULL == pcInfoBuf) {
        return  (LW_NULL);
    }

    iRet = clientRecv(&iCommand, pcInfoBuf, &iRecvLen, msgHeader.iMsgTotalLen);

    if (msgHeader.iMsgTotalLen != iRecvLen) {
        printf(" Received request query msg length incorrect msgHeader,"
               " iRet=%d, iMsgTotalLen=%d, iRecvLen=%d, iCommand=0x%x\n",
               iRet, msgHeader.iMsgTotalLen, iRecvLen, iCommand);
    }

    *iTotalNum = msgHeader.iNodeTotalNum;

    return  (PVOID)(pcInfoBuf);
}

/*********************************************************************************************************
** 函数名称: requestConfigMsg
** 功能描述: 向服务器端发送配置消息，并获取配置结果。
** 输　入  : iCmdId         查询命令字
**           pArg           配置参数
**           iArgLen        配置参数长度
** 输　出  : NONE
** 返　回  : INT iResult 配置结果
*********************************************************************************************************/
INT  requestConfigMsg (INT iCmdId, PVOID  pArg, INT iArgLen)
{
    INT          iRet      = PX_ERROR;
    MSG_HEADER   msgHeader = {0};

    iRet = cliSendAndRecv(iCmdId, pArg, iArgLen, &msgHeader, sizeof(MSG_HEADER));
    if (0 > iRet) {                                                     /*  获取失败                    */
        return  (PX_ERROR);
    }

    return msgHeader.iResult;
}

/*********************************************************************************************************
** 函数名称: replyQueryMsg
** 功能描述: 回复客户端查询请求，返回查询结果和查询条数。
** 输　入  : iCmdId          查询命令字
**           iNodeLength     单个节点长度
**           iNodeTotalNum   查询结果总数
**           pvMsgBuff       Msg内容
** 输　出  : NONE
** 返　回  : iRet           消息发送结果
*********************************************************************************************************/
INT replyQueryMsg (INT iCmdId, INT iNodeLength, INT iNodeTotalNum, PVOID  pvMsgBuff)
{
    MSG_HEADER   msgHeader ={0};
    INT         iRet       = 0;

    if(iNodeLength <=0)
    {
        return PX_ERROR;
    }

    msgHeader.iMsgTotalLen  = iNodeLength * iNodeTotalNum ;
    msgHeader.iNodeTotalNum = iNodeTotalNum;
    msgHeader.iNodeLength   = iNodeLength;
    iRet = serverSend(iCmdId, &msgHeader, sizeof(MSG_HEADER));

    if (0 == msgHeader.iNodeTotalNum) {
        return iRet;
    }

    iRet = serverSend(iCmdId, pvMsgBuff, msgHeader.iMsgTotalLen);

    return iRet;
}

/*********************************************************************************************************
** 函数名称: replyConfigMsg
** 功能描述: 回复客户端配置请求，返回配置结果
** 输　入  : iCmdId             配置命令字
**           iResult            配置结果
** 输　出  : NONE
** 返　回  : 配置结果
*********************************************************************************************************/
INT replyConfigMsg(INT iCmdId, INT iResult)
{
    MSG_HEADER   msgHeader = {0};

    msgHeader.iMsgCmdId     = iCmdId;
    msgHeader.iMsgTotalLen  = sizeof(INT);
    msgHeader.iNodeLength   = sizeof(INT);
    msgHeader.iNodeTotalNum = 1;
    msgHeader.iResult       = iResult;

    return serverSend(iCmdId, &msgHeader, sizeof(msgHeader));
}

