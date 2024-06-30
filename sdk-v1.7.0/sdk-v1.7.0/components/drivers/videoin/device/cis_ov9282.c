#include "cis_ov9282.h"
#include "aiva_utils.h"
#include "sysctl.h"
#include "i2c.h"
#include "gpio.h"
#include "syslog.h"
#include "udevice.h"
#include "aiva_sleep.h"

#define SENSOR_ADDR_WR              (0xc0 >> 1) 

#define TAG                      "cis_ov9282"

#define W_VGA                    (640)
#define H_VGA                    (480)

#define W_720P                   (1280)
#define H_720P                   (720)

#define W_1280                   (1280)
#define H_800                    (800)

#define REG_DLY                  (0xffff)

#define USE_30FPS                (1)

typedef struct _OV9282_REG_T {
    uint16_t        reg_addr;
    uint8_t         data;
}OV9282_REG_T;


#define DEFAULT_AGAIN            (8)     // 4x
#define DEFAULT_DGAIN            (1)     // 1x

#define MAX_EXPOSURE             (885)   // (0x038e - 25) 

/*#define DEFAULT_EXPOSURE         (681)     // unit: lines*/
/*#define DEFAULT_EXPOSURE         (800)     // unit: lines*/
#define DEFAULT_EXPOSURE         (200)     // unit: lines

#define STROBE_OUTPUT 1

static OV9282_REG_T ov9282_start_regs[] __ATTR_ALIGN__(32) = {
	{0x0100, 0x01},
};

static OV9282_REG_T ov9282_stop_regs[] __ATTR_ALIGN__(32) = {
    {0x0100, 0x00},
};

static OV9282_REG_T ov9282_master_regs[] __ATTR_ALIGN__(32) = {
    {0x3006, 0x02}, // enable FSIN output
    {0x3823, 0x00},
};

static OV9282_REG_T ov9282_slave_regs[] __ATTR_ALIGN__(32) = {
    {0x3006, 0x00},	// enable FSIN inputput
    {0x38b3, 0x07},	
    {0x3885, 0x07},	
    {0x382b, 0x5a},	
    {0x3670, 0x68},	
    {0x3823, 0x30},	
    {0x3824, 0x00},	// cs counter reset value at vs_ext
    {0x3825, 0x08},	

    {0x3826, (MAX_EXPOSURE + 25 - 4) >> 8},	// r counter reset value at vs_ext; default VTS-4
    {0x3827, (MAX_EXPOSURE + 25 - 4) & 0xff},	


    {0x3740, 0x01},	
    {0x3741, 0x00},	
    {0x3742, 0x08},	
};

// mipi 1 lane, 1280*800， 30fps
static const OV9282_REG_T ov9282_common_30fps[] __ATTR_ALIGN__(32) = {
    {0x0103, 0x01},
    {0x0302, 0x32},
    {0x030d, 0x50},
    {0x030e, 0x06},
    {0x3001, 0x00},
    {0x3004, 0x00},
    {0x3005, 0x00},
    {0x3006, 0x04},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x3022, 0x01},
    {0x3030, 0x10},
    {0x3039, 0x32},
    {0x303a, 0x00},

    // 0x3500 Bit[3:0]: Exposure[19:16]
    // 0x3501 Bit[7:0]: Exposure[15:8]
    // 0x3502 Bit[7:0]: Exposure[7:0] Low 4 bits are fraction bits
    {0x3500, (DEFAULT_EXPOSURE & 0xf000) >> 12},
    {0x3501, (DEFAULT_EXPOSURE & 0x0ff0) >> 4},
    {0x3502, (DEFAULT_EXPOSURE & 0x0f) << 4},

    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03}, // gain shift option
    {0x3508, 0x00},

    // If 0x3503[2] = 0, gain[7:0] is real gain format,
    // where low 4 bits are fraction bits
    {0x3509, DEFAULT_AGAIN << 4},

    {0x3610, 0x80},
    {0x3611, 0xa0},
    {0x3620, 0x6f},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x01},
    {0x3666, 0x00},
    {0x366f, 0x5a},
    {0x3680, 0x84},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x80},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x4a},
    {0x3799, 0x20},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x05},
    {0x3805, 0x0f},
    {0x3806, 0x03},
    {0x3807, 0x2f},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x03},
    {0x380b, 0x20},
    {0x380c, 0x05},
    {0x380d, 0xb0},

    // Maximum exposure time is frame length -25 row periods,
    // where frame length is set by registers {0x380E, 0x380F}
    {0x380e, (MAX_EXPOSURE + 25) >> 8},
    {0x380f, (MAX_EXPOSURE + 25) & 0xff},

    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x08},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3820, 0x40},
    {0x3821, 0x00},
    {0x3881, 0x42},
    {0x38b1, 0x00},
    {0x3920, 0xff},
    {0x4003, 0x40},
    {0x4008, 0x04},
    {0x4009, 0x0b},
    {0x400c, 0x00},
    {0x400d, 0x07},
    {0x4010, 0x40},
    {0x4043, 0x40},
    {0x4307, 0x30},
    {0x4317, 0x00},
    {0x4501, 0x00},
    {0x4507, 0x00},
    {0x4509, 0x00},
    {0x450a, 0x08},
    {0x4601, 0x04},
    {0x470f, 0x00},
    {0x4f07, 0x00},
    {0x4800, 0x00},
    {0x5000, 0x9f},
    {0x5001, 0x00},
    {0x5e00, 0x00},
    {0x5d00, 0x07},
    {0x5d01, 0x00},
};

// mipi 1 lane, 1280*800, raw 10, 60fps
static const OV9282_REG_T ov9282_common_60fps[] __ATTR_ALIGN__(32) = {
    {0x0103, 0x01},
    {0x0302, 0x32},
    {0x030d, 0x50},
    {0x030e, 0x02},
    {0x3001, 0x00},
    {0x3004, 0x00},
    {0x3005, 0x00},
    {0x3006, 0x04},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x301c, 0xf0},
    {0x3022, 0x01},
    {0x3030, 0x10},
    {0x3039, 0x32},
    {0x303a, 0x00},

    // 0x3500 Bit[3:0]: Exposure[19:16]
    // 0x3501 Bit[7:0]: Exposure[15:8]
    // 0x3502 Bit[7:0]: Exposure[7:0] Low 4 bits are fraction bits
    {0x3500, (DEFAULT_EXPOSURE & 0xf000) >> 12},
    {0x3501, (DEFAULT_EXPOSURE & 0x0ff0) >> 4},
    {0x3502, (DEFAULT_EXPOSURE & 0x0f) << 4},

    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03},
    {0x3508, 0x00},
    /*{0x3509, 0x10},*/
    // If 0x3503[2] = 0, gain[7:0] is real gain format,
    // where low 4 bits are fraction bits
    {0x3509, DEFAULT_AGAIN << 4},
    {0x3610, 0x80},
    {0x3611, 0xa0},
    {0x3620, 0x6e},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x01},
    {0x3666, 0x00},
    {0x366f, 0x5a},
    {0x3680, 0x84},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x80},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x4a},
    {0x3799, 0x20},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x05},
    {0x3805, 0x0f},
    {0x3806, 0x03},
    {0x3807, 0x2f},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x03},
    {0x380b, 0x20},
    {0x380c, 0x05},
    {0x380d, 0xb0},

    {0x380e, (MAX_EXPOSURE + 25) >> 8},
    {0x380f, (MAX_EXPOSURE + 25) & 0xff},

    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x08},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3820, 0x40},
    {0x3821, 0x00},
    {0x382c, 0x05},
    {0x382d, 0xb0},
    {0x389d, 0x00},
    {0x3881, 0x42},
    {0x3882, 0x01},
    {0x3883, 0x00},
    {0x3885, 0x02},
    {0x38a8, 0x02},
    {0x38a9, 0x80},
    {0x38b1, 0x00},
    {0x38b3, 0x02},
    {0x38c4, 0x00},
    {0x38c5, 0xc0},
    {0x38c6, 0x04},
    {0x38c7, 0x80},
    {0x3920, 0xff},
    {0x4003, 0x40},
    {0x4008, 0x04},
    {0x4009, 0x0b},
    {0x400c, 0x00},
    {0x400d, 0x07},
    {0x4010, 0x40},
    {0x4043, 0x40},
    {0x4307, 0x30},
    {0x4317, 0x00},
    {0x4501, 0x00},
    {0x4507, 0x00},
    {0x4509, 0x00},
    {0x450a, 0x08},
    {0x4601, 0x04},
    {0x470f, 0x00},
    {0x4f07, 0x00},
    {0x4800, 0x00},
    {0x5000, 0x9f},
    {0x5001, 0x00},
    {0x5e00, 0x00},
    {0x5d00, 0x07},
    {0x5d01, 0x00},
    {0x4f00, 0x04},
    {0x4f10, 0x00},
    {0x4f11, 0x98},
    {0x4f12, 0x0f},
    {0x4f13, 0xc4},
};

static const OV9282_REG_T ov9282_strobe_master[] __ATTR_ALIGN__(32) = {
    {0x3006, 0x0e}, // 0x08 + 0x04 + 0x02 
    {0x3210, 0x00},
    {0x3007, 0x02},
    {0x301c, 0x20},
    {0x3020, 0x20},
    {0x3025, 0x02},
    {0x382c, 0x07},
    {0x382d, 0x10},
    {0x3920, 0xff},

    {0x3923, 0x00}, // shift
    {0x3924, 0x00},

    {0x3925, 0x00}, // width
    {0x3926, 0x00},
    {0x3927, 0x00},
    // {0x3928, 0x80},
    /** As suggested by ruixi vcsel, we need to provide about 10% pwm to vcsel,
     * so in strobe mode, we provide about 3ms strobe time per frame(30fps).
     */
    /*{0x3928, 0x30},*/
    {0x3928, 0x90},

    {0x392b, 0x00},
    {0x392c, 0x00},

    {0x392d, 0x05}, // same as 380c/380d 
    {0x392e, 0xb0},

    {0x392f, 0xcb},
    {0x38b3, 0x07},
    {0x3885, 0x07},
    {0x382b, 0x5a},
    {0x3670, 0x68},


    {0x3208, 0x00},
    // strobe start point is VTS - Texposure - 7 - Tnegative
    // VTS 0x380e/0x380f
    // Texposure 0x3500/0x3501/0x3502
    {0x3929, (MAX_EXPOSURE + 25 - DEFAULT_EXPOSURE - 7) >> 8},
    {0x392A, (MAX_EXPOSURE + 25 - DEFAULT_EXPOSURE - 7) & 0xff},
    
    {0x3208, 0x10},
    {0x3208, 0xA0},
};

static const OV9282_REG_T ov9282_strobe_slave[] __ATTR_ALIGN__(32) = {
    {0x3006, 0x0c}, // 0x08 + 0x04
    {0x3210, 0x00},
    {0x3007, 0x02},
    {0x301c, 0x20},
    {0x3020, 0x20},
    {0x3025, 0x02},
    {0x382c, 0x07},
    {0x382d, 0x10},
    {0x3920, 0xff},

    {0x3923, 0x00}, // shift
    {0x3924, 0x00},

    {0x3925, 0x00}, // width
    {0x3926, 0x00},
    {0x3927, 0x00},
    // {0x3928, 0x80},
    /** As suggested by ruixi vcsel, we need to provide about 10% pwm to vcsel,
     * so in strobe mode, we provide about 3ms strobe time per frame(30fps).
     */
    /*{0x3928, 0x30},*/
    {0x3928, 0x90},

    {0x392b, 0x00},
    {0x392c, 0x00},

    {0x392d, 0x05}, // same as 380c/380d 
    {0x392e, 0xb0},

    {0x392f, 0xcb},
    {0x38b3, 0x07},
    {0x3885, 0x07},
    {0x382b, 0x5a},
    {0x3670, 0x68},


    {0x3208, 0x00},
    // strobe start point is VTS - Texposure - 7 - Tnegative
    // VTS 0x380e/0x380f
    // Texposure 0x3500/0x3501/0x3502
    {0x3929, (MAX_EXPOSURE + 25 - DEFAULT_EXPOSURE - 7) >> 8},
    {0x392A, (MAX_EXPOSURE + 25 - DEFAULT_EXPOSURE - 7) & 0xff},
    
    {0x3208, 0x10},
    {0x3208, 0xA0},
};

static const OV9282_REG_T ov9282_720p_dvpi[] __ATTR_ALIGN__(32) = {
    {0x5A03, 0x28}, // YSTART MAN = 40
    {0x5A06, 0x02}, // YWIN MAN = 720
    {0x5A07, 0xD0},
    {0x5A08, 0x87}, // output window using manual parameters      
};


static void ov9282_busy_delay(int32_t ms)
{
    aiva_msleep(ms);
}

static int ov9282_write_reg(int i2c_num, uint16_t reg_addr, uint8_t reg_val)
{
    int ret;
    uint8_t data_buf[3];

    data_buf[0] = (reg_addr >> 8) & 0xff;
    data_buf[1] = (reg_addr >> 0) & 0xff;
    data_buf[2] = reg_val;
    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, data_buf, 3);

    return ret;
}

static int ov9282_read_reg(int i2c_num, uint16_t reg_addr, uint8_t *reg_val)
{
    int ret;

    uint8_t addr_buf[2];

    addr_buf[0] = (reg_addr >> 8) & 0xff;
    addr_buf[1] = (reg_addr >> 0) & 0xff;
    
    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, &addr_buf[0], 2);
    if (ret < 0) {
        return ret;
    }

    ret = i2c_recv_data(i2c_num, SENSOR_ADDR_WR, 0, 0, reg_val, 1);
    
    return ret;
}


static int ov9282_program_regs(
        int             i2c_num,
        const OV9282_REG_T    *reg_list,
        int             cnt
        )
{
    int i;
    int ret = 0;
    for (i = 0; i < cnt; i++) {
        if (reg_list[i].reg_addr != REG_DLY) {
            ret = ov9282_write_reg(i2c_num, reg_list[i].reg_addr, reg_list[i].data);
            if (ret < 0) {
                LOGE(__func__, "error: i = %d, reg_addr 0x%x, data 0x%x.",
                        i, reg_list[i].reg_addr, reg_list[i].data);
                break;
            }
        }
        else {
            ov9282_busy_delay(reg_list[i].data);
        }
    }
    return ret;
}

static int ov9282_i2c_test(int i2c_num)
{
    uint8_t pid_hi, pid_lo, ver;
    uint8_t val;

    ov9282_read_reg(i2c_num, 0x300a, &pid_hi); 
    ov9282_read_reg(i2c_num, 0x300b, &pid_lo); 
    ov9282_read_reg(i2c_num, 0x300c, &ver); 

    LOGD(TAG, "ov9282_i2c_test: pid is 0x%x:0x%x, ver is 0x%x.", pid_hi, pid_lo, ver);

    if ( pid_hi != 0x92 || pid_lo != 0x81 )
    {
        LOGE(TAG, "ov9282_i2c_test: pid not right!");
        return -1;        
    }
   
    LOGD(TAG, "ov9282_i2c_test: success!");
    return 0;
}

static int ov9282_init(int i2c_num)
{
    int ret;

    i2c_init(i2c_num, SENSOR_ADDR_WR, 7, 350*1000);

    if (ov9282_i2c_test(i2c_num) < 0) {
        return -1;
    }

#if USE_30FPS
    ret = ov9282_program_regs(i2c_num, ov9282_common_30fps, AIVA_ARRAY_LEN(ov9282_common_30fps));
#else
    ret = ov9282_program_regs(i2c_num, ov9282_common_60fps, AIVA_ARRAY_LEN(ov9282_common_60fps));
#endif

    LOGD(TAG, "ov9282_init() common return: %d.", ret);

    return ret;
}

static int cis_ov9282_init(cis_dev_driver_t *dev_driver)
{
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

    if (ov9282_init(dev_driver->i2c_num) != 0) {
        LOGE(TAG, "ov9282 init fail\n");
        return -1;
    }

#if STROBE_OUTPUT    
    if (ov9282_program_regs(dev_driver->i2c_num, ov9282_strobe_slave, AIVA_ARRAY_LEN(ov9282_strobe_slave)) != 0) {
        LOGD(TAG, "ov9282 init strobe fail\n");
        return -1;
    }
#endif

    return 0;
}

static int cis_ov9282_init_master(cis_dev_driver_t *dev_driver)
{
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

    if (ov9282_init(dev_driver->i2c_num) != 0) {
        LOGE(TAG, "ov9282 init fail\n");
        return -1;
    }

    if (ov9282_program_regs(dev_driver->i2c_num, ov9282_master_regs, AIVA_ARRAY_LEN(ov9282_master_regs)) != 0)
    {
        LOGE(TAG, "ov9282 init fail\n");
        return -1;        
    }

#if STROBE_OUTPUT    
    if (ov9282_program_regs(dev_driver->i2c_num, ov9282_strobe_master, AIVA_ARRAY_LEN(ov9282_strobe_master)) != 0) {
        LOGD(TAG, "ov9282 init strobe fail\n");
        return -1;
    }
#endif

    return 0;
}

static int cis_ov9282_init_slave(cis_dev_driver_t *dev_driver)
{
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

    if (ov9282_init(dev_driver->i2c_num) != 0) {
        LOGE(TAG, "ov9282 init fail\n");
        return -1;
    }

    if (ov9282_program_regs(dev_driver->i2c_num, ov9282_slave_regs, AIVA_ARRAY_LEN(ov9282_slave_regs)) != 0)
    {
        LOGE(TAG, "ov9282 init fail\n");
        return -1;        
    }

#if STROBE_OUTPUT    
    if (ov9282_program_regs(dev_driver->i2c_num, ov9282_strobe_slave, AIVA_ARRAY_LEN(ov9282_strobe_slave)) != 0) {
        LOGD(TAG, "ov9282 init strobe fail\n");
        return -1;
    }
#endif

    return 0;
}

static int cis_ov9282_start_stream(cis_dev_driver_t *dev_driver, const cis_config_t *config)
{
    int ret;
    OV9282_REG_T        *reg_list;
    int                 cnt;

    reg_list = ov9282_start_regs;
    cnt     = AIVA_ARRAY_LEN(ov9282_start_regs);
    ret     = ov9282_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0) {
        LOGE(__func__, "ov9282_start failed!");
    }
    return ret;
}

static int cis_ov9282_stop_stream(cis_dev_driver_t *dev_driver)
{
    int ret;
    OV9282_REG_T        *reg_list;
    int                 cnt;

    reg_list = ov9282_stop_regs;
    cnt     = AIVA_ARRAY_LEN(ov9282_start_regs);
    ret     = ov9282_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0) {
        LOGE(__func__, "ov9282_start failed!");
    }
    return ret;
}

static void cis_ov9282_power_on(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    ov9282_busy_delay(3);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    ov9282_busy_delay(5);
    return;
}

static void cis_ov9282_power_off(cis_dev_driver_t *dev_driver)
{
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    ov9282_busy_delay(5);
    return;
}

static void cis_ov9282_reset(cis_dev_driver_t *dev_driver)
{
    return;
}

static int cis_ov9282_get_interface_param(cis_dev_driver_t *dev_driver, cis_interface_param_t *param)
{
    param->interface_type                   = CIS_INTERFACE_TYPE_MIPI;
    param->mipi_param.freq                  = 800;
    param->mipi_param.lane_num              = 1;
    param->mipi_param.vc_num                = 1;
    param->mipi_param.virtual_channels[0]   = MIPI_VIRTUAL_CHANNEL_0;
    param->mipi_param.data_type             = MIPI_DATA_TYPE_RAW_10;
    return 0;
}

static int cis_ov9282_get_exposure_param(cis_dev_driver_t *dev_driver, cis_exposure_param_t *exp_param)
{
    exp_param->min_again    = 1.0;
    exp_param->min_dgain    = 1.0;
    exp_param->min_itime    = 10;

    exp_param->max_again    = 255/16.;
    exp_param->max_dgain    = 1.0;
    /*exp_param->max_itime    = MAX_EXPOSURE;*/
    exp_param->max_itime    = MAX_EXPOSURE / 4;

    exp_param->step_again   = 1/16.;
    exp_param->step_dgain   = 1;
    exp_param->step_itime   = 1;

    exp_param->initial_again = DEFAULT_AGAIN;
    exp_param->initial_dgain = DEFAULT_DGAIN;
    exp_param->initial_itime = DEFAULT_EXPOSURE;

    return 0;
}

static int cis_ov9282_set_exposure(cis_dev_driver_t *dev_driver, const cis_exposure_t *exp)
{
    int i2c_num = dev_driver->i2c_num;
    i2c_init(i2c_num, SENSOR_ADDR_WR, 7, 350*1000);

    float again = exp->again;
    if (again > 255/16.) {
        again = 255/16.;
    }
    else if (again < 0) {
        again = 0;
    }

    if (again > 0) {
        uint8_t analog_gain  = (uint8_t)(16 * again);
        /*LOGD(__func__, "analog_gain is %d", analog_gain);*/
        ov9282_write_reg(i2c_num, 0x3509, analog_gain);
    }

    int expo_lines = exp->itime;
    if (expo_lines > MAX_EXPOSURE) {
        expo_lines = MAX_EXPOSURE;
    }

    // 0x3500 Bit[3:0]: Exposure[19:16]
    // 0x3501 Bit[7:0]: Exposure[15:8]
    // 0x3502 Bit[7:0]: Exposure[7:0] Low 4 bits are fraction bits
    ov9282_write_reg(i2c_num, 0x3500, (expo_lines & 0xf000) >> 12);
    ov9282_write_reg(i2c_num, 0x3501, (expo_lines & 0x0ff0) >> 4);
    ov9282_write_reg(i2c_num, 0x3502, (expo_lines & 0x0f) << 4);

#if STROBE_OUTPUT    
    ov9282_write_reg(i2c_num, 0x3208, 0x00);
    // strobe start point is VTS - Texposure - 7 - Tnegative
    // VTS 0x380e/0x380f
    // Texposure 0x3500/0x3501/0x3502    
    ov9282_write_reg(i2c_num, 0x3929, (MAX_EXPOSURE + 25 - expo_lines - 7) >> 8);
    ov9282_write_reg(i2c_num, 0x392A, (MAX_EXPOSURE + 25 - expo_lines - 7) & 0xff);
    ov9282_write_reg(i2c_num, 0x3208, 0x10);
    ov9282_write_reg(i2c_num, 0x3208, 0xA0);
#endif

    return 0;
}

static int cis_ov9282_get_frame_parameter(cis_dev_driver_t *dev_driver, cis_frame_param_t *param)
{
    param->width = W_1280;
    param->height = H_800;

#if USE_30FPS
    param->framerate = 30;
#else
    param->framerate = 60;
#endif

    param->format = CIS_FORMAT_RGB_BAYER10;

    return 0;
}

static cis_dev_driver_t ov9282_dev0 = {
    .name                   = "ov9282_dev0",
    .i2c_num                = I2C_DEVICE_0,
    .power_pin              = GPIO_PIN0,
    .reset_pin              = GPIO_PIN0,
    .mclk_id                = CIS_MCLK_ID_MCLK0,
    .context                = NULL,
    .init                   = cis_ov9282_init,
    .start_stream           = cis_ov9282_start_stream,
    .stop_stream            = cis_ov9282_stop_stream,
    .power_on               = cis_ov9282_power_on,
    .power_off              = cis_ov9282_power_off,
    .reset                  = cis_ov9282_reset,
    .get_interface_param    = cis_ov9282_get_interface_param,
    .get_exposure_param     = cis_ov9282_get_exposure_param,
    .set_exposure           = cis_ov9282_set_exposure,
    .get_frame_parameter         = cis_ov9282_get_frame_parameter
};


static cis_dev_driver_t ov9282_dev1 = {
    .name                   = "ov9282_dev1",
    .i2c_num                = I2C_DEVICE_1,
    .power_pin              = GPIO_PIN1,
    .reset_pin              = GPIO_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .context                = NULL,
    .init                   = cis_ov9282_init_slave,
    .start_stream           = cis_ov9282_start_stream,
    .stop_stream            = cis_ov9282_stop_stream,
    .power_on               = cis_ov9282_power_on,
    .power_off              = cis_ov9282_power_off,
    .reset                  = cis_ov9282_reset,
    .get_interface_param    = cis_ov9282_get_interface_param,
    .get_exposure_param     = cis_ov9282_get_exposure_param,
    .set_exposure           = cis_ov9282_set_exposure,
    .get_frame_parameter         = cis_ov9282_get_frame_parameter
};



static cis_dev_driver_t ov9282_dev2 = {
    .name                   = "ov9282_dev2",
    .i2c_num                = I2C_DEVICE_2,
    .power_pin              = GPIO_PIN1,
    .reset_pin              = GPIO_PIN1,
    .mclk_id                = CIS_MCLK_ID_MCLK1,
    .context                = NULL,
    .init                   = cis_ov9282_init_master,
    .start_stream           = cis_ov9282_start_stream,
    .stop_stream            = cis_ov9282_stop_stream,
    .power_on               = cis_ov9282_power_on,
    .power_off              = cis_ov9282_power_off,
    .reset                  = cis_ov9282_reset,
    .get_interface_param    = cis_ov9282_get_interface_param,
    .get_exposure_param     = cis_ov9282_get_exposure_param,
    .set_exposure           = cis_ov9282_set_exposure,
    .get_frame_parameter         = cis_ov9282_get_frame_parameter
};



cis_dev_driver_t *cis_ov9282[] = {
    &ov9282_dev0,
    &ov9282_dev1,
    &ov9282_dev2
};

/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov9282_dev0,    0,      &ov9282_dev0,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov9282_dev1,    0,      &ov9282_dev1,    NULL);
UDEVICE_EXPORT(UCLASS_CIS_MIPI,     ov9282_dev2,    0,      &ov9282_dev2,    NULL);
