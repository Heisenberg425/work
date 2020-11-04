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
** 文   件   名: udial.c
**
** 创   建   人: Zeng.Bo (曾波)
**
** 文件创建日期: 2015 年 01 月 07 日
**
** 描        述: 一个简单的 PPP 拨号工具
** BUG:
2015.07.08  v0.01  初始版本. 实现基本的拨号功能
2016.02.22  v0.02  增加 通过拨号配置文件 进行拨号的功能.
2016.02.23  v0.02  增加 当链接断开后自动重连的功能(还需进一步完善).
2016.02.24  v0.02  增加 每一个 udial 进程管理独立的一个 PPP 连接接口的支持.
2016.04.07  v1.00  修改 软件版本.
2016.12.05  v1.01  修改 当设备拔出后退出程序.
2017.02.23  v1.02  修正 键值尾部字符有'\r'的情况.
                   增加 是否设置为默认接口的配置.
                   增加 是否使能自动重连的配置.
                   增加 程序退出时, 清理 PPP 接口.
                   修正 键值尾部字符有空格的情况.
2018.01.26  v1.03  增加 PPPoE 拨号功能支持.
*********************************************************************************************************/
#define __SYLIXOS_STDIO
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SylixOS.h>
#include <termios.h>
#include <net/lwip/tools/ppp/lwip_ppp.h>
#include <net/lwip/event/lwip_netevent.h>
#include "udial_parser.h"

/*
 * version
 */
#define UDIAL_VERSION   "v1.03"

/*
 * OpenRT support
 */
#define OPENRT_EN     0
#if OPENRT_EN
#include "network/lwip/netif.h"
#include <pthread.h>
/*********************************************************************************************************
 用于配置拨号信息的结构
*********************************************************************************************************/
struct udial_handle {
    CHAR               cUsrname[20];                                    /*  拨号账号                    */
    CHAR               cPassword[20];                                   /*  拨号密码                    */
    CHAR               cNetifname[NETIF_NAMESIZE];                      /*  拨号物理网卡名              */
    CHAR               cPppname[NETIF_NAMESIZE];                        /*  拨号PPP网卡名               */
    INT                iUdialStatus;                                    /*  拨号状态                    */
    UINT32             uiPid;                                           /*  拨号进程 PID                */
};
typedef struct udial_handle   __UDIAL_HANDLE;
typedef struct udial_handle  *__PUDIAL_HANDLE;
#endif

/*
 * help macro
 */
#define UDIAL_INIT_CNT      5
#define UDIAL_RESP_SIZE     1024

#define udial_debug(fmt, arg...)\
        do {\
            if (enable_debug > 0) {\
                fprintf(stderr, "[UDIAL] " fmt, ##arg);\
            }\
        } while (0)

#define udial_process(fmt, arg...)\
        do {\
            if (enable_process > 0) {\
                printf("[UDIAL] " fmt, ##arg);\
            }\
        } while (0)

#define udial_trace()      printf("[UTRACE] %s() : line <%d>\n", __func__, __LINE__); msleep(500)

#define msleep(x)          API_TimeMSleep(x)

#define UDIAL_ITEM_LEN     128
#define UDIAL_ITEM_COUNT   (UDIAL_INIT_CNT + 8)

#define NETIF_NAME_MAX     IF_NAMESIZE

#define _BUG()             printf("%s<%d>\n", __func__, __LINE__)


/*
 * a dialogue config data
 */
typedef struct u_d_c {
    char         *tty_name;
    char         *if_name;
    int           baud;
    char         *init_str[UDIAL_INIT_CNT];
    char         *dial_str0;
    char         *dial_str1;

    char         *ppp_user;
    char         *ppp_psswd;
    char         *baud_str;
    char         *default_if;
    char         *auto_reconnect;

    char         *cfg_desc;

    char          _item[UDIAL_ITEM_COUNT][UDIAL_ITEM_LEN];

#define __tty_name          _item[0]
#define __if_name           _item[0]

#define __baud_str          _item[1]
#define __dial_str0         _item[2]
#define __dial_str1         _item[3]
#define __ppp_user          _item[4]
#define __ppp_psswd         _item[5]
#define __default_if        _item[6]
#define __auto_reconnect    _item[7]
#define __init_str0         __init_str_(0)
#define __init_str1         __init_str_(1)
#define __init_str2         __init_str_(2)
#define __init_str3         __init_str_(3)
#define __init_str4         __init_str_(4)

#define init_str0           init_str[0]
#define init_str1           init_str[1]
#define init_str2           init_str[2]
#define init_str3           init_str[3]
#define init_str4           init_str[4]

#define __init_str_(x)      _item[8 + x]

    int           do_auto_reconnect;
    int           set_default_if;

    struct u_d_c *next;
} udial_dial_cfg_t;

/*
 * dialogue mode
 */
#define UDIAL_PPPOS  0
#define UDIAL_PPPOE  1
static int udial_mode = UDIAL_PPPOS;

#define is_pppos()  (udial_mode == UDIAL_PPPOS)
#define is_pppoe()  (udial_mode == UDIAL_PPPOE)

/*
 * init phase response data
 */
static const char *init_resp[] = {
    "ok",
    "error",
    NULL
};
#define INIT_RESP_OK_STR    init_resp[0]

/*
 * dial phase response data
 */
static const char *dial_resp[] = {
    "connect",
    "no carrier",
    "no dialtone",
    "no dial tone",
    "busy",
    "error",
    "voice",
    "fclass",
    "no answer",
    NULL
};
#define DIAL_RESP_OK_STR        dial_resp[0]

static const char *__unkonwn_err = "unknown error";
#define RESP_UNKNOWN_STR        __unkonwn_err

static const char *prompt_str[] = {
    "}!}",
    "!}!",
    NULL
};

/*
 * for cmdline argv option
 */
#define UDIAL_OPTION_STRING     "vdpc:hsfwe"

static const char *__help_desc =
        "\nudial help information:\n"
        "   -v: show udial version.\n"
        "   -e: enable PPPoE dialogue, otherwise PPPoS dialogue.\n"
        "   -d: enable debug information output.\n"
        "   -p: enable showing the dialogue processing.\n"
        "   -h: show help information.\n"
        "   -c: use a given dialogue descripton name to do dialogue.\n"
        "   -f: use default dialogue descripton name [" UDIAL_DEFAULT "] to do dialogue.\n"
        "   -w: wait the device ready.\n"
        "   -s: show default supported dialogue description name.\n";

/*
 * AT resp min timeout
 */
#define UDIAL_RESP_WAIT_MIN     3000

/*
 * ppp netevent
 */
typedef struct {
    int    evt_num;
    char   if_name[NETIF_NAME_MAX];
} ppp_evt_t;

/*
 * pre-declare
 */
static int udial_init_modem(udial_dial_cfg_t *cfg, int is_first);
static int udial_hangup_modem(udial_dial_cfg_t *cfg);
static int udial_start_ppp(udial_dial_cfg_t *cfg, int re_conn);
static int udial_stop_ppp(char *if_name, int do_dis, int do_del);
static int udial_auto_reconnect(udial_dial_cfg_t *cfg, int do_init);
static int udial_wait_if_exit(int netevt_fd);
#if OPENRT_EN
static int udial_monitor (VOID  (*udialCallBack)(), __PUDIAL_HANDLE   pUdialInfo);
#else
static int udial_monitor(void *arg);
#endif

static int  udial_set_default(void);
static void udial_exit_handle(int i);

static int udial_parse_net_if_remove(char *evt_buf, char *if_name);
static int udial_parse_ppp_evt(ppp_evt_t  *out, char *evt_buf);
static const char *udial_ppp_evt_2_str(int evt);
static int udial_parse_arg(int argc, char *argv[]);

static const char *udial_check_resp(const char **resp_tbl, const char *resp_curr);

static void udial_init_cfg(void *arg);
static void udial_show_cfg_one(udial_dial_cfg_t *cfg);

static void udial_modify_cfg(udial_dial_cfg_t *cfg);

static void udial_show_support_cfg(void);
static void udial_show_help(void);

static void udial_insert_cfg(udial_dial_cfg_t *cfg);
static udial_dial_cfg_t *udial_search_cfg(char *cfg_desc);

static void udial_drain_modem(int modem_fd);
static ssize_t udial_write_modem(int modem_fd, char *data, size_t size);
static const char *__udial_wait_modem_resp(int modem_fd, char *resp, char *resp_tmp,
                                           size_t maxsize, const char **resp_tbl, int timeout_ms);

#define drain()    udial_drain_modem(modem_fd)
#define carrier()  TRUE
#define udial_wait_modem_resp(a, b, c, d, e) \
        __udial_wait_modem_resp(a, b, c, d, e, resp_wait_msec)

static int udial_if_ignore(char *if_name);

static void replace_char(void *string, char c1, char c2, int length);
static void strip_parity(char *buf, size_t size );
static char *__strlwr(char *string);

/*
 * globals
 */
static udial_dial_cfg_t *dial_cfg_hdr = NULL;
static udial_dial_cfg_t *dial_cfg_curr;

static char  resp_buf[UDIAL_RESP_SIZE];
static char  resp_tmp[UDIAL_RESP_SIZE];

static int   enable_debug   = 0;
static int   enable_process = 0;

static char *curr_desc_name = NULL;
static char  curr_if_name[NETIF_NAME_MAX];

static int   resp_wait_msec = UDIAL_RESP_WAIT_MIN;
static int   wait_dev_ready = 0;

static int   udial_reconn_del_if = 0;

static int  iFlagExit = 0;

/*
 * app entry
 */
int main (int argc, char *argv[])
{
    udial_dial_cfg_t  *cfg_curr;
    char              *curr_desc = NULL;
    int                ret;

    udial_init_cfg(NULL);  /* default config */

    ret = udial_parse_arg(argc, argv);
    if (ret == -1) {
        return  0;
    }

    if (ret == 0) {
        printf("use -h option for details.\r\n");
        return 0;
    }

    udial_process("use [%s] dialogue config\r\n", curr_desc_name);

    udial_init_cfg(curr_desc_name);

    curr_desc = curr_desc_name;
    if (curr_desc == NULL) {
        printf("has't given dialogue descriptor name!\n use -h for details\r\n");
        return  -1;
    }

    cfg_curr = udial_search_cfg(curr_desc);
    if (cfg_curr == NULL) {
        printf("no valid dialogue configure to handle, abort!\n use -h for details\r\n");
        return  -1;
    }

    dial_cfg_curr = cfg_curr;

    udial_process("now use following dialogue config: \r\n");
    udial_show_cfg_one(cfg_curr);

    signal(SIGQUIT, udial_exit_handle);
    signal(SIGABRT, udial_exit_handle);
    signal(SIGSEGV, udial_exit_handle);
    atexit((void (*)(void))udial_exit_handle);

    ret = udial_init_modem(cfg_curr, 1);
    if (ret != 0) {
        udial_debug("initialize the modem failed, abort!\r\n");
        return  -1;
    }

#if OPENRT_EN

#endif

    ret = udial_start_ppp(cfg_curr, 0);
    if (ret != 0) {
        udial_debug("start the ppp interface failed, abort!\r\n");
        return  -1;
    }

#if OPENRT_EN
    udialInfo.iUdialStatus = NET_EVENT_PPP_INIT;                        /*  更新一次连接状态            */

    udialCallBack(&udialInfo);

    udial_process("all work done successfully!\r\n\n");

    udial_monitor(udialCallBack, &udialInfo);
#else
    udial_process("all work done successfully!\r\n\n");

    udial_monitor(NULL);
#endif

    return  0;
}

static void udial_set_baud (int modem_fd, int baud)
{
    static char at_cmd[32];
    const char *resp_chk;

    sprintf(at_cmd, "AT+IPR=%d\r\n", baud);
    udial_process("now set modem baud, send: %s", at_cmd);

    drain();
    udial_write_modem(modem_fd, at_cmd, strlen(at_cmd));

    resp_chk = udial_wait_modem_resp(modem_fd, resp_buf, resp_tmp, UDIAL_RESP_SIZE, init_resp);
    if (resp_chk != INIT_RESP_OK_STR) {
        udial_process("set baud to %d failed, use lastest setting.\n", baud);
    } else {
        udial_process("set baud to %d successfully.\n", baud);
    }
}

/*
 * do the modem initializing for the next PPP handling
 */
static int udial_init_modem (udial_dial_cfg_t *cfg, int is_first)
{
    char      **init_str;
    char       *curr_str;
    ssize_t     tmp_len;
    size_t      data_len;
    const char *resp_chk;
    int         modem_fd;
    int         i;

    if (cfg == NULL) {
        return -1;
    }

    if (is_pppoe()) {
        return 0;
    }

    if (wait_dev_ready) {
        udial_process("now wait <%s> ready.\n", cfg->tty_name);
        do {
            if (access(cfg->tty_name, R_OK)) {
                msleep(500);
            } else {
                break;
            }
        } while (1);
    }

    if (wait_dev_ready) {
        msleep(500);
    }

    modem_fd = open(cfg->tty_name, O_RDWR);
    if (modem_fd < 0) {
        udial_debug("can't open <%s>, abort!\r\n", cfg->tty_name);
        exit(-1);
    }

    drain();

    udial_set_baud(modem_fd, cfg->baud);

    ioctl(modem_fd, SIO_BAUD_SET, cfg->baud);
    ioctl(modem_fd, SIO_MCTRL_BITS_SET, SIO_MODEM_DTR | SIO_MODEM_RTS);
    ioctl(modem_fd, FIOSETOPTIONS, OPT_RAW);
    write(modem_fd, "\r\r\r\r\r", 5);                                   /*  sync                        */

    /*
     * first do the init string
     */
    udial_process("now start to send initialize string to modem.\r\n");
    init_str = cfg->init_str;
    for (i = 0; i < UDIAL_INIT_CNT; i++) {
        curr_str = *init_str;
        if (curr_str) {
            /*
             * write init data
             */
            data_len = strlen(curr_str);
            udial_process("send: %s\r\n", curr_str);

            drain();
            tmp_len = udial_write_modem(modem_fd, curr_str, data_len);
            if (tmp_len != data_len) {
                udial_debug("write modem error with str: %s, size: %d/%d, abort!\r\n",
                            curr_str, (int)tmp_len, (int)data_len);
                goto __err;
            }

            resp_chk = udial_wait_modem_resp(modem_fd, resp_buf, resp_tmp, UDIAL_RESP_SIZE, init_resp);
            udial_process("resp: %s\r\n", resp_buf);
            if (resp_chk != INIT_RESP_OK_STR) {
                /*
                 * if in auto-reconnect working..
                 * check if prompt exist,
                 */
                if (!is_first) {
                    resp_chk = udial_check_resp(prompt_str, resp_tmp);
                    if (resp_chk != RESP_UNKNOWN_STR) {
                        /*
                         * here we think that the modem is OK!
                         */
                        goto __ok;
                    }
                }

                udial_debug("response error <%s>, it may be an invalid dialogue command, abort!\r\n",
                            resp_chk ? resp_chk : "invalid");
                goto __err;
            }
        }

        init_str++;
    }
    udial_process("modem has been initialized successfully!\r\n");

    /*
     * do dialogue
     */
    udial_process("now start to send dialogue string to modem.\r\n");
    if (cfg->dial_str0 != NULL) {
        curr_str = cfg->dial_str0;
    } else if (cfg->dial_str1 != NULL) {
        curr_str = cfg->dial_str1;
    } else {
        udial_debug("there is no dialogue data to send, abort!\r\n");
        goto __err;
    }

    data_len = strlen(curr_str);
    udial_process("send: %s\r\n", curr_str);

    drain();
    tmp_len = udial_write_modem(modem_fd, curr_str, data_len);
    if (tmp_len != data_len) {
        udial_debug("write modem error with str: %s, size: %d/%d, abort!\r\n",
                    curr_str, (int)tmp_len, (int)data_len);
        goto __err;
    }

    resp_chk = udial_wait_modem_resp(modem_fd, resp_buf, resp_tmp, UDIAL_RESP_SIZE, dial_resp);
    udial_process("resp: %s\r\n", resp_buf);
    if (resp_chk != DIAL_RESP_OK_STR) {
        udial_debug("response error <%s>, it may be an invalid dialogue command, abort!\r\n",
                    resp_chk ? resp_chk : "invalid");
        goto __err;
    }

    /*
     * now wait promte
     * promte wait time shouldn't be too long
     */
    udial_process("connected, wait prompt...\r\n");
    resp_chk = __udial_wait_modem_resp(modem_fd, resp_buf, resp_tmp, UDIAL_RESP_SIZE, prompt_str, 1000);
    if (resp_chk) {
        udial_process("prompt: %s\r\n", resp_buf);
    } else {
        udial_process("prompt: waiting timeout, ignore!\r\n");
    }

__ok:
    udial_process("modem connect successfully!\r\n");

    close(modem_fd);
    return 0;

__err:
    close(modem_fd);
    return  -1;
}

static int udial_hangup_modem (udial_dial_cfg_t *cfg)
{
    int  modem_fd;
    int  i;

    if (is_pppoe()) {
        return  0;
    }

    udial_process("now hangup modem...\r\n");

    modem_fd = open(cfg->tty_name, O_RDWR);
    if (modem_fd < 0) {
        udial_debug("can't open <%s>, abort!\r\n", cfg->tty_name);
        return  -1;
    }
    drain();
    udial_write_modem(modem_fd, "\r", 1);
    for (i = 0; i < 10; i++) {
        usleep(200 * 1000);
        udial_write_modem(modem_fd, "\r", 1);
    }
    drain();

    /*
     * then drop DTR for a while
     */
    ioctl(modem_fd, SIO_BAUD_SET, 0);
    for (i = 0; carrier() && (i < 10); i++) {
        usleep(100 * 1000);
    }

    udial_write_modem(modem_fd, "+++", 3);
    usleep(1500 * 1000);
    udial_write_modem(modem_fd, "ATH\r", 4);

    for (i = 0; carrier() && (i < 5); i++) {
        usleep(100 * 1000);
    }

    close(modem_fd);

    return  0;
}

/*
 * start the ppp
 */
static int udial_start_ppp (udial_dial_cfg_t *cfg, int re_conn)
{
    int          ret = -1;
    char         new_if_name[NETIF_NAME_MAX];
    LW_PPP_TTY   ppp_tty;
    LW_PPP_DIAL  ppp_dial;

    if (cfg == NULL) {
        return -1;
    }

    udial_process("now start ppp connect.\r\n");

    if (re_conn) {
        strcpy(new_if_name, curr_if_name);
    }

    if (re_conn && !udial_reconn_del_if) {
        goto __connect;
    }

    if (is_pppos()) {
        ppp_tty.baud = cfg->baud;
        ppp_tty.parity = 0;
        ppp_tty.stop_bits = 0;
        ret = API_PppOsCreate(cfg->tty_name, &ppp_tty, new_if_name, NETIF_NAME_MAX);
    }

    if (is_pppoe()) {
        ret = API_PppOeCreate(cfg->if_name, new_if_name, NETIF_NAME_MAX);
    }

    if (ret != 0) {
        udial_debug("create ppp interface error, abort!\r\n");
        return -1;
    } else {
        udial_process("\"%s\" create successfully!\r\n", new_if_name);
    }

__connect:
    ppp_dial.user   = cfg->ppp_user;
    ppp_dial.passwd = cfg->ppp_psswd;
    ret = API_PppConnect(new_if_name, &ppp_dial);
    if (ret != 0) {
        API_PppDelete(new_if_name);
        udial_debug("connect ppp interface error\r\n");
        return -1;
    } else {
        udial_process("connect ppp interface successfully!\r\n");
    }

    if (!re_conn) {
        strcpy(curr_if_name, new_if_name);
    }

    return 0;
}

static int udial_stop_ppp (char *if_name, int do_dis, int do_del)
{
    if (if_name) {
        if (do_dis) {
            API_PppDisconnect(if_name, TRUE);
        }
        if (do_del) {
            API_PppDelete(if_name);
        }
    }

    return  0;
}

static int udial_auto_reconnect (udial_dial_cfg_t *cfg, int do_init)
{
    int ret;

    if (do_init) {
        ret = udial_init_modem(cfg, 0);
        if (ret != 0) {
            udial_debug("re-initialize the modem failed!\r\n");
            return  -1;
        }
    }

    ret = udial_start_ppp(cfg, 1);
    if (ret != 0) {
        udial_debug("re-start the ppp interface failed!\r\n");
        return  -1;
    }

    return  0;
}

/*
 * when delete the ppp if, the if won't be deleted immdiatelly
 * it will be deleted by another thread
 * here we monitor the net event to ensure that
 */
static int udial_wait_if_exit (int netevt_fd)
{
    char    *evt_buf = resp_buf;
    ssize_t  read_len;
    char     if_name[NETIF_NAME_MAX];
    int      ret;

    if (!udial_reconn_del_if) {
        return  (0);
    }

    while (1) {
        read_len = read(netevt_fd, evt_buf, UDIAL_RESP_SIZE);
        if (read_len > 0) {
            ret = udial_parse_net_if_remove(evt_buf, if_name);
            if (ret != 0) {
                continue;
            }

            if (udial_if_ignore(if_name)) {
                continue;
            }

            udial_process("%s has been deleted.\r\n", if_name);
            sleep(1);
            break;
        }

        if (read_len < 0) {
            udial_debug("monidor netevent error, it seems the /dev/netevent device error!\r\n");
            return  -1;
        }
    }

    return  0;
}

/*
 * monitor the link status
 * this will never return
 */
#define UDIAL_RECONN_MIN_SEC    2
#define UDIAL_RECONN_INC_SEC    1
#define UDIAL_RECONN_MAX_SEC    5

#if OPENRT_EN
static int udial_monitor (VOID  (*udialCallBack)(), __PUDIAL_HANDLE   pUdialInfo)
#else
static int udial_monitor (void *arg)
#endif
{
    int               fd;
    char             *evt_buf = resp_buf;
    ssize_t           read_len;
    ppp_evt_t         evt;
    udial_dial_cfg_t *cfg;
    int               reconn_delay = UDIAL_RECONN_MIN_SEC;
    int               do_init = 1;
    int               ret;

    fd = open("/dev/netevent", O_RDONLY);
    if (fd < 0) {
        udial_debug("open /dev/netevent failed, can't monitor the ppp link status!\r\n");
        return -1;
    }

    cfg = udial_search_cfg(curr_desc_name);
    if (!cfg) {
        return  -1;
    }

    while (1) {
        read_len = read(fd, evt_buf, UDIAL_RESP_SIZE);
        if (read_len > 0) {
            ret = udial_parse_ppp_evt(&evt, evt_buf);
            if (ret == 0) {
                udial_process("udial monitor event: "
                              "<%s> %s\r\n",
                              evt.if_name, udial_ppp_evt_2_str(evt.evt_num));
            } else {
                continue;
            }

            if (udial_if_ignore(evt.if_name)) {
                continue;
            }

            /*
             * do auto-reconnect while connect dead
             */
            switch (evt.evt_num) {
            case NET_EVENT_PPP_RUN:
#if OPENRT_EN
                pUdialInfo->iUdialStatus = NET_EVENT_PPP_RUN;
                udialCallBack(pUdialInfo);
#endif
                if (cfg->set_default_if) {
                    udial_set_default();
                }
                break;

            case NET_EVENT_PPP_DISCONN:
#if OPENRT_EN
                pUdialInfo->iUdialStatus = NET_EVENT_PPP_DISCONN;
                udialCallBack(pUdialInfo);
#endif
                break;

            case NET_EVENT_PPP_DEAD:
#if OPENRT_EN
                pUdialInfo->iUdialStatus = NET_EVENT_PPP_DEAD;
                udialCallBack(pUdialInfo);
#endif
                if (udial_reconn_del_if) {
                    udial_process("now stop ppp by delete interface...\r\n");
                }
                udial_stop_ppp(evt.if_name, 0, udial_reconn_del_if);

                ret = udial_wait_if_exit(fd);
                if (ret != 0) {
                    return -1;
                }

                udial_hangup_modem(cfg);

                if (!cfg->auto_reconnect) {
                    exit(0);
                }

                while (1) {
                    udial_process("auto reconnect %s after %d seconds...\r\n",
                                  evt.if_name, reconn_delay);

                    sleep(reconn_delay);

                    ret = udial_auto_reconnect(cfg, do_init);
                    if (ret == 0) {
                        reconn_delay = UDIAL_RECONN_MIN_SEC;
                        do_init = 1;
                        break;
                    }

                    reconn_delay += UDIAL_RECONN_INC_SEC;
                    if (reconn_delay > UDIAL_RECONN_MAX_SEC) {
                        reconn_delay = UDIAL_RECONN_MIN_SEC;
                    }
                }
                break;

            default:
                break;
            }
        }

        if (read_len < 0) {
#if OPENRT_EN
                pUdialInfo->iUdialStatus = 0;
                udialCallBack(pUdialInfo);
#endif
            udial_debug("monidor netevent error, it seems the /dev/netevent device error!\r\n");
            close(fd);
            return  -1;
        }
    }

    return  0;
}

static int  udial_set_default (void)
{
    int         ret = -1;
    static char if_cmd[64];

    if (curr_if_name[0]) {
        snprintf(if_cmd, 64, "route add default dev %s\r\n", curr_if_name);
        udial_process("now set default by executing cmd : %s", if_cmd);
        ret = system(if_cmd);
    }

    return  (ret);
}


static void udial_exit_handle (int i)
{
    if (curr_if_name[0]) {
        udial_process("stop ppp.\n");
        udial_stop_ppp(curr_if_name, 1, 1);
        udial_hangup_modem(dial_cfg_curr);
    }

    iFlagExit = 1;
    udial_process("udial exit.\n");
}

/*
 * return:
 *      0: it's a if remove event
 *     -1: it isn't a if remove event, caller should ignore
 */
static int udial_parse_net_if_remove (char *evt_buf, char *if_name)
{
    unsigned char *buf = (unsigned char *)evt_buf;
    int            evt;

    /*
     * evt is make in big-endian
     */
    evt = (((unsigned int)buf[0]) << 24)
        | (((unsigned int)buf[1]) << 16)
        | (((unsigned int)buf[2]) <<  8)
        | (((unsigned int)buf[3]) <<  0);

    if (evt == NET_EVENT_REMOVE) {
        if_name[0] = buf[4];
        if_name[1] = buf[5];
        if_name[2] = buf[6];
        if_name[3] = 0;
        return  0;
    }

    return -1;
}

/*
 * return:
 *      0: it's a ppp event
 *     -1: it isn't a ppp event, caller should ignore
 */
static int udial_parse_ppp_evt (ppp_evt_t  *out, char *evt_buf)
{
    unsigned int    evt;
    unsigned char  *buf = (unsigned char *)evt_buf;

    /*
     * evt is make in big-endian
     */
    evt = (((unsigned int)buf[0]) << 24)
        | (((unsigned int)buf[1]) << 16)
        | (((unsigned int)buf[2]) <<  8)
        | (((unsigned int)buf[3]) <<  0);

    if (evt >= NET_EVENT_PPP_DEAD && evt <= NET_EVENT_PPP_DISCONN) {
        out->evt_num = evt;
        memcpy(out->if_name, buf + 4, NETIF_NAME_MAX);
        return  0;
    }

    return -1;
}

/*
 * convert evt type into descripting string
 * NULL if any error
 */
static const char *udial_ppp_evt_2_str (int evt)
{
    const char *ret;

    switch (evt) {
    case NET_EVENT_PPP_DEAD:
        ret = "ppp link is dead";
        break;
    case NET_EVENT_PPP_INIT:
        ret = "ppp is doing initializing";
        break;
    case NET_EVENT_PPP_AUTH:
        ret = "ppp is doing authing";
        break;
    case NET_EVENT_PPP_RUN:
        ret = "ppp linked and is running";
        break;
    case NET_EVENT_PPP_DISCONN:
        ret = "ppp link is disconnect";
        break;
    default:
        ret = "unknow ppp event";
        break;
    }

    return  ret;
}

/*
 * parse the usr option argument
 * return:
 *  -1: the caller need just return without doing anything
 *   0: the caller need tell usr use valid argument
 *   1: the caller need do dialogue work
 */
static int udial_parse_arg (int argc, char *argv[])
{
    int  c;
    int  cnt     = 0;
    int  do_dial = 0;

    if (argc < 2) {
        printf("no arguments!\r\n");
        return 0;
    }

    while (1) {
        c = getopt(argc, argv, UDIAL_OPTION_STRING);
        if (c == -1) {
            break;
        }

        cnt++;

        switch(c) {
        case 'e':
            udial_mode = UDIAL_PPPOE;
            break;
        case 'v':
            printf("udial version: %s(%s %s)\r\n", UDIAL_VERSION, __DATE__, __TIME__);
            break;
        case 'd':
            enable_debug = 1;
            break;
        case 'p':
            enable_process = 1;
            break;
        case 'c':
            curr_desc_name = optarg;
            do_dial = 1;
            break;
        case 'f':
            if (!curr_desc_name) {
                curr_desc_name = UDIAL_DEFAULT;
                do_dial = 1;
            }
            break;
        case 'w':
            wait_dev_ready = 1;
            break;
        case 'h':
            udial_show_help();
            break;
        case 's':
            udial_show_support_cfg();
            break;
        default:
            cnt--;
            break;
        }
    }

    if (cnt < 1) {
        printf("invalid parameter options!\r\n");
        return  0;
    }

    if (do_dial) {
        return  1;
    }

    return   -1;
}

/*
 * check the response from modem.
 */
static const char *udial_check_resp (const char **resp_tbl, const char *resp_curr)
{
    const char *resp_tmp;

    while (!!(resp_tmp = *resp_tbl++)) {
        if (strstr(resp_curr, resp_tmp)) {
            return  resp_tmp;
        }
    }

    return  RESP_UNKNOWN_STR;
}

#define key_value(key)     udial_config_key_value_get(config, key)
#define key_add(key, mem)  do {\
        value = key_value(key);\
        if (value) {\
            strncpy(cfg_curr->__##mem, value, UDIAL_ITEM_LEN);\
            cfg_curr->mem = cfg_curr->__##mem;\
        } else {\
            cfg_curr->mem = NULL;\
        }\
        } while (0)

/*
 * initialize the dialogue configure
 * !!this allow usr load a specific config file from disk
 */
static void udial_init_cfg (void *arg)
{
    void                *config;
    const char          *value;
    udial_dial_cfg_t    *cfg_curr;
    int                  init_ok = 0;

    config = udial_config_load((char *)arg);
    if (config) {
        cfg_curr = (udial_dial_cfg_t *)malloc(sizeof(udial_dial_cfg_t));
        if (!cfg_curr) {
            udial_debug("udial_init_cfg(): system low memory\r\n");
            goto __skip;
        }
        memset(cfg_curr, 0, sizeof(udial_dial_cfg_t));

        cfg_curr->cfg_desc = (char *)arg;

        /*
         * get resp wait time of seconds
         */
        value = key_value(UKEY_RESP_WAIT);
        if (value) {
            int tmp = 0;
            if (sscanf(value, "%d", &tmp) > 0) {
                if (tmp < UDIAL_RESP_WAIT_MIN) {
                    tmp = UDIAL_RESP_WAIT_MIN;
                }
                resp_wait_msec = tmp;
            }
        }

        key_add(UKEY_IFNAME,      if_name);
        key_add(UKEY_MODEM,       tty_name);
        key_add(UKEY_BAUD,        baud_str);
        key_add(UKEY_INIT1,       init_str0);
        key_add(UKEY_INIT2,       init_str1);
        key_add(UKEY_INIT3,       init_str2);
        key_add(UKEY_INIT4,       init_str3);
        key_add(UKEY_INIT5,       init_str4);
        key_add(UKEY_PHONE,       dial_str0);
        key_add(UKEY_DIAL_CMD,    dial_str1);
        key_add(UKEY_USRNAME,     ppp_user);
        key_add(UKEY_PASSWORD,    ppp_psswd);
        key_add(UKEY_DEFAULT_IF,  default_if);
        key_add(UKEY_AUTO_RECONN, auto_reconnect);

        if (is_pppos()) {
            if (!cfg_curr->baud_str) {
                cfg_curr->baud = 115200;
                udial_debug("udial_init_cfg() warning: baud is invalid, default to 115200\r\n");
            } else {
                if (sscanf(cfg_curr->baud_str, "%d", &cfg_curr->baud) < 1) {
                    cfg_curr->baud = 115200;
                    udial_debug("udial_init_cfg() warning: baud is invalid, default to 115200\r\n");
                }
            }
        }

        if (cfg_curr->default_if) {
            __strlwr(cfg_curr->default_if);
            if (strcmp(cfg_curr->default_if, "yes") == 0) {
                cfg_curr->set_default_if = 1;
            }
        }

        if (cfg_curr->auto_reconnect) {
            __strlwr(cfg_curr->auto_reconnect);
            if (strcmp(cfg_curr->auto_reconnect, "yes") == 0) {
                cfg_curr->do_auto_reconnect = 1;
            }
        }

        udial_modify_cfg(cfg_curr);

        init_ok = 1;
        udial_insert_cfg(cfg_curr);

__skip:
        udial_config_unload(config);
    }

    if (init_ok) {
        return;
    }

    /*
     * inner default config support
     */
    cfg_curr = (udial_dial_cfg_t *)malloc(sizeof(udial_dial_cfg_t));
    if (cfg_curr == NULL) {
        udial_debug("udial_init_cfg(): system low memory\r\n");
        return;
    }
    memset(cfg_curr, 0, sizeof(udial_dial_cfg_t));

    /*
     * just test for huawei-e220 usb 3g modem
     */
    cfg_curr->cfg_desc    = "HUAWEI-E220";
    cfg_curr->tty_name    = "/dev/ttyUSB0";
    cfg_curr->baud        = 3600000;

    cfg_curr->init_str[0] = "ATZ\r";
    cfg_curr->init_str[1] = "ATQ0 V1 E1 S0=0 &C1 &D2\r";
    cfg_curr->init_str[2] = "AT+CGDCONT=1,\"IP\",\"3gnet\"\r";
    cfg_curr->dial_str0   = "ATDT*99#\r";

    cfg_curr->ppp_user    = "PPP";
    cfg_curr->ppp_psswd   = "PPP";

    udial_insert_cfg(cfg_curr);
}

/*
 * the init string and dial string need '\r' at tail.
 * here we ensure this!!!
 */
#define __suffix    '\r'
static void udial_add_suffix (char *str, char suffix)
{
    int len;

    if (!str) {
        return;
    }
    len = strlen(str);
    if (len < 1) {
        return;
    }
    if (str[len - 1] != suffix) {
        str[len] = suffix;
        str[len + 1] = '\0';
    }
}

static void udial_modify_cfg (udial_dial_cfg_t *cfg)
{
    udial_add_suffix(cfg->init_str0, __suffix);
    udial_add_suffix(cfg->init_str1, __suffix);
    udial_add_suffix(cfg->init_str2, __suffix);
    udial_add_suffix(cfg->init_str3, __suffix);
    udial_add_suffix(cfg->init_str4, __suffix);
    udial_add_suffix(cfg->dial_str0, __suffix);
    udial_add_suffix(cfg->dial_str1, __suffix);
}

/*
 * show a dialogue configure
 */
static void udial_show_cfg_one (udial_dial_cfg_t *cfg)
{
    int                i;
    char             **init_str;
    static const char *init_key[] = {
            "init str0",
            "init str1",
            "init str2",
            "init str3",
            "init str4",
            "init str5",
    };

#define udial_key_fmt                   "%14s : "
#define udial_show(key, fmt, val)       printf(udial_key_fmt fmt, key, val)

    if (cfg == NULL) {
        return;
    }

    if (!enable_process) {
        return;
    }

    printf("-------------------------------------------------------\r\n");
    udial_show("decsription",   "%s\r\n",     cfg->cfg_desc);
    if (is_pppos()) {
        udial_show("tty device",    "%s\r\n",     cfg->tty_name);
        udial_show("baud",          "%d bps\r\n", cfg->baud);
        init_str = cfg->init_str;
        for (i = 0; i < UDIAL_INIT_CNT; i++) {
            if (*init_str) {
                udial_show(init_key[i], "%s\r\n", *init_str);
            }
            init_str++;
        }

        if (cfg->dial_str0) {
            udial_show("dial str0", "%s\r\n", cfg->dial_str0);
        }
        if (cfg->dial_str1) {
            udial_show("dial str1", "%s\r\n", cfg->dial_str1);
        }
    }

    if (is_pppoe()) {
        udial_show("net if",    "%s\r\n",     cfg->if_name);
    }

    udial_show("PPP user",       "%s\r\n",    cfg->ppp_user);
    udial_show("PPP pswd",       "%s\r\n",    cfg->ppp_psswd);
    udial_show("resp wait",      "%d ms\r\n", resp_wait_msec);
    udial_show("set default",    "%s\r\n",    cfg->set_default_if ? "yes" : "no");
    udial_show("auto reconnect", "%s\r\n",    cfg->do_auto_reconnect ? "yes" : "no");
    printf("-------------------------------------------------------\r\n");
}

/*
 * show the descriptor of current supported cfg
 */
static void udial_show_support_cfg (void)
{
    udial_dial_cfg_t *curr_cfg = dial_cfg_hdr;

    printf("current default supported configure descriptor: \r\n");
    printf("-------------------------------------------------------\r\n");
    while (curr_cfg) {
        printf("[%s] ", curr_cfg->cfg_desc);
        curr_cfg = curr_cfg->next;
    }
    printf("\r\n");
}

static void udial_show_help (void)
{
   printf("%s", __help_desc);
}

/*
 * insert a configure into the list at the head position
 */
static void udial_insert_cfg (udial_dial_cfg_t *cfg)
{
    if (dial_cfg_hdr) {
        cfg->next = dial_cfg_hdr;
        dial_cfg_hdr = cfg;
    } else {
        dial_cfg_hdr = cfg;
    }
}


/*
 * search a configure by the configure descriptor
 */
static udial_dial_cfg_t *udial_search_cfg (char *cfg_desc)
{
    udial_dial_cfg_t *cfg_curr = dial_cfg_hdr;

    if (cfg_desc == NULL) {
        return  NULL;
    }

    while (cfg_curr) {
        if (strcmp(cfg_desc, cfg_curr->cfg_desc) == 0) {
            return  cfg_curr;
        }
        cfg_curr = cfg_curr->next;
    }

    return  NULL;
}

static void udial_drain_modem (int modem_fd)
{
    ioctl(modem_fd, FIORFLUSH, 0);
}

static inline void udial_data_fmt (char *data, int len)
{
    replace_char(data, '\0', ' ', len);
    strip_parity(data, len);
    replace_char(data, '\0', ' ', len);
    data[len] = '\0';
}

/*
 * read modem : read all the data from modem buffer and do some convert
 * here we use loop query to judge if there is data in buffer,
 * we assume the tty device doesn't support select!!
 */
static const char *__udial_wait_modem_resp (int modem_fd, char *resp, char *resp_tmp,
                                            size_t maxsize, const char **resp_tbl, int timeout_ms)
{
    int   retry;
    int   len     = 0;
    int   tmp_len = 0;
    char *tmp_buf = resp_tmp;
    const char *ret;

    /*
     * first, wait data valid
     */
    memset(resp, 0, maxsize);
    memset(resp_tmp, 0, maxsize);

    retry = timeout_ms / 20;
    while (retry--) {
        if (ioctl(modem_fd, FIONREAD, &tmp_len) == 0) {
            if (tmp_len > 0) {
                break;
            }
        } else {
            udial_debug("device unknown exception!\r\n");
            return  NULL;
        }

        msleep(20);  /* 20 ms interval */
    }
    if (retry < 0) {
        udial_debug("wait data valid timeout!\r\n");
        return  NULL;
    }

    /*
     * then read till no data
     */
    do {
        tmp_len = read(modem_fd, tmp_buf, maxsize - len);

        udial_data_fmt(tmp_buf, tmp_len);
        memcpy(resp, tmp_buf, tmp_len);
        resp += tmp_len;

        /*
         * check the current resp
         * if got any valid resp pattern, return
         */
        __strlwr(tmp_buf);
        ret = udial_check_resp(resp_tbl, resp_tmp);
        if (ret != RESP_UNKNOWN_STR) {
            return  ret;
        }

        /*
         * havn't got valid(enough) resp, continue
         */
        len += tmp_len;
        tmp_buf += tmp_len;
        if (len >= maxsize) {
            len = maxsize;
            break;
        }

        retry = timeout_ms / 10;
        while (retry--) {
            tmp_len = 0;
            if (ioctl(modem_fd, FIONREAD, &tmp_len) == 0) {
                if (tmp_len > 0) {
                    break;
                }
            } else {
                return  NULL;
            }
            msleep(10);  /* 10 ms interval */
        }

        if (retry < 0) {
            break;
        }

    } while (1);

    return  NULL;
}

/*
 * just care about the if created by myself
 */
static int udial_if_ignore (char *if_name)
{
    if (if_name) {
        if (strcmp(if_name, curr_if_name) == 0) {
            return  0;
        }
    }

    return  1;
}

/*
 * write data to modem device
 */
static ssize_t udial_write_modem (int modem_fd, char *data, size_t size)
{
    ssize_t  ret;

    ret = write(modem_fd, (const char *)data, size);

    return  ret;
}

/*
 * replace c1 with c2 from the start of 'string' in length
 */
static void replace_char (void *string, char c1, char c2, int length)
{
    char *str_curr = (char *)string;
    int   i;

    for (i=0; i < length; i++) {
        if (*(str_curr + i) == c1) {
            *(str_curr + i) = c2;
        }
    }
}

/*
 * the modem response data may contain the parity bit in every byte
 * this clear the bit
 */
static void strip_parity (char *buf, size_t size)
{
    while(size-- > 0) {
        *buf = *buf & 0x7f;
        buf++;
    }
}

/*
 * convert upper word into lower word in a string
 * like clib <string.h> strlwr()
 * here we assume os doesn't support strlwr
 */
static char *__strlwr (char *string)
{
    char *p = string;

    while (p && *p) {
        *p = tolower(*p);
        p++;
    }

    return string;
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
