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
** 文   件   名: configini.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2019 年 1 月 2 日
**
** 描        述: 读取配置文件
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PX_EOS
#define PX_EOS '\0'
#endif

/*
 * ini file key=value
 */
typedef struct ini_key_value {
    struct ini_key_value *next;
    CHAR                 *keyword;
    CHAR                 *value;
} ini_key_value_t;

/*
 * ini file [sector]
 */
typedef struct {
    struct ini_key_value *list;
} ini_sector_t;


/*********************************************************************************************************
** 函数名称: ini_load_sector
** 功能描述: 加载一个段
** 输　入  : sec                段句柄
**           fp                 ini 文件句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID ini_load_sector (ini_sector_t *sec, FILE *fp)
{
#define INI_BUF_SZ        256

#define IS_WHITE(c)       (c == ' ' || c == '\t' || c == '\r' || c == '\n')
#define IS_END(c)         (c == PX_EOS)
#define SKIP_WHITE(str)   while (IS_WHITE(*str)) {  \
                              str++;  \
                          }
#define NEXT_WHITE(str)   while (!IS_WHITE(*str) && !IS_END(*str)) { \
                              str++;  \
                          }

    ini_key_value_t *pinikey;

    CHAR buf[INI_BUF_SZ];
    CHAR *line;
    CHAR *end;
    CHAR *equ;

    CHAR *key;
    size_t key_len;
    CHAR *value;
    size_t value_len;

    for (;;) {
        line = fgets(buf, INI_BUF_SZ, fp);
        if (!line) {
            break;
        }

        SKIP_WHITE(line);
        if (IS_END(*line) || (*line == ';') || (*line == '#')) {
            continue;
        }

        if (*line == '[') {
            break;
        }

        equ = strchr(line, '=');
        if (!equ) {
            continue;
        }
        *equ = PX_EOS;

        end  = line;
        NEXT_WHITE(end);
        *end = PX_EOS;
        key  = line;

        line = ++equ;
        SKIP_WHITE(line);
        end = line;
        NEXT_WHITE(end);
        *end  = PX_EOS;
        value = line;

        key_len   = strlen(key);
        value_len = strlen(value);

        pinikey = (ini_key_value_t *)malloc(sizeof(ini_key_value_t) + key_len + value_len + 2);
        if (!pinikey) {
            fprintf(stderr, "[KidVPN] malloc error(%d): %s\n", errno, strerror(errno));
            break;
        }

        pinikey->keyword = (CHAR *)pinikey + sizeof(ini_key_value_t);
        strcpy(pinikey->keyword, key);

        pinikey->value = pinikey->keyword + key_len + 1;
        strcpy(pinikey->value, value);

        pinikey->next = sec->list;
        sec->list = pinikey;
    }
}

/*********************************************************************************************************
** 函数名称: __iniLoadFile
** 功能描述: 加载 ini 配置文件
** 输　入  : file               文件路径
**           sector             段名称
** 输　出  : NONE
** 返　回  : 段句柄
*********************************************************************************************************/
static ini_sector_t *__iniLoadFile (const CHAR *file, const CHAR *sector)
{
    ini_sector_t *pinisec;
    FILE *fp;
    CHAR sec[INI_BUF_SZ];
    CHAR buf[INI_BUF_SZ];
    CHAR *line;
    CHAR *end;

    if (strlen(sector) > (INI_BUF_SZ - 3)) {
        return  (NULL);
    }

    pinisec = (ini_sector_t *)malloc(sizeof(ini_sector_t));
    if (!pinisec) {
        return  (NULL);
    }
    bzero(pinisec, sizeof(ini_sector_t));

    fp = fopen(file, "r");
    if (!fp) {
        free(pinisec);
        return  (NULL);
    }

    snprintf(sec, INI_BUF_SZ, "[%s]", sector);

    for (;;) {
        line = fgets(buf, INI_BUF_SZ, fp);
        if (!line) {
            goto    error;
        }

        SKIP_WHITE(line);
        if (IS_END(*line) || (*line == ';') || (*line == '#')) {
            continue;
        }

        end = line;
        NEXT_WHITE(end);
        *end = PX_EOS;

        if (strcmp(sec, line)) {
            continue;

        } else {
            break;
        }
    }

    ini_load_sector(pinisec, fp);
    fclose(fp);

    return  (pinisec);

error:
    fclose(fp);
    free(pinisec);
    return  (NULL);
}

/*********************************************************************************************************
** 函数名称: __iniUnloadSector
** 功能描述: 释放段
** 输　入  : pinisec            段句柄
** 输　出  : NONE
** 返　回  : NONE
*********************************************************************************************************/
static VOID __iniUnloadSector (ini_sector_t *pinisec)
{
    ini_key_value_t *pinikey;

    while (pinisec->list) {
        pinikey = pinisec->list;
        pinisec->list = pinikey->next;
        free(pinikey);
    }

    free(pinisec);
}

/*********************************************************************************************************
** 函数名称: __iniGetInteger
** 功能描述: 获取段的一个数值
** 输　入  : pinisec            段句柄
**           keyword            字段名称
** 输　出  : def                获取的值
** 返　回  : 获取的值
*********************************************************************************************************/
static int __iniGetInteger (ini_sector_t *pinisec, const CHAR *keyword, int def)
{
    ini_key_value_t *pinikey;
    int  ret = def;

    for (pinikey = pinisec->list; pinikey != NULL; pinikey = pinikey->next) {
        if (strcmp(pinikey->keyword, keyword) == 0) {
            ret = atoi(pinikey->value);
            break;
        }
    }

    return  (ret);
}

/*********************************************************************************************************
** 函数名称: __iniGetString
** 功能描述: 获取段的一个字符串值
** 输　入  : pinisec            段句柄
**           keyword            字段名称
** 输　出  : def                获取的字符串
** 返　回  : 获取的字符串
*********************************************************************************************************/
static CHAR *__iniGetString (ini_sector_t *pinisec, const CHAR *keyword, CHAR *def)
{
    ini_key_value_t *pinikey;
    CHAR *ret = def;

    for (pinikey = pinisec->list; pinikey != NULL; pinikey = pinikey->next) {
        if (strcmp(pinikey->keyword, keyword) == 0) {
            ret = pinikey->value;
            break;
        }
    }

    return  (ret);
}

/*********************************************************************************************************
** 函数名称: asdCfgLoad
** 功能描述: 加载配置文件
** 输　入  : file               文件路径
**           sector             段名称
** 输　出  : NONE
** 返　回  : 配置文件句柄
*********************************************************************************************************/
VOID *asdCfgLoad (const CHAR *file, const CHAR *sector)
{
    ini_sector_t *pinisec;

    if (!file) {
        return  (NULL);
    }

    pinisec = __iniLoadFile(file, sector);
    if (!pinisec) {
        printk("[arpSpoofDefense] No configure for [%s] from %s\n", sector, file);

        return  (NULL);
    }

    return  ((VOID *)pinisec);
}

/*********************************************************************************************************
** 函数名称: asdCfgUnload
** 功能描述: 卸载配置文件
** 输　入  : loadret            ini文件句柄
** 输　出  : NONE
** 返　回  : 操作结果
*********************************************************************************************************/
VOID asdCfgUnload (VOID *loadret)
{
    ini_sector_t *pinisec = (ini_sector_t *)loadret;

    if (!pinisec) {
        return;
    }

    __iniUnloadSector(pinisec);
}

/*********************************************************************************************************
** 函数名称: asdCfgGetInt
** 功能描述: 获取配置参数数字
** 输　入  : loadret            ini文件句柄
**           keyword            配置参数字段
** 输　出  : def                获取的配置参数内容
** 返　回  : 操作结果
*********************************************************************************************************/
INT32 asdCfgGetInt (VOID *loadret, const CHAR  *keyword, int def)
{
    ini_sector_t *pinisec = (ini_sector_t *)loadret;

    if (!pinisec || !keyword) {
        return  (def);
    }

    return  (__iniGetInteger(pinisec, keyword, def));
}


/*********************************************************************************************************
** 函数名称: asdCfgGetString
** 功能描述: 获取配置参数字符串
** 输　入  : loadret            ini文件句柄
**           keyword            配置参数字段
** 输　出  : def                获取的配置参数内容
** 返　回  : 操作结果
*********************************************************************************************************/
CHAR *asdCfgGetString (VOID  *loadret, const CHAR  *keyword, CHAR  *def)
{
    ini_sector_t *pinisec = (ini_sector_t *)loadret;

    if (!pinisec || !keyword) {
        return  (def);
    }

    return  (__iniGetString(pinisec, keyword, def));
}

/*********************************************************************************************************
  END
*********************************************************************************************************/
