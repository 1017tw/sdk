PROJ_ROOT   := .
SDK_ROOT    :=$(PROJ_ROOT)/../../sdk
TOOLS_ROOT  :=$(SDK_ROOT)/tools
PROJ_NAME   := usb_device_example

include $(TOOLS_ROOT)/common_setting.mk

all: elf bin img

INC_APP_DIR  += -I$(PROJ_ROOT)
SOURCE_APP_C += $(PROJ_ROOT)/usb_device_example.c
SOURCE_APP_C += $(PROJ_ROOT)/usb_lpbk.c
SOURCE_APP_C += $(PROJ_ROOT)/usb_lpbk_ep0.c

include $(TOOLS_ROOT)/build.mk
