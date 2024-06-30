#include <math.h>
#include "cis_sc030iot.h"
#include "aiva_utils.h"
#include "aiva_sleep.h"
#include "sysctl.h"
#include "i2c.h"
#include "gpio.h"
#include "syslog.h"
#include "udevice.h"

#define REG_DLY (0xffff)
#define SENSOR_ADDR_WR (0xd0 >> 1)

typedef struct _SC030IOT_REG_T
{
    uint8_t reg_addr;
    uint8_t data;
} SC030IOT_REG_T;

static const char *TAG = "cis_sc030iot";

static SC030IOT_REG_T sc030iot_start_regs[] __ATTR_ALIGN__(32) = {

};

static SC030IOT_REG_T sc030iot_stop_regs[] __ATTR_ALIGN__(32) = {

};

// mclk 24Mhz
// window size 1600*1031
// mipiclk 696Mbps/lane
// framelength 1125
// rowtime 29.42529us
// pattern rggb
static const SC030IOT_REG_T sc030iot_640x480[] __ATTR_ALIGN__(32) = {
    {0xf0,0x30},
    {0x01,0x00},
    {0x02,0x00},
    {0x22,0x00},
    {0x19,0x00},
    {0x3f,0x82},
    {0x30,0x02},
    {0x31,0x08},
    {0x37,0x00},
    {0xf0,0x01},
    {0x76,0x81},
    {0x77,0x8c},
    {0x78,0xe1},
    {0x79,0xc1},
    {0xf4,0x0a},
    {0xf0,0x36},
    {0x37,0x79},
    {0x31,0x82},
    {0x3e,0x60},
    {0x30,0xf0},
    {0x33,0x33},
    {0xf0,0x32},
    {0x48,0x02},
    {0xf0,0x33},
    {0x02,0x12},
    {0x7c,0x02},
    {0x7d,0x0e},
    {0xa2,0x04},
    {0x5e,0x06},
    {0x5f,0x0a},
    {0x0b,0x58},
    {0x06,0x38},
    {0xf0,0x32},
    {0x48,0x02},
    {0xf0,0x39},
    {0x02,0x70},
    {0xf0,0x45},
    {0x09,0x1c},
    {0xf0,0x37},
    {0x22,0x0d},
    {0xf0,0x33},
    {0x33,0x10},
    {0xb1,0x80},
    {0x34,0x40},
    {0x0b,0x54},
    {0xb2,0x78},
    {0xf0,0x36},
    {0x11,0x80},
    {0xf0,0x30},
    {0x38,0x44},
    {0xf0,0x33},
    {0xb3,0x51},
    {0x01,0x10},
    {0x0b,0x6c},
    {0x06,0x24},
    {0xf0,0x36},
    {0x31,0x82},
    {0x3e,0x60},
    {0x30,0xf0},
    {0x33,0x33},
    {0xf0,0x34},
    {0x9f,0x02},
    {0xa6,0x40},
    {0xa7,0x47},
    {0xe8,0x5f},
    {0xa8,0x51},
    {0xa9,0x44},
    {0xe9,0x36},
    {0xf0,0x33},
    {0xb3,0x51},
    {0x64,0x17},
    {0x90,0x01},
    {0x91,0x03},
    {0x92,0x07},
    {0x01,0x10},
    {0x93,0x10},
    {0x94,0x10},
    {0x95,0x10},
    {0x96,0x01},
    {0x97,0x07},
    {0x98,0x1f},
    {0x99,0x10},
    {0x9a,0x20},
    {0x9b,0x28},
    {0x9c,0x28},
    {0xf0,0x36},
    {0x70,0x54},
    {0xb6,0x40},
    {0xb7,0x41},
    {0xb8,0x43},
    {0xb9,0x47},
    {0xba,0x4f},
    {0xb0,0x8b},
    {0xb1,0x8b},
    {0xb2,0x8b},
    {0xb3,0x9b},
    {0xb4,0xb8},
    {0xb5,0xf0},
    {0x7e,0x41},
    {0x7f,0x47},
    {0x77,0x83},
    {0x78,0x84},
    {0x79,0x8a},
    {0xa0,0x47},
    {0xa1,0x5f},
    {0x96,0x43},
    {0x97,0x44},
    {0x98,0x54},
    {0xf0,0x3f},
    {0x03,0x8a},
    {0xf0,0x00},
    {0xf0,0x01},
    {0x73,0x00},
    {0x74,0xe0},
    {0x70,0x00},
    {0x71,0x80},
    {0xf0,0x36},
    {0x37,0x74},
    {0xf0,0x36},
    {0x11,0x80},
    {0xf0,0x01},
    {0x79,0xc1},
    {0xf0,0x37},
    {0x24,0x21},
    {0xf0,0x36},
    {0x41,0x60},

    {0xf0,0x36},
    {0xe9,0x24},
    {0xea,0x06},
    {0xeb,0x03},
    {0xec,0x19},
    {0xed,0x18},
    {0xf0,0x32},
    {0x0e,0x02},
    {0x0f,0xbb},
    {0xf0,0x32},
    {0x21,0xff},
    {0xf0,0x48},
    {0x19,0x09},
    {0x1b,0x05},
    {0x1d,0x13},
    {0x1f,0x04},
    {0x21,0x0a},
    {0x23,0x05},
    {0x25,0x04},
    {0x27,0x05},
    {0x29,0x08},

    {0xf0,0x00},//ISP
    {0xf4,0x08},
    {0x7c,0x5a},
    {0x72,0x38},//AE target [6:0]*2
    {0x7a,0x80},//maximum analog gain(16X)
    {0x85,0x18},
    {0x9b,0x35},
    {0x9e,0x20},//maximum digital gain(2x)
    {0xd0,0x66},//edge	[3:0]neg edge enhancement [7:4] pos_edge enhancement
    {0xd1,0x34},
    {0Xd3,0x44},
    {0xd6,0x44},//low light denoise slope
    {0xb0,0x41},//AWB	//minimum of blue gain
    {0xb2,0x48},		//minimum of blue gain
    {0xb3,0xf4},		//maximum of blue gain
    {0xb4,0x0b},		//minimum of red gain
    {0xb5,0x78},		//maximum of red gain
    {0xba,0xff},
    {0xbb,0xc0},
    {0xbc,0x90},
    {0xbd,0x3a},
    {0xc1,0x67},
    {0xf0,0x01},//ccm
    {0x20,0x11},
    {0x23,0x90},
    {0x24,0x15},
    {0x25,0x87},
    {0xbc,0x9f},
    {0xbd,0x3a},
    {0x48,0xe6},//saturation
    {0x49,0xc0},
    {0x4a,0xd0},
    {0x4b,0x48},

    {0xf0,0x00},
    {0x71,0x92},
    {0x7c,0x03},
    {0x84,0xb4},
    {0xf0,0x33},
};

static void sc030iot_busy_delay(int32_t ms)
{
    aiva_msleep(ms);
}

static int sc030iot_write_reg(int i2c_num, uint8_t reg_addr, uint8_t reg_val)
{
    int ret;
    uint8_t data_buf[2];
    data_buf[0] = reg_addr & 0xff;
    data_buf[1] = reg_val;
    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, data_buf, 2);
    return ret;
}

static int sc030iot_read_reg(int i2c_num, uint8_t reg_addr, uint8_t *reg_val)
{
    int ret;

    ret = i2c_send_data(i2c_num, SENSOR_ADDR_WR, (const uint8_t*) &reg_addr, 1);
    /*ret = i2c_send_data_dma(DMAC_CHANNEL0, i2c_num, &reg_addr, 1);*/
    if (ret < 0)
    {
        return ret;
    }
    ret = i2c_recv_data(i2c_num, SENSOR_ADDR_WR, 0, 0, reg_val, 1);
        
    /*ret = i2c_recv_data_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, i2c_num, 0, 0, reg_val, 1);*/

    // if (ret < 0) {
    // }

    return ret;
}

static int sc030iot_program_regs(
    int i2c_num,
    const SC030IOT_REG_T *reg_list,
    int cnt)
{
    int i;
    int ret = 0;
    for (i = 0; i < cnt; i++)
    {
        if (reg_list[i].reg_addr != REG_DLY)
        {
            ret = sc030iot_write_reg(i2c_num, reg_list[i].reg_addr, reg_list[i].data);
            if (ret < 0)
            {
                LOGE(__func__, "error: i = %d, reg_addr 0x%x, data 0x%x.",
                     i, reg_list[i].reg_addr, reg_list[i].data);
                break;
            }
        }
        else
        {
            sc030iot_busy_delay(reg_list[i].data);
        }
    }
    return ret;
}

static uint16_t sc030iot_get_sensor_id(i2c_device_number_t i2c_num)
{
    uint8_t id_h = 0;
    uint8_t id_l = 0;

    sc030iot_read_reg(i2c_num, 0xf7, &id_h);
    sc030iot_read_reg(i2c_num, 0xf8, &id_l);

    return ((id_h << 8) | id_l);
}

static int sc030iot_i2c_test(int i2c_num)
{
    uint16_t val;

    val = sc030iot_get_sensor_id(i2c_num);
    LOGD(TAG, "sc030iot readed id is 0x%4x", val);

    if (val != 0x9a46)
    {
        LOGE(TAG, "sc030iot_i2c_test: read id fail!, get 0x%4x, expected 0x9a46", val);
        return -1;
    }
    else {
        LOGD(TAG, "sc030iot_i2c_test: read id success!");
    }


    return 0;
}

static int sc030iot_init(const SC030IOT_REG_T *reg_list, int cnt, int i2c_num)
{
    int ret;


    i2c_init(i2c_num, SENSOR_ADDR_WR, 7, 10 * 1000);

    if (sc030iot_i2c_test(i2c_num) < 0)
    {
        LOGE(TAG, "sc030iot i2c test fail\n");
        return -1;
    }

    ret = sc030iot_program_regs(i2c_num, reg_list, cnt);

    /*LOGD(TAG, "sc030iot_init() return: %d.", ret);*/
    if (ret < 0)
    {
        LOGE(__func__, "line %d, sc030iot_program_regs failed!", __LINE__);
        return ret;
    }

    return ret;
}

static int sc030iot_640x480_init(int i2c_num)
{
    int ret;
    ret = sc030iot_init(sc030iot_640x480, AIVA_ARRAY_LEN(sc030iot_640x480), i2c_num);
    return ret;
}

static int cis_sc030iot_init(cis_dev_driver_t *dev_driver)
{
    // enable sensor mclk
    sysctl_set_sens_mclk(MCLK_ID0, 24 * 1000 * 1000);
    sysctl_set_io_switch(IO_SWITCH_MCLK0, 1);

    if (sc030iot_640x480_init(dev_driver->i2c_num) != 0)
    {
        LOGE(TAG, "sc030iot_init_640x480 fail\n");
        return -1;
    }
    return 0;
}

static int cis_sc030iot_start_stream(cis_dev_driver_t *dev_driver, const cis_config_t *config)
{
    int ret;
    SC030IOT_REG_T *reg_list;
    int cnt;

    reg_list = sc030iot_start_regs;
    cnt = AIVA_ARRAY_LEN(sc030iot_start_regs);
    ret = sc030iot_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0)
    {
        LOGE(TAG, "sc030iot_start failed!");
    }
    return ret;
}

static int cis_sc030iot_stop_stream(cis_dev_driver_t *dev_driver)
{
    int ret;
    SC030IOT_REG_T *reg_list;
    int cnt;

    reg_list = sc030iot_stop_regs;
    cnt = AIVA_ARRAY_LEN(sc030iot_stop_regs);
    ret = sc030iot_program_regs(dev_driver->i2c_num, reg_list, cnt);
    if (ret < 0)
    {
        LOGE(TAG, "sc030iot_stop failed!");
    }
    return ret;
}

static void cis_sc030iot_power_on(cis_dev_driver_t *dev_driver)
{
#if 0
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    sc030iot_busy_delay(3);
    gpio_set_pin(power_pin, GPIO_PV_LOW);
    sc030iot_busy_delay(5);
#endif
    return;
}

static void cis_sc030iot_power_off(cis_dev_driver_t *dev_driver)
{
#if 0
    int power_pin = dev_driver->power_pin;

    gpio_set_drive_mode(power_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_pin, GPIO_PV_HIGH);
    sc030iot_busy_delay(5);
#endif
    return;
}

static void cis_sc030iot_reset(cis_dev_driver_t *dev_driver)
{
    int reset_pin = dev_driver->reset_pin;

    gpio_set_drive_mode(reset_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(reset_pin, GPIO_PV_LOW);
    sc030iot_busy_delay(3);
    gpio_set_pin(reset_pin, GPIO_PV_HIGH);
    sc030iot_busy_delay(5);

    return;
}

static int cis_sc030iot_get_interface_param(cis_dev_driver_t *dev_driver, cis_interface_param_t *param)
{
    param->interface_type = CIS_INTERFACE_TYPE_MIPI;
    param->mipi_param.freq = 696 * 2;
    param->mipi_param.lane_num = 1;
    param->mipi_param.vc_num = 1;
    param->mipi_param.virtual_channels[0] = MIPI_VIRTUAL_CHANNEL_0;
    param->mipi_param.data_type = MIPI_DATA_TYPE_YUV422_8;
    return 0;
}

static int cis_sc030iot_get_frame_parameter(cis_dev_driver_t *dev_driver, cis_frame_param_t *param)
{
    // param->width = 640;
    param->width = 1280;
    param->height = 480;
    param->framerate = 15;
    param->format = CIS_FORMAT_RGB_YUYV;
    param->bayer_pat = CIS_BAYER_PAT_RGGB;

    return 0;
}

static int cis_sc030iot_get_exposure_param(cis_dev_driver_t *dev_driver, cis_exposure_param_t *exp_param)
{
    (void*)dev_driver;
    (void*)exp_param;
    return 0;
}

static int cis_sc030iot_set_exposure_param(cis_dev_driver_t *dev_driver, const cis_exposure_t *exp)
{
    (void*)dev_driver;
    (void*)exp;
    return 0;
}

static int cis_sc030iot_wake(cis_dev_driver_t *dev_driver)
{
    (void*)dev_driver;
    return 0;
}

static int cis_sc030iot_sleep(cis_dev_driver_t *dev_driver)
{
    (void*)dev_driver;
    return 0;
}

static cis_dev_driver_t sc030iot_dev0 = {
    .name = "sc030iot_dev0",
    .i2c_num = I2C_DEVICE_2,
    .power_pin = GPIOC_PIN2,
    .reset_pin = GPIOB_PIN9,
    .mclk_id = CIS_MCLK_ID_MCLK0,
    .context = NULL,
    .init = cis_sc030iot_init,
    .start_stream = cis_sc030iot_start_stream,
    .stop_stream = cis_sc030iot_stop_stream,
    .wake = cis_sc030iot_wake,
    .sleep = cis_sc030iot_sleep,
    .power_on = cis_sc030iot_power_on,
    .power_off = cis_sc030iot_power_off,
    .reset = cis_sc030iot_reset,
    .get_interface_param = cis_sc030iot_get_interface_param,
    .get_exposure_param = cis_sc030iot_get_exposure_param,
    .set_exposure = cis_sc030iot_set_exposure_param,
    .get_frame_parameter = cis_sc030iot_get_frame_parameter};


cis_dev_driver_t *cis_sc030iot[] = {
    &sc030iot_dev0};

/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */
UDEVICE_EXPORT(UCLASS_CIS_MIPI, sc030iot_dev0, 0, &sc030iot_dev0, NULL);