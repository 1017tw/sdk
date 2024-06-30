# Videoin接口

## 概述

    Videoin，是软件层对MIPI和DVPI硬件接口的抽象；通过该接口获取视频流数据，包括初始化硬件，启动sensor数据流、停止sensor数据流、唤醒sensor以及sensor睡眠等操作；同时，Videoin接口提供了对sensor驱动的操作，包括获取驱动，重新配置关键参数等操作；
    SDK中已经包含了videoin接口的驱动，并且在sdk目录下example/videoin目录中包含有相关的使用例程，这里对相关接口简要说明。

## API参考

    对应的头文件 videoin.h， cis_dev_driver.h

    为用户提供以下接口

- [videoin_init](#videoin_init)
- [videoin_start_stream](#videoin_start_stream)
- [videoin_stop_stream](#videoin_stop_stream)
- [videoin_release](#videoin_release)
- [videoin_get_frame](#videoin_get_frame)
- [videoin_set_exposure](#videoin_set_exposure)
- [videoin_register_framestart_callback](#videoin_register_framestart_callback)
- [videoin_register_frameend_callback](#videoin_register_frameend_callback)
- [videoin_wake_cis](#videoin_wake_cis)
- [videoin_sleep_cis](#videoin_sleep_cis)
- [cis_find_dev_driver](#cis_find_dev_driver)
- [cis_find_dev_driver_list](#cis_find_dev_driver_list)

### videoin_init

#### 描述

    根据当前配置，初始化MIPI或者DVPI硬件

#### 函数定义
```c
    videoin_context_t* videoin_init(const videoin_config_t *config)
```
#### 参数

| 参数名称      | 描述          | 输入输出  |
| :------       | ----------    | -------- |
| config       | 初始化视频流相关硬件 | 输入 |

#### 返回值
    videoin接口的上下文指针

### videoin_start_stream

#### 描述

    调用sensor对应的start_stream接口，开始输出视频流

#### 函数定义
```c
    int videoin_start_stream(videoin_context_t *context)
```

#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| context       | videoin接口的上下文指针  | 输入 |

#### 返回值
    0：成功；其他：失败

### videoin_stop_stream

#### 描述

    调用sensor对应的stop_stream接口，停止输出视频流

#### 函数定义
```c
    int videoin_stop_stream(videoin_context_t *context)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| context       |videoin接口的上下文指针  | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_release

#### 描述

    释放获取视频流的硬件

#### 函数定义
```c
    int videoin_release(videoin_context_t *context)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| context       |videoin接口的上下文指针  | 输入 |

#### 返回值
    0：成功；其他：失败

### videoin_get_frame

#### 描述

    获取一帧图像数据

#### 函数定义
```c
    int videoin_get_frame(videoin_context_t *context, videoin_frame_t *videoin_frame, int timeout_ms)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| context  | videoin接口的上下文指针 | 输入      |
| videoin_frame | 用于保存以及描述一帧图像 | 输出|
| timeout_ms    | 超时时间          | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_set_exposure

#### 描述

    设置sensor曝光参数，调用sensor驱动对应的set_exposure接口

#### 函数定义
```c
    int videoin_set_exposure(videoin_id_t videoin_id, cis_exposure_t *exposure_param)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id  | sensor对应的MIPI id | 输入      |
| exposure_param  | sensor的曝光参数 | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_register_cis_driver

#### 描述

    注册sensor的驱动

#### 函数定义
```c
    int videoin_register_cis_driver(videoin_id_t videoin_id, cis_dev_driver_t *driver);
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id    | 注册sensor驱动对应的MIPI_ID  | 输入  |
| driver        | sensor的驱动      | 输入   |

#### 返回值
    0：成功；其他：失败

### videoin_register_framestart_callback

#### 描述

    注册开始传输一帧数据的中断，对应有一帧传输结束的中断

#### 函数定义
```c
    int videoin_register_framestart_callback(videoin_id_t videoin_id, videoin_event_callback_t callback, void *ctx)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id    | 传输数据的硬件ID   | 输入      |
| callback      | 中断回调函数       | 输入      |
| ctx           | 中断回调函数的参数  | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_register_frameend_callback

#### 描述

    注册传输一帧数据的结束中断，对应有开始传输一帧的中断

#### 函数定义
```c
    int videoin_register_frameend_callback(videoin_id_t videoin_id, videoin_event_callback_t callback, void *ctx)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id    | 传输数据的硬件ID   | 输入      |
| callback      | 中断回调函数       | 输入      |
| ctx           | 中断回调函数的参数  | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_wake_cis

#### 描述

    唤醒sensor，调用sensor对应的wake接口，从睡眠中唤醒

#### 函数定义
```c
    int videoin_wake_cis(videoin_id_t videoin_id)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id    | 硬件上对应的ID     | 输入      |

#### 返回值
    0：成功；其他：失败

### videoin_sleep_cis

#### 描述

    让sensor睡眠，调用sensor对应的sleep接口，进入睡眠状态

#### 函数定义
```c
    int videoin_sleep_cis(videoin_id_t videoin_id)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| videoin_id    | 硬件上对应的ID     | 输入      |

#### 返回值
    0：成功；其他：失败

### cis_find_dev_driver

#### 描述

    通过sensor的name找到sensor对应的驱动，对udevice接口的封装

#### 函数定义
```c
    cis_dev_driver_t  *cis_find_dev_driver(const char *sens_dev_name)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| sens_dev_name | sensor的名称      | 输入      |

#### 返回值
    0：成功；其他：失败

### cis_find_dev_driver_list

#### 描述

    使用sensor类型找到该sensor的所有驱动，对sens_type有拼接，最终都是找name变量

#### 函数定义
```c
    int cis_find_dev_driver_list(const char *sens_type, cis_dev_driver_t **dev_list, int dev_cnt)
```
#### 参数

| 参数名称      | 描述              | 输入输出  |
| :------------ | ----------------- | -------- |
| sens_type     | sensor的类型        | 输入      |
| dev_list      | 找到的sensor驱动列表 | 输入      |
| dev_cnt       | sensor设备的个数     | 输入      |

#### 返回值
    0：成功；其他：失败

## 数据类型

​    相关数据类型、结构体如下：

- [videoin_config_t](#videoin_config_t)
- [videoin_mode_t](#videoin_mode_t)
- [frame_stereo_mode_t](#frame_stereo_mode_t)
- [capture_config_t](#capture_config_t)
- [videoin_id_t](#videoin_id_t)
- [cis_exposure_t](#cis_exposure_t)
- [cis_dev_driver_t](#cis_dev_driver_t)
- [videoin_event_callback_t](#videoin_event_callback_t)

### videoin_config_t

videoin接口配置参数
```c 
    videoin_mode_t      mode;           // 抓图模式
    int                 camera_num;     // 摄像头数量
    frame_stereo_mode_t stereo_mode;    // 是否为双目摄像头以及双目图像拼接方式
    int                 skip_power_on;  // 是否跳过上电操作
    int                 max_frame_cnt;  // 最大帧个数
    capture_config_t    capture_config[MAX_CAMERA_NUM]; // 抓图配置
```

### videoin_mode_t

抓图模式
```c
    VIDEOIN_MODE_CONTINUE = 0,       // 连续抓图模式
    VIDEOIN_MODE_ONE_SHOT = 1,       // 非连续抓图
```
### frame_stereo_mode_t

是否为双目摄像头以及双目图像拼接方式
```c
    FRAME_STEREO_NONE = 0,          // 单目镜头配置
    FRAME_STEREO_H_CONCAT = 1,      // 双目镜头，图像左右拼接
    FRAME_STEREO_V_CONCAT = 2,      // 双目镜头，图像上下拼接
```
### capture_config_t

```c
    videoin_id_t    videoin_id;     // videoin接口ID
    int             hoffset;        // 水平上偏移
    int             voffset;        // 垂直偏移
    int             hsize;          // 水平大小
    int             vsize;          // 垂直大小
    int             hstride;        // 水平stride
    int             channels;       // 通道数
    frame_fmt_t     fmt; // FAME_FMT_RAW8/FRAME_FMT_RAW10/FRAME_FMT_RAW12
```

### videoin_id_t

```c
    VIDEOIN_ID_MIPI0    = 0,    // video_id mipi0
    VIDEOIN_ID_MIPI1    = 1,    // video_id mipi1
    VIDEOIN_ID_MIPI2    = 2,    // video_id mipi2
    VIDEOIN_ID_DVPI0    = 3,    // video_id DVPI0
    VIDEOIN_ID_MAX      = 4,    // 无效的video_id 
```

### cis_exposure_t
曝光参数
```c
    union {
        float again;        // 模拟增益
        float again_hdr[3];
    };

    union {
        float dgain;        // 数字增益
        float dgain_hdr[3];
    };

    union {
        float itime;        // 曝光行数
        float itime_hdr[3];
    };
```

### cis_dev_driver_t
```c
    const char              *name;          // sensor驱动名字
    i2c_device_number_t     i2c_num;        // 用于配置sensor的I2C号码
	uint32_t 			    i2c_tar_addr;   // sensor做为I2C设备的设备地址
    int                     power_pin;      // 电源管脚
    int                     reset_pin;      // 复位管脚
    cis_mclk_id_t           mclk_id;        // MCLK的ID号
    uint32_t                mclk_freq;      // MCLK频率，单位Hz
    int                     fps;            // sensor的帧率
    int                     mf_mode;        // 出图方向模式
    void                    *context;       // 驱动的私有参数
    int  (*init)(struct cis_dev_driver *dev_driver);        // sensor初始化接口
    int  (*wake)(struct cis_dev_driver *dev_driver);        // sensor唤醒接口
    int  (*sleep)(struct cis_dev_driver *dev_driver);       // sensor睡眠接口
    void (*reset)(struct cis_dev_driver *dev_driver);       // sensor复位接口
    void (*power_on)(struct cis_dev_driver *dev_driver);    // sensor上电接口
    void (*power_off)(struct cis_dev_driver *dev_driver);   // sensor下电接口
    int  (*stop_stream)(struct cis_dev_driver *dev_driver); // sensor停止数据流传输接口
    int  (*set_exposure)(struct cis_dev_driver *dev_driver, const cis_exposure_t *exp);             // sensor设置曝光接口
    int  (*start_stream)(struct cis_dev_driver *dev_driver, const cis_config_t *config);            // sensor唤醒接口
    int  (*get_vcm_param)(struct cis_dev_driver *dev_driver, cis_vcm_param_t *vcm_param);           // sensor开始数据流传输接口
    int  (*get_frame_parameter)(struct cis_dev_driver *dev_driver, cis_frame_param_t *param);       // sensor获取一帧图像参数接口
    int  (*get_interface_param)(struct cis_dev_driver *dev_driver, cis_interface_param_t *param);   // 获取sensor配置的接口
    int  (*get_exposure_param)(struct cis_dev_driver *dev_driver, cis_exposure_param_t *exp_param); // 获取sensor曝光参数的接口
```

### videoin_event_callback_t

```c
    int (*videoin_event_callback_t)(videoin_id_t videoin_id, void *ctx)
```

## 例程

```c
//  videoin接口简单使用逻辑，详细请查看sdk/example/videoin例子
    cis_dev_driver_t *drv[3];
    // 根据sensor的类型找到驱动
    ret = cis_find_dev_driver_list(sens_type, &drv[0], 3);
    // 获取一帧图像的参数
    drv[0]->get_frame_parameter(drv[0], &frame_param);
    // 获取sensor的参数
    drv[0]->get_interface_param(drv[0], &param_inf);
    videoin_register_cis_driver(0, drv[0]); // 注册驱动
    // 初始化videoin
    videoin_context_t *context = videoin_init(&config);
    // 开始传输数据
    ret = videoin_start_stream(context);
    // 注册回调函数
    videoin_register_frameend_callback(id, videoin_frameend_callback, NULL);
    // 开始获取一帧图像
    videoin_get_frame(m_videoin_context, &videoin_frame, VIDEOIN_WAIT_FOREVER);
```
