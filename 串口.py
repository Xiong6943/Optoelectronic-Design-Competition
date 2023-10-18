# Untitled - By: Xjj69 - 周日 7月 2 2023

import sensor, image, time
from machine import UART
import machine
from board import board_info
from fpioa_manager import fm
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()
fm.register(6,fm.fpioa.UART2_TX)
fm.register(8,fm.fpioa.UART2_RX)
uart_A = UART(UART.UART1, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)


while(True):
    clock.tick()
    img = sensor.snapshot()
    print(clock.fps())
