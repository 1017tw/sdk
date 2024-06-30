CUR_PATH := $(call get_relative_path)

INC_APP_DIR  += -I$(CUR_PATH)/device

ifeq ($(CONFIG_DISPLAY_DVPOUT_ILI9806E),y)
SOURCE_APP_C += $(CUR_PATH)/device/dvpout_ili9806e.c
endif

ifeq ($(CONFIG_DISPLAY_I80_JLT28010C),y)
SOURCE_APP_C += $(CUR_PATH)/device/i80_jlt28010c.c
endif

ifeq ($(CONFIG_DISPLAY_I80_JLT28024A),y)
SOURCE_APP_C += $(CUR_PATH)/device/i80_jlt28024a.c
endif

ifeq ($(CONFIG_DISPLAY_I80_ALIENTEK28),y)
SOURCE_APP_C += $(CUR_PATH)/device/i80_alientek28.c
endif

ifeq ($(CONFIG_DISPLAY_SPI_LCD_ILI9341),y)
SOURCE_APP_C += $(CUR_PATH)/device/spi_lcd_ili9341.c
endif

ifeq ($(CONFIG_DISPLAY_SPI_LCD_ILI9488),y)
SOURCE_APP_C += $(CUR_PATH)/device/spi_lcd_ili9488.c
endif

ifeq ($(CONFIG_DISPLAY_SPI_LCD_ST7796),y)
SOURCE_APP_C += $(CUR_PATH)/device/spi_lcd_st7796.c
endif