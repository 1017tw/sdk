CUR_PATH := $(call get_relative_path)

INC_LIB_DIR  += -I$(CUR_PATH)/
INC_LIB_DIR  += -I$(CUR_PATH)/device

ifeq ($(CONFIG_AUDIO_ES8374),y)
SOURCE_LIB_C += $(CUR_PATH)/device/audio_es8374.c
endif

ifeq ($(CONFIG_AUDIO_ES8311),y)
SOURCE_LIB_C += $(CUR_PATH)/device/audio_es8311.c
endif

ifeq ($(CONFIG_AUDIO_ES8388),y)
SOURCE_LIB_C += $(CUR_PATH)/device/audio_es8388.c
endif

SOURCE_LIB_C += $(CUR_PATH)/audio_dev.c

