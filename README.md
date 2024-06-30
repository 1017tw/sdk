[SDK 使用文档](docs/readme.md)

## AIVA development tool script
The tool script provide quick start guide of this SDK to user. The users could select the frequencly used actions in development.

Script Usage:

```./aiva_dev_tool.sh "ttyPORT" "Project_Path"```

1. ttyPort: the tty port which connects to UART0 of target board
2. Project_Path: which is the root path of target project

For example, if we want to develop "ai_face_gimbal" project, and the target board connecnt to "/dev/ttyUSB0", then use this command:

```./aiva_dev_tool.sh /dev/ttyUSB0 examples/ai_face_gimbal```

## AIVA open sdk firmware downloader

### Usage of flash tool: aiva_uart_dl.py
```
$ python tools/aiva_uart_dl.py -h                                                                                     usage: aiva_uart_dl.py [-h] [-p PARTITION] [-f FILE] [-w] [-j JUMP]

AIVA open sdk firmware downloader.

options:
  -h, --help            show this help message and exit
  -p PARTITION, --partition PARTITION
                        the target partition to flash
  -f FILE, --file FILE  file to flash
  -w, --wait            wait bootloader start pattern
  -j JUMP, --jump JUMP  bootloader pc_jump to addr

Copyright(r), 2024

```
-p: target partiton to flash, see examples/ai_face_gimbal/cfg/part_nor.cfg to get avaiable partitions. Normally, the bootloader will try to load application from partition fw0, if load failed, it will try load from fw1.
-f: the target file to flash
-w: wait and stop in bootloader
-j: jump to application in partition fw1 or fw0

#### Flash application to fw1 backup partition through bootloader
```python tools/aiva_uart_dl.py -p fw1 -f examples/ttyLoader/bin/ttyloader.img -w```

#### Flash application to fw0 main partition through bootloader
```python tools/aiva_uart_dl.py -p fw0 -f examples/ai_face_gimbal/bin/owl.img -w```

#### Flash fw0 main application from fw1 loader application
```python tools/aiva_uart_dl.py -p fw0 -f examples/ai_face_gimbal/bin/owl.img -j 0x300e50a0```
Here, 0x300e50a0 is the start address of the fw1 loader application. 