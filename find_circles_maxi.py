# Find Circles Example
#
# This example shows off how to find circles in the image using the Hough
# Transform. https://en.wikipedia.org/wiki/Circle_Hough_Transform
#
# Note that the find_circles() method will only find circles which are completely
# inside of the image. Circles which go outside of the image/roi are ignored...
#初始化
import sensor, image, time , lcd, math ,array
import gc
from machine import UART
import machine
import utime
from board import board_info
from fpioa_manager import fm
gc.enable()
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(True) # must turn this off to prevent image washout...
sensor.set_hmirror(True)# 水平方向翻转
sensor.set_vflip(True)# 垂直方向翻转
clock = time.clock()
fm.register(8,fm.fpioa.UART2_RX,force=True)
fm.register(6,fm.fpioa.UART2_TX,force=True)
uart_A = UART(UART.UART2, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)
true_times=0;
threshold_number=2100;
while(True):
    clock.tick()
    img = sensor.snapshot()
    i=0;
    mid_point_x=0.0;
    mid_point_y=0.0;
    point_list_x=array.array("I")
    point_list_y=array.array("I")
    # Circle objects have four values: x, y, r (radius), and magnitude. The
    # magnitude is the strength of the detection of the circle. Higher is
    # better...

    # `threshold` controls how many circles are found. Increase its value
    # to decrease the number of circles detected...

    # `x_margin`, `y_margin`, and `r_margin` control the merging of similar
    # circles in the x, y, and r (radius) directions.

    # r_min, r_max, and r_step control what radiuses of circles are tested.
    # Shrinking the number of tested circle radiuses yields a big performance boost.
    for c in img.find_circles(threshold = threshold_number, x_margin = 10, y_margin = 10, r_margin = 10,
            r_min = 3, r_max = 5, r_step = 1):
        img.draw_circle(c.x(), c.y(), c.r(), color = (255, 255, 255))
        mid_point_x=mid_point_x+c.x()
        mid_point_y=mid_point_y+c.y()
        print(c)
        i=i+1;
        point_list_x.append(c.x());
        point_list_y.append(c.y());
    img.draw_string(0, 0, "%i" %(i), color=(255, 255, 255), scale=2.0)
    if i==8:
        mid_point_x=mid_point_x/8.0
        mid_point_y=mid_point_y/8.0
        img.draw_circle(int(mid_point_x), int(mid_point_y), 6, color = (255, 255, 255))
        print(mid_point_x)
        print(mid_point_y)
        if int(mid_point_x)<180 and int(mid_point_x)>140 and int(mid_point_y)>100 and int(mid_point_y)<140:
            true_times=true_times+1
    elif i>8:
        threshold_number=threshold_number+10;
    elif i<8:
        threshold_number=threshold_number-10;
    if i!=8 and true_times>=1:
        true_times=true_times-1;
    img.draw_string(0, 40, "%d" %(threshold_number), color=(255, 255, 255), scale=2.0)
    img.draw_rectangle(140,100,40,40,color=(255, 255, 255),thickness=2,fill=False)
    lcd.display(img)

    print("FPS %f" % clock.fps())
    print(i)
    if true_times==20:
        break;
#垃圾回收
gc.collect()

primary=[[217,62],[56,64],[171,86],[80,110],[240,131],[149,155],[104,178],[265,177]]
final=[]
for i in range(0,8):
    primary[i][0]=point_list_x[i];
    primary[i][1]=point_list_y[i];
def change(data):
    x=0
    y=0
    for i in range(len(data)):
        x += data[i][0]
        y += data[i][1]
    x=x/8
    y=y/8
    second=[]
    distance=0
    max=7.7
    for i in range(len(data)):
        second.append([data[i][0]-x,data[i][1]-y])
    x = 0
    y = 0
    flag=0
    mid=second
    for i in range(len(second)):
        if math.sqrt((second[i][0]-x)**2+(second[i][1]-y)**2)>distance:
            distance=math.sqrt((second[i][0]-x)**2+(second[i][1]-y)**2)
            flag=i

    for j in range(1,20):
        if distance<max:
            break
        else:
            for i in range(len(second)):
                second[i][0] =mid[i][0]/j
                second[i][1] =mid[i][1]/j
            distance=math.sqrt((second[flag][0]-x)**2+(second[flag][1]-y)**2)

    for i in range(len(second)):
        second[i][0] = round(second[i][0]+5.5)
        second[i][1] = round(second[i][1]+5.5)-1
    for i in range(len(second)):
        second[i][1] = 10-second[i][1]
    print(second)
    return second
final=change(primary)
uart_A.write(chr(127))
for i in range (0,8):
#final[i][1]-1)*10+final[i][0]
    num=(final[i][1]-1)*10+final[i][0]
    #chr(253)为标志位
    uart_A.write(chr(num))
    img.draw_string(final[i][0]*24, (10-final[i][1])*24, "%d %d" %(final[i][0],final[i][1]), color=(255, 255, 255), scale=2.0)

lcd.display(img)
utime.sleep_ms(30000)
gc.collect()


import sensor, image
import lcd, time
import gc
import KPU as kpu
gc.enable()
enable_sensor=0;
uart_read=0;
lcd.init()
sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_windowing((224, 224))
sensor.set_hmirror(True)# 水平方向翻转
sensor.set_vflip(True)# 垂直方向翻转
sensor.run(1)     # Wait for settings take effect.
#clock = time.clock()                # Create a clock object to track the FPS.
task = kpu.load(0x300000)  # 加载 flash 中的模型
labels = ['红三角', '蓝三角', '红圆', '蓝圆']
anchors = (1.53, 2.47, 0.94, 1.53, 1.72, 2.81, 1.22, 1.97, 2.06, 3.03)
#task=kpu.load("/sd/KPU/model-58333.kmodel")
kpu.init_yolo2(task,0.4,0.3,len(anchors)//2,anchors)
#kpu.memtest()
true_flag=0;
true_0_flag=0;
true_1_flag=0;
true_2_flag=0;
true_3_flag=0;

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
                if (i.classid()==3 or i.classid()==0 or i.classid()==1 or i.classid()==2):
                    if(i.classid()==0):
                        true_0_flag=true_0_flag+1;
                        true_1_flag=true_1_flag-1;
                        true_2_flag=true_2_flag-1;
                        true_3_flag=true_3_flag-1;
                    elif(i.classid()==1):
                        true_0_flag=true_0_flag-1;
                        true_1_flag=true_1_flag+1;
                        true_2_flag=true_2_flag-1;
                        true_3_flag=true_3_flag-1;
                    elif(i.classid()==2):
                            true_0_flag=true_0_flag-1;
                            true_1_flag=true_1_flag-1;
                            true_2_flag=true_2_flag+1;
                            true_3_flag=true_3_flag-1;
                    elif(i.classid()==3):
                            true_0_flag=true_0_flag-1;
                            true_1_flag=true_1_flag-1;
                            true_2_flag=true_2_flag-1;
                            true_3_flag=true_3_flag+1;
                    #true_flag=true_flag+1
                    if(true_0_flag<0):
                        true_0_flag=0
                    if(true_1_flag<0):
                        true_1_flag=0
                    if(true_2_flag<0):
                        true_2_flag=0
                    if(true_3_flag<0):
                        true_3_flag=0
                    if(true_0_flag==5 or true_1_flag==5 or true_2_flag==5 or true_3_flag==5):
                        uart_A.write(chr(100+i.classid()))
                        uart_read=0
                        true_0_flag=0;
                        true_1_flag=0;
                        true_2_flag=0;
                        true_3_flag=0;
                        #true_flag=0;
                    #utime.sleep_ms(3000)
        #else:
            #true_flag=true_flag-1;
        lcd.display(img)
kpu.deinit()
