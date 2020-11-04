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
** 文   件   名: situationCommon.h
**
** 创   建   人: Bai.QiangQiang (柏强强)
**
** 文件创建日期: 2018 年 10 月 18 日
**
** 描        述: 设备名称设置头文件
*********************************************************************************************************/
#ifndef __SITUATION_COMMON_H
#define __SITUATION_COMMON_H

/*********************************************************************************************************
  OpenRT 模式宏定义
*********************************************************************************************************/
#define MODE_MIDDLE_ENT           "mid-Enterprise"                      /*  模式：中/小企业             */
#define MODE_LARGE_ENT            "large-Enterprise"                    /*  模式：大型企业              */
#define MODE_FAMILY               "family"                              /*  模式：家庭                  */
#define MODE_GOVERN               "govern"                              /*  模式：政府                  */
/*********************************************************************************************************
  OpenRT 状态宏定义
*********************************************************************************************************/
#define STATUS_UNCONFIG           "Noconfig"                            /*  状态：未配置                */
#define STATUS_CONFIGING          "Configing"                           /*  状态：配置中                */
#define STATUS_FINISHED           "Finished"                            /*  状态：配置完成              */
/*********************************************************************************************************
  OpenRT 位置宏定义
*********************************************************************************************************/
#define POS_STARTING_SET          "Starting-Set-Page"                   /*  位置：开始界面              */
#define POS_CONNECT_SUBMIT        "Connect-Submit-Page"                 /*  位置：外网连接提交          */
#define POS_CONNECT_INTERNET      "Connecting-Internet-Page"            /*  位置：外网接入              */
#define POS_INTRANET_SET          "Intranet-Set-Page"                   /*  位置：内网接入              */
#define POS_CLIVPN_CONFIG         "Client-KidVPN-Config-Page"           /*  位置：KidVPN 客户端配置     */
#define POS_CLIENT_KIDVPN         "Client-KidVPN-Set-Page"              /*  位置：KidVPN 客户端         */
#define POS_SERVPN_CONFIG         "Server-KidVPN-Config-Page"           /*  位置：KidVPN 服务器配置     */
#define POS_SERVER_KIDVPN         "Server-KidVPN-Set-Page"              /*  位置：KidVPN 服务器         */
#define POS_CONFIG_COMPLETE       "Configure_Cmplete_Page"              /*  位置：配置完成              */

#endif                                                                  /*  __SITUATION_COMMON_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
