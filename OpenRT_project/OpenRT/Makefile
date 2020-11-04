#*********************************************************************************************************
# Include config.mk
#*********************************************************************************************************
CONFIG_MK_EXIST = $(shell if [ -f ../config.mk ]; then echo exist; else echo notexist; fi;)
ifeq ($(CONFIG_MK_EXIST), exist)
include ../config.mk
else
CONFIG_MK_EXIST = $(shell if [ -f config.mk ]; then echo exist; else echo notexist; fi;)
ifeq ($(CONFIG_MK_EXIST), exist)
include config.mk
else
CONFIG_MK_EXIST =
endif
endif

#*********************************************************************************************************
# Include sylixos base config.mk
#*********************************************************************************************************
EMPTY =
SPACE = $(EMPTY) $(EMPTY)

SYLIXOS_BASE_PATH_BAK := $(SYLIXOS_BASE_PATH)
TOOLCHAIN_PREFIX_BAK  := $(TOOLCHAIN_PREFIX)
DEBUG_LEVEL_BAK       := $(DEBUG_LEVEL)
CPU_TYPE_BAK          := $(CPU_TYPE)
FPU_TYPE_BAK          := $(FPU_TYPE)

SYLIXOS_BASE_CONFIGMK = $(subst $(SPACE),\ ,$(SYLIXOS_BASE_PATH))/config.mk
include $(SYLIXOS_BASE_CONFIGMK)

SYLIXOS_BASE_PATH := $(SYLIXOS_BASE_PATH_BAK)
DEBUG_LEVEL       := $(DEBUG_LEVEL_BAK)

ifneq ($(TOOLCHAIN_PREFIX_BAK),)
TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX_BAK)
endif

ifneq ($(CPU_TYPE_BAK),)
CPU_TYPE := $(CPU_TYPE_BAK)
endif

ifneq ($(FPU_TYPE_BAK),)
FPU_TYPE := $(FPU_TYPE_BAK)
endif

#*********************************************************************************************************
# Include header.mk
#*********************************************************************************************************
MKTEMP = $(subst $(SPACE),\ ,$(SYLIXOS_BASE_PATH))/libsylixos/SylixOS/mktemp

include $(MKTEMP)/header.mk

export PROJECT_PATH ?= $(WORKSPACE_OpenRT)

include publish.mk

OpenRT_COMPONENTS :=  \
src/CommunicateServer/libcomm \
src/CommunicateServer/libnethandle \
src/WebServer/goahead \
src/OpenRTServer \
Tools/others 

clean-invalid:
	-rm -rf $(DEBUG_LEVEL)
	-rm -rf publish/install-Packet
	
all: $(OpenRT_COMPONENTS)
	@for target in $(OpenRT_COMPONENTS); do make -C $$target all; done;
	make makePubDir;
	make copyPubFile;
	make tarPublish;


clean: $(OpenRT_COMPONENTS)
	@for target in $(OpenRT_COMPONENTS); do make -C $$target clean; done;
	make -C Tools/others clean_tool;
	make publishClean;
	make clean-invalid;