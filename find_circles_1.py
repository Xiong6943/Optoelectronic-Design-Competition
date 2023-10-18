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
from modules import ybkey
KEY = ybkey()
from modules import ybrgb
RGB = ybrgb()
gc.enable()
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(True) # must turn this off to prevent image washout...
clock = time.clock()


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
    for c in img.find_circles(threshold = 2500, x_margin = 10, y_margin = 10, r_margin = 10,
            r_min = 4, r_max = 6, r_step = 1):
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
        RGB.set(0,255,0)
    else:
        RGB.set(0,0,0)
    lcd.display(img)
    print("FPS %f" % clock.fps())
    print(i)
    if KEY.is_press() and i==8:
        RGB.set(0,0,0)
        break
#垃圾回收
gc.collect()
import  math
primary=[[217,62],[56,64],[171,86],[80,110],[240,131],[149,155],[104,178],[265,177]]

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
        second[i][1] = round(second[i][1]+5.5)
    for i in range(len(second)):
        second[i][1] = 10-second[i][1]
    print(second)
change(primary)
