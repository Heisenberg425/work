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
** ��   ��   ��: fileparser.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 �� 5 �� 10 ��
**
** ��        ��: �ļ������Ĺ��ܽӿ�
*********************************************************************************************************/

#ifndef __FILEPARSER_FILEPARASER_H_
#define __FILEPARSER_FILEPARASER_H_

char  *compact_strtrim (char *s);
int specifiedStrFind (char          *cpFilename,
                      char          *cpStrFind,
                      char         **cppLineBuf,
                      unsigned int  *uipLineNum,
                      unsigned int  *uipLineOffset);
ssize_t specifiedLineRead (char          *cpFilename,
                           unsigned int   uiLineNum,
                           char         **cppLineBuf,
                           size_t         *psBufLen);
INT32  specifiedLineInfoGet (PCHAR  pcKeyChStart,
                             PCHAR  pcKeyChEnd,
                             PCHAR  pcLineBuff,
                             PCHAR  pcInfoBuff);
#endif                                                                  /*  __FILEPARSER_FILEPARASER_H_ */
/*********************************************************************************************************
  END
*********************************************************************************************************/
