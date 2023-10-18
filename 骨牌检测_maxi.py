import sensor, image
import lcd, time
import gc
import KPU as kpu
gc.enable()
import machine
from board import board_info
from fpioa_manager import fm
import utime
from machine import UART
lcd.init()
sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_windowing((224, 224))
sensor.set_hmirror(True)# 水平方向翻转
sensor.set_vflip(True)# 垂直方向翻转
sensor.run(1)     # Wait for settings take effect.
fm.register(8,fm.fpioa.UART2_RX,force=True)
fm.register(6,fm.fpioa.UART2_TX,force=True)
uart_A = UART(UART.UART2, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)
#clock = time.clock()                # Create a clock object to track the FPS.
task = kpu.load(0x300000)  # 加载 flash 中的模型
labels = ['红三角', '蓝三角', '红圆', '蓝圆']
anchors = (1.53, 2.47, 0.94, 1.53, 1.72, 2.81, 1.22, 1.97, 2.06, 3.03)
#task=kpu.load("/sd/KPU/model-58333.kmodel")
kpu.init_yolo2(task,0.5,0.3,len(anchors)//2,anchors)
enable_sensor=0;
uart_read=0;
uart_enable=0;
#kpu.memtest()
while True:
    #print("mem free:",gc.mem_free())
    #clock.tick()                    # Update the FPS clock.
    if uart_read!=b'~':
        uart_read=uart_A.read(1)
    if uart_read==b'~':
        img = sensor.snapshot()
        lcd.display(img)
        #lcd.display(img)
        #img = img.resize(224, 224)
        #img.pix_to_ai()
        dect = kpu.run_yolo2(task,img)
        #fps = clock.fps()
        if dect:
            for i in dect: # 多张
                print(i)   # 打印信息
                a = img.draw_rectangle(i.rect()) # 在图上框出
                img.draw_string(i.x(), i.y()-10,str(i.classid()), color=(0, 255, 0), scale=2.0)
                if i.classid()==3 or i.classid()==0 or i.classid()==1 or i.classid()==2:
                 uart_A.write(chr(100+i.classid()))
                 uart_read=0
                 #utime.sleep_ms(3000)
        lcd.display(img)
kpu.deinit()
