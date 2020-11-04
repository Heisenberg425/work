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
** 文   件   名: udial_parser.h
**
** 创   建   人: Zeng.Bo (曾波)
**
** 文件创建日期: 2016 年 01 月 30 日
**
** 描        述: PPP 拨号配置文件解析器
*********************************************************************************************************/

#ifndef __UDIAL_PARSER_H
#define __UDIAL_PARSER_H

//#define UKEY_IFNAME         "Netif"
//#define UKEY_MODEM          "Modem"
//#define UKEY_BAUD           "Baud"
//#define UKEY_INIT1          "Init1"
//#define UKEY_INIT2          "Init2"
//#define UKEY_INIT3          "Init3"
//#define UKEY_INIT4          "Init4"
//#define UKEY_INIT5          "Init5"
//#define UKEY_AREA_CODE      "Area Code"
//#define UKEY_PHONE          "Phone"
//#define UKEY_USRNAME        "Username"
//#define UKEY_PASSWORD       "Password"
//#define UKEY_DIAL_CMD       "Dial Command"
//#define UKEY_RESP_WAIT      "Resp Wait"
//#define UKEY_DEFAULT_IF     "Default Interface"
//#define UKEY_AUTO_RECONN    "Auto Reconnect"

#define UDIAL_DEFAULT       "Dialer Defaults"
//
#define UKEY_IFNAME         "netif"
#define UKEY_MODEM          "modem"
#define UKEY_BAUD           "baud"
#define UKEY_INIT1          "init1"
#define UKEY_INIT2          "init2"
#define UKEY_INIT3          "init3"
#define UKEY_INIT4          "init4"
#define UKEY_INIT5          "init5"
#define UKEY_AREA_CODE      "area code"
#define UKEY_PHONE          "phone"
#define UKEY_USRNAME        "username"
#define UKEY_PASSWORD       "password"
#define UKEY_DIAL_CMD       "dial command"
#define UKEY_RESP_WAIT      "resp wait"
#define UKEY_DEFAULT_IF     "default interface"
#define UKEY_AUTO_RECONN    "auto reconnect"



/*
 * not use following keys in current version
 */
//Ask Password = 0
//Stupid Mode = 1
//Compuserve = 0
//Force Address =
//Idle Seconds = 0
//DialMessage1 =
//DialMessage2 =
//ISDN = 0
//Auto DNS = 1

void       *udial_config_load(const char *name);
void        udial_config_unload(void *config);
const char *udial_config_key_value_get(void *config, const char *key);

void        udial_config_parser_test(const char *name);

#endif                                                                  /*  __UDIAL_PARSER_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
