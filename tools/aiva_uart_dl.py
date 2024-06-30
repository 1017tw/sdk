import time
import sys
import serial
import argparse
from YModem import YModem

parser = argparse.ArgumentParser(description=f"AIVA open sdk firmware downloader.", epilog='Copyright(r), 2024')

parser.add_argument("-p", "--partition", type=str, help="the target partition to flash")
parser.add_argument("-f", "--file", type=str, help="file to flash")
parser.add_argument('-w', '--wait', action='store_true', help='wait bootloader start pattern')
parser.add_argument("-j", "--jump", type=str, help="bootloader pc_jump to addr")

# Parse the arguments
args = parser.parse_args()

if not args.partition or not args.file:
    print("usage error")
    parser.print_help()
    sys.exit(-1)

def wait_pattern(serial_io, pattern, timeout):
    print(f'waiting \"{pattern}\" match.....')
    for k in range(timeout) :
        resp = serial_io.read_until()
        try:
            print(f'- {resp.decode()}', end="")
        except:
            pass
        if pattern.encode() in resp:
            serial_io.write("\r\n".encode())
            print(f'resp matched....ok')
            return True

    # print(f'k {k}')
    if k + 1 == timeout:
        print("timeout....")
        return False

'''
下载底层或脚本
'''
def _dl(tp, _path=None):
    serial_io = serial.Serial()
    serial_io.port = '/dev/ttyUSB0'
    serial_io.baudrate = "460800"
    serial_io.parity = "N"
    serial_io.bytesize = 8
    serial_io.stopbits = 1
    serial_io.timeout = 2
    #serial_io.rtscts = 1
    try:
        serial_io.open()
    except Exception as e:
        raise Exception("Failed to open serial port!")
    def sender_getc(size):
        return serial_io.read(size) or None
    def sender_putc(data, timeout=15):
        return serial_io.write(data)
    ## 适配CH340的RTS接到W600的RTS脚
    ## 如果无法下载, 先尝试手动复位模块, 还是不行的话, 把rts的值从当前的 1和0 改成 0和1
    serial_io.rts = 1
    time.sleep(0.5)
    serial_io.rts = 0

    if args.wait:
        ret = wait_pattern(serial_io, 'shell...', 1000000)
        if not ret:
            print('timeout, exit...')
            sys.exit(-1)

    if args.jump:
        '''
        fw0     	0x00010000  0x000d5000
        fw1     	0x000e5000  0x0005a000
        pc_jump 0x300100A0
        pc_jump 0x300e50A0
        '''
        serial_io.write(f"pc_jump {args.jump}\r\n".encode())
        ret = wait_pattern(serial_io, 'aiva:/$', 50)
        if not ret:
            print('timeout, exit...')
            sys.exit(-1)

    if tp == "xnn":
        serial_io.write("load part xnn_wgt\r\n".encode())
    elif tp == "fw0":
        serial_io.write("load part fw0\r\n".encode())
    elif tp == "fw1":
        serial_io.write("load part fw1\r\n".encode())


    ''' wait for device ready '''
    ret = wait_pattern(serial_io, 'C', 10)
    if not ret:
        print('timeout, exit...')
        sys.exit(-1)


    time.sleep(0.5)
    print(f"\n>> 开始 Ymodem 发送文件: {_path}")

    serial_io.write("ry\r\n".encode())
    serial_io.read_until()
    sender = YModem(sender_getc, sender_putc)
    sent = sender.send_file(_path, retry=1)
    print (f"\n>> total {sent} bytes sent")

    for k in range(5) :
        resp = serial_io.read_until()
        print(f'- {resp.decode()}', end="")
        if 'K>'.encode() in resp or 'Verify end: SUCCESS!'.encode() in resp:
            print(f'\nDone!')
            break

    serial_io.close()

# _dl("xnn", 'examples/ai_face_gimbal/xnn_models/facelock_sram_0KB_v0.0.4_db920b9.xpack')
# _dl("fw0", 'examples/ai_face_gimbal/bin/owl.img')
_dl (args.partition, args.file)
