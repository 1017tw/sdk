CUR_PATH := $(call get_relative_path)

INC_LIB_DIR  += -I$(CUR_PATH)/device

ifeq ($(CONFIG_GPIO_LED_DRIVER),y)
SOURCE_LIB_C += $(CUR_PATH)/device/gpio_led.c
# SOURCE_LIB_C += $(CUR_PATH)/device/gpio_led_driver.c
endif

ifeq ($(CONFIG_LM3644),y)
# SOURCE_LIB_C += $(CUR_PATH)/device/lm3644.c
SOURCE_LIB_C += $(CUR_PATH)/device/lm3644_2.c
endif

ifeq ($(CONFIG_VAS1071),y)
SOURCE_LIB_C += $(CUR_PATH)/device/vas1071.c
endif
