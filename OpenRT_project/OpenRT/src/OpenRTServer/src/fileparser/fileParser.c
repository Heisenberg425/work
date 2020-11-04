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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*********************************************************************************************************
** 函数名称: compact_strtrim
** 功能描述: 去除两边空格及"\n"
** 输　入  : s                  字符串
** 输　出  : none
** 返　回  : s                  修改后的字符串
*********************************************************************************************************/
char  *compact_strtrim (char *s)
{
  int i = 0;
  int j = strlen(s) - 1;
  int k = 0;

  while (isspace(s[i]) && s[i] != '\0') {
      i++;
  }

  while (isspace(s[j]) && j >= 0) {
      j--;
  }

  while(i <= j) {
      s[k++] = s[i++];
  }

  s[k] = '\0';

  return s;
}
/*********************************************************************************************************
** 函数名称: add_byte
** 功能描述: 往一行添加一个字符
** 输　入  : c                  添加的字符
**           lineptr            buf
**           slen               当前读到行长度
**           n                  buf长度
** 输　出  : none
** 返　回  : -1                 失败
**           0                  成功
*********************************************************************************************************/
static int add_byte (int c, char **lineptr, size_t slen, size_t *n)
{
#define LINE_GROWTH 16
    char *nline;
    size_t new_size = slen + 1;
    if (new_size > *n) {
        new_size += LINE_GROWTH - 1;
        new_size *= 2;
        nline = realloc(*lineptr, new_size);
        if (!nline)
            return -1;

        *lineptr = nline;
        *n = new_size;
    }

    (*lineptr)[slen] = c;

    return 0;
}

/*********************************************************************************************************
** 函数名称: compat_getline(getline兼容函数)
** 功能描述: 读取文件一行内容
** 输　入  : lineptr            buf
**           n                  buf长度
**           stream             文件流
** 输　出  : none
** 返　回  : slen               一行长度
*********************************************************************************************************/
static ssize_t compat_getline(char **lineptr, size_t *n, FILE *stream)
{
    char *line = *lineptr;
    size_t slen = 0;

    for (;;) {
        int c = getc(stream);
        switch (c) {
        case '\n':
            if (add_byte(c, &line, slen, n) < 0)
                goto e_out;
            slen++;
            /* fall through */
        case EOF:
            if (add_byte('\0', &line, slen, n) < 0)
                goto e_out;
            *lineptr = line;
            if (slen == 0)
                return -1;
            return slen;
        default:
            if (add_byte(c, &line, slen, n) < 0)
                goto e_out;
            slen++;
        }
    }

e_out:
    line[slen-1] = '\0';
    *lineptr = line;
    return -1;
}

/*********************************************************************************************************
** 函数名称: specifiedStrFind
** 功能描述: 文件指定内容寻找(第一次出现位置)
** 输　入  : cpFilename         文件名
**           cpStrFind          指定寻找内容
**           cppLineBuf         所在行的内容
**           uipLineNum         所在行数
**           uipLineOffset      所在行数对应的偏移
** 输　出  : none
** 返　回  : slen               所在行总长度
**          -1                  失败
*********************************************************************************************************/
int specifiedStrFind (char          *cpFilename,
                      char          *cpStrFind,
                      char         **cppLineBuf,
                      unsigned int  *uipLineNum,
                      unsigned int  *uipLineOffset)
{
    char   *cpTmp     = NULL;
    size_t  sBufLen   = 0;
    size_t  slen      = -1;

    *uipLineNum       = 0;
    *uipLineOffset    = 0;
    *cppLineBuf       = NULL;

    if (NULL == cpFilename || NULL == cpStrFind) {
        return  (-1);
    }

    FILE  *pFiledId = NULL;

    pFiledId = fopen(cpFilename, "r");
    if (NULL == pFiledId) {
        printf("open file failed.\n");

        return  (PX_ERROR);
    }

    rewind(pFiledId);                                                   /*  确保从头读                  */

    for (;;) {
        slen = compat_getline(cppLineBuf, &sBufLen, pFiledId);
        (*uipLineNum)++;
        if (-1 != slen) {
            cpTmp = strstr(*cppLineBuf, cpStrFind);
            if (NULL != cpTmp) {                                        /*  找到第一次出现的位置        */
                (*uipLineOffset)  += cpTmp - *cppLineBuf;               /*  获取最终偏移大小            */
                fclose(pFiledId);

                return  (slen);
            } else {
                free(*cppLineBuf);
                *cppLineBuf  = NULL;
                sBufLen      = 0;
            }
        } else {
            break;
        }
    }

    fclose(pFiledId);

    return  (-1);
}

/*********************************************************************************************************
** 函数名称: specifiedLineRead
** 功能描述: 读取文件指定一行内容
**           (注：本函数内部使用compat_getline实现，cpLineBuf和spBufLen与
**                getline中的lineptr和n相同；另外使用fgets先读之前行内容，最终
**                需要读取的一行使用compat_getline实现)
** 输　入  : cpFilename         文件名
**           uiLineNum          指定行
**           cppLineBuf         buf地址
**           psBufLen           buf长度(为0，表示buf为空指针)
** 输　出  : none
** 返　回  : slen               所在行总长度
**          -1                  失败
*********************************************************************************************************/
ssize_t specifiedLineRead (char          *cpFilename,
                           unsigned int   uiLineNum,
                           char         **cppLineBuf,
                           size_t         *psBufLen)
{
    int           i    = 0;
    int           c;
    size_t        slen = -1;

    if (NULL == cpFilename || uiLineNum == 0) {
        return  (-1);
    }

    FILE  *pFiledId = NULL;

    pFiledId = fopen(cpFilename, "r");
    if (NULL == pFiledId) {
        printf("open file failed.\n");

        return  (-1);
    }

    rewind(pFiledId);                                                   /*  确保从头读                  */

    while((i < (uiLineNum - 1)) && (c = fgetc(pFiledId)) != EOF) {      /*  跳过对应行数                */
        if (c == '\n') {
            i++;
        }
    }

    slen = compat_getline(cppLineBuf, psBufLen, pFiledId);

    fclose(pFiledId);

    return  (slen);
}

/*********************************************************************************************************
** 函数名称: specifiedLineInfoGet
** 功能描述: 获取一行中指定字段中间的内容
** 输　入  : pcKeyChStart       开始字段
**           pcKeyChEnd         结束字段
**           pcLineBuff         行内容
** 输　出  : pcInfoBuff         获取到的信息
** 返　回  : ERROE_CODE
*********************************************************************************************************/
INT32  specifiedLineInfoGet (PCHAR  pcKeyChStart, PCHAR  pcKeyChEnd, PCHAR  pcLineBuff, PCHAR  pcInfoBuff)
{
    PCHAR           pcStartBuff  = LW_NULL;
    PCHAR           pcEndBuff    = LW_NULL;

    if (LW_NULL == pcLineBuff   || LW_NULL == pcInfoBuff) {
        return  (PX_ERROR);
    }

    if (LW_NULL == pcKeyChStart && LW_NULL == pcKeyChStart) {           /*  没有指定前后字段,直接拷贝   */
        strncpy(pcInfoBuff, pcLineBuff, strlen(pcLineBuff));
    }

    if (LW_NULL != pcKeyChStart) {                                      /*  指定了开始字段              */
        pcStartBuff = strstr(pcLineBuff, pcKeyChStart);
        if (LW_NULL == pcStartBuff) {                                   /*  找不到开始字段              */
            return  (PX_ERROR);
        }
        pcStartBuff += strlen(pcKeyChStart);
    } else {                                                            /*  没有指定开始字段            */
        pcStartBuff = pcLineBuff;
    }

    if (LW_NULL != pcKeyChEnd) {                                        /*  指定了结束字段              */
        pcEndBuff = strstr(pcLineBuff, pcKeyChEnd);
        if (LW_NULL == pcStartBuff) {                                   /*  找不到结束字段              */
            return  (PX_ERROR);
        }

        if (pcEndBuff - pcStartBuff < 0) {                              /*  前后字段写反                */
            return  (PX_ERROR);
        }

        strncpy(pcInfoBuff, pcStartBuff, pcEndBuff - pcStartBuff);
    } else {                                                            /*  没有指定结束字段            */
        strncpy(pcInfoBuff, pcStartBuff, strlen(pcStartBuff));
    }

    return  (ERROR_NONE);
}
