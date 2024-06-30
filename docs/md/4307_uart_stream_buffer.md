# 带缓存 UART 操作示例

## 示例解决的问题
由于底层串口接口往往只是对 UART 控制器的读写操作，以接收或者发送数据。但实际的系统中，有多个任务在运行，所以需要一个更好的机制，来发送或者接收串口数据。
这样做的好处如下：
1. 更好的管理数据，不易丢失数据；
2. 更好的做多任务处理，且程序结构清晰；

## 实现方式描述
示例见 “examples/uart/uart_stream_buffer.c”，实现了异步接收、发送串口数据，大体逻辑如下：
1. 开一个串口接收线程，将数据通过函数 xStreamBufferSend() 写入 RxStreamBuffer 缓存起来；
2. 程序应用逻辑通过调用函数 xStreamBufferReceive() 获取 RxSteamBuffer 中的内容，做相应的解析处理；
3. 开一个串口发送线程，通过函数 xStreamBufferReceive() 获取 TxSteamBuffer 的内容，并调用串口发送 API 实际发送串口数据；
4. 程序应用逻辑通过调用函数 xStreamBufferSend() 将希望发送的数据写入 TxStreamBuffer。


## 示例改善思考
1. 串口接收，使用中断或者 DMA 的方式，替代串口读取线程，提升效率。
2. 串口接收，使用中断或者 DMA 的方式，替代串口发送线程，提升效率。