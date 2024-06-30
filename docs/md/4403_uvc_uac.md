# UVC接口

## 概述

    UVC（USB Video Class）USB视屏接口类，是USB总线下的子类，用于传输图像、视频的接口；在AI31xx系列的芯片上，目前仅作为从机设备，用于捕获图像，通过该接口将视频流传输到主机；
    SDK中已经包含了UVC接口的驱动，并且在sdk目录下example/videoin目录中包含有相关的使用例程，这里对相关接口简要说明。

## API参考

    对应的头文件 uvc_comm.h
    
    为用户提供以下接口

- [uvc_init](#uvc_init)
- [uvc_cleanup](#uvc_cleanup)
- [uvc_commit_frame](#uvc_commit_frame)
- [uvc_register_user_start_cb](#uvc_register_user_start_cb)
- [uvc_register_user_stop_cb](#uvc_register_user_stop_cb)

### uvc_init
<div id="uvc_init"></div>

#### 描述

    初始化UVC设备，并设置UVC参数。

#### 函数定义
```c
    int  uvc_init(const char *dev_name, uvc_param_t *param)
```
#### 参数
| 参数名称      | 描述       | 输入输出 |
| :----------- | ---------- | -------- |
| dev_name     | UVC设备名称 | 输入     |
| param        | UVC设备需要配置的参数   | 输入     |

#### 返回值
    0：成功；其他：失败

### uvc_cleanup
<div id="uvc_cleanup"></div>

#### 描述

    释放以及清除UVC设备资源

#### 函数定义
```c
    int  uvc_cleanup(void)
```
#### 参数
    无。

#### 返回值
    0：成功；其他：失败

### uvc_commit_frame
<div id="uvc_commit_frame"></div>

#### 描述

    发送一帧图像数据到主机

#### 函数定义
```c
    int  uvc_commit_frame(frame_buf_t *cur)
```
#### 参数

| 参数名称      | 描述       | 输入输出 |
| :-----------  | ---------- | -------- |
| cur           | 一帧图像数据| 输入     |

#### 返回值
   0：成功；其他：失败

### uvc_register_user_start_cb
<div id="uvc_register_user_start_cb"></div>

#### 描述

    注册发送UVC数据起始帧的回调函数

#### 函数定义
```c
    void uvc_register_user_start_cb(uvc_user_start_func_t func)
```
#### 参数

| 参数名称      | 描述       | 输入输出 |
| :----------- | ---------- | -------- |
| func         | 回调函数    | 输入     |

#### 返回值
   无

### uvc_register_user_stop_cb
<div id="uvc_register_user_stop_cb"></div>

#### 描述

    注册发送UVC数据帧结束时的回调函数

#### 函数定义
```c
    void uvc_register_user_stop_cb(uvc_user_stop_func_t func)
```
#### 参数

| 参数名称      | 描述       | 输入输出 |
| :----------- | ---------- | -------- |
| func         | 回调函数    | 输入     |

#### 返回值
   无


## 数据类型

​    相关数据类型、结构体如下：

- [uvc_param_t](#uvc_param_t): 配置UVC的参数类型
- [usb_ext4_frame_resolution_t](#usb_ext4_frame_resolution_t): USB扩展分辨率
- [frame_buf_t](#frame_buf_t): 用于描述一帧图像
- [uvc_user_start_func_t](#uvc_user_start_func_t): UVC起始帧的回调函数
- [uvc_user_stop_func_t](#uvc_user_stop_func_t): UVC结束帧的回调函数

### uvc_param_t
<div id="uvc_param_t"></div>

#### 描述

    配置UVC的参数类型

#### 定义

```c
    uint32_t max_payload_size;      // USB传输数据的最大负载
    int      ep0_desc_sel;          // 支持分辨率选择
    int 	 ext4_frame_resolution_num;     // UVC扩展分辨率的个数
    usb_ext4_frame_resolution_t *usb_ext4_frame_resolution; // UVC扩展分辨率(w*h)
    int      uvc_h264_enable;       // 是否支持H264
    int      max_request_size;      // UVC最大请求输出大小
```

### usb_ext4_frame_resolution_t
<div id="usb_ext4_frame_resolution_t"></div>

```c
	int width;      // 分辨率宽度
	int height;     // 分辨率高度
```

### frame_buf_t
<div id="frame_buf_t"></div>

#### 描述

    帧缓冲区

#### 定义

```c 
    uint8_t *data[3];
    int      stride[3];         // in bytes
    int      fmt;               // frame format
    int      width;             // in pixels
    int      height;            // in pixels
    int      channels;          // 1~3
    int      used_bytes;        // currently used bytes of data buffers
    int      buf_bytes;         // buffer total size in bytes;
    int      is_continous;      // continous memory allocation
    int      stereo_mode;       // stereo concat mode if stereo-frame
    uint32_t frame_index;
```

### uvc_user_start_func_t
<div id="uvc_user_start_func_t"></div>

#### 描述

    UVC起始帧的回调函数

#### 定义

```c
void (*uvc_user_start_func_t)(uvc_fmt_t fmt, int w, int h, int fps);
```

### uvc_register_user_stop_cb
<div id="uvc_register_user_stop_cb"></div>

#### 描述

    UVC起始帧的回调函数

#### 定义

```c
    void (*uvc_register_user_stop_cb)(uvc_fmt_t fmt, int w, int h, int fps);
```

## 例程

```C
/** 
 * 初始化UVC设备，并配置参数
*/
uvc_param_t uvc_param = UVC_PARAM_INITIALIZER;
uvc_param.ep0_desc_sel     = 0;
uvc_param.max_payload_size = 512;
/*  uvc_uac_isoc_gadget: 支持UAC UVC的同步传输USB驱动;
    uvc_isoc_camera: 仅支持UVC的同步传输的USB驱动;
    uvc_bulk_gadget: 仅支持UVC的批量传输的USB驱动;
*/
ret = uvc_init("uvc_uac_isoc_gadget", &uvc_param);

// 注册UVC回调函数
uvc_register_user_start_cb(user_start_cb);
uvc_register_user_stop_cb(user_stop_cb);

// 发送一帧数据
uvc_commit_frame(frame_buf);
```
