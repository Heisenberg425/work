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
** 文   件   名: unixSocket.c
**
** 创   建   人: Wu.PengCheng (吴鹏程)
**
** 文件创建日期: 2018 年6 月 9 日
**
** 描        述: unix 域 socket 使用接口
*********************************************************************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
/*********************************************************************************************************
** 函数名称: unixDgramSSocket
** 功能描述: 创建一个 DGRAM 类型的 UNIX 域socket
** 输　入  : cpcPszPath         文件名
** 输　出  : NONE
** 返　回  : iSockfd            socket 描述符
*********************************************************************************************************/
INT unixDgramSocket (CPCHAR  cpcPszPath)
{
    INT                  iSockfd;
    struct sockaddr_un   src;
    INT                  iLen;

    if (LW_NULL == cpcPszPath) {
        return  (PX_ERROR);
    }

    unlink (cpcPszPath);
    iSockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (0 > iSockfd) {
        perror ("create socket failed");

        return  (PX_ERROR);
    }

    memset (&src, 0, sizeof(src));
    src.sun_family = AF_UNIX;
    strcpy (src.sun_path, cpcPszPath);

    iLen = offsetof (struct sockaddr_un, sun_path) + strlen(cpcPszPath);

    if (bind(iSockfd, (struct sockaddr *)&src, iLen) < 0) {
        perror ("bind socket failed");

        return  (PX_ERROR);
    }

    return iSockfd;
}

/*********************************************************************************************************
** 函数名称: unixDgramSockAddr
** 功能描述: 生成目的 socket 地址和长度
** 输　入  : cpcDstPath         文件名
** 输　出  : pDst               socket 目的地址
**           iLen               socket 地址长度
** 返　回  : ERROR_CODE
*********************************************************************************************************/
INT  unixDgramSockAddr (CPCHAR  cpcDstPath, struct sockaddr_un  *pDst, INT  *piLen)
{
    if (LW_NULL == cpcDstPath || LW_NULL == pDst || LW_NULL == piLen) {
        return  (PX_ERROR);
    }

    memset (pDst, 0, sizeof(struct sockaddr_un));
    pDst->sun_family = AF_UNIX;
    strcpy (pDst->sun_path, cpcDstPath);
    *piLen = offsetof (struct sockaddr_un, sun_path) + strlen(cpcDstPath);

    return  (ERROR_NONE);
}
