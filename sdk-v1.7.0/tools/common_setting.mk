SHELL:=/bin/bash
CROSS_COMPLE_PREFIX:=arm-none-eabi-
CC:=$(CROSS_COMPLE_PREFIX)gcc
CPPC:=$(CROSS_COMPLE_PREFIX)g++
AR:=$(CROSS_COMPLE_PREFIX)ar
NM:=$(CROSS_COMPLE_PREFIX)nm
OBJDUMP:= $(CROSS_COMPLE_PREFIX)objdump
OBJCOPY:= $(CROSS_COMPLE_PREFIX)objcopy

# clear vars
CFLAGS		   :=
CXXFLAGS	   :=
LFLAGS		   :=
INC_SDK_DIR    :=
INC_LIB_DIR    :=
INC_APP_DIR    :=
SOURCE_APP_C   :=
SOURCE_APP_CPP :=
SOURCE_LIB_S   :=
SOURCE_LIB_C   :=
SOURCE_LIB_CPP :=
DEPEND_LIBS	   :=

get_relative_path = $(shell realpath $(dir $(lastword $(MAKEFILE_LIST))) --relative-to=$(CURDIR))

ifneq ("$(wildcard $(PROJ_ROOT)/.config)","")
include $(PROJ_ROOT)/.config
endif

ifeq ($(CONFIG_EXECUTE_ON_DDR),y)
	CFLAGS += -DEXECUTE_ON_DDR=1
ifeq ($(USE_DYNAMIC_XNN_WORKBUFFER),1)
	CFLAGS += -DWITH_DYNAMIC_XNN_WORKBUFFER=1
endif
endif

ifeq ($(CONFIG_EXECUTE_ON_ROM),y)
	CFLAGS += -DEXECUTE_ON_ROM=1
endif

ifeq ($(CONFIG_EXECUTE_ON_ROM),y)
	CONFIG_FW_CODE_SIZE_KB=0
endif

ifneq ($(findstring $(CONFIG_CHIP_AI1101A)$(CONFIG_CHIP_AI1102A)$(CONFIG_CHIP_AI1103A), y), )
	CONFIG_DDR0_SIZE_MB=0
	CONFIG_DDR1_SIZE_MB=4
endif

ifneq ($(findstring $(CONFIG_CHIP_AI1102B)$(CONFIG_CHIP_AI1103B), y), )
	CONFIG_DDR0_SIZE_MB=4
	CONFIG_DDR1_SIZE_MB=4
endif

ifneq ($(findstring $(CONFIG_CHIP_AI1102C)$(CONFIG_CHIP_AI1103C), y), )
	CONFIG_DDR0_SIZE_MB=0
	CONFIG_DDR1_SIZE_MB=8
endif


ifeq ($(CONFIG_CHIP_AI1101A), y)
	CHIP := 1101a
	CFLAGS += -DCONFIG_CHIP_AI1101A
endif

ifeq ($(CONFIG_CHIP_AI1102A), y)
	CHIP := 1102a
	CFLAGS += -DCONFIG_CHIP_AI1102A
endif

ifeq ($(CONFIG_CHIP_AI1102B), y)
	CHIP := 1102b
	CFLAGS += -DCONFIG_CHIP_AI1102B
endif

ifeq ($(CONFIG_CHIP_AI1102C), y)
	CHIP := 1102c
	CFLAGS += -DCONFIG_CHIP_AI1102C
endif

ifeq ($(CONFIG_CHIP_AI1103A), y)
	CHIP := 1103a
	CFLAGS += -DCONFIG_CHIP_AI1103A
endif

ifeq ($(CONFIG_CHIP_AI1103B), y)
	CHIP := 1103b
	CFLAGS += -DCONFIG_CHIP_AI1103B
endif

ifeq ($(CONFIG_CHIP_AI1103C), y)
	CHIP := 1103c
	CFLAGS += -DCONFIG_CHIP_AI1103C
endif

CFLAGS += -DCONFIG_DDR0_SIZE_MB=$(CONFIG_DDR0_SIZE_MB)
CFLAGS += -DCONFIG_DDR1_SIZE_MB=$(CONFIG_DDR1_SIZE_MB)
CFLAGS += -DCONFIG_FW_CODE_SIZE_KB=$(CONFIG_FW_CODE_SIZE_KB)
CFLAGS += -DCONFIG_SRAM_CODE_SIZE_KB=$(CONFIG_SRAM_CODE_SIZE_KB)
CFLAGS += -DCONFIG_DMA_MEM_SIZE_KB=$(CONFIG_DMA_MEM_SIZE_KB)
CFLAGS += -DCONFIG_DMA_HEAP_SIZE_KB=$(CONFIG_DMA_HEAP_SIZE_KB)
CFLAGS += -DCONFIG_SMALL_MEM_SIZE_BYTE=$(CONFIG_SMALL_MEM_SIZE_BYTE)
CFLAGS += -DCONFIG_SMALL_MEM_POOL_SIZE_BYTE=$(CONFIG_SMALL_MEM_POOL_SIZE_BYTE)
# CONFIG_SRAM_CODE_SIZE_KB=90
# CONFIG_DMA_MEM_SIZE_KB=128
# CONFIG_DMA_HEAP_SIZE_KB=56
# CONFIG_SMALL_MEM_SIZE_BYTE=1060
# CONFIG_SMALL_MEM_POOL_SIZE_BYTE=413696

ifeq ($(CONFIG_XNN_PING_PONG_BUF), y)
	CFLAGS += -DXNN_PING_PONG_BUF=1
endif

include $(SDK_ROOT)/components/Makefile

# magic, used to conrol sdk generating when build project depends on sdk source code
SDK_INCLUDE_WITH_SRC_CODE:=0
ifneq ("$(wildcard $(SDK_ROOT)/Makefile)","")
SDK_SRC_ROOT:=$(SDK_ROOT)
SDK_ROOT:=$(SDK_SRC_ROOT)/sdk
SDK_INCLUDE_WITH_SRC_CODE=1
endif # ifneq ("$(wildcard $(SDK_ROOT)/Makefile)","")

BIN2IMAGE:=$(SDK_ROOT)/tools/bin2image/bin2image
BIN2OTA:=$(SDK_ROOT)/tools/bin2ota/bin2ota
MULTI_BIN2OTA:=$(SDK_ROOT)/tools/bin2ota/multi_bin2ota
BIN2ROM:=$(SDK_ROOT)/tools/bin2rom/bin2rom
SWAPDW:=$(SDK_ROOT)/tools/swapdw/swapdw
MKLFS:=$(SDK_ROOT)/tools/mklfs/mklfs
PACKXFILES:=$(SDK_ROOT)/tools/packxfiles/packxfiles

TARGET_ELF      :=$(PROJ_NAME).elf
TARGET_BIN      :=$(PROJ_NAME).bin
TARGET_IMG      :=$(PROJ_NAME).img
TARGET_DIS      :=$(PROJ_NAME).dis
TARGET_LIB		:=lib$(PROJ_NAME).a
TARGET_LIST     :=$(PROJ_NAME).list

LIBSDK:=$(SDK_ROOT)/lib/libai11xxsdk.a

OBJ_DIR:= $(PROJ_ROOT)/obj
BIN_DIR:= $(PROJ_ROOT)/bin

ifeq ($(CONFIG_EXECUTE_ON_ROM),y)
LDSCRIPT_SOURCE := $(SDK_ROOT)/ldscript/AI3100_LDSCRIPT_ROM.c
endif

ifeq ($(CONFIG_EXECUTE_ON_DDR),y)
LDSCRIPT_SOURCE := $(SDK_ROOT)/ldscript/AI3100_LDSCRIPT_DDR.c
endif

LDSCRIPT:=$(BIN_DIR)/ai11xx.ld

SOURCE_APP_C += $(SDK_ROOT)/ldscript/boot.c

SDK_INC:=$(SDK_ROOT)/include
-include $(SDK_ROOT)/scripts/Makefile.libflags
-include $(SDK_ROOT)/scripts/Makefile.libincs

CFLAGS += -Wall -Wextra
CFLAGS += -mcpu=cortex-a5 -mfpu=neon-fp16 -mfloat-abi=softfp
CFLAGS += -Wl,--wrap,malloc -Wl,--wrap,_malloc_r -Wl,--wrap,calloc -Wl,--wrap,_calloc_r -Wl,--wrap,realloc -Wl,--wrap,_realloc_r -Wl,--wrap,free -Wl,--wrap,_free_r -Wl,--wrap,puts -Wl,--wrap,printf -Wl,--wrap,fprintf

ifeq ($(CONFIG_DEBUG),y)
	CFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG_DEBUG_WITH_OPT),y)
	# CFLAGS += -g -O0
	# add -Wl,--undefined=malloc so will linker keep malloc function which will be used when download data to memory by gdb
	# https://stackoverflow.com/questions/73887908/prevent-gcc-from-optimization-removal-of-variables-when-using-wl-gc-sections
	CFLAGS += -O0 -g -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--undefined=malloc
	CFLAGS += -Wl,--undefined=aiva_default_print_memory_info -Wl,--undefined=aiva_tlsf_print_memory_info
	CFLAGS += -Wl,--undefined=aiva_default_get_max_free_heap_size -Wl,--undefined=aiva_tlsf_get_max_free_heap_size
else ifeq ($(CONFIG_RELEASE_WITH_NANOSPEC),y)
	CFLAGS += --specs=nano.specs
	CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--strip-all
else
	#CFLAGS += -O2
	CFLAGS += -Os -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--strip-all
endif
CFLAGS += -Werror=implicit-int
CFLAGS += -Werror=incompatible-pointer-types
CFLAGS += -Werror=implicit-function-declaration
# treat return-type warning as error
CFLAGS += -Werror=return-type
#CFLAGS += -Wno-unused-parameter
#CFLAGS += -Wno-unused-variable
#CFLAGS += -Wno-unused-function
CFLAGS += -Wno-missing-braces
#CFLAGS += -Wno-comment

CFLAGS   += $(INC_SDK_DIR)
CFLAGS   += $(INC_LIB_DIR)
CFLAGS 	 += $(INC_APP_DIR)
CXXFLAGS += -std=c++11 -fno-exceptions

LFLAGS += -lm -lstdc++
LFLAGS += -T $(LDSCRIPT)

MAKEDEBUG := 0
ifeq ($(MAKEDEBUG),1)
MAKENOISE  := echo
MAKEFLAGS  := --print-directory
else
MAKENOISE  := : # Do nothing with arg string.
MAKEFLAGS  := --quiet --no-print-directory
endif
