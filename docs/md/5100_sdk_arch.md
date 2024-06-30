

## SDK 目录结构介绍
``` bash
sdk-release-v1.3.0
├── docs                -- SDK 使用文档集合
│   ├── images
│   ├── md
│   ├── md_en
│   └── readme.md       -- 文档总目录
├── examples            -- 芯片的各种使用示例，包括一些实用示例。
├── gdbinit             -- GDB 初始化脚本
├── README.md
├── run_gdb.sh          -- 运行交叉编译的 GDB 程序
├── run_jlink.sh        -- 运行 JLINK 服务程序
├── scripts
│   └── gdb             -- GDB 相关的脚本
├── sdk -> sdk-v1.3.0
├── sdk-v1.3.0
│   ├── chipKconfig
│   ├── components          -- 向开发者开放的代码部分
│   │   ├── drivers         -- 摄像头驱动、flash 驱动、显示驱动、audio code 驱动、UVC device 驱动等
│   │   ├── Kconfig
│   │   ├── Makefile
│   │   └── shell_utils     -- letter-shell 相关的命令程序，通过 menuconfig 配置
│   ├── include             -- SDK 包含的各种头文件都在此目录
│   ├── Kconfig
│   ├── ldscript            -- 固件的 gcc 链接脚本
│   │   ├── ai1101a1.ld
│   │   └── ai11xx.ld -> ai1101a1.ld
│   ├── lib
│   │   ├── libai1101a1_sdk_647bda4.a           -- SDK 静态库，见本文档后面的详细介绍。
│   │   └── libai11xxsdk.a -> ./libai1101a1_sdk_647bda4.a
│   ├── scripts
│   │   ├── ai1101a1_Makefile.libflags          -- 编译时的宏定义
│   │   ├── ai1101a1_Makefile.libincs           -- 编译时头文件路径
│   │   ├── Makefile.libflags -> ai1101a1_Makefile.libflags
│   │   └── Makefile.libincs -> ai1101a1_Makefile.libincs
│   │
│   └── tools               -- SDK 包含的各种编译、格式转换、打包相关程序或脚本
│       ├── bin2image
│       ├── bin2ota
│       ├── bin2rom
│       ├── build.mk
│       ├── common_setting.mk
│       ├── menuconfig
│       ├── mklfs           -- little fs 文件系统工具
│       ├── packxfiles
│       ├── post_chip_select.sh
│       └── swapdw
├── serial_debug.sh         -- 串口 debug
└── setupenv.sh
```

## SDK 中 "libai31xxsdk.a" 包含功能
1. 支持 [FreeROTS](https://www.freertos.org/FreeRTOS-quick-start-guide.html)、[FreeROTS POSIX](https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_POSIX/index.html)
2. 各外设总线驱动支持，包括 I2C、SPI、UART、I2S、USB OTG、MIPI 等底层驱动的支持
3. 芯片核心功能模块支持，如 摄像头接入, XNN, MJPEG decoder, image rsz， image rotation 等
4. 集成了一些库，如 [Tiny AES in C](https://github.com/kokke/tiny-AES-c/), [jpeglib](https://www.ijg.org/), libcrc, [libgzip](https://www.gzip.org/), [zlib](https://zlib.net/), [little fs](https://os.mbed.com/docs/mbed-os/v6.16/apis/littlefilesystem.html), [fatfs](https://irtos.sourceforge.net/FAT32_ChaN/doc/00index_e.html)，[cJSON](https://github.com/DaveGamble/cJSON)，[letter-shell](https://github.com/NevermindZZT/letter-shell)

## SDK 包含代码模块 (sdk/components)
在 "sdk/components" 目录下，包含如下代码，可通过在工程目录下执行命令 "make menuconfig" 配置是否编译。
1. LVGL 库代码
2. 摄像头驱动、flash 驱动、显示驱动、audio code 驱动、UVC device 驱动等
3. letter-shell 相关的命令程序

## 编译系统
### Kconfig 配置系统
使用 [Kconfiglib](https://github.com/ulfalizer/Kconfiglib/tree/master) 实现配置：
1. 配置 sdk 中的模块（驱动、库等）是否被编译
2. 配置部分 C/C++ 代码中的宏定义
3. 其他编译、打包相关配置

### Makefile 编译
Makefile 文件，在 sdk 中的形式：
1. 在 example 目录下的 Makefile，作为项目的编译入口，如 ./examples/rtos/Makefile，在编译时，会引入 SDK 中的通用 makefile；同时加入本工程包含的源文件到编译列表；
2. "sdk/tools/common_setting.mk"，配置交叉编译工具，引入 Kconfig 配置生成的 .config ， 定义编译器选项等；
3. "sdk/tools/build.mk", 定义源文件到目标文件的编译规则。

### 链接脚本
sdk/ldscript/ai11xx.ld 为 sdk 提供的链接文件，在 sdk/tools/common_setting.mk 中定义 LDSCRIPT
``` makefile
LDSCRIPT:=$(SDK_ROOT)/ldscript/ai11xx.ld
```
最终 arm-none-eabi-ld 按照链接脚本组织链接固件。


## SDK 使用示例
请参考接口示例相关文档。

## 串口 debug
使用芯片串口 0；串口工具配置：波特率 460800, 8N1，关闭硬件和软件流控制。
建议使用串口工具 aiva_debug_tool、screen、minicom 等。

## 交叉环境 GDB 使用
请参考快速入门文档。
