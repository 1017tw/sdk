CUR_PATH := $(call get_relative_path)

INC_LIB_DIR  += -I$(CUR_PATH)/inc

ifeq ($(CONFIG_SHELL_CAT),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_cat.c
endif
ifeq ($(CONFIG_SHELL_CD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_cd.c
endif
ifeq ($(CONFIG_SHELL_CP),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_cp.c
endif
ifeq ($(CONFIG_SHELL_DF),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_df.c
endif
ifeq ($(CONFIG_SHELL_ECHO),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_echo.c
endif
ifeq ($(CONFIG_SHELL_I2C),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_i2c.c
endif
ifeq ($(CONFIG_SHELL_LED_DRIVER),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_led_driver.c
endif
ifeq ($(CONFIG_SHELL_LOAD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_load.c
endif
ifeq ($(CONFIG_SHELL_LS),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_ls.c
endif
ifeq ($(CONFIG_SHELL_LS_TASK),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_ls_task.c
endif
ifeq ($(CONFIG_SHELL_MEM_RW),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_mem_rw.c
endif
ifeq ($(CONFIG_SHELL_MKDIR),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_mkdir.c
endif
ifeq ($(CONFIG_SHELL_ACM_SHELL),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_os_acm_shell.c
endif
ifeq ($(CONFIG_SHELL_PART),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_part.c
endif
ifeq ($(CONFIG_SHELL_PWD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_pwd.c
endif
ifeq ($(CONFIG_SHELL_REBOOT),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_reboot.c
endif
ifeq ($(CONFIG_SHELL_REMOVE),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_remove.c
endif
ifeq ($(CONFIG_SHELL_SENSOR_EXPO),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_sensor_expo.c
endif
ifeq ($(CONFIG_SHELL_UPLOAD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_upload.c
endif
ifeq ($(CONFIG_SHELL_USB_ACM_LOAD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_usb_acm_load.c
endif
ifeq ($(CONFIG_SHELL_USB_ACM_UPLOAD),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_usb_acm_upload.c
endif
SOURCE_LIB_C += $(CUR_PATH)/cmds_log_cfg.c
ifeq ($(CONFIG_SHELL_MEMMAP),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_memmap.c
endif
ifeq ($(CONFIG_SHELL_MEM_INFO),y)
SOURCE_LIB_C += $(CUR_PATH)/cmds_mem_info.c
endif
