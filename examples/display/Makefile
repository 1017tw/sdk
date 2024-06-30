PROJ_ROOT   := .
SDK_ROOT    :=$(PROJ_ROOT)/../../sdk
TOOLS_ROOT  :=$(SDK_ROOT)/tools
PROJ_NAME   := display_example

include $(TOOLS_ROOT)/common_setting.mk

all: elf bin img

INC_APP_DIR  += -I$(PROJ_ROOT)


ifeq ($(CONFIG_USE_DISPLAY_DVPOUT),y)
SOURCE_APP_C += $(PROJ_ROOT)/display_dvpout_example.c
endif

ifeq ($(CONFIG_USE_DISPLAY_I80),y)
SOURCE_APP_C += $(PROJ_ROOT)/display_i80_example.c
endif

include $(TOOLS_ROOT)/build.mk
