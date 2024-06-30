#include "cis_ov02b.h"
#include "aiva_utils.h"
#include "sysctl.h"
#include "i2c.h"
#include "gpio.h"
#include "syslog.h"
#include "udevice.h"
#include "aiva_sleep.h"
#include <math.h>

#define ENABLE_QVGA              (0)
#define W_QVGA                   (320)
#define H_QVGA                   (240)

#define W_VGA                    (640)
#define H_VGA                    (480)

#define W_1600                   (1600)
#define H_1200                   (1200)

#define DEFAULT_AGAIN            (8.0f)     // 8x
#define DEFAULT_DGAIN            (1.0f)     // 1x
#define DEFAULT_EXPOSURE         (1000)     // 1000 lines

#define REG_DLY                  (0xffff)
#define SENSOR_ADDR_WR           (0x78 >> 1)


typedef struct _OV02B_REG_T {
    uint16_t        reg_addr;
    uint8_t         data;
} OV02B_REG_T;


static const char* TAG = "cis_ov02b";

// static float m_again_cur = DEFAULT_AGAIN;
// static float m_dgain_cur = DEFAULT_DGAIN;
// static uint16_t m_expo_lines  = DEFAULT_EXPOSURE;


static OV02B_REG_T ov02b_start_regs[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x03},
    {0xc2, 0x01},
};

static OV02B_REG_T ov02b_stop_regs[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x03},
    {0xc2, 0x00},
};

static const OV02B_REG_T ov02b_master[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x00},
    {0x36, 0x00},
    {0x37, 0x03},
    {0xfd, 0x01},
    {0x92, 0x10},
    {0x93, 0x01},
    {0x90, 0x1c},
};

static const OV02B_REG_T ov02b_slave[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x18, 0x01},
    {0xfe, 0x02},
    {0xfd, 0x00},
    {0x36, 0x00},
    {0x37, 0x0d},
    {0x7b, 0x80},
    {0xfd, 0x01},
    {0x94, 0x02},
    {0x95, 0xc9},
};

static const OV02B_REG_T ov02b_sleep[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x03},
    {0xc2, 0x00},
    /*{REG_DLY, 0x64},*/
    {REG_DLY, 80},
    {0xfd, 0x00},
    {0x1f, 0x06},
};

static const OV02B_REG_T ov02b_wake[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x00},
    {0x1f, 0x00},
    {0xfd, 0x03},
    {0xc2, 0x01},
};

#if ENABLE_QVGA
// qvga: mclk = 24M, mipi data rate = 114 bps, raw10 10fps
// NOTE master/slave sync setting don't set, st data rate set to 330, then it can output normal.
static const OV02B_REG_T ov02b_qvga[] __ATTR_ALIGN__(32) = {
    {0xfc, 0x01},
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},
    {0x25, 0x06},
    {0x29, 0x03},
    {0x2a, 0x90},
    {0x1e, 0x17},
    {0x33, 0x07},
    {0x35, 0x07},
    {0x4a, 0x0c},
    {0x3a, 0x05},
    {0x3b, 0x02},
    {0x3e, 0x00},
    {0x46, 0x01},
    {0x6d, 0x03},
    {0xfd, 0x01},
    {0x0e, 0x02},
    {0x0f, 0x1a},
    {0x18, 0x00},
    {0x22, 0x8f},
    {0x23, 0x02},
    {0x17, 0x2c},
    {0x19, 0x20},
    {0x1b, 0x06},
    {0x1c, 0x04},
    {0x20, 0x03},
    {0x30, 0x01},
    {0x33, 0x01},
    {0x31, 0x0a},
    {0x32, 0x09},
    {0x38, 0x01},
    {0x39, 0x01},
    {0x3a, 0x01},
    {0x3b, 0x01},
    {0x4f, 0x04},
    {0x4e, 0x05},
    {0x50, 0x01},
    {0x35, 0x0c},
    {0x45, 0x2a},
    {0x46, 0x2a},
    {0x47, 0x2a},
    {0x48, 0x2a},
    {0x4a, 0x2c},
    {0x4b, 0x2c},
    {0x4c, 0x2c},
    {0x4d, 0x2c},
    {0x56, 0x3a},
    {0x57, 0x0a},
    {0x58, 0x24},
    {0x59, 0x20},
    {0x5a, 0x0a},
    {0x5b, 0xff},
    {0x37, 0x0a},
    {0x42, 0x0e},
    {0x68, 0x90},
    {0x69, 0xcd},
    {0x6a, 0x8f},
    {0x7c, 0x0a},
    {0x7d, 0x0a},
    {0x7e, 0x0a},
    {0x7f, 0x08},
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},
    {0x88, 0x0f},
    {0x94, 0x02},
    {0x98, 0xd1},
    {0xfe, 0x02},
    {0xfd, 0x03},

    {0x81, 0x01}, // test pattern

    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xa1, 0x40},
    {0xb1, 0x40},
    {0xaf, 0x04},
    {0xae, 0x0d},
    {0x88, 0x5b},
    {0x89, 0x7c},
    {0xb4, 0x05},
    {0x8c, 0x40},
    {0x8e, 0x40},
    {0x90, 0x40},
    {0x92, 0x40},
    {0x9b, 0x46},
    {0xac, 0x40},
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},
    {0xfd, 0x00},
    {0x4f, 0x01},
    {0x50, 0x40},
    {0x51, 0x00},
    {0x52, 0xf0},
    {0xfd, 0x01},
    {0x03, 0x70},
    {0x05, 0x10},
    {0x06, 0x00},
    {0x07, 0xa0},
    {0x08, 0x00},
    {0x09, 0xf0},
    {0xfd, 0x01},
    {0x14, 0x01},
    {0x15, 0x62},
    {0x01, 0x01},
    {0xfe, 0x02},
    {0xfd, 0x03},
    {0xc2, 0x01},
    {0xfb, 0x01},
    {0xfd, 0x01},
};
#endif

#if 0
// qvga: mclk = 24M, mipi data rate = 330 bps, raw10
static const OV02B_REG_T ov02b_qvga[] __ATTR_ALIGN__(32) = {
    {0xfc, 0x01},
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},
    {0x25, 0x06},
    {0x29, 0x03},
    {0x2a, 0xb4},
    {0x1e, 0x17},
    {0x33, 0x07},
    {0x35, 0x07},
    {0x4a, 0x0c},
    {0x3a, 0x05},
    {0x3b, 0x02},
    {0x3e, 0x00},
    {0x46, 0x01},
    {0x6d, 0x03},
    {0xfd, 0x01},
    {0x0e, 0x02},
    {0x0f, 0x1a},
    {0x18, 0x00},
    {0x22, 0x8f},
    {0x23, 0x02},
    {0x17, 0x2c},
    {0x19, 0x20},
    {0x1b, 0x06},
    {0x1c, 0x04},
    {0x20, 0x03},
    {0x30, 0x01},
    {0x33, 0x01},
    {0x31, 0x0a},
    {0x32, 0x09},
    {0x38, 0x01},
    {0x39, 0x01},
    {0x3a, 0x01},
    {0x3b, 0x01},
    {0x4f, 0x04},
    {0x4e, 0x05},
    {0x50, 0x01},
    {0x35, 0x0c},
    {0x45, 0x2a},
    {0x46, 0x2a},
    {0x47, 0x2a},
    {0x48, 0x2a},
    {0x4a, 0x2c},
    {0x4b, 0x2c},
    {0x4c, 0x2c},
    {0x4d, 0x2c},
    {0x56, 0x3a},
    {0x57, 0x0a},
    {0x58, 0x24},
    {0x59, 0x20},
    {0x5a, 0x0a},
    {0x5b, 0xff},
    {0x37, 0x0a},
    {0x42, 0x0e},
    {0x68, 0x90},
    {0x69, 0xcd},
    {0x6a, 0x8f},
    {0x7c, 0x0a},
    {0x7d, 0x0a},
    {0x7e, 0x0a},
    {0x7f, 0x08},
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},
    {0x88, 0x0f},
    {0x94, 0x02},
    {0x98, 0xd1},
    {0xfe, 0x02},
    {0xfd, 0x03},

    {0x81, 0x01}, // test pattern

    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xa1, 0x40},
    {0xb1, 0x40},
    {0xaf, 0x04},
    {0xae, 0x0d},
    {0x88, 0x5b},
    {0x89, 0x7c},
    {0xb4, 0x05},
    {0x8c, 0x40},
    {0x8e, 0x40},
    {0x90, 0x40},
    {0x92, 0x40},
    {0x9b, 0x46},
    {0xac, 0x40},
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},
    {0xfd, 0x00},
    {0x4f, 0x01},
    {0x50, 0x40},
    {0x51, 0x00},
    {0x52, 0xf0},
    {0xfd, 0x01},
    {0x03, 0x70},
    {0x05, 0x10},
    {0x06, 0x00},
    {0x07, 0xa0},
    {0x08, 0x00},
    {0x09, 0xf0},
    {0xfd, 0x01},
    {0x14, 0x01},
    {0x15, 0x62},
    {0x01, 0x01},
    {0xfe, 0x02},
    {0xfd, 0x03},
    {0xc2, 0x01},
    {0xfb, 0x01},
    {0xfd, 0x01},
};
#endif

#if 0
// qvga: mclk = 24M, mipi data rate = 660M bps, raw10
static const OV02B_REG_T ov02b_qvga[] __ATTR_ALIGN__(32) = {
    {0xfc, 0x01},
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},
    {0x25, 0x06},
    {0x29, 0x03},
    {0x2a, 0x34},
    {0x1e, 0x17},
    {0x33, 0x07},
    {0x35, 0x07},
    {0x4a, 0x0c},
    {0x3a, 0x05},
    {0x3b, 0x02},
    {0x3e, 0x00},
    {0x46, 0x01},
    {0x6d, 0x03},
    {0xfd, 0x01},
    {0x0e, 0x02},
    {0x0f, 0x1a},
    {0x18, 0x00},
    {0x22, 0xff},
    {0x23, 0x02},
    {0x17, 0x2c},
    {0x19, 0x20},
    {0x1b, 0x06},
    {0x1c, 0x04},
    {0x20, 0x03},
    {0x30, 0x01},
    {0x33, 0x01},
    {0x31, 0x0a},
    {0x32, 0x09},
    {0x38, 0x01},
    {0x39, 0x01},
    {0x3a, 0x01},
    {0x3b, 0x01},
    {0x4f, 0x04},
    {0x4e, 0x05},
    {0x50, 0x01},
    {0x35, 0x0c},
    {0x45, 0x2a},
    {0x46, 0x2a},
    {0x47, 0x2a},
    {0x48, 0x2a},
    {0x4a, 0x2c},
    {0x4b, 0x2c},
    {0x4c, 0x2c},
    {0x4d, 0x2c},
    {0x56, 0x3a},
    {0x57, 0x0a},
    {0x58, 0x24},
    {0x59, 0x20},
    {0x5a, 0x0a},
    {0x5b, 0xff},
    {0x37, 0x0a},
    {0x42, 0x0e},
    {0x68, 0x90},
    {0x69, 0xcd},
    {0x6a, 0x8f},
    {0x7c, 0x0a},
    {0x7d, 0x0a},
    {0x7e, 0x0a},
    {0x7f, 0x08},
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},
    {0x88, 0x0f},
    {0x94, 0x02},
    {0x98, 0xd1},
    {0xfe, 0x02},
    {0xfd, 0x03},
    
    {0x81, 0x01}, // test pattern

    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xa1, 0x40},
    {0xb1, 0x40},
    {0xaf, 0x04},
    {0xae, 0x0d},
    {0x88, 0x5b},
    {0x89, 0x7c},
    {0xb4, 0x05},
    {0x8c, 0x40},
    {0x8e, 0x40},
    {0x90, 0x40},
    {0x92, 0x40},
    {0x9b, 0x46},
    {0xac, 0x40},
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},
    {0xfd, 0x00},
    {0x55, 0x2b}, // raw8 2a, raw10 2b
    {0x4f, 0x01},
    {0x50, 0x40},
    {0x51, 0x00},
    {0x52, 0xf0},
    {0xfd, 0x01},
    {0x03, 0x70},
    {0x05, 0x10},
    {0x06, 0x00},
    {0x07, 0xa0},  
    {0x08, 0x00},
    {0x09, 0xf0},
    {0xfd, 0x03},
    {0xc2, 0x01},
    {0xfb, 0x01},
    {0xfd, 0x01},
};
#endif

//1600x1200: mclk = 24M, mipi data rate = 660M bps, raw10
#if 0
static const OV02B_REG_T ov02b_1600x1200[] __ATTR_ALIGN__(32) = {
    {0xfc, 0x01},
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},
    {0x25, 0x06},
    {0x29, 0x03},
    {0x2a, 0x34},
    {0x1e, 0x17},
    {0x33, 0x07},
    {0x35, 0x07},
    {0x4a, 0x0c},
    {0x3a, 0x05},
    {0x3b, 0x02},
    {0x3e, 0x00},
    {0x46, 0x01},
    {0x6d, 0x03},
    {0xfd, 0x01},
    /*{0x0e, 0x02},*/
    /*{0x0f, 0x1a},*/
    {0x0e, DEFAULT_EXPOSURE >> 8},
    {0x0f, DEFAULT_EXPOSURE & 0xff},
    {0x18, 0x00},
    /*{0x22, 0xff},*/
    {0x22, (uint8_t)(DEFAULT_AGAIN * 16 - 1) & 0xff}, // A-Gain, 0x10 - 1x

    {0x23, 0x02},
    {0x17, 0x2c},
    {0x19, 0x20},
    {0x1b, 0x06},
    {0x1c, 0x04},
    {0x20, 0x03},
    {0x30, 0x01},
    {0x33, 0x01},
    {0x31, 0x0a},
    {0x32, 0x09},
    {0x38, 0x01},
    {0x39, 0x01},
    {0x3a, 0x01},
    {0x3b, 0x01},
    {0x4f, 0x04},
    {0x4e, 0x05},
    {0x50, 0x01},
    {0x35, 0x0c},
    {0x45, 0x2a},
    {0x46, 0x2a},
    {0x47, 0x2a},
    {0x48, 0x2a},
    {0x4a, 0x2c},
    {0x4b, 0x2c},
    {0x4c, 0x2c},
    {0x4d, 0x2c},
    {0x56, 0x3a},
    {0x57, 0x0a},
    {0x58, 0x24},
    {0x59, 0x20},
    {0x5a, 0x0a},
    {0x5b, 0xff},
    {0x37, 0x0a},
    {0x42, 0x0e},
    {0x68, 0x90},
    {0x69, 0xcd},
    {0x6a, 0x8f},
    {0x7c, 0x0a},
    {0x7d, 0x0a},
    {0x7e, 0x0a},
    {0x7f, 0x08},
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},
    {0x88, 0x0f},
    {0x94, 0x02},
    {0x98, 0xd1},
    {0xfe, 0x02},
    {0xfd, 0x03},

    //{0x81, 0x01}, // test pattern

    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xa1, 0x40},
    {0xb1, 0x40},
    {0xaf, 0x04},
    {0xae, 0x0d},
    {0x88, 0x5b},
    {0x89, 0x7c},
    {0xb4, 0x05},
    {0x8c, 0x40},
    {0x8e, 0x40},
    {0x90, 0x40},
    {0x92, 0x40},
    {0x9b, (uint8_t)(DEFAULT_DGAIN * 64 - 1) & 0xff}, // D-Gain, 0x40 - 1x
    {0xac, 0x40},
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},
    {0xfd, 0x00},
    {0x50, 0x40},
    {0x52, 0xb0},
    {0xfd, 0x01},
    {0x03, 0x70},
    {0x05, 0x10},
    {0x07, 0x20},
    {0x09, 0xb0},
#ifdef USE_SENSOR_OV02B_STROBE
    // strobe setting
    {0xfd, 0x00},
    {0x1c, 0x00}, // strobe pin output enable
    {0x1d, 0x11}, // FSIN and STROBE output driving setting

    {0xfd, 0x01},
    {0x76, 0xc1}, // [7]Strobe on/off (strobe_req), [6]strobe_polarity [2:0]: mode
    {0x77, 0x02}, // Dummy row number added at strobe[15:8]
    {0x78, 0x63}, // Dummy row number added at strobe[7:0]

    {0x14, 0x02},
    {0x15, 0x63},
    {0xfe, 0x02},

    {0x79, 0x30}, // Bit[6:4]: start_point_dly
#endif
    {0xfd, 0x03},
    {0xc2, 0x01},
    {0xfb, 0x01},
    {0xfd, 0x01},
};
#endif

static const OV02B_REG_T ov02b_1600x1200_head[] __ATTR_ALIGN__(32) = {
    {0xfc, 0x01},
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},
    {0x25, 0x06},
    {0x29, 0x03},
    {0x2a, 0x34},
    {0x1e, 0x17},
    {0x33, 0x07},
    {0x35, 0x07},
    {0x4a, 0x0c},
    {0x3a, 0x05},
    {0x3b, 0x02},
    {0x3e, 0x00},
    {0x46, 0x01},
    {0x6d, 0x03},
    {0xfd, 0x01},
    /*{0x0e, 0x02},*/
    /*{0x0f, 0x1a},*/
    {0x0e, DEFAULT_EXPOSURE >> 8},
    {0x0f, DEFAULT_EXPOSURE & 0xff},
    {0x18, 0x00},
    /*{0x22, 0xff},*/
    {0x22, (uint8_t)(DEFAULT_AGAIN * 16 - 1) & 0xff}, // A-Gain, 0x10 - 1x

    {0x23, 0x02},
    {0x17, 0x2c},
    {0x19, 0x20},
    {0x1b, 0x06},
    {0x1c, 0x04},
    {0x20, 0x03},
    {0x30, 0x01},
    {0x33, 0x01},
    {0x31, 0x0a},
    {0x32, 0x09},
    {0x38, 0x01},
    {0x39, 0x01},
    {0x3a, 0x01},
    {0x3b, 0x01},
    {0x4f, 0x04},
    {0x4e, 0x05},
    {0x50, 0x01},
    {0x35, 0x0c},
    {0x45, 0x2a},
    {0x46, 0x2a},
    {0x47, 0x2a},
    {0x48, 0x2a},
    {0x4a, 0x2c},
    {0x4b, 0x2c},
    {0x4c, 0x2c},
    {0x4d, 0x2c},
    {0x56, 0x3a},
    {0x57, 0x0a},
    {0x58, 0x24},
    {0x59, 0x20},
    {0x5a, 0x0a},
    {0x5b, 0xff},
    {0x37, 0x0a},
    {0x42, 0x0e},
    {0x68, 0x90},
    {0x69, 0xcd},
    {0x6a, 0x8f},
    {0x7c, 0x0a},
    {0x7d, 0x0a},
    {0x7e, 0x0a},
    {0x7f, 0x08},
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},
    {0x88, 0x0f},
    {0x94, 0x02},
    {0x98, 0xd1},
    {0xfe, 0x02},
    {0xfd, 0x03},

    //{0x81, 0x01}, // test pattern

    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xa1, 0x40},
    {0xb1, 0x40},
    {0xaf, 0x04},
    {0xae, 0x0d},
    {0x88, 0x5b},
    {0x89, 0x7c},
    {0xb4, 0x05},
    {0x8c, 0x40},
    {0x8e, 0x40},
    {0x90, 0x40},
    {0x92, 0x40},
    {0x9b, (uint8_t)(DEFAULT_DGAIN * 64 - 1) & 0xff}, // D-Gain, 0x40 - 1x
    {0xac, 0x40},
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},
    {0xfd, 0x00},
    {0x50, 0x40},
    {0x52, 0xb0},
    {0xfd, 0x00},
};

static const OV02B_REG_T ov02b_1600x1200_15fps[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x14, 0x04},
    {0x15, 0xc6},
    {0xfd, 0x00},
};

// static const OV02B_REG_T ov02b_1600x1200_30fps[] __ATTR_ALIGN__(32) = {
// };

// mirror and flip mode
static const OV02B_REG_T ov02b_1600x1200_mf_none[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x12, 0x00},
    {0xfd, 0x00},
};

static const OV02B_REG_T ov02b_1600x1200_mf_flip[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x12, 0x01},
    {0xfd, 0x00},
};

static const OV02B_REG_T ov02b_1600x1200_mf_mirror[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x12, 0x02},
    {0xfd, 0x00},
};

static const OV02B_REG_T ov02b_1600x1200_mf_mirror_and_flip[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x12, 0x03},
    {0xfd, 0x00},
};

static const OV02B_REG_T ov02b_1600x1200_tail[] __ATTR_ALIGN__(32) = {
    {0xfd, 0x01},
    {0x03, 0x70},
    {0x05, 0x10},
    {0x07, 0x20},
    {0x09, 0xb0},
#ifdef USE_SENSOR_OV02B_STROBE
    // strobe setting
    {0xfd, 0x00},
    {0x1c, 0x00}, // strobe pin output enable
    {0x1d, 0x11}, // FSIN and STROBE output driving setting

    {0xfd, 0x01},
    {0x76, 0xc1}, // [7]Strobe on/off (strobe_req), [6]strobe_polarity [2:0]: mode
    {0x77, 0x02}, // Dummy row number added at strobe[15:8]
    {0x78, 0x63}, // Dummy row number added at strobe[7:0]

    {0x14, 0x02},
    {0x15, 0x63},
    {0xfe, 0x02},

    {0x79, 0x30}, // Bit[6:4]: start_point_dly
#endif
    {0xfd, 0x03},
    // {0xc2, 0x01},
    {0xfb, 0x01},
    {0xfd, 0x01},
};

static void ov02b_busy_delay(int32_t ms)
{
    aiva_msleep(ms);
}

static int ov02b_write_reg(int i2c_num, uint32_t i2c_tar_addr, uint16_t reg_addr, uint8_t reg_val)
{
    int ret;
    uint8_t data_buf[2];
    data_buf[0] = reg_addr;
    data_buf[1] = reg_val;
    ret = i2c_send_data(i2c_num, i2c_tar_addr, data_buf, 2);
    /*ret = i2c_send_data_dma(DMAC_CHANNEL0, i2c_num, data_buf, 2);*/

    return ret;
}

static int ov02b_read_reg(int i2c_num, uint32_t i2c_tar_addr, uint16_t reg_addr, uint8_t *reg_val)
{
    int ret;
    
    ret = i2c_send_data(i2c_num, i2c_tar_addr, (const uint8_t *)&reg_addr, 1);
    /*ret = i2c_send_data_dma(DMAC_CHANNEL0, i2c_num, &reg_addr, 1);*/
    if (ret < 0) {
        return ret;
    }
    ret = i2c_recv_data(i2c_num, i2c_tar_addr, 0, 0, reg_val, 1);
    /*ret = i2c_recv_data_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, i2c_num, 0, 0, reg_val, 1);*/
    
    return ret;
}


static int ov02b_program_regs(
        int             i2c_num,
		uint32_t 		i2c_tar_addr,
        const OV02B_REG_T    *reg_list,
        int             cnt
        )
{
    int i;
    int ret = 0;
    for (i = 0; i < cnt; i++) {
        if (reg_list[i].reg_addr != REG_DLY) {
            ret = ov02b_write_reg(i2c_num, i2c_tar_addr, reg_list[i].reg_addr, reg_list[i].data);
            if (ret < 0) {
                LOGE(__func__, "error: i = %d, reg_addr 0x%x, data 0x%x.",
                        i, reg_list[i].reg_addr, reg_list[i].data);
                break;
            }
        } else {
            ov02b_busy_delay(reg_list[i].data);
        }
    }
    return ret;
}



static int ov02b_i2c_test(int i2c_num, uint32_t i2c_tar_addr)
{
    uint8_t pid_hi, pid_lo;

    ov02b_read_reg(i2c_num, i2c_tar_addr, 0x02, &pid_hi); 
    ov02b_read_reg(i2c_num, i2c_tar_addr, 0x03, &pid_lo); 

    LOGD(TAG, "ov02b_i2c_test: pid is 0x%2x:0x%2x", pid_hi, pid_lo);


    if (pid_lo != 0x2b) {
        LOGE(TAG, "ov02b_i2c_test: read reg(0x03) fail!, get 0x%x, expected 0x2b", pid_lo);
        return -1;
    }
   
    LOGD(TAG, "ov02b_i2c_test: read reg(0x03) success!");
    return 0;
}


static int ov02b_init(const OV02B_REG_T *reg_list, int cnt, int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;

    i2c_init(i2c_num, i2c_tar_addr, 7, 350*1000);

    if (ov02b_i2c_test(i2c_num, i2c_tar_addr) < 0) {
        LOGE(TAG, "ov02b i2c test fail\n");
        return -1;
    }

    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, reg_list, cnt);
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }
    return ret;
}


#if ENABLE_QVGA
static int ov02b_qvga_init(int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;

    ret = ov02b_init(
            ov02b_qvga,
            AIVA_ARRAY_LEN(ov02b_qvga),
            i2c_num,
			i2c_tar_addr
            );

    return ret;
}

static int ov02b_qvga_init_master(int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;

    ret = ov02b_init(
            ov02b_qvga,
            AIVA_ARRAY_LEN(ov02b_qvga),
            i2c_num,
			i2c_tar_addr
            );

    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_master, AIVA_ARRAY_LEN(ov02b_master));
    /*LOGD(TAG, "master return: %d.", ret);*/
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }


    return ret;
}

static int ov02b_qvga_init_slave(int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;

    ret = ov02b_init(ov02b_qvga, AIVA_ARRAY_LEN(ov02b_qvga), i2c_num, i2c_tar_addr);
    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_slave, AIVA_ARRAY_LEN(ov02b_slave));
    /*LOGD(TAG, "master return: %d.", ret);*/
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }
    return ret;
}
#endif

static int ov02b_1600x1200_init(int fps, int mf_mode, int i2c_num, uint32_t i2c_tar_addr)
{
    // int ret;
    // ret = ov02b_init(ov02b_1600x1200, AIVA_ARRAY_LEN(ov02b_1600x1200), i2c_num, i2c_tar_addr);
    if (ov02b_init(ov02b_1600x1200_head, AIVA_ARRAY_LEN(ov02b_1600x1200_head), i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_1600x1200_head init failed", __LINE__);
        return -1;
    }

    if (fps == 30) {
        // LOGE(__func__, "line %d, ov02b_1600x1200_30fps", __LINE__);
        // if (ov02b_init(ov02b_1600x1200_30fps, AIVA_ARRAY_LEN(ov02b_1600x1200_30fps), i2c_num, i2c_tar_addr) < 0) {
        //     LOGE(__func__, "line %d, ov02b_1600x1200_30fps failed", __LINE__);
        //     return -1;
        // }
    }
    else if (fps == 15) {
        if (ov02b_init(ov02b_1600x1200_15fps, AIVA_ARRAY_LEN(ov02b_1600x1200_15fps), i2c_num, i2c_tar_addr) < 0) {
            LOGE(__func__, "line %d, ov02b_1600x1200_15fps init failed", __LINE__);
            return -1;
        }
    }
    else {
        LOGE(__func__, "line %d, ov02b not support %d fps", __LINE__, fps);
        return -1;
    }

    switch (mf_mode) {
        case 0:
            if (ov02b_init(ov02b_1600x1200_mf_none, AIVA_ARRAY_LEN(ov02b_1600x1200_mf_none), i2c_num, i2c_tar_addr) < 0) {
                LOGE(__func__, "line %d, ov02b_1600x1200_mf_none init failed", __LINE__);
                return -1;
            }
            break;
        case 1:
            if (ov02b_init(ov02b_1600x1200_mf_flip, AIVA_ARRAY_LEN(ov02b_1600x1200_mf_flip), i2c_num, i2c_tar_addr) < 0) {
                LOGE(__func__, "line %d, ov02b_1600x1200_mf_flip init failed", __LINE__);
                return -1;
            }
            break;
        case 2:
            if (ov02b_init(ov02b_1600x1200_mf_mirror, AIVA_ARRAY_LEN(ov02b_1600x1200_mf_mirror), i2c_num, i2c_tar_addr) < 0) {
                LOGE(__func__, "line %d, ov02b_1600x1200_mf_mirror init failed", __LINE__);
                return -1;
            }
            break;
        case 3:
            if (ov02b_init(ov02b_1600x1200_mf_mirror_and_flip, AIVA_ARRAY_LEN(ov02b_1600x1200_mf_mirror_and_flip), i2c_num, i2c_tar_addr) < 0) {
                LOGE(__func__, "line %d, ov02b_1600x1200_mf_mirror_and_flip init failed", __LINE__);
                return -1;
            }
            break;
        default:
            LOGE(__func__, "line %d, ov02b not support %d mirror and flip mode", __LINE__, mf_mode);
            return -1;
            break;
    }
    if (ov02b_init(ov02b_1600x1200_tail, AIVA_ARRAY_LEN(ov02b_1600x1200_tail), i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_1600x1200_tail init failed", __LINE__);
        return -1;
    }
    return 0;
}

static int ov02b_1600x1200_init_master(int fps, int mf_mode, int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
    // ret = ov02b_init(ov02b_1600x1200, AIVA_ARRAY_LEN(ov02b_1600x1200), i2c_num, i2c_tar_addr);
    if (ov02b_1600x1200_init(fps, mf_mode, i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_1600x1200_init failed!", __LINE__);
        return -1;
    }
    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_master, AIVA_ARRAY_LEN(ov02b_master));
    /*LOGD(TAG, "master return: %d.", ret);*/
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }
    return ret;
}

static int ov02b_1600x1200_init_slave(int fps, int mf_mode, int i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
    // ret = ov02b_init(ov02b_1600x1200, AIVA_ARRAY_LEN(ov02b_1600x1200), i2c_num, i2c_tar_addr);
    if (ov02b_1600x1200_init(fps, mf_mode, i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_1600x1200_init failed!", __LINE__);
        return -1;
    }
    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_slave, AIVA_ARRAY_LEN(ov02b_slave));
    /*LOGD(TAG, "slave return: %d.", ret);*/
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }
    return ret;
}

static int cis_ov02b_init_common(cis_dev_driver_t *dev_driver)
{
    // enable sensor mclk
    if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK0) {
        sysctl_set_sens_mclk(MCLK_ID0, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);
    } else if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK1) {
        sysctl_set_sens_mclk(MCLK_ID1, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK1, 1);
    } else {
        LOGE(TAG, "invalid mclk id: %d\n", dev_driver->mclk_id);
    }

    if (ov02b_1600x1200_init(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr) != 0) {
        LOGE(TAG, "ov02b_init_1080p fail\n");
        return -1;
    }
    return 0;
}

static int cis_ov02b_init_master(cis_dev_driver_t *dev_driver)
{
    // enable sensor mclk
    if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK0) {
        sysctl_set_sens_mclk(MCLK_ID0, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);
    } else if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK1) {
        sysctl_set_sens_mclk(MCLK_ID1, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK1, 1);
    } else {
        LOGE(TAG, "invalid mclk id: %d\n", dev_driver->mclk_id);
    }

    if (ov02b_1600x1200_init_master(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr) != 0) {
        LOGE(TAG, "ov02b_init_1080p fail\n");
        return -1;
    }
    return 0;
}


static int cis_ov02b_init_slave(cis_dev_driver_t *dev_driver)
{
    // enable sensor mclk
    if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK0) {
        sysctl_set_sens_mclk(MCLK_ID0, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);
    } else if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK1) {
        sysctl_set_sens_mclk(MCLK_ID1, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK1, 1);
    } else {
        LOGE(TAG, "invalid mclk id: %d\n", dev_driver->mclk_id);
    }

    if (ov02b_1600x1200_init_slave(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr) != 0) {
        LOGE(TAG, "ov02b_init_1080p fail\n");
        return -1;
    }
    return 0;
}

static int cis_ov02b_start_stream(cis_dev_driver_t *dev_driver, const cis_config_t *config)
{
    (void)config;

    int ret;
    OV02B_REG_T       *reg_list;
    int                 cnt;

    reg_list = ov02b_start_regs;
    cnt     = AIVA_ARRAY_LEN(ov02b_start_regs);
    ret     = ov02b_program_regs(dev_driver->i2c_num, dev_driver->i2c_tar_addr, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "ov02b_start failed!");
    }
    return ret;
}


static int cis_ov02b_stop_stream(cis_dev_driver_t *dev_driver)
{
    int ret;
    OV02B_REG_T       *reg_list;
    int                 cnt;

    reg_list = ov02b_stop_regs;
    cnt     = AIVA_ARRAY_LEN(ov02b_stop_regs);
    ret     = ov02b_program_regs(dev_driver->i2c_num, dev_driver->i2c_tar_addr, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "ov02b_stop failed!");
    }
    return ret;
}

static int cis_ov02b_wake(cis_dev_driver_t *dev_driver)
{
    int ret;
    const OV02B_REG_T  *reg_list;
    int                 cnt;
    int i2c_num = dev_driver->i2c_num;
    
    /*LOGW(__func__, "in, i2c %d ... %x", i2c_num, dev_driver);*/
    
    i2c_init(i2c_num, dev_driver->i2c_tar_addr, 7, 350*1000);

    reg_list = ov02b_wake;
    cnt     = AIVA_ARRAY_LEN(ov02b_wake);
    ret     = ov02b_program_regs(dev_driver->i2c_num, dev_driver->i2c_tar_addr, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "ov02b_wake failed!");
    }
    return ret;
}

static int cis_ov02b_sleep(cis_dev_driver_t *dev_driver)
{
    int ret;
    const OV02B_REG_T  *reg_list;
    int                 cnt;
    int i2c_num = dev_driver->i2c_num;
    
    /*LOGW(__func__, "in, i2c %d ... %x", i2c_num, dev_driver);*/
    
    i2c_init(i2c_num, dev_driver->i2c_tar_addr, 7, 350*1000);

    reg_list = ov02b_sleep;
    cnt     = AIVA_ARRAY_LEN(ov02b_sleep);
    ret     = ov02b_program_regs(dev_driver->i2c_num, dev_driver->i2c_tar_addr, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "ov02b_sleep failed!");
    }
    return ret;
}

static void cis_ov02b_power_on(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    ov02b_busy_delay(3);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    ov02b_busy_delay(5);
    return;
}


static void cis_ov02b_power_off(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    ov02b_busy_delay(5);
    return;
}


static void cis_ov02b_reset(cis_dev_driver_t *dev_driver)
{
    (void)dev_driver;

    return;
}


static int cis_ov02b_get_interface_param(cis_dev_driver_t *dev_driver, cis_interface_param_t *param)
{
    (void)dev_driver;

    param->interface_type                   = CIS_INTERFACE_TYPE_MIPI;
    param->mipi_param.freq                  = 660;
    param->mipi_param.lane_num              = 1;
    param->mipi_param.vc_num                = 1;
    param->mipi_param.virtual_channels[0]   = MIPI_VIRTUAL_CHANNEL_0;
    param->mipi_param.data_type             = MIPI_DATA_TYPE_RAW_10;
    return 0;
}

static int cis_ov02b_get_exposure_param(cis_dev_driver_t *dev_driver, cis_exposure_param_t *exp_param)
{
    (void)dev_driver;

    exp_param->min_again        = 1.0;
    exp_param->max_again        = 8.0;
    exp_param->step_again       = 1.0;
    exp_param->min_dgain        = 1.0;
    exp_param->max_dgain        = 8.0;
    exp_param->step_dgain       = 1.0;
    exp_param->min_itime        = 0.0005;
    exp_param->max_itime        = 0.03;
    exp_param->step_itime       = 0.001;
    exp_param->initial_again    = 8.0;
    exp_param->initial_dgain    = 1.0;
    exp_param->initial_itime    = 0.01;

    return 0;
}

static int cis_ov02b_mapping_itime(float itime, uint8_t *itime_h, uint8_t *itime_l)
{
    int itime_i = 0;

    if (itime > 1.0) {
        // for itime > 1.0, we suppose it is "exposure lines"
        itime_i = itime;
    } else {
        // for itime <= 1.0 we suppose it is "exposure time"
        // float mipi_data_rate = 371.25 * 1000000;    // 371.25Mbps
        float mipi_data_rate = 660 * 1000000;     
        float mipi_lane_num  = 1;                   // 2 lane
        float mipi_data_bit  = 10;                  // raw 10
        // int   line_length    = ((0x04 << 8) | (0x4c)) * 2;
        float pclk           = mipi_data_rate * mipi_lane_num / mipi_data_bit;
        float T_pclk         = 1/pclk;
        // float T_line         = line_length * T_pclk;
        float T_line         = 0x1c0 * T_pclk * 4;  // 0x1c0 is hts in ov02b, meas 4xPCLK according to datasheet
        int   itime_lines    = (int)round(itime / T_line);
        //LOGI(__func__, "line_length=%d, pclk=%f, T_line=%f, itime_lines=%d", line_length, pclk, T_line, itime_lines);
        itime_i              = itime_lines;
    }
    *itime_h = (itime_i >> 8) & 0xff;
    *itime_l = itime_i & 0xff;
    return 0;
}

static int cis_ov02b_set_exposure(cis_dev_driver_t *dev_driver, const cis_exposure_t *exp)
{
    int i2c_num = dev_driver->i2c_num;
	uint8_t i2c_tar_addr = dev_driver->i2c_tar_addr;
    int again = exp->again * 16;
    int dgain = exp->dgain * 64;
    // int itime = exp->itime[0];
    uint8_t itime_h, itime_l;
    cis_ov02b_mapping_itime(exp->itime, &itime_h, &itime_l);
    // LOGE(__func__, "again %d, dgain %d, itimeh %d, itimel %d", again, dgain, itime_h, itime_l);

    if (again > 0xF8) {
        //FIXME:
        // again: max 15.5x
        again = 0xF8;
    }

    if (dgain > 0xFF) {
        //FIXME:
        // dgain: max 4x
        dgain = 0xFF;
    }

    i2c_init(i2c_num, i2c_tar_addr, 7, 350*1000);

    // LOGI(__func__, "OV02B SET EXPOSURE: i2c_num: %d, again: 0x%x, dgain: 0x%x, itime: 0x%x, 0x%x",
    //         i2c_num, again&0xff, dgain&0xff, itime>>8, itime&0xff);

    ov02b_write_reg(i2c_num, i2c_tar_addr, 0xFD, 0x1);
    ov02b_write_reg(i2c_num, i2c_tar_addr, 0x0E, itime_h);
    ov02b_write_reg(i2c_num, i2c_tar_addr, 0x0F, itime_l);
    ov02b_write_reg(i2c_num, i2c_tar_addr, 0x22, again & 0xff);

    ov02b_write_reg(i2c_num, i2c_tar_addr, 0xFD, 0x3);
    ov02b_write_reg(i2c_num, i2c_tar_addr, 0x9B, dgain & 0xff);

    ov02b_write_reg(i2c_num, i2c_tar_addr, 0xFE, 0x02);
    return 0;
}

static int cis_ov02b_get_frame_parameter(cis_dev_driver_t *dev_driver, cis_frame_param_t *param)
{
    (void)dev_driver;

    param->width     = 1600;
    param->height    = 1200;
    param->framerate = 30;
    param->format    = CIS_FORMAT_RGB_BAYER10;
    param->bayer_pat = CIS_BAYER_PAT_BGGR;
    /*param->bayer_pat = CIS_BAYER_PAT_RGGB;*/
    /*param->bayer_pat = CIS_BAYER_PAT_GRBG;*/

    return 0;
}

static cis_dev_driver_t ov02b_dev0 = {
    .name                   = "ov02b_dev0",
    .i2c_num                = I2C_DEVICE_0,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
#if (defined USE_L18_MX) || (defined USE_L18_MI) || (defined USE_L18_MK)
    .power_pin              = GPIOA_PIN1,
    .reset_pin              = GPIOA_PIN1,
#else
    .power_pin              = GPIOA_PIN0,
    .reset_pin              = GPIOA_PIN0,
#endif
#if (defined USE_L18_MX) || (defined USE_L18_MI) || (defined USE_L18_MK)
    .mclk_id                = CIS_MCLK_ID_MCLK1,
#else
    .mclk_id                = CIS_MCLK_ID_MCLK0,
#endif
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_common,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};


static cis_dev_driver_t ov02b_dev1 = {
    .name                   = "ov02b_dev1",
    .i2c_num                = I2C_DEVICE_1,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN1,
    .reset_pin              = GPIOA_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_slave,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,    
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};



static cis_dev_driver_t ov02b_dev2 = {
    .name                   = "ov02b_dev2",
    .i2c_num                = I2C_DEVICE_2,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN1,
    .reset_pin              = GPIOA_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1, 
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_master,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,    
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};



cis_dev_driver_t *cis_ov02b[] = {
    &ov02b_dev0,
    &ov02b_dev1,
    &ov02b_dev2
};

// SU18/SU22, IR-Right
static cis_dev_driver_t ov02b_suxx_dev0_master = {
    .name                   = "ov02b_suxx_dev0",
    .i2c_num                = I2C_DEVICE_0,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN0,
    .reset_pin              = GPIOA_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_master,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};

static cis_dev_driver_t ov02b_suxx_dev0_slave = {
    .name                   = "ov02b_suxx_dev0",
    .i2c_num                = I2C_DEVICE_0,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN0,
    .reset_pin              = GPIOA_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_slave,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};

// SU18/SU22, IR-Left
static cis_dev_driver_t ov02b_suxx_dev1 = {
    .name                   = "ov02b_suxx_dev1",
    .i2c_num                = I2C_DEVICE_1,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN1,
    .reset_pin              = GPIOA_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_master,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,    
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};

// for factory tets only
static cis_dev_driver_t ov02b_suxx_dev1_master = {
    .name                   = "ov02b_suxx_dev1",
    .i2c_num                = I2C_DEVICE_1,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN1,
    .reset_pin              = GPIOA_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_master,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,    
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};

// SU18/SU22, RGB
static cis_dev_driver_t ov02b_suxx_dev2 = {
    .name                   = "ov02b_suxx_dev2",
    .i2c_num                = I2C_DEVICE_2,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIOA_PIN2,
    .reset_pin              = GPIOA_PIN2,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = NULL,
    .init                   = cis_ov02b_init_common,
    .start_stream           = cis_ov02b_start_stream,
    .stop_stream            = cis_ov02b_stop_stream,
    .wake                   = cis_ov02b_wake,
    .sleep                  = cis_ov02b_sleep,    
    .power_on               = cis_ov02b_power_on,
    .power_off              = cis_ov02b_power_off,
    .reset                  = cis_ov02b_reset,
    .get_interface_param    = cis_ov02b_get_interface_param,
    .get_exposure_param     = cis_ov02b_get_exposure_param,
    .set_exposure           = cis_ov02b_set_exposure,
    .get_frame_parameter    = cis_ov02b_get_frame_parameter
};



/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */

/* L18/L18-MINI */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_dev0,    0,      &ov02b_dev0,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_dev1,    0,      &ov02b_dev1,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_dev2,    0,      &ov02b_dev2,    NULL);

/* SU18/SU22 factory test */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev0,    0,      &ov02b_suxx_dev0_master,    NULL);
// UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev1,    0,      &ov02b_suxx_dev1,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev1,    0,      &ov02b_suxx_dev1_master,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev2,    0,      &ov02b_suxx_dev2,    NULL);

/* SU18/SU22 product */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_dev0,    0,      &ov02b_suxx_dev0_slave,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_dev1,    0,      &ov02b_suxx_dev1,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_dev2,    0,      &ov02b_suxx_dev2,    NULL);
