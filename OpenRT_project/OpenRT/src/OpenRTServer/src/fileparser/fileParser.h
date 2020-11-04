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
** 文   件   名: fileparser.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年 5 月 10 日
**
** 描        述: 文件解析的功能接口
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
