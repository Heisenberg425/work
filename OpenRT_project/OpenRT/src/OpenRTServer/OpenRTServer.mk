#*********************************************************************************************************
#
#                                    中国软件开源组织
#
#                                   嵌入式实时操作系统
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: OpenRTCfgTool.mk
#
# 创   建   人: RealEvo-IDE
#
# 文件创建日期: 2018 年 06 月 28 日
#
# 描        述: 本文件由 RealEvo-IDE 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := OpenRTServer

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS :=  \
src/arp/arpDB.c \
src/arp/arpEntry.c \
src/arp/arpExec.c \
src/asdefense/asdefenseDB.c \
src/asdefense/asdefenseEntry.c \
src/asdefense/asdefenseRecovery.c \
src/common.c \
src/database/dataBaseHandle.c \
src/devname/devnameDB.c \
src/devname/devnameEntry.c \
src/dhcp/dhcpEntry.c \
src/dhcp/dhcpExec.c \
src/dhcpserver/dhcpserverDB.c \
src/dhcpserver/dhcpServerEntry.c \
src/dhcpserver/dhcpserverExec.c \
src/dns/dnsDB.c \
src/dns/dnsEntry.c \
src/dns/dnsExec.c \
src/fileparser/fileParser.c \
src/flowctl/flowctlDB.c \
src/flowctl/flowctlEntry.c \
src/flowctl/flowctlExec.c \
src/hardware/hardwareEntry.c \
src/hardware/hardwareExec.c \
src/intadaption/intAdaEntry.c \
src/intadaption/intAdaptionDB.c \
src/intadaption/intAdaptionExec.c \
src/interface/interfaceEntry.c \
src/interface/interfaceExec.c \
src/kidvpn/kidvpnDB.c \
src/kidvpn/kidvpnEntry.c \
src/kidvpn/kidvpnExec.c \
src/kidvpn/ipdb.c \
src/kidvpn/kidvpnRecovery.c \
src/log/logCommon.c \
src/log/logEntry.c \
src/log/operLog.c \
src/log/udialLog.c \
src/log/vpnLog.c \
src/login/loginDB.c \
src/login/loginEntry.c \
src/monitor/monitorEntry.c \
src/monitor/monitorExec.c \
src/nat/natDB.c \
src/nat/natEntry.c \
src/nat/natExec.c \
src/nat/natRecovery.c \
src/nat/natStatus.c \
src/npf/npfDB.c \
src/npf/npfEntry.c \
src/npf/npfExec.c \
src/OpenRTCfgTool.c \
src/ping/pingEntry.c \
src/ping/pingExec.c \
src/power/powerDB.c \
src/power/powerEntry.c \
src/power/powerExec.c \
src/qos/qosDB.c \
src/qos/qosEntry.c \
src/qos/qosExec.c \
src/quagga/quaggaDB.c \
src/quagga/quaggaExec.c \
src/route/routeDB.c \
src/route/routeEntry.c \
src/route/routeExec.c \
src/timeset/timesetEntry.c \
src/timeset/timesetExec.c \
src/situationMode/situationDB.c \
src/situationMode/situationStatus.c \
src/situationMode/situationEntry.c \
src/situationMode/situationExec.c \
src/situationMode/cJSON.c \
src/udial/udialDB.c \
src/udial/udialEntry.c \
src/udial/udialExec.c \
src/udial/udialRecovery.c \
src/updatefirmware/updatefirmwareEntry.c \
src/updatefirmware/updatefirmwareExec.c \
src/version/versionEntry.c \
src/vlan/vlanDB.c \
src/vlan/vlanEntry.c \
src/vlan/vlanExec.c \
src/vnd/vndDB.c \
src/vnd/vndEntry.c \
src/vnd/vndExec.c \
src/vnd/vndRecovery.c

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH :=  \
-I"../WebServer/goahead/embedthis-goahead/goaction" \
-I"$(PROJECT_PATH)/src/include" \
-I"$(PROJECT_PATH)/src/CommunicateServer/libcomm/include" \
-I"$(SYLIXOS_BASE_PATH)/libsqlite3/sqlite3" \
-I"$(PROJECT_PATH)/src/OpenRTServer/src" 

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := 

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS := 
LOCAL_CXXFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB :=  \
-lsqlite3 \
-lcomm
LOCAL_DEPEND_LIB_PATH :=  \
-L"$(SYLIXOS_BASE_PATH)/libsqlite3/$(OUTDIR)/strip" \
-L"$(PROJECT_PATH)/src/CommunicateServer/libcomm/$(OUTDIR)/strip" 

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
# OpenMP config
#*********************************************************************************************************
LOCAL_USE_OMP := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD := 
LOCAL_POST_LINK_CMD := 
LOCAL_PRE_STRIP_CMD := 
LOCAL_POST_STRIP_CMD := 

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET :=  \
$(SYLIXOS_BASE_PATH)/libsqlite3/$(OUTDIR)/strip/libsqlite3.so \
$(PROJECT_PATH)/src/CommunicateServer/libcomm/$(OUTDIR)/strip/libcomm.so 

removedir:
	@if [ -d "../Release" ]; then \
	rm  -r "../Release"; fi
	@if [ -d "../Debug" ]; then \
	rm  -r "../Debug"; fi
TARGETS += removedir

include $(APPLICATION_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
