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
** 文   件   名: msgcomm.h
**
** 创   建   人: Chen.Hongbang (陈洪邦)
**
** 文件创建日期: 2018 年 6 月 20 日
**
** 描        述: 业务通信库接口
*********************************************************************************************************/
#ifndef _MSGCOMM_H_
#define _MSGCOMM_H_
#include <SylixOS.h>

/*********************************************************************************************************
** 函数名称: requestMsgFree
** 功能描述: 释放资源接口
** 输　入  : pvSource           内存首地址
** 输　出  : none
** 返　回  : none
*********************************************************************************************************/
VOID requestMsgFree (PVOID  pvSource);

/*********************************************************************************************************
** 函数名称: requestQueryMsg
** 功能描述: 向服务器端发送查询消息，并获取查询结果和查询条数。
** 输　入  : iCmdId         查询命令字
**           pArg           查询参数
**           iArgLen        查询参数长度
** 输　出  : iTotalNum      查询结果条数
** 返　回  : (PVOID)(pcInfoBuf) 查询信息结果指针
*********************************************************************************************************/
PVOID  requestQueryMsg (INT iCmdId, PVOID  pArg, INT iArgLen, INT* iTotalNum);

/*********************************************************************************************************
** 函数名称: sendConfigMsg
** 功能描述: 向服务器端发送配置消息，并获取配置结果。
** 输　入  : iCmdId         查询命令字
**           pArg           查询参数
**           iArgLen        查询参数长度
** 输　出  : NONE
** 返　回  : INT iResult 配置结果
*********************************************************************************************************/
INT  requestConfigMsg (INT iCmdId, PVOID  pArg, INT iArgLen);

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
INT replyQueryMsg (INT iCmdId, INT iNodeLength, INT iNodeTotalNum, PVOID  pvMsgBuff);

/*********************************************************************************************************
** 函数名称: replyConfigMsg
** 功能描述: 回复客户端配置请求，返回配置结果
** 输　入  : iCmdId             配置命令字
**           iResult            配置结果
** 输　出  : NONE
** 返　回  : 配置结果
*********************************************************************************************************/
INT replyConfigMsg(INT iCmdId, INT iResult);

#endif                                                                  /* _MSGCOMM_H_                  */
