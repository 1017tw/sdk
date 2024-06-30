
# Ai1101 Overview

### 应用处理器（AP）
* ARM Contex-A5 @400MHz
* 32KB I Cache + 32KB D-Cache
* NEON + FPU
* 最大支持 64MB DRAM(Embedded)

### 神经网络处理器（XNN）
* 最大算力 Int8: 0.5TOPS
* 可配置支持多种神经网络
* 支持 PyTorch/Onnx/Caffe 等模型

### 图像信号处理器 (VPU)
* 图像缩放加速引擎(RSZ)
  * 支持 256x 图像放大
  * 支持 256x 图像缩小
* 图像旋转加速引擎(ROT)
  * 支持 0/90/180/270 度旋转
  * 支持图像水平/垂直翻转
* 3D DMA
  * 支持三维图像数据拷贝

### MJPEG 硬件解码器（MJPEG-DEC）
* 兼容 JFIF 1.02
* 支 持 YUV444, YUV422H, YUV422V, YUV420 等格式
* 最大分辨率支持 1920x1080
* VGA@180fps 720p@60fps 1080p@30fps

### 外部存储
* QSPI NOR/NAND flash
* XIP 支持

### 内置 PSRAM
* 4MB、4MB+4MB并行、8MB，三种规格可选

### 视频输入接口（Video IN）
* MIPI CSI2 RX 2-lane @ 1.5Gbps x2

### 视频输出接口（Video OUT）
* 8/16 bits DVP 并行输出
* I8080 LCD 8/16 bits RGB 输出

### USB2.0 OTG 接口
* Host/Device 模式可配置
* Host 模式支持 EHCI 协议
* Device 模式支持最大 16 个 EP
* High-Speed(HS, 480Mbps)
* Full-Speed(FS, 12Mbps)
* Low-Speed(LS, 1.5Mbps)

### 操作系统
* FreeRTOS

### 固件加载
* Boot ROM 从 QSPI0 启动加载
* 4-line @Max 100MHz

### 外设接口
* QSPI Master x2
* SPI Master x1
* MIPI-CSI x2
* I2S x1
* I2C x6
* UART x9
* PWM x12
* PWC x6
* Timer x3
* GPIO x70
