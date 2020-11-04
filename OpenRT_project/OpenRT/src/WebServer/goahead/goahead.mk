#*********************************************************************************************************
#
#                                    �й������Դ��֯
#
#                                   Ƕ��ʽʵʱ����ϵͳ
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------�ļ���Ϣ--------------------------------------------------------------------------------
#
# ��   ��   ��: goahead.mk
#
# ��   ��   ��: RealEvo-IDE
#
# �ļ���������: 2016 �� 11 �� 28 ��
#
# ��        ��: ���ļ��� RealEvo-IDE ���ɣ��������� Makefile ���ܣ������ֶ��޸�
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := goahead

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
embedthis-goahead/src/goahead.c \
embedthis-goahead/goaction/action/flowctlaction.c \
embedthis-goahead/goaction/action/intadaaction.c \
embedthis-goahead/goaction/action/interfaceAction.c \
embedthis-goahead/goaction/action/kidvpnAction.c \
embedthis-goahead/goaction/action/natAction.c \
embedthis-goahead/goaction/action/routeaction.c \
embedthis-goahead/goaction/action/udialAction.c \
embedthis-goahead/goaction/action/vlanaction.c \
embedthis-goahead/goaction/action/vndAction.c \
embedthis-goahead/goaction/action/arpaction.c \
embedthis-goahead/goaction/action/dhcpAction.c \
embedthis-goahead/goaction/action/npfAction.c \
embedthis-goahead/goaction/action/dnsAction.c \
embedthis-goahead/goaction/action/logAction.c \
embedthis-goahead/goaction/action/loginAction.c \
embedthis-goahead/goaction/action/monitorAction.c \
embedthis-goahead/goaction/action/timeaction.c \
embedthis-goahead/goaction/action/powerAction.c \
embedthis-goahead/goaction/action/devnameAction.c \
embedthis-goahead/goaction/action/pingAction.c \
embedthis-goahead/goaction/action/hwAction.c \
embedthis-goahead/goaction/action/dhcpserveraction.c \
embedthis-goahead/goaction/action/updatefirmwareaction.c \
embedthis-goahead/goaction/action/versionaction.c \
embedthis-goahead/goaction/action/qosaction.c \
embedthis-goahead/goaction/action/userAuthAction.c \
embedthis-goahead/goaction/action/usrScriptAction.c \
embedthis-goahead/goaction/action/webbindnetifAction \
embedthis-goahead/goaction/commonAction/publicNetAction.c \
embedthis-goahead/goaction/commonAction/localNetAction.c \
embedthis-goahead/goaction/commonAction/kidvpnConfAction.c \
embedthis-goahead/goaction/commonAction/arpConfAction.c \
embedthis-goahead/goaction/commonAction/commonAction.c \
embedthis-goahead/goaction/cJSON.c \
embedthis-goahead/goaction/operLog.c \
embedthis-goahead/goaction/userAdmin.c \
embedthis-goahead/goaction/libgoaction.c \
embedthis-goahead/goaction/webHandle.c

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your hearder files search path")
#*********************************************************************************************************
LOCAL_INC_PATH :=  \
-I"./embedthis-goahead/src" \
-I"$(SYLIXOS_BASE_PATH)/libcextern/libcextern/include" \
-I"$(PROJECT_PATH)/src/CommunicateServer/libnethandle/include" \
-I"$(PROJECT_PATH)/src/CommunicateServer/libcomm/include" \
-I"$(PROJECT_PATH)/src/include" 

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := 

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS   := 
LOCAL_CXXFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#-lwebActionHandle -L"$(WORKSPACE_webActionHandle)/Release"  -lwebgoaction
#*********************************************************************************************************
LOCAL_DEPEND_LIB :=  \
-lgoahead \
-lcextern \
-lcomm \
-lnethandle
LOCAL_DEPEND_LIB_PATH :=  \
-L"$(OUTDIR)" \
-L"$(SYLIXOS_BASE_PATH)/libcextern/$(OUTDIR)" \
-L"$(PROJECT_PATH)/src/CommunicateServer/libcomm/$(OUTDIR)/strip" \
-L"$(PROJECT_PATH)/src/CommunicateServer/libnethandle/$(OUTDIR)/strip" 

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := no
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD   := 
LOCAL_POST_LINK_CMD  := 
LOCAL_PRE_STRIP_CMD  := 
LOCAL_POST_STRIP_CMD := 

include $(APPLICATION_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
