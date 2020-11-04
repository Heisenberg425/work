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
** ��   ��   ��: unixSocket.c
**
** ��   ��   ��: Wu.PengCheng (������)
**
** �ļ���������: 2018 ��6 �� 9 ��
**
** ��        ��: unix �� socket ʹ�ýӿ�
*********************************************************************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
/*********************************************************************************************************
** ��������: unixDgramSSocket
** ��������: ����һ�� DGRAM ���͵� UNIX ��socket
** �䡡��  : cpcPszPath         �ļ���
** �䡡��  : NONE
** ������  : iSockfd            socket ������
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
** ��������: unixDgramSockAddr
** ��������: ����Ŀ�� socket ��ַ�ͳ���
** �䡡��  : cpcDstPath         �ļ���
** �䡡��  : pDst               socket Ŀ�ĵ�ַ
**           iLen               socket ��ַ����
** ������  : ERROR_CODE
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
