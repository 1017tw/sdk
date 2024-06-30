usb 需要暴露什么样的接口？

看了 usb 的头文件，
usb_ep0.h
usb_func.h

没有什么有意义的接口。

usb.h 主要是结构体定义。

考虑 usb device 开放到什么程度，太复杂不容易支持。
1. 实现了 uvc、uac，开放uvc、uac 功能
2. 开放 ep0 相关的接口
3. 其它功能暂不开放。