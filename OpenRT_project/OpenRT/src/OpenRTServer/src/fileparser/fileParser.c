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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*********************************************************************************************************
** ��������: compact_strtrim
** ��������: ȥ�����߿ո�"\n"
** �䡡��  : s                  �ַ���
** �䡡��  : none
** ������  : s                  �޸ĺ���ַ���
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
** ��������: add_byte
** ��������: ��һ�����һ���ַ�
** �䡡��  : c                  ��ӵ��ַ�
**           lineptr            buf
**           slen               ��ǰ�����г���
**           n                  buf����
** �䡡��  : none
** ������  : -1                 ʧ��
**           0                  �ɹ�
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
** ��������: compat_getline(getline���ݺ���)
** ��������: ��ȡ�ļ�һ������
** �䡡��  : lineptr            buf
**           n                  buf����
**           stream             �ļ���
** �䡡��  : none
** ������  : slen               һ�г���
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
** ��������: specifiedStrFind
** ��������: �ļ�ָ������Ѱ��(��һ�γ���λ��)
** �䡡��  : cpFilename         �ļ���
**           cpStrFind          ָ��Ѱ������
**           cppLineBuf         �����е�����
**           uipLineNum         ��������
**           uipLineOffset      ����������Ӧ��ƫ��
** �䡡��  : none
** ������  : slen               �������ܳ���
**          -1                  ʧ��
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

    rewind(pFiledId);                                                   /*  ȷ����ͷ��                  */

    for (;;) {
        slen = compat_getline(cppLineBuf, &sBufLen, pFiledId);
        (*uipLineNum)++;
        if (-1 != slen) {
            cpTmp = strstr(*cppLineBuf, cpStrFind);
            if (NULL != cpTmp) {                                        /*  �ҵ���һ�γ��ֵ�λ��        */
                (*uipLineOffset)  += cpTmp - *cppLineBuf;               /*  ��ȡ����ƫ�ƴ�С            */
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
** ��������: specifiedLineRead
** ��������: ��ȡ�ļ�ָ��һ������
**           (ע���������ڲ�ʹ��compat_getlineʵ�֣�cpLineBuf��spBufLen��
**                getline�е�lineptr��n��ͬ������ʹ��fgets�ȶ�֮ǰ�����ݣ�����
**                ��Ҫ��ȡ��һ��ʹ��compat_getlineʵ��)
** �䡡��  : cpFilename         �ļ���
**           uiLineNum          ָ����
**           cppLineBuf         buf��ַ
**           psBufLen           buf����(Ϊ0����ʾbufΪ��ָ��)
** �䡡��  : none
** ������  : slen               �������ܳ���
**          -1                  ʧ��
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

    rewind(pFiledId);                                                   /*  ȷ����ͷ��                  */

    while((i < (uiLineNum - 1)) && (c = fgetc(pFiledId)) != EOF) {      /*  ������Ӧ����                */
        if (c == '\n') {
            i++;
        }
    }

    slen = compat_getline(cppLineBuf, psBufLen, pFiledId);

    fclose(pFiledId);

    return  (slen);
}

/*********************************************************************************************************
** ��������: specifiedLineInfoGet
** ��������: ��ȡһ����ָ���ֶ��м������
** �䡡��  : pcKeyChStart       ��ʼ�ֶ�
**           pcKeyChEnd         �����ֶ�
**           pcLineBuff         ������
** �䡡��  : pcInfoBuff         ��ȡ������Ϣ
** ������  : ERROE_CODE
*********************************************************************************************************/
INT32  specifiedLineInfoGet (PCHAR  pcKeyChStart, PCHAR  pcKeyChEnd, PCHAR  pcLineBuff, PCHAR  pcInfoBuff)
{
    PCHAR           pcStartBuff  = LW_NULL;
    PCHAR           pcEndBuff    = LW_NULL;

    if (LW_NULL == pcLineBuff   || LW_NULL == pcInfoBuff) {
        return  (PX_ERROR);
    }

    if (LW_NULL == pcKeyChStart && LW_NULL == pcKeyChStart) {           /*  û��ָ��ǰ���ֶ�,ֱ�ӿ���   */
        strncpy(pcInfoBuff, pcLineBuff, strlen(pcLineBuff));
    }

    if (LW_NULL != pcKeyChStart) {                                      /*  ָ���˿�ʼ�ֶ�              */
        pcStartBuff = strstr(pcLineBuff, pcKeyChStart);
        if (LW_NULL == pcStartBuff) {                                   /*  �Ҳ�����ʼ�ֶ�              */
            return  (PX_ERROR);
        }
        pcStartBuff += strlen(pcKeyChStart);
    } else {                                                            /*  û��ָ����ʼ�ֶ�            */
        pcStartBuff = pcLineBuff;
    }

    if (LW_NULL != pcKeyChEnd) {                                        /*  ָ���˽����ֶ�              */
        pcEndBuff = strstr(pcLineBuff, pcKeyChEnd);
        if (LW_NULL == pcStartBuff) {                                   /*  �Ҳ��������ֶ�              */
            return  (PX_ERROR);
        }

        if (pcEndBuff - pcStartBuff < 0) {                              /*  ǰ���ֶ�д��                */
            return  (PX_ERROR);
        }

        strncpy(pcInfoBuff, pcStartBuff, pcEndBuff - pcStartBuff);
    } else {                                                            /*  û��ָ�������ֶ�            */
        strncpy(pcInfoBuff, pcStartBuff, strlen(pcStartBuff));
    }

    return  (ERROR_NONE);
}
