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
** ��   ��   ��: udial_parser.c
**
** ��   ��   ��: Zeng.Bo (����)
**
** �ļ���������: 2016 �� 01 �� 30 ��
**
** ��        ��: PPP ���������ļ�������
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#define  __SYLIXOS_STDIO
#include <SylixOS.h>
#include <stdio.h>
#include <malloc.h>
#include "udial_parser.h"
/*********************************************************************************************************
  ini ����
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE     INIK_lineManage;
    PCHAR            INIK_pcKey;
    PCHAR            INIK_pcValue;
} LW_INI_KEY;
typedef LW_INI_KEY  *PLW_INI_KEY;

typedef struct {
    PLW_LIST_LINE    INIS_plineKey;
} LW_INI_SEC;
typedef LW_INI_SEC  *PLW_INI_SEC;

/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define LW_INI_BUF_SZ       128

#define __IS_WHITE(c)       (c == ' ' || c == '\t' || c == '\r' || c == '\n')
#define __IS_END(c)         (c == PX_EOS)
#define __SKIP_WHITE(str)   while (__IS_WHITE(*str)) {  \
                                str++;  \
                            }
#define __NEXT_WHITE(str)   while (!__IS_WHITE(*str) && !__IS_END(*str)) { \
                                str++;  \
                            }

#define __IS_NEWLINE(c)     (c == '\r' || c == '\n')
#define __IS_WHITE2(c)      (c == ' ' || c == '\t')
#define __SKIP_WHITE2(str)  while (__IS_WHITE2(*str)) {  \
                                str++;  \
                            }

#define __SKIP_WHITE3(str)  while (__IS_WHITE2(*str)) {  \
                                str--;  \
                            }


#define __UTIAL_PPP_PATH   "/etc/ppp/ppp.ini"
/*********************************************************************************************************
** ��������: __iniLoadSec
** ��������: װ��һ�������ļ�ָ����
** �䡡��  : pinisec       INI ���
**           fp            �ļ����
** �䡡��  : INI ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __iniLoadSec (PLW_INI_SEC  pinisec, FILE  *fp)
{
    PLW_INI_KEY     pinikey;
    CHAR            cBuf[LW_INI_BUF_SZ];
    PCHAR           pcLine;
    PCHAR           pcEnd;
    PCHAR           pcEqu;

    PCHAR           pcKey;
    size_t          stKeyLen;
    PCHAR           pcValue;
    size_t          stValueLen;

    for (;;) {
        pcLine = fgets(cBuf, LW_INI_BUF_SZ, fp);
        if (!pcLine) {
            break;
        }

        __SKIP_WHITE(pcLine);
        if (__IS_END(*pcLine) || (*pcLine == ';') || (*pcLine == '#')) {
            continue;
        }

        if (*pcLine == '[') {                                           /*  �Ѿ�����һ����              */
            break;
        }

        pcEqu = lib_strchr(pcLine, '=');
        if (!pcEqu) {
            continue;
        }

        /*
         * Area Code = cc
         * the key may contain null-space
         */
        pcEnd = pcEqu - 1;
        __SKIP_WHITE3(pcEnd);
        pcEnd++;
        *pcEnd = PX_EOS;

        pcEqu++;

        __SKIP_WHITE2(pcEqu);

        if (__IS_NEWLINE(*pcEqu)) {
            pcValue    = LW_NULL;
            stValueLen = 0;
        } else {
            pcValue    = pcEqu;
            stValueLen = lib_strlen(pcValue);
        }

        pcKey      = pcLine;
        stKeyLen   = lib_strlen(pcKey);

        pinikey = (PLW_INI_KEY)malloc(sizeof(LW_INI_KEY) + stKeyLen + stValueLen + 2);
        if (!pinikey) {
            printf("system low memory.\r\n");
            break;
        }

        pinikey->INIK_pcKey = (PCHAR)pinikey + sizeof(LW_INI_KEY);
        lib_strcpy(pinikey->INIK_pcKey, pcKey);

        if (pcValue) {
            /*
             * strip '\n' or '\r' in tail
             */
            while (stValueLen) {
                if ((pcValue[stValueLen - 1] == '\n') ||
                    (pcValue[stValueLen - 1] == '\r') ||
                    (pcValue[stValueLen - 1] == '\t') ||
                    (pcValue[stValueLen - 1] == ' ')) {
                    pcValue[stValueLen - 1] = '\0';
                    stValueLen--;
                } else {
                    break;
                }
            }

            pinikey->INIK_pcValue = pinikey->INIK_pcKey + stKeyLen + 1;
            lib_strcpy(pinikey->INIK_pcValue, pcValue);
        } else {
            pinikey->INIK_pcValue = LW_NULL;
        }

        _List_Line_Add_Tail(&pinikey->INIK_lineManage, &pinisec->INIS_plineKey);
    }
}
/*********************************************************************************************************
** ��������: __iniLoad
** ��������: װ��һ�������ļ�
** �䡡��  : pcFile        �ļ���
**           pcSec         ��
** �䡡��  : INI ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static PLW_INI_SEC  __iniLoad (CPCHAR  pcFile, CPCHAR  pcSec)
{
    PLW_INI_SEC   pinisec;
    FILE         *fp;
    CHAR          cSec[LW_INI_BUF_SZ];
    CHAR          cBuf[LW_INI_BUF_SZ];
    PCHAR         pcLine;
    PCHAR         pcEnd;

    if (lib_strlen(pcSec) > (LW_INI_BUF_SZ - 3)) {
        return  (LW_NULL);
    }

    pinisec = (PLW_INI_SEC)malloc(sizeof(LW_INI_SEC));
    if (!pinisec) {
        return  (LW_NULL);
    }
    lib_bzero(pinisec, sizeof(LW_INI_SEC));

    fp = fopen(pcFile, "r");
    if (!fp) {
        printf("please put config file into :  %s\r\n", __UTIAL_PPP_PATH);
        free(pinisec);
        return  (LW_NULL);
    }

    snprintf(cSec, LW_INI_BUF_SZ, "[%s]", pcSec);

    for (;;) {
        pcLine = fgets(cBuf, LW_INI_BUF_SZ, fp);
        if (!pcLine) {
            goto    __error;
        }

        __SKIP_WHITE(pcLine);
        if (__IS_END(*pcLine) || (*pcLine == ';') || (*pcLine == '#')) {
            continue;
        }

        pcEnd = lib_strchr(pcLine, ']');
        if (!pcEnd) {
            continue;
        }

        pcEnd++;
        *pcEnd = PX_EOS;

        if (lib_strcmp(cSec, pcLine)) {
            continue;

        } else {
            break;
        }
    }

    __iniLoadSec(pinisec, fp);

    if (!pinisec->INIS_plineKey) {
        goto    __error;
    }

    fclose(fp);

    return  ((PLW_INI_SEC)pinisec);

__error:
    fclose(fp);
    free(pinisec);
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __iniUnload
** ��������: ж��һ�������ļ�
**
** �䡡��  : pinisec       INI ���
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __iniUnload (PLW_INI_SEC  pinisec)
{
    PLW_INI_KEY     pinikey;

    while (pinisec->INIS_plineKey) {
        pinikey = _LIST_ENTRY(pinisec->INIS_plineKey, LW_INI_KEY, INIK_lineManage);
        pinisec->INIS_plineKey = _list_line_get_next(pinisec->INIS_plineKey);

        free(pinikey);
    }

    free(pinisec);
}
/*********************************************************************************************************
** ��������: __iniGetStr
** ��������: ���ָ���������ַ���
** �䡡��  : pinisec       INI ���
**           pcKey         ָ����
**           pcDefault     Ĭ��ֵ
** �䡡��  : ��ȡ��ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static CPCHAR  __iniGetStr (PLW_INI_SEC  pinisec, CPCHAR  pcKey, CPCHAR  pcDefault)
{
    PLW_INI_KEY     pinikey;
    PLW_LIST_LINE   pline;
    PCHAR           pcRet = (PCHAR)pcDefault;

    for (pline  = pinisec->INIS_plineKey;
         pline != LW_NULL;
         pline  = _list_line_get_next(pline)) {

        pinikey = _LIST_ENTRY(pline, LW_INI_KEY, INIK_lineManage);
        if (lib_strcmp(pinikey->INIK_pcKey, pcKey) == 0) {
            pcRet = (PCHAR)pinikey->INIK_pcValue;
            break;
        }
    }

    return  ((CPCHAR)pcRet);
}
/*********************************************************************************************************
** ��������: udial_config_load
** ��������: װ��ָ������
** �䡡��  : name          ��������
** �䡡��  : ���þ��
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
void  *udial_config_load (const char *name)
{
    PLW_INI_SEC  pinisec;

    if (!name) {
        return  (LW_NULL);
    }

    pinisec = __iniLoad(__UTIAL_PPP_PATH, name);
    if (!pinisec) {
        return  (LW_NULL);
    }

    return  ((void *)pinisec);
}
/*********************************************************************************************************
** ��������: if_param_unload
** ��������: ж��ָ������
** �䡡��  : pifparam      ���þ��
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
void  udial_config_unload (void *config)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)config;

    if (!pinisec) {
        return;
    }

    __iniUnload(pinisec);
}
/*********************************************************************************************************
** ��������: udial_config_key_value_get
** ��������: ��ü�ֵ
** �䡡��  : config      ���þ��
**           key         ���ַ���
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
const char *udial_config_key_value_get (void *config, const char *key)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)config;

    if (!pinisec || !key) {
        return  (LW_NULL);
    }

    value = __iniGetStr(pinisec, key, LW_NULL);

    return  (value);
}
/*********************************************************************************************************
** ��������: udial_config_parser_test
** ��������: ��ü�ֵ
** �䡡��  : name
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
void   udial_config_parser_test (const char *name)
{
    int         i;
    void       *config;
    const char *value;
    const char *keys[] = {
            UKEY_MODEM      ,/* "Modem"         */
            UKEY_BAUD       ,/* "Baud"          */
            UKEY_INIT1      ,/* "Init1"         */
            UKEY_INIT2      ,/* "Init2"         */
            UKEY_INIT3      ,/* "Init3"         */
            UKEY_AREA_CODE  ,/* "Area Code"     */
            UKEY_PHONE      ,/* "Phone"         */
            UKEY_USRNAME    ,/* "Username"      */
            UKEY_PASSWORD   ,/* "Password"      */
            UKEY_DIAL_CMD   ,/* "Dial Command"  */
            UKEY_RESP_WAIT  ,/* "Resp Wait"     */
    };

    if (!name) {
        name = UDIAL_DEFAULT;
    }

    config = udial_config_load(name);
    if (!config) {
        printf("load config: %s failed.\n", name);
        return;
    }

    printf("\n[%s]\n", name);
    for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
        value = udial_config_key_value_get(config, keys[i]);
        printf("%s = %s\n", keys[i], value ? value : "NULL\n");
    }

    udial_config_unload(config);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
