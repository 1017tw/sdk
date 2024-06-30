CUR_PATH := $(call get_relative_path)

ifeq ($(CONFIG_USB_SINK_LPBK),y)
-include $(CUR_PATH)/functions/usb_sink_lpbk/Makefile
endif

ifeq ($(CONFIG_UVC_CAMERA),y)
-include $(CUR_PATH)/functions/uvc_camera/Makefile
endif

