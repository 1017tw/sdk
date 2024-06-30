#include "cis_dev_driver.h"
#include "aiva_utils.h"
#include "aiva_sleep.h"
#include "sysctl.h"
#include "i2c.h"
#include "gpio.h"
#include "syslog.h"
#include "udevice.h"
#include <math.h>
#include <string.h>
#include <stdbool.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define DEFAULT_AGAIN            (8.0f)     // 8x
#define DEFAULT_DGAIN            (1.0f)     // 1x
#define DEFAULT_EXPOSURE         (1000)     // 1000 lines
#define MAX_ITIME_MS             (120)
#define DEFAULT_SLEEP_DELAY_MS   (80)       // this delay should be larger than itime

#define REG_DLY                  (0xff)
// #define SENSOR_ADDR_WR           (0x7a >> 1)
#define SENSOR_ADDR_WR           0x3D

typedef struct sensor_reg {
    uint8_t        reg_addr;
    uint8_t         data;
} sensor_reg_t;

typedef struct cis_dev_params {
    int width;
    int height;
} cis_dev_params_t;

static cis_dev_params_t m_big_resolution = {
    .width = 1600,
    .height = 1200,
    // .width = 800,
    // .height = 600,
};

static cis_dev_params_t m_small_resolution = {
    // .width = 1600,
    // .height = 1200,
    .width = 800,
    .height = 600,
};

static const char* TAG = "cis_ov02b";

static sensor_reg_t ov02b_start_regs[] = {
    {0xfd, 0x03},
    {0xc2, 0x01},
};

static sensor_reg_t ov02b_stop_regs[] = {
    {0xfd, 0x03},
    {0xc2, 0x00},
};

static const sensor_reg_t ov02b_master[] = {
    {0xfd, 0x00},
    {0x36, 0x00},
    {0x37, 0x03},
    {0xfd, 0x01},
    {0x92, 0x10},
    {0x93, 0x01},
    {0x90, 0x1c},
};

static const sensor_reg_t ov02b_slave[] = {
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

static const sensor_reg_t ov02b_sleep[] = {
    {0xfd, 0x03},
    {0xc2, 0x00},
    /*{REG_DLY, 0x64},*/
    {REG_DLY, DEFAULT_SLEEP_DELAY_MS},
    {0xfd, 0x00},
    {0x1f, 0x06},
};

static const sensor_reg_t ov02b_sleep_rgb[] = {
    {0xfd, 0x03},
    {0xc2, 0x00},
    /*{REG_DLY, 0x64},*/
    {REG_DLY, MAX_ITIME_MS+3}, // +3 to make sure delay more than one frame
    {0xfd, 0x00},
    {0x1f, 0x06},
};

static const sensor_reg_t ov02b_wake[] = {
    {0xfd, 0x00},
    {0x1f, 0x00},
    {0xfd, 0x03},
    {0xc2, 0x01},
};

static const sensor_reg_t ov02b_800x600_head[] = {
    {0xfc, 0x01},   //soft reset
    {0xfd, 0x00},
    {0xfd, 0x00},
    {0x24, 0x02},   //pll_mc
    {0x25, 0x06},   //pll_nc,dpll clk 72M
    {0x29, 0x03},
    {0x2a, 0xb4},   //mpll_nc, mpll clk 330M
    {0x1e, 0x17},   //vlow 0.53v
    {0x33, 0x07},   //ipx 2.84u
    {0x35, 0x07},   //pcp off
    {0x4a, 0x0c},   //ncp -1.4v
    {0x3a, 0x05},   //icomp1 4.25u
    {0x3b, 0x02},   //icomp2 1.18u
    {0x3e, 0x00},
    {0x46, 0x01},
    {0xfd, 0x01},
    // {0x0e, 0x02},
    // {0x0f, 0x1a},   //exp
    {0x0e, DEFAULT_EXPOSURE >> 8},
    {0x0f, DEFAULT_EXPOSURE & 0xff},
    {0x18, 0x00},   //un fixed-fps
    // {0x22, 0xff},   //analog gain
    {0x22, (uint8_t)(DEFAULT_AGAIN * 16 - 1) & 0xff}, // A-Gain, 0x10 - 1x
    {0x23, 0x02},   //adc_range 0.595v
    // {0x14, 0x0e},   // vblank
    // {0x15, 0x52},   // vblank
    {0x17, 0x2c},   //pd reset row address time
    {0x19, 0x20},   //dac_d0 1024
    {0x1b, 0x06},   //rst_num1 96
    {0x1c, 0x04},   //rst_num2 64
    {0x20, 0x03},
    {0x30, 0x01},   //p0
    {0x33, 0x01},   //p3
    {0x31, 0x0a},   //p1
    {0x32, 0x09},   //p2
    {0x38, 0x01},
    {0x39, 0x01},   //p9
    {0x3a, 0x01},   //p10
    {0x3b, 0x01},
    {0x4f, 0x04},   //p24
    {0x4e, 0x05},   //p23
    {0x50, 0x01},   //p25
    {0x35, 0x0c},   //p5
    {0x45, 0x2a},   //sc1,p20_1
    {0x46, 0x2a},   //p20_2
    {0x47, 0x2a},   //p20_3
    {0x48, 0x2a},   //p20_4
    {0x4a, 0x2c},   //sc2,p22_1
    {0x4b, 0x2c},   //p22_2
    {0x4c, 0x2c},   //p22_3
    {0x4d, 0x2c},   //p22_4
    {0x56, 0x3a},   //p31, 1st d0
    {0x57, 0x0a},   //p32, 1st d1
    {0x58, 0x24},   //col_en1
    {0x59, 0x20},   //p34 2nd d0
    {0x5a, 0x0a},   //p34 2nd d1
    {0x5b, 0xff},   //col_en2
    {0x37, 0x0a},   //p7, tx
    {0x42, 0x0e},   //p17, psw
    {0x68, 0x90},
    {0x69, 0xcd},   //blk en, no sig_clamp
    {0x7c, 0x08},
    {0x7d, 0x08},
    {0x7e, 0x08},
    {0x7f, 0x08},   //vbl1_4
    {0x83, 0x14},
    {0x84, 0x14},
    {0x86, 0x14},
    {0x87, 0x07},   //vbl2_4
    {0x88, 0x0f},
    {0x94, 0x02},   //evsync del frame
    {0x98, 0xd1},   //del bad frame
    {0xfe, 0x02},
    {0xfd, 0x03},   //RegPage
    {0x97, 0x6c},
    {0x98, 0x60},
    {0x99, 0x60},
    {0x9a, 0x6c},
    {0xae, 0x0d},   //bit0=1,high 8bit
    {0x88, 0x49},   //BLC_ABL
    {0x89, 0x7c},   //bit6=1 trigger en
    {0xb4, 0x05},   //mean trigger 5
    {0xbd, 0x0d},   //blc_rpc_coe
    {0x8c, 0x40},   //BLC_BLUE_SUBOFFSET_8lsb
    {0x8e, 0x40},   //BLC_RED_SUBOFFSET_8lsb
    {0x90, 0x40},   //BLC_GR_SUBOFFSET_8lsb
    {0x92, 0x40},   //BLC_GB_SUBOFFSET_8lsb
    // {0x9b, 0x49},   //digtal gain
    {0x9b, (uint8_t)(DEFAULT_DGAIN * 64 - 1) & 0xff}, // D-Gain, 0x40 - 1x
    {0xac, 0x40},   //blc random noise rpc_th 4x
    {0xfd, 0x00},
    {0x5a, 0x15},
    {0x74, 0x01},   //PD_MIPI:turn on mipi phy

    {0xfd, 0x00},   //binning 800x600
    {0x28, 0x03},
    {0x4f, 0x03},   //mipi size
    {0x50, 0x20},
    {0x51, 0x02},
    {0x52, 0x58},
};

static const sensor_reg_t ov02b_800x600_15fps[] = {
    {0xfd, 0x01},
    {0x14, 0x0e},   // vblank
    {0x15, 0x52},   // vblank
};

static const sensor_reg_t ov02b_800x600_tail[] = {
    {0xfd, 0x01},

    {0x03, 0x70},  //h-start
    {0x05, 0x10},  //v-start
    {0x07, 0x20},
    {0x09, 0xb0},

    // strobe setting
#ifdef USE_SENSOR_OV02B_STROBE
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
    {0x6c, 0x09},  //binning22 en
    {0xfe, 0x02},
    {0xfb, 0x01},
};
//1600x1200: mclk = 24M, mipi data rate = 660M bps, raw10
static const sensor_reg_t ov02b_1600x1200_head[] = {
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
};

static const sensor_reg_t ov02b_1600x1200_15fps[] = {
    {0xfd, 0x01},
    {0x14, 0x04},
    {0x15, 0xc6},
};

static const sensor_reg_t ov02b_1600x1200_tail[] = {
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
};

// mirror and flip mode
static const sensor_reg_t ov02b_mf_none[] = {
    {0xfd, 0x01},
    {0x12, 0x00},
};

static const sensor_reg_t ov02b_mf_flip[] = {
    {0xfd, 0x01},
    {0x12, 0x01},
};

static const sensor_reg_t ov02b_mf_mirror[] = {
    {0xfd, 0x01},
    {0x12, 0x02},
};

static const sensor_reg_t ov02b_mf_mirror_and_flip[] = {
    {0xfd, 0x01},
    {0x12, 0x03},
};

static bool is_small_resolution(cis_dev_params_t *param)
{
    return param->width == 800 && param->height == 600;
}

static int ov02b_write_reg(i2c_device_number_t i2c_num, uint32_t i2c_tar_addr, uint16_t reg_addr, uint8_t reg_val)
{
    int ret;
    uint8_t data_buf[2];
    data_buf[0] = reg_addr;
    data_buf[1] = reg_val;
    ret = i2c_send_data(i2c_num, i2c_tar_addr, data_buf, 2);

    return ret;
}

static int ov02b_read_reg(i2c_device_number_t i2c_num, uint32_t i2c_tar_addr, uint16_t reg_addr, uint8_t *reg_val)
{
    int ret;

    ret = i2c_send_data(i2c_num, i2c_tar_addr, (const uint8_t *)&reg_addr, 1);
    if (ret < 0) {
        return ret;
    }
    ret = i2c_recv_data(i2c_num, i2c_tar_addr, 0, 0, reg_val, 1);

    return ret;
}


static int ov02b_program_regs(
        i2c_device_number_t i2c_num,
		uint32_t 		i2c_tar_addr,
        const sensor_reg_t    *reg_list,
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
            aiva_msleep(reg_list[i].data);
        }
    }
    return ret;
}

static void cis_ov02b_enable_mclk(cis_dev_driver_t *dev_driver)
{
    if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK0) {
        sysctl_set_sens_mclk(MCLK_ID0, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);
    } else if (dev_driver->mclk_id == CIS_MCLK_ID_MCLK1) {
        sysctl_set_sens_mclk(MCLK_ID1, dev_driver->mclk_freq);
        sysctl_set_io_switch(IO_SWITCH_MCLK1, 1);
    } else {
        LOGE(TAG, "invalid mclk id: %d\n", dev_driver->mclk_id);
    }
}

static int ov02b_i2c_test(i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    uint8_t pid_hi, pid_lo;

    ov02b_write_reg(i2c_num, i2c_tar_addr, 0xfd, 0x00);
    ov02b_read_reg(i2c_num, i2c_tar_addr, 0x02, &pid_hi);
    ov02b_read_reg(i2c_num, i2c_tar_addr, 0x03, &pid_lo);

    LOGD(TAG, "ov02b_i2c_test: pid is 0x%02x:0x%02x", pid_hi, pid_lo);


    if (pid_lo != 0x2b) {
        LOGE(TAG, "ov02b_i2c_test: i2c_num:%d addr:0x%02x read reg(0x03) fail!, get 0x%x, expected 0x2b", (int)i2c_num, i2c_tar_addr, pid_lo);
        return -1;
    }

    // LOGD(TAG, "ov02b_i2c_test: read reg(0x03) success!");
    return 0;
}


static int ov02b_init(cis_dev_driver_t *dev_driver)
{
    int ret = 0;

    cis_ov02b_enable_mclk(dev_driver);

    i2c_init(dev_driver->i2c_num, dev_driver->i2c_tar_addr, 7, 350*1000);

    if (ov02b_i2c_test(dev_driver->i2c_num, dev_driver->i2c_tar_addr) < 0) {
        LOGE(TAG, "ov02b i2c test fail\n");
        return -1;
    }

    return ret;
}

static int ov02b_800x600_init(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    // int ret;
    if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_800x600_head, AIVA_ARRAY_LEN(ov02b_800x600_head)) < 0) {
        LOGE(__func__, "line %d, ov02b_800x600_head init failed", __LINE__);
        return -1;
    }

    if (fps == 30) {
    }
    else if (fps == 15) {
        if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_800x600_15fps, AIVA_ARRAY_LEN(ov02b_800x600_15fps)) < 0) {
            LOGE(__func__, "line %d, ov02b_800x600_15fps init failed", __LINE__);
            return -1;
        }
    }
    else {
        LOGE(__func__, "line %d, ov02b not support %d fps", __LINE__, fps);
        return -1;
    }

    switch (mf_mode) {
        case 0:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_none, AIVA_ARRAY_LEN(ov02b_mf_none)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_none init failed", __LINE__);
                return -1;
            }
            break;
        case 1:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_flip, AIVA_ARRAY_LEN(ov02b_mf_flip)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_flip init failed", __LINE__);
                return -1;
            }
            break;
        case 2:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_mirror, AIVA_ARRAY_LEN(ov02b_mf_mirror)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_mirror init failed", __LINE__);
                return -1;
            }
            break;
        case 3:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_mirror_and_flip, AIVA_ARRAY_LEN(ov02b_mf_mirror_and_flip)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_mirror_and_flip init failed", __LINE__);
                return -1;
            }
            break;
        default:
            LOGE(__func__, "line %d, ov02b not support %d mirror and flip mode", __LINE__, mf_mode);
            return -1;
            break;
    }
    if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_800x600_tail, AIVA_ARRAY_LEN(ov02b_800x600_tail)) < 0) {
        LOGE(__func__, "line %d, ov02b_800x600_tail init failed", __LINE__);
        return -1;
    }

    return 0;
}

static int ov02b_800x600_init_master(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
    if (ov02b_800x600_init(fps, mf_mode, i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_800x600_init failed!", __LINE__);
        return -1;
    }
    ret = ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_master, AIVA_ARRAY_LEN(ov02b_master));
    /*LOGD(TAG, "master return: %d.", ret);*/
    if (ret < 0) {
        LOGE(__func__, "line %d, ov02b_program_regs failed!", __LINE__);
        return ret;
    }
    return 0;
}

static int ov02b_800x600_init_slave(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
    if (ov02b_800x600_init(fps, mf_mode, i2c_num, i2c_tar_addr) < 0) {
        LOGE(__func__, "line %d, ov02b_800x600_init failed!", __LINE__);
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

static int ov02b_1600x1200_init(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    // int ret;
    if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_1600x1200_head, AIVA_ARRAY_LEN(ov02b_1600x1200_head)) < 0) {
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
        if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_1600x1200_15fps, AIVA_ARRAY_LEN(ov02b_1600x1200_15fps)) < 0) {
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
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_none, AIVA_ARRAY_LEN(ov02b_mf_none)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_none init failed", __LINE__);
                return -1;
            }
            break;
        case 1:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_flip, AIVA_ARRAY_LEN(ov02b_mf_flip)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_flip init failed", __LINE__);
                return -1;
            }
            break;
        case 2:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_mirror, AIVA_ARRAY_LEN(ov02b_mf_mirror)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_mirror init failed", __LINE__);
                return -1;
            }
            break;
        case 3:
            if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_mf_mirror_and_flip, AIVA_ARRAY_LEN(ov02b_mf_mirror_and_flip)) < 0) {
                LOGE(__func__, "line %d, ov02b_mf_mirror_and_flip init failed", __LINE__);
                return -1;
            }
            break;
        default:
            LOGE(__func__, "line %d, ov02b not support %d mirror and flip mode", __LINE__, mf_mode);
            return -1;
            break;
    }
    if (ov02b_program_regs(i2c_num, i2c_tar_addr, ov02b_1600x1200_tail, AIVA_ARRAY_LEN(ov02b_1600x1200_tail)) < 0) {
        LOGE(__func__, "line %d, ov02b_1600x1200_tail init failed", __LINE__);
        return -1;
    }
    return 0;
}

static int ov02b_1600x1200_init_master(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
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

static int ov02b_1600x1200_init_slave(int fps, int mf_mode, i2c_device_number_t i2c_num, uint32_t i2c_tar_addr)
{
    int ret;
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
    int ret = 0;

    ov02b_init(dev_driver);

    if (is_small_resolution((cis_dev_params_t*)dev_driver->context)) {
        ret = ov02b_800x600_init(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    } else {
        ret = ov02b_1600x1200_init(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    }

    if (ret != 0) {
        LOGE(TAG, "ov02b_init fail:%d\n", __LINE__);
    }

    return 0;
}

static int cis_ov02b_init_master(cis_dev_driver_t *dev_driver)
{
    int ret = 0;

    ov02b_init(dev_driver);

    if (is_small_resolution((cis_dev_params_t*)dev_driver->context)) {
        ret = ov02b_800x600_init_master(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    } else {
        ret = ov02b_1600x1200_init_master(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    }

    if (ret != 0) {
        LOGE(TAG, "ov02b_init fail:%d\n", __LINE__);
    }

    return 0;
}

static int cis_ov02b_init_slave(cis_dev_driver_t *dev_driver)
{
    int ret = 0;

    ov02b_init(dev_driver);

    if (is_small_resolution((cis_dev_params_t*)dev_driver->context)) {
        ret = ov02b_800x600_init_slave(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    } else {
        ret = ov02b_1600x1200_init_slave(dev_driver->fps, dev_driver->mf_mode, dev_driver->i2c_num, dev_driver->i2c_tar_addr);
    }

    if (ret != 0) {
        LOGE(TAG, "ov02b_init fail:%d\n", __LINE__);
    }

    return ret;
}

static int cis_ov02b_start_stream(cis_dev_driver_t *dev_driver, const cis_config_t *config)
{
    (void)config;
    int ret;
    sensor_reg_t       *reg_list;
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
    sensor_reg_t       *reg_list;
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
    int ret = 0;

    const sensor_reg_t  *reg_list;
    int                 cnt;
    i2c_device_number_t i2c_num = dev_driver->i2c_num;

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

// NOTE: ov02b need to delay some time after sleep, before next wakeup,
// this dealy time should larger than itime.
// for facelock app. we fixed itime to 1000 for IR sensor, so default
// dealy time DEFAULT_SLEEP_DELAY_MS is ok;
// but for rgb sensor, we want better image quality, so we set max itime
// to MAX_ITIME_MS, and for facelock app, rgb will always use "ov02b_dev0"
// so we sleep different time for rgb sensor
static int is_rgb_sensor(const char* dev_name)
{
    static const char* rgb_dev_name = "ov02b_dev0";
    return strncmp(dev_name, rgb_dev_name, strlen(rgb_dev_name)) == 0;
}

static int cis_ov02b_sleep(cis_dev_driver_t *dev_driver)
{
    int ret = 0;

    const sensor_reg_t  *reg_list;
    int                 cnt;
    i2c_device_number_t i2c_num = dev_driver->i2c_num;

    /*LOGW(__func__, "in, i2c %d ... %x", i2c_num, dev_driver);*/

    i2c_init(i2c_num, dev_driver->i2c_tar_addr, 7, 350*1000);

    if (is_rgb_sensor(dev_driver->name)) {
        reg_list = ov02b_sleep_rgb;
        cnt     = AIVA_ARRAY_LEN(ov02b_sleep_rgb);
    } else {
        reg_list = ov02b_sleep;
        cnt     = AIVA_ARRAY_LEN(ov02b_sleep);
    }
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
    aiva_msleep(3);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    aiva_msleep(5);
    return;
}


static void cis_ov02b_power_off(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    aiva_msleep(5);
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
    exp_param->step_again       = 0.1;
    exp_param->min_dgain        = 1.0;
    exp_param->max_dgain        = 4.0;
    exp_param->step_dgain       = 0.5;
    exp_param->min_itime        = 100;
    exp_param->max_itime        = 1000;
    exp_param->step_itime       = 1;
    exp_param->initial_again    = 8.0;
    exp_param->initial_dgain    = 1.0;
    exp_param->initial_itime    = 1000;

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
        float mipi_data_rate = 660 * 1000000;
        float mipi_lane_num  = 1;                   // 1 lane
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
    i2c_device_number_t i2c_num = dev_driver->i2c_num;
	uint8_t i2c_tar_addr = dev_driver->i2c_tar_addr;
    int again = exp->again * 16;
    int dgain = exp->dgain * 64;
    // int itime = exp->itime;
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
    //         i2c_num, again&0xff, dgain&0xff, itime_h, itime_l);

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
    cis_dev_params_t *_param = (cis_dev_params_t*)dev_driver->context;
    param->width     = _param->width;
    param->height    = _param->height;
    param->framerate = 30;
    param->format    = CIS_FORMAT_RGB_BAYER10;
    param->bayer_pat = CIS_BAYER_PAT_BGGR;

    return 0;
}

// for factory tets only
static cis_dev_driver_t ov02b_suxx_dev0_master = {
    .name                   = "ov02b_suxx_dev0_master",
    .i2c_num                = I2C_DEVICE_0,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = &m_big_resolution,
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

static cis_dev_driver_t ov02b_suxx_dev1_master = {
    .name                   = "ov02b_suxx_dev1_master",
    .i2c_num                = I2C_DEVICE_1,
	.i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = &m_big_resolution,
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

// SU18/SU22
static cis_dev_driver_t ov02b_suxx_dev0 = {
    .name                   = "ov02b_suxx_dev0",
    .i2c_num                = I2C_DEVICE_0,
    .i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = &m_small_resolution,
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

// SU18/SU22
static cis_dev_driver_t ov02b_suxx_dev1 = {
    .name                   = "ov02b_suxx_dev1",
    .i2c_num                = I2C_DEVICE_1,
    .i2c_tar_addr           = SENSOR_ADDR_WR,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .mclk_freq              = 24 * 1000 * 1000,
    .fps                    = 15,
    .mf_mode                = 0,
    .context                = &m_big_resolution,
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

cis_dev_driver_t *cis_ov02b[] = {
    &ov02b_suxx_dev0,
    &ov02b_suxx_dev1,
    NULL,
};

/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */
/* SU18/SU22 product */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_dev0,    0,      &ov02b_suxx_dev0,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_dev1,    0,      &ov02b_suxx_dev1,    NULL);

/* SU18/SU22 factory test */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev0,    0,      &ov02b_suxx_dev0_master,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov02b_suxx_factory_dev1,    0,      &ov02b_suxx_dev1_master,    NULL);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
