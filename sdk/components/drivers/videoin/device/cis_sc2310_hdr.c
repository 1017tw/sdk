#include "cis_sc2310_hdr.h"
#include "aiva_utils.h"
#include "aiva_sleep.h"
#include "sysctl.h"
#include "i2c.h"
#include "gpio.h"
#include "syslog.h"
#include "udevice.h"
#include <math.h>

#ifdef USE_FACELOCK_V2 // v2 need 30 fps to sync with ir camera
#define USE_30FPS   (1)
#else
#define USE_30FPS   (0)
#endif

#define SC2310_REG_CTRL_MODE       0x0100
#define CHIP_ID                    0x2311
#define SC2310_REG_CHIP_ID         0x3107

#define REG_DLY                    (0xffff)
#define SENSOR_ADDR_WR             (0x30)


typedef struct _SC2310_REG_T {
    uint16_t        reg_addr;
    uint8_t         data;
} SC2310_REG_T;


typedef struct _SC2310_GAIN_T {
    uint8_t coarse;
    uint8_t fine;
} SC2310_GAIN_T;


static const char* TAG = "cis_sc2310_hdr";

static SC2310_REG_T sc2310_hdr_start_regs[] __ATTR_ALIGN__(32) = {
    {SC2310_REG_CTRL_MODE, 0x01},
};

static SC2310_REG_T sc2310_hdr_stop_regs[] __ATTR_ALIGN__(32) = {
    {SC2310_REG_CTRL_MODE, 0x00},
};


// 0.5 step again table, 1.0 ~ 32.0
static const SC2310_GAIN_T sc2310_again_table[] = {
    {0x03, 0x40}, // 1.0
    {0x03, 0x60}, // 1.5
    {0x07, 0x40}, // 2.0
    {0x07, 0x50}, // 2.5
    {0x23, 0x47}, // 3.0
    {0x23, 0x52}, // 3.5
    {0x23, 0x5e}, // 4.0
    {0x23, 0x6a}, // 4.5
    {0x23, 0x76}, // 5.0
    {0x27, 0x41}, // 5.5
    {0x27, 0x47}, // 6.0
    {0x27, 0x4c}, // 6.5
    {0x27, 0x52}, // 7.0
    {0x27, 0x58}, // 7.5
    {0x27, 0x5e}, // 8.0
    {0x27, 0x64}, // 8.5
    {0x27, 0x6a}, // 9.0
    {0x27, 0x70}, // 9.5
    {0x27, 0x76}, // 10.0
    {0x27, 0x7b}, // 10.5
    {0x2f, 0x40}, // 11.0
    {0x2f, 0x44}, // 11.5
    {0x2f, 0x46}, // 12.0
    {0x2f, 0x49}, // 12.5
    {0x2f, 0x4c}, // 13.0
    {0x2f, 0x4f}, // 13.5
    {0x2f, 0x52}, // 14.0
    {0x2f, 0x55}, // 14.5
    {0x2f, 0x58}, // 15.0
    {0x2f, 0x5b}, // 15.5
    {0x2f, 0x5e}, // 16.0
    {0x2f, 0x61}, // 16.5
    {0x2f, 0x64}, // 17.0
    {0x2f, 0x67}, // 17.5
    {0x2f, 0x6a}, // 18.0
    {0x2f, 0x6d}, // 18.5
    {0x2f, 0x6f}, // 19.0
    {0x2f, 0x73}, // 19.5
    {0x2f, 0x76}, // 20.0
    {0x2f, 0x79}, // 20.5
    {0x2f, 0x7b}, // 21.0
    {0x3f, 0x41}, // 22.0
    {0x3f, 0x42}, // 22.5
    {0x3f, 0x44}, // 23.0
    {0x3f, 0x45}, // 23.5
    {0x3f, 0x47}, // 24.0
    {0x3f, 0x48}, // 24.5
    {0x3f, 0x4a}, // 25.0
    {0x3f, 0x4b}, // 25.5
    {0x3f, 0x4d}, // 26.0
    {0x3f, 0x4e}, // 26.5
    {0x3f, 0x50}, // 27.0
    {0x3f, 0x51}, // 27.5
    {0x3f, 0x52}, // 28.0
    {0x3f, 0x54}, // 28.5
    {0x3f, 0x55}, // 29.0
    {0x3f, 0x57}, // 29.5
    {0x3f, 0x58}, // 30.0
    {0x3f, 0x5a}, // 30.5
    {0x3f, 0x5b}, // 31.0
    {0x3f, 0x5d}, // 31.5
    {0x3f, 0x5e}, // 32.0
};



// 0.5 step again table, 1.0 ~ 31.5
static const SC2310_GAIN_T sc2310_dgain_table[] = {
    {0x00, 0x80}, // 1.0
    {0x00, 0xc0}, // 1.5
    {0x01, 0x80}, // 2.0
    {0x01, 0xa0}, // 2.5
    {0x01, 0xc0}, // 3.0
    {0x01, 0xe0}, // 3.5
    {0x03, 0x88}, // 4.0
    {0x03, 0x90}, // 4.5
    {0x03, 0xa0}, // 5.0
    {0x03, 0xb0}, // 5.5
    {0x03, 0xc0}, // 6.0
    {0x03, 0xd0}, // 6.5
    {0x03, 0xe0}, // 7.0
    {0x03, 0xf0}, // 7.5
    {0x07, 0x80}, // 8.0
    {0x07, 0x88}, // 8.5
    {0x07, 0x90}, // 9.0
    {0x07, 0x98}, // 9.5
    {0x07, 0xa0}, // 10.0
    {0x07, 0xa8}, // 10.5
    {0x07, 0xb0}, // 11.0
    {0x07, 0xb8}, // 11.5
    {0x07, 0xc0}, // 12.0
    {0x07, 0xc8}, // 12.5
    {0x07, 0xd0}, // 13.0
    {0x07, 0xd8}, // 13.5
    {0x07, 0xe0}, // 14.0
    {0x07, 0xe8}, // 14.5
    {0x07, 0xf0}, // 15.0
    {0x07, 0xf8}, // 15.5
    {0x0f, 0x80}, // 16.0
    {0x0f, 0x84}, // 16.5
    {0x0f, 0x88}, // 17.0
    {0x0f, 0x8c}, // 17.5
    {0x0f, 0x90}, // 18.0
    {0x0f, 0x94}, // 18.5
    {0x0f, 0x98}, // 19.0
    {0x0f, 0x9c}, // 19.5
    {0x0f, 0xa0}, // 20.0
    {0x0f, 0xa4}, // 20.5
    {0x0f, 0xa8}, // 21.0
    {0x0f, 0xac}, // 21.5
    {0x0f, 0xb0}, // 22.0
    {0x0f, 0xb4}, // 22.5
    {0x0f, 0xb8}, // 23.0
    {0x0f, 0xbc}, // 23.5
    {0x0f, 0xc0}, // 24.0
    {0x0f, 0xc4}, // 24.5
    {0x0f, 0xc8}, // 25.0
    {0x0f, 0xcc}, // 25.5
    {0x0f, 0xd0}, // 26.0
    {0x0f, 0xd4}, // 26.5
    {0x0f, 0xd8}, // 27.0
    {0x0f, 0xdc}, // 27.5
    {0x0f, 0xe0}, // 28.0
    {0x0f, 0xe4}, // 28.5
    {0x0f, 0xe8}, // 29.0
    {0x0f, 0xec}, // 29.5
    {0x0f, 0xf0}, // 30.0
    {0x0f, 0xf4}, // 30.5
    {0x0f, 0xf8}, // 31.0
    {0x0f, 0xfc}, // 31.5
};

#if USE_30FPS
// 24MHz input, 760Mbps, 2 lane, 10bit, 1920x1080@30fps
static const SC2310_REG_T sc2310_hdr_1080p_mipi_2lane[] __ATTR_ALIGN__(32) = {
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3001,0xfe},
    {0x3018,0x33},
    {0x301c,0x78},
    {0x301f,0xfd},
    {0x3031,0x0a},
    {0x3037,0x24},
    {0x3038,0x44},
    {0x303f,0x01},
    {0x3200,0x00},
    {0x3201,0x04},
    {0x3202,0x00},
    {0x3203,0x00},
    {0x3204,0x07},
    {0x3205,0x8b},
    {0x3206,0x04},
    {0x3207,0x3f},
    {0x3208,0x07},
    {0x3209,0x80},
    {0x320a,0x04},
    {0x320b,0x38},
    {0x320c,0x04},
    {0x320d,0x4c},
    {0x320e,0x08},
    {0x320f,0xfe},
    {0x3211,0x04},
    {0x3213,0x04},
    {0x3220,0x51},
    {0x3221,0xE0}, // up_down flip
    {0x3222,0x29},
    {0x3235,0x08},
    {0x3236,0xfc},
    {0x3301,0x10},
    {0x3302,0x10},
    {0x3303,0x30},
    {0x3306,0x5c},
    {0x3308,0x10},
    {0x3309,0x48},
    {0x330a,0x00},
    {0x330b,0xb8},
    {0x330e,0x30},
    {0x3314,0x04},
    {0x331b,0x83},
    {0x331e,0x21},
    {0x331f,0x39},
    {0x3320,0x01},
    {0x3324,0x02},
    {0x3325,0x02},
    {0x3326,0x00},
    {0x3333,0x30},
    {0x3334,0x40},
    {0x333d,0x08},
    {0x3341,0x07},
    {0x3343,0x03},
    {0x3364,0x1d},
    {0x3366,0xc0},
    {0x3367,0x08},
    {0x3368,0x04},
    {0x3369,0x00},
    {0x336a,0x00},
    {0x336b,0x00},
    {0x336c,0x42},
    {0x337f,0x03},
    {0x3380,0x1b},
    {0x33aa,0x00},
    {0x33b6,0x07},
    {0x33b7,0x07},
    {0x33b8,0x10},
    {0x33b9,0x10},
    {0x33ba,0x10},
    {0x33bb,0x07},
    {0x33bc,0x07},
    {0x33bd,0x18},
    {0x33be,0x18},
    {0x33bf,0x18},
    {0x33c0,0x05},
    {0x360f,0x05},
    {0x3621,0xac},
    {0x3622,0xf6},
    {0x3623,0x18},
    {0x3624,0x47},
    {0x3625,0x01},
    {0x3630,0xc8},
    {0x3631,0x88},
    {0x3632,0x18},
    {0x3633,0x22},
    {0x3634,0x44},
    {0x3635,0x20},
    {0x3636,0x62},
    {0x3637,0x0c},
    {0x3638,0x24},
    {0x363a,0x83},
    {0x363b,0x08},
    {0x363c,0x05},
    {0x363d,0x05},
    {0x3640,0x00},
    {0x366e,0x04},
    {0x3670,0x6a},
    {0x3671,0xf6},
    {0x3672,0x16},
    {0x3673,0x16},
    {0x3674,0xc8},
    {0x3675,0x54},
    {0x3676,0x18},
    {0x3677,0x22},
    {0x3678,0x53},
    {0x3679,0x55},
    {0x367a,0x40},
    {0x367b,0x40},
    {0x367c,0x40},
    {0x367d,0x58},
    {0x367e,0x40},
    {0x367f,0x58},
    {0x3693,0x20},
    {0x3694,0x40},
    {0x3695,0x40},
    {0x3696,0x9f},
    {0x3697,0x9f},
    {0x3698,0x9f},
    {0x369e,0x40},
    {0x369f,0x40},
    {0x36a0,0x58},
    {0x36a1,0x78},
    {0x36ea,0x2d},
    {0x36eb,0x0a},
    {0x36ec,0x0e},
    {0x36ed,0x23},
    {0x36fa,0x6a},
    {0x36fb,0x20},
    {0x3802,0x00},
    {0x3901,0x02},
    {0x3907,0x01},
    {0x3908,0x01},
    {0x391d,0x24},
    {0x391e,0x00},
    {0x391f,0xc0},
    {0x3933,0x28},
    {0x3934,0x0a},
    {0x3940,0x1b},
    {0x3941,0x40},
    {0x3942,0x08},
    {0x3943,0x0e},
    {0x3e00,0x00},
    {0x3e01,0x95},
    {0x3e02,0xa0},
    {0x3e03,0x0b},
    {0x3e04,0x08},
    {0x3e05,0xc0},
    {0x3e06,0x00},
    {0x3e07,0x80},
    {0x3e08,0x03},
    {0x3e09,0x40},
    {0x3e0e,0x66},
    {0x3e14,0xb0},
    {0x3e1e,0x35},
    {0x3e23,0x00},
    {0x3e24,0x4a},
    {0x3e25,0x03},
    {0x3e26,0x40},
    {0x3f00,0x0d},
    {0x3f04,0x02},
    {0x3f05,0x22},
    {0x3f08,0x04},
    {0x4500,0x59},
    {0x4501,0xa4},
    {0x4509,0x10},
    {0x4602,0x0f},
    {0x4603,0x00},
    {0x4809,0x01},
    {0x4816,0x51},
    {0x4837,0x1a},
    {0x5000,0x06},
    {0x5780,0x7f},
    {0x5781,0x06},
    {0x5782,0x04},
    {0x5783,0x02},
    {0x5784,0x01},
    {0x5785,0x16},
    {0x5786,0x12},
    {0x5787,0x08},
    {0x5788,0x02},
    {0x57a0,0x00},
    {0x57a1,0x74},
    {0x57a2,0x01},
    {0x57a3,0xf4},
    {0x57a4,0xf0},
    {0x6000,0x06},
    {0x6002,0x06},
    {0x36e9,0x40},
    {0x36f9,0x05},
    // {0x0100,0x01},
};
#else
// 24MHz input, 396Mbps, 2 lane, 10bit, 1920x1080@15fps
static const SC2310_REG_T sc2310_hdr_1080p_mipi_2lane[] __ATTR_ALIGN__(32) = {
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3001,0xfe},
    {0x3018,0x33},
    {0x301c,0x78},
    {0x301f,0x51},
    {0x3031,0x0a},
    {0x3037,0x24},
    {0x3038,0x44},
    {0x303f,0x01},
    {0x3200,0x00},
    {0x3201,0x04},
    {0x3202,0x00},
    {0x3203,0x04},
    {0x3204,0x07},
    {0x3205,0x8b},
    {0x3206,0x04},
    {0x3207,0x43},
    {0x3208,0x07},
    {0x3209,0x80},
    {0x320a,0x04},
    {0x320b,0x38},
    {0x320c,0x04},
    {0x320d,0x4c},
    {0x320e,0x09}, // 2*({16'h320e, 16'h320f} - {16'h3e23, 16'h3e24}-5 max long exposure time
    {0x320f,0x60},
    {0x3211,0x04},
    {0x3213,0x04},
    {0x3220,0x51},
    {0x3221,0xE0}, // up_down flip
    {0x3222,0x29},
    {0x3235,0x09},
    {0x3236,0x5e},
    {0x3250,0x3f},
    {0x3301,0x10},
    {0x3302,0x10},
    {0x3303,0x30},
    {0x3306,0x30},
    {0x3308,0x08},
    {0x3309,0x48},
    {0x330a,0x00},
    {0x330b,0x68},
    {0x330e,0x18},
    {0x3314,0x04},
    {0x331b,0x83},
    {0x331e,0x21},
    {0x331f,0x39},
    {0x3320,0x01},
    {0x3324,0x02},
    {0x3325,0x02},
    {0x3326,0x00},
    {0x3333,0x30},
    {0x3334,0x40},
    {0x333d,0x08},
    {0x3341,0x07},
    {0x3343,0x03},
    {0x3364,0x1d},
    {0x3366,0xc0},
    {0x3367,0x08},
    {0x3368,0x04},
    {0x3369,0x00},
    {0x336a,0x00},
    {0x336b,0x00},
    {0x336c,0x42},
    {0x337f,0x03},
    {0x3380,0x1b},
    {0x33aa,0x00},
    {0x33b6,0x07},
    {0x33b7,0x07},
    {0x33b8,0x10},
    {0x33b9,0x10},
    {0x33ba,0x10},
    {0x33bb,0x07},
    {0x33bc,0x07},
    {0x33bd,0x18},
    {0x33be,0x18},
    {0x33bf,0x18},
    {0x33c0,0x05},
    {0x360f,0x05},
    {0x3621,0xac},
    {0x3622,0xf6},
    {0x3623,0x18},
    {0x3624,0x47},
    {0x3625,0x01},
    {0x3630,0xc8},
    {0x3631,0x88},
    {0x3632,0x18},
    {0x3633,0x22},
    {0x3634,0x44},
    {0x3635,0x20},
    {0x3636,0x62},
    {0x3637,0x0c},
    {0x3638,0x24},
    {0x363a,0x83},
    {0x363b,0x08},
    {0x363c,0x05},
    {0x363d,0x05},
    {0x3640,0x00},
    {0x366e,0x04},
    {0x3670,0x6a},
    {0x3671,0xf6},
    {0x3672,0x16},
    {0x3673,0x16},
    {0x3674,0xc8},
    {0x3675,0x54},
    {0x3676,0x18},
    {0x3677,0x22},
    {0x3678,0x53},
    {0x3679,0x55},
    {0x367a,0x40},
    {0x367b,0x40},
    {0x367c,0x40},
    {0x367d,0x58},
    {0x367e,0x40},
    {0x367f,0x58},
    {0x3693,0x20},
    {0x3694,0x40},
    {0x3695,0x40},
    {0x3696,0x83},
    {0x3697,0x87},
    {0x3698,0x9f},
    {0x369e,0x40},
    {0x369f,0x40},
    {0x36a0,0x58},
    {0x36a1,0x78},
    {0x36ea,0x35},
    {0x36eb,0x0e},
    {0x36ec,0x1e},
    {0x36ed,0x03},
    {0x36fa,0xa8},
    {0x36fb,0x30},
    {0x3802,0x00},
    {0x3901,0x02},
    {0x3907,0x01},
    {0x3908,0x01},
    {0x391d,0x24},
    {0x391e,0x00},
    {0x391f,0xc0},
    {0x3933,0x28},
    {0x3934,0x0a},
    {0x3940,0x1b},
    {0x3941,0x40},
    {0x3942,0x08},
    {0x3943,0x0e},
    {0x3e00,0x00},
    {0x3e01,0x8c}, // {16'h3e01[7:0], 16'h3e02[7:4]} long exposure time (half lines)
    {0x3e02,0x00},
    //{0x3e01,0x0c}, // {16'h3e01[7:0], 16'h3e02[7:4]} long exposure time (half lines)
    //{0x3e02,0x80},
    //{0x3e01,0x04}, // {16'h3e01[7:0], 16'h3e02[7:4]} long exposure time (half lines)
    //{0x3e02,0x60},
    //{0x3e01,0x02}, // {16'h3e01[7:0], 16'h3e02[7:4]} long exposure time (half lines)
    //{0x3e02,0x30},
    {0x3e03,0x0b},
    //{0x3e04,0x05}, // {16'h 3e04[7:0], 16'h3e05[7:4]} shot exposure time
    //{0x3e05,0x00},
    //{0x3e04,0x08}, // {16'h 3e04[7:0], 16'h3e05[7:4]} shot exposure time
    //{0x3e05,0x00},
    {0x3e04,0x04}, // {16'h 3e04[7:0], 16'h3e05[7:4]} shot exposure time
    {0x3e05,0x00},
    {0x3e14,0xb0}, // 0x3e14[0]: 1 seperate control long gain and short gain, 0 with same gain
    //{0x3e06,0x01}, // 2x long digital gain
    //{0x3e07,0x80},
    //{0x3e08,0x03}, // 1x long analog gain
    //{0x3e09,0x40},
    {0x3e08,0x23}, // 4x
    {0x3e09,0x5f},
    //{0x3e08,0x27}, // 8x
    //{0x3e09,0x5e},
    //{0x3e08,0x2f}, // 16x
    //{0x3e09,0x5e},
    //{0x3e08,0x3f}, // 32x
    //{0x3e09,0x5e},
    {0x3e0e,0x66},
    //{0x3e12,0x03}, // 1x short analog gain
    //{0x3e13,0x40},
    //{0x3e12,0x23}, // 4x short analog gain
    //{0x3e13,0x5f},
    //{0x3e12,0x27}, // 8x short analog gain
    //{0x3e13,0x5e},
    //{0x3e12,0x27}, // 10x short analog gain
    //{0x3e13,0x75},
    //{0x3e12,0x2f}, // 16x short analog gain
    //{0x3e13,0x5e},
    //{0x3e10,0x00},
    //{0x3e11,0x80}, // 1x short digital gain

    {0x3e1e,0x35},
    {0x3e23,0x00}, // {16'h3e23, 16'h3e24} max short exposure
    {0x3e24,0x80},
    {0x3e25,0x03},
    {0x3e26,0x40},
    {0x3f00,0x0d},
    {0x3f04,0x02},
    {0x3f05,0x1e},
    {0x3f08,0x04},
    {0x4500,0x59},
    {0x4501,0xa4},
    {0x4509,0x10},
    {0x4602,0x0f},
    {0x4603,0x00},
    {0x4809,0x01},
    {0x4816,0x51},
    {0x4837,0x32},
    {0x5000,0x06},
    {0x5780,0x7f},
    {0x5781,0x06},
    {0x5782,0x04},
    {0x5783,0x02},
    {0x5784,0x01},
    {0x5785,0x16},
    {0x5786,0x12},
    {0x5787,0x08},
    {0x5788,0x02},
    {0x57a0,0x00},
    {0x57a1,0x74},
    {0x57a2,0x01},
    {0x57a3,0xf4},
    {0x57a4,0xf0},
    {0x6000,0x06},
    {0x6002,0x06},
    {0x36e9,0x03},
    {0x36f9,0x01},
    // {0x0100,0x01},
};
#endif


static void sc2310_hdr_busy_delay(int32_t ms)
{
    aiva_msleep(ms);
}

static int sc2310_hdr_write_reg(int i2c_num, uint16_t reg_addr, uint8_t reg_val)
{
    int ret;
    uint8_t data_buf[3];
    data_buf[0] = (reg_addr >> 8) & 0xff;
    data_buf[1] = (reg_addr >> 0) & 0xff;
    data_buf[2] = reg_val;
    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, data_buf, 3);

    return ret;
}

static int sc2310_hdr_read_reg(int i2c_num, uint16_t reg_addr, uint8_t *reg_val)
{
    int ret;

    uint8_t addr_buf[2];

    addr_buf[0] = (reg_addr >> 8) & 0xff;
    addr_buf[1] = (reg_addr >> 0) & 0xff;
    
    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, &addr_buf[0], 2);
    /*ret = i2c_send_data_dma(DMAC_CHANNEL0, i2c_num, addr_buf, 2);*/
    if (ret < 0) {
        return ret;
    }

    ret = i2c_recv_data(i2c_num, SENSOR_ADDR_WR, 0, 0, reg_val, 1);
    /*ret = i2c_recv_data_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, i2c_num, 0, 0, reg_val, 1);*/
    
    return ret;
}


static int sc2310_hdr_program_regs(
        int             i2c_num,
        const SC2310_REG_T    *reg_list,
        int             cnt
        )
{
    int i;
    int ret = 0;
    for (i = 0; i < cnt; i++) {
        if (reg_list[i].reg_addr != REG_DLY) {
            ret = sc2310_hdr_write_reg(i2c_num, reg_list[i].reg_addr, reg_list[i].data);
            if (ret < 0) {
                LOGE(__func__, "error: i = %d, reg_addr 0x%x, data 0x%x.",
                        i, reg_list[i].reg_addr, reg_list[i].data);
                break;
            }
        } else {
            sc2310_hdr_busy_delay(reg_list[i].data);
        }
    }
    return ret;
}

static int sc2310_hdr_i2c_test(int i2c_num)
{
    uint8_t id_hi, id_lo;

    int ret = sc2310_hdr_read_reg(i2c_num, SC2310_REG_CHIP_ID, &id_hi);
    if (ret != 0) {
        LOGE(TAG, "read sensor id error\n");
        return ret;
    }
    ret = sc2310_hdr_read_reg(i2c_num, SC2310_REG_CHIP_ID+1, &id_lo);
    if (ret != 0) {
        LOGE(TAG, "read sensor id error\n");
        return ret;
    }
    LOGD(TAG, "read sensor id: id_hi=0x%x, id_lo=0x%x.", id_hi, id_lo);
    uint16_t id = (id_hi << 8) | id_lo;
    if (id != CHIP_ID) {
        LOGE(TAG, "read sensor id error, id=0x%x, expected: 0x2311\n", id);
        return -1;
    }
    return 0;
}



static int sc2310_hdr_init_1080p(int i2c_num)
{
    int ret;
    uint8_t tmp;

    //ret = sc2310_hdr_program_regs(i2c_num, sc2310_hdr_1080p_mipi_1lane, AIVA_ARRAY_LEN(sc2310_hdr_1080p_mipi_1lane));
    ret = sc2310_hdr_program_regs(i2c_num, sc2310_hdr_1080p_mipi_2lane, AIVA_ARRAY_LEN(sc2310_hdr_1080p_mipi_2lane));
    if (ret != 0) {
        LOGE(TAG, "sc2310_hdr program regs fail\n");
    }
    sc2310_hdr_read_reg(i2c_num, 0x3634, &tmp);
    LOGD(TAG, "reg 0x3634: %x", tmp);
    return ret;
}


static int cis_sc2310_hdr_init(cis_dev_driver_t *dev_driver)
{
    int i2c_num = dev_driver->i2c_num;

    // enable sensor mclk
    if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK0) {
        sysctl_set_sens_mclk(MCLK_ID0, 24*1000*1000);
        sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);
    } else if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK1) {
        sysctl_set_sens_mclk(MCLK_ID1, 24*1000*1000);
        sysctl_set_io_switch(IO_SWITCH_MCLK1, 1);
    } else {
        LOGE(TAG, "invalid mclk id: %d\n", dev_driver->mclk_id);
    }

    i2c_init(i2c_num, SENSOR_ADDR_WR, 7, 50*1000);

    if (sc2310_hdr_i2c_test(i2c_num) < 0) {
        LOGE(TAG, "sc2310_hdr i2c test fail\n");
        return -1;
    }

    if (sc2310_hdr_init_1080p(dev_driver->i2c_num) != 0) {
        LOGE(TAG, "sc2310_hdr_init_1080p fail\n");
        return -1;
    }
    return 0;
}

static int cis_sc2310_hdr_start_stream(cis_dev_driver_t *dev_driver, const cis_config_t *config)
{
    int ret;
    SC2310_REG_T       *reg_list;
    int                 cnt;

    reg_list = sc2310_hdr_start_regs;
    cnt     = AIVA_ARRAY_LEN(sc2310_hdr_start_regs);
    ret     = sc2310_hdr_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "sc2310_hdr_start failed!");
    }
    return ret;
}


static int cis_sc2310_hdr_stop_stream(cis_dev_driver_t *dev_driver)
{
    int ret;
    SC2310_REG_T       *reg_list;
    int                 cnt;

    reg_list = sc2310_hdr_stop_regs;
    cnt     = AIVA_ARRAY_LEN(sc2310_hdr_stop_regs);
    ret     = sc2310_hdr_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0) {
        LOGE(TAG, "sc2310_hdr_stop failed!");
    }
    return ret;
}


static void cis_sc2310_hdr_power_on(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    sc2310_hdr_busy_delay(3);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    sc2310_hdr_busy_delay(5);
    /*return 0;*/
}


static void cis_sc2310_hdr_power_off(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    sc2310_hdr_busy_delay(5);
    /*return 0;*/
}


static void cis_sc2310_hdr_reset(cis_dev_driver_t *dev_driver)
{
    /*return 0;*/
}


static int cis_sc2310_hdr_get_interface_param(cis_dev_driver_t *dev_driver, cis_interface_param_t *param)
{
    param->interface_type                   = CIS_INTERFACE_TYPE_MIPI;
#if USE_30FPS
    param->mipi_param.freq                  = 760;
#else
    param->mipi_param.freq                  = 396;
#endif
    param->mipi_param.lane_num              = 2;
    param->mipi_param.vc_num                = 2;
    param->mipi_param.virtual_channels[0]   = MIPI_VIRTUAL_CHANNEL_0;
    param->mipi_param.virtual_channels[1]   = MIPI_VIRTUAL_CHANNEL_1;
    param->mipi_param.data_type             = MIPI_DATA_TYPE_RAW_10;
    return 0;
}


static float map_regvalue_to_time(int lines)
{
    float mipi_data_rate = (396 / 2) * 1000000; // 396Mbps /2 for 2 frame(long and short)
    float mipi_lane_num  = 2;                   // 2 lane
    float mipi_data_bit  = 10;                  // raw 10
    int   line_length    = ((0x04 << 8) | (0x4c)) * 2;
    float pclk           = mipi_data_rate * mipi_lane_num / mipi_data_bit;
    float T_pclk         = 1/pclk;
    float T_line         = line_length * T_pclk;
    float itime          = lines * T_line;

    return itime;
}

static int cis_sc2310_hdr_get_exposure_param(cis_dev_driver_t *dev_driver, cis_exposure_param_t *exp_param)
{
    exp_param->min_again_hdr[0]     = 1.0;
    exp_param->max_again_hdr[0]     = 31.0;
    exp_param->step_again_hdr[0]    = 0.5;
    exp_param->min_dgain_hdr[0]     = 1.0;
    exp_param->max_dgain_hdr[0]     = 8.0;
    exp_param->step_dgain_hdr[0]    = 0.5;
    exp_param->min_itime_hdr[0]     = map_regvalue_to_time(32);
    exp_param->max_itime_hdr[0]     = map_regvalue_to_time(2200);
    exp_param->step_itime_hdr[0]    = map_regvalue_to_time(1);
    exp_param->min_again_hdr[1]     = 1.0;
    exp_param->max_again_hdr[1]     = 31.0;
    exp_param->step_again_hdr[1]    = 0.5;
    exp_param->min_dgain_hdr[1]     = 1.0;
    exp_param->max_dgain_hdr[1]     = 8.0;
    exp_param->step_dgain_hdr[1]    = 0.5;
    exp_param->min_itime_hdr[1]     = map_regvalue_to_time(32);
    exp_param->max_itime_hdr[1]     = map_regvalue_to_time(128);
    exp_param->step_itime_hdr[1]    = map_regvalue_to_time(1);

    LOGI(__func__, "exposure param: min_itime_hdr[0]=%f, max_itime_hdr[0]=%f",
            exp_param->min_itime_hdr[0], exp_param->max_itime_hdr[0]);
    LOGI(__func__, "exposure param: min_itimev[1]=%f, max_itime_hdr[1]=%f",
            exp_param->min_itime_hdr[1], exp_param->max_itime_hdr[1]);
    return 0;
}


static int cis_sc2310_hdr_split_again(float again, uint8_t *again_coarse, uint8_t *again_fine)
{
    int idx = 0;

    idx = (int)round((again - 1.0) / 0.5);
    if (idx >= sizeof(sc2310_again_table)/sizeof(SC2310_GAIN_T)) {
        LOGE(__func__, "analog gain out of range: again=%f, idx=%d", again, idx);
        *again_coarse = 0x00;
        *again_fine = 0x00;
        return -1;
    }
    *again_coarse = sc2310_again_table[idx].coarse;
    *again_fine   = sc2310_again_table[idx].fine;
    return 0;
}



static int cis_sc2310_hdr_split_dgain(float dgain, uint8_t *dgain_coarse, uint8_t *dgain_fine)
{
    int idx = 0;

    idx = (int)round((dgain - 1.0) / 0.5);
    if (idx >= sizeof(sc2310_dgain_table)/sizeof(SC2310_GAIN_T)) {
        LOGE(__func__, "digital gain out of range: dgain=%f, idx=%d", dgain, idx);
        *dgain_coarse = 0x00;
        *dgain_fine = 0x00;
        return -1;
    }
    *dgain_coarse = sc2310_dgain_table[idx].coarse;
    *dgain_fine   = sc2310_dgain_table[idx].fine;
    return 0;
}



static int cis_sc2310_hdr_mapping_itime(float itime, uint8_t *itime_h, uint8_t *itime_l)
{
    int itime_i = 0;

    if (itime > 10.0) {
        // for itime > 10.0, we suppose it is represet in lines
        itime_i = itime;
    } else {
        // for itime <= 10.0l we suppose it is represet in seconds
        float mipi_data_rate = (396 / 2) * 1000000;       // 396Mbps
        float mipi_lane_num  = 2;                   // 2 lane
        float mipi_data_bit  = 10;                  // raw 10
        int   line_length    = ((0x04 << 8) | (0x4c)) * 2;
        float pclk           = mipi_data_rate * mipi_lane_num / mipi_data_bit;
        float T_pclk         = 1/pclk;
        float T_line         = line_length * T_pclk;
        int   itime_lines    = (int)round(itime / T_line);
        // LOGI(__func__, "line_length=%d, pclk=%f, T_line=%f, itime_lines=%d", line_length, pclk, T_line, itime_lines);
        itime_i              = itime_lines;
    }
    *itime_h = (itime_i >> 4) & 0xff;
    *itime_l = (itime_i & 0xf) << 4;
    return 0;
}

static int cis_sc2310_hdr_set_exposure(cis_dev_driver_t *dev_driver, const cis_exposure_t *exp)
{
    int i2c_num = dev_driver->i2c_num;
    float again0 = exp->again_hdr[0];
    float dgain0 = exp->dgain_hdr[0];
    float itime0 = exp->itime_hdr[0];
    float again1 = exp->again_hdr[1];
    float dgain1 = exp->dgain_hdr[1];
    float itime1 = exp->itime_hdr[1];
    uint8_t itime_h;
    uint8_t itime_l;
    uint8_t again_coarse;
    uint8_t again_fine;
    uint8_t dgain_coarse;
    uint8_t dgain_fine;

    i2c_init(i2c_num, SENSOR_ADDR_WR, 7, 350*1000);

    uint8_t reg_itime_l0;
    uint8_t reg_itime_l1;
    sc2310_hdr_read_reg(i2c_num, 0x3e02, &reg_itime_l0);
    sc2310_hdr_read_reg(i2c_num, 0x3e05, &reg_itime_l1);



    //-- group hold start
    //sc2310_hdr_write_reg(i2c_num, 0x3812, 0x00);

    //--itime
    if (cis_sc2310_hdr_mapping_itime(itime0, &itime_h, &itime_l) != 0) {
        LOGE(__func__, "mapping itime fail");
        return -1;
    }
    itime_l = itime_l | (reg_itime_l0 & 0x0f);
    sc2310_hdr_write_reg(i2c_num, 0x3e01, itime_h);
    sc2310_hdr_write_reg(i2c_num, 0x3e02, itime_l);

// TODO delete these log after hdr function ok
#ifndef USE_FACELOCK_V2 // disable log for facelock_v2
    LOGI(__func__, "set 0x3e01: 0x%x", itime_h);
    LOGI(__func__, "set 0x3e02: 0x%x", itime_l);
#endif

    if (cis_sc2310_hdr_mapping_itime(itime1, &itime_h, &itime_l) != 0) {
        LOGE(__func__, "mapping itime fail");
        return -1;
    }
    itime_l = itime_l | (reg_itime_l0 & 0x0f);
    sc2310_hdr_write_reg(i2c_num, 0x3e04, itime_h);
    sc2310_hdr_write_reg(i2c_num, 0x3e05, itime_l);

// TODO delete these log after hdr function ok
#ifndef USE_FACELOCK_V2 // disable log for facelock_v2
    LOGI(__func__, "set 0x3e04: 0x%x", itime_h);
    LOGI(__func__, "set 0x3e05: 0x%x", itime_l);
#endif

    //--again
    if (cis_sc2310_hdr_split_again(again0, &again_coarse, &again_fine) != 0) {
        LOGE(__func__, "split again fail");
        return -1;
    }
    sc2310_hdr_write_reg(i2c_num, 0x3e08, again_coarse);
    sc2310_hdr_write_reg(i2c_num, 0x3e09, again_fine);

    if (cis_sc2310_hdr_split_again(again1, &again_coarse, &again_fine) != 0) {
        LOGE(__func__, "split again fail");
        return -1;
    }
    sc2310_hdr_write_reg(i2c_num, 0x3e12, again_coarse);
    sc2310_hdr_write_reg(i2c_num, 0x3e13, again_fine);

    //--dgain
    if (cis_sc2310_hdr_split_dgain(dgain0, &dgain_coarse, &dgain_fine) != 0) {
        LOGE(__func__, "split again fail");
        return -1;
    }
    sc2310_hdr_write_reg(i2c_num, 0x3e06, dgain_coarse);
    sc2310_hdr_write_reg(i2c_num, 0x3e07, dgain_fine);

    if (cis_sc2310_hdr_split_dgain(dgain1, &dgain_coarse, &dgain_fine) != 0) {
        LOGE(__func__, "split again fail");
        return -1;
    }
    sc2310_hdr_write_reg(i2c_num, 0x3e10, dgain_coarse);
    sc2310_hdr_write_reg(i2c_num, 0x3e11, dgain_fine);

    //-- group hold end
    //sc2310_hdr_write_reg(i2c_num, 0x3812, 0x30);

// TODO delete these log after hdr function ok
#ifndef USE_FACELOCK_V2 // disable log for facelock_v2
    uint8_t regVal;
    sc2310_hdr_read_reg(i2c_num, 0x3e00, &regVal);
    LOGI(__func__, "0x3e00=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e01, &regVal);
    LOGI(__func__, "0x3e01=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e02, &regVal);
    LOGI(__func__, "0x3e02=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e03, &regVal);
    LOGI(__func__, "0x3e03=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e04, &regVal);
    LOGI(__func__, "0x3e04=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e05, &regVal);
    LOGI(__func__, "0x3e05=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e06, &regVal);
    LOGI(__func__, "0x3e06=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e07, &regVal);
    LOGI(__func__, "0x3e07=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e08, &regVal);
    LOGI(__func__, "0x3e08=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e09, &regVal);
    LOGI(__func__, "0x3e09=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e10, &regVal);
    LOGI(__func__, "0x3e10=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e11, &regVal);
    LOGI(__func__, "0x3e11=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e12, &regVal);
    LOGI(__func__, "0x3e12=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e13, &regVal);
    LOGI(__func__, "0x3e13=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e23, &regVal);
    LOGI(__func__, "0x3e23=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x3e24, &regVal);
    LOGI(__func__, "0x3e24=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x320e, &regVal);
    LOGI(__func__, "0x320e=0x%x", regVal);
    sc2310_hdr_read_reg(i2c_num, 0x320f, &regVal);
    LOGI(__func__, "0x320f=0x%x", regVal);
#endif
    return 0;
}

static int cis_sc2310_hdr_get_frame_parameter(cis_dev_driver_t *dev_driver, cis_frame_param_t *param)
{
    param->width = 1920;
    param->height = 1080;
    param->framerate = 30;
    param->format = CIS_FORMAT_RGB_BAYER10;
    param->bayer_pat = CIS_BAYER_PAT_BGGR; // after vflip

    return 0;
}

static int cis_sc2310_hdr_get_vcm_param(cis_dev_driver_t *dev_driver, cis_vcm_param_t *param)
{
    param->has_vcm_motor = 0;
    param->min_vcm_pos   = 0.0;
    param->max_vcm_pos   = 0.0;
    param->min_vcm_step  = 0.0;
    return 0;
}


static cis_dev_driver_t sc2310_hdr_dev0 = {
    .name                   = "sc2310_hdr_dev0",
    .i2c_num                = I2C_DEVICE_0,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .context                = NULL,
    .init                   = cis_sc2310_hdr_init,
    .start_stream           = cis_sc2310_hdr_start_stream,
    .stop_stream            = cis_sc2310_hdr_stop_stream,
    .power_on               = cis_sc2310_hdr_power_on,
    .power_off              = cis_sc2310_hdr_power_off,
    .reset                  = cis_sc2310_hdr_reset,
    .get_interface_param    = cis_sc2310_hdr_get_interface_param,
    .get_exposure_param     = cis_sc2310_hdr_get_exposure_param,
    .get_vcm_param          = cis_sc2310_hdr_get_vcm_param,
    .set_exposure           = cis_sc2310_hdr_set_exposure,
    .get_frame_parameter         = cis_sc2310_hdr_get_frame_parameter,
};


static cis_dev_driver_t sc2310_hdr_dev1 = {
    .name                   = "sc2310_hdr_dev1",
    .i2c_num                = I2C_DEVICE_1,
    .power_pin              = GPIO_PIN1,
    .reset_pin              = GPIO_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .context                = NULL,
    .init                   = cis_sc2310_hdr_init,
    .start_stream           = cis_sc2310_hdr_start_stream,
    .stop_stream            = cis_sc2310_hdr_stop_stream,
    .power_on               = cis_sc2310_hdr_power_on,
    .power_off              = cis_sc2310_hdr_power_off,
    .reset                  = cis_sc2310_hdr_reset,
    .get_interface_param    = cis_sc2310_hdr_get_interface_param,
    .get_exposure_param     = cis_sc2310_hdr_get_exposure_param,
    .get_vcm_param          = cis_sc2310_hdr_get_vcm_param,
    .set_exposure           = cis_sc2310_hdr_set_exposure,
    .get_frame_parameter         = cis_sc2310_hdr_get_frame_parameter,
};



static cis_dev_driver_t sc2310_hdr_dev2 = {
    .name                   = "sc2310_hdr_dev2",
    .i2c_num                = I2C_DEVICE_2,
    .power_pin              = GPIO_PIN1,
    .reset_pin              = GPIO_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .context                = NULL,
    .init                   = cis_sc2310_hdr_init,
    .start_stream           = cis_sc2310_hdr_start_stream,
    .stop_stream            = cis_sc2310_hdr_stop_stream,
    .power_on               = cis_sc2310_hdr_power_on,
    .power_off              = cis_sc2310_hdr_power_off,
    .reset                  = cis_sc2310_hdr_reset,
    .get_interface_param    = cis_sc2310_hdr_get_interface_param,
    .get_exposure_param     = cis_sc2310_hdr_get_exposure_param,
    .get_vcm_param          = cis_sc2310_hdr_get_vcm_param,
    .set_exposure           = cis_sc2310_hdr_set_exposure,
    .get_frame_parameter         = cis_sc2310_hdr_get_frame_parameter,
};



cis_dev_driver_t *cis_sc2310_hdr[] = {
    &sc2310_hdr_dev0,
    &sc2310_hdr_dev1,
    &sc2310_hdr_dev2
};



int sc2310_hdr_read(int i2c, uint16_t addr)
{
    uint8_t data;
    i2c_init(i2c, SENSOR_ADDR_WR, 7, 50*1000);
    sc2310_hdr_read_reg(i2c, addr, &data);
    LOGD(__func__, "addr 0x%x, data 0x%x", addr, data);
    return 0;
}


int sc2310_hdr_write(int i2c, uint16_t addr, uint8_t value)
{
    uint8_t data;
    i2c_init(i2c, SENSOR_ADDR_WR, 7, 50*1000);
    sc2310_hdr_write_reg(i2c, addr, value);
    LOGD(__func__, "addr 0x%x, data 0x%x", addr, value);
    return 0;
}

/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     sc2310_hdr_dev0,    0,      &sc2310_hdr_dev0,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     sc2310_hdr_dev1,    0,      &sc2310_hdr_dev1,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     sc2310_hdr_dev2,    0,      &sc2310_hdr_dev2,    NULL);
