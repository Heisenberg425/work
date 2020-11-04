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
** ��   ��   ��: msgcomm.c
**
** ��   ��   ��: Chen.Hongbang (�º��)
**
** �ļ���������: 2018 �� 6 �� 20 ��
**
** ��        ��: ҵ��ͨ�ſ�ӿ�
*********************************************************************************************************/
#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/libcomm.h"
/*********************************************************************************************************
  ��Ϣͷ
*********************************************************************************************************/
struct msg_header {
    INT  iMsgCmdId;                                                       /*  ������                  */
    INT  iMsgTotalLen;                                                    /*  ��Ϣ�ܳ���                */
    INT  iNodeTotalNum;                                                   /*  �ڵ��ܸ���                */
    INT  iNodeLength;                                                     /*  �����ڵ㳤��(�ֽڣ�       */
    INT  iResult;                                                         /*  ������                  */
};
typedef struct msg_header MSG_HEADER;
/*********************************************************************************************************
** ��������: requestMsgFree
** ��������: �ͷ���Դ�ӿ�
** �䡡��  : pvSource           �ڴ��׵�ַ
** �䡡��  : none
** ������  : none
*********************************************************************************************************/
VOID requestMsgFree (PVOID  pvReSource)
{
    if (pvReSource) {
        free(pvReSource);
    }
}

/*********************************************************************************************************
** ��������: requestQueryMsg
** ��������: ��������˷��Ͳ�ѯ��Ϣ������ȡ��ѯ����Ͳ�ѯ������
** �䡡��  : iCmdId         ��ѯ������
**           pArg           ��ѯ����
**           iArgLen        ��ѯ��������
** �䡡��  : iTotalNum      ��ѯ�������
** ������  : (PVOID)(pcInfoBuf) ��ѯ��Ϣ���ָ��
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
** ��������: requestConfigMsg
** ��������: ��������˷���������Ϣ������ȡ���ý����
** �䡡��  : iCmdId         ��ѯ������
**           pArg           ���ò���
**           iArgLen        ���ò�������
** �䡡��  : NONE
** ������  : INT iResult ���ý��
*********************************************************************************************************/
INT  requestConfigMsg (INT iCmdId, PVOID  pArg, INT iArgLen)
{
    INT          iRet      = PX_ERROR;
    MSG_HEADER   msgHeader = {0};

    iRet = cliSendAndRecv(iCmdId, pArg, iArgLen, &msgHeader, sizeof(MSG_HEADER));
    if (0 > iRet) {                                                     /*  ��ȡʧ��                    */
        return  (PX_ERROR);
    }

    return msgHeader.iResult;
}

/*********************************************************************************************************
** ��������: replyQueryMsg
** ��������: �ظ��ͻ��˲�ѯ���󣬷��ز�ѯ����Ͳ�ѯ������
** �䡡��  : iCmdId          ��ѯ������
**           iNodeLength     �����ڵ㳤��
**           iNodeTotalNum   ��ѯ�������
**           pvMsgBuff       Msg����
** �䡡��  : NONE
** ������  : iRet           ��Ϣ���ͽ��
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
** ��������: replyConfigMsg
** ��������: �ظ��ͻ����������󣬷������ý��
** �䡡��  : iCmdId             ����������
**           iResult            ���ý��
** �䡡��  : NONE
** ������  : ���ý��
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

