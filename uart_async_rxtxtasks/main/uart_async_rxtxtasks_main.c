/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include<stdio.h>
#include<math.h>
#include<string.h>
//#include"rtwtypes.h"
//#include"tmwtypes.h"
#include"Output_path_allpoint_types.h"
#include"Output_path_allpoint_emxutil.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
//#include "output_path.c"
static const int RX_BUF_SIZE = 1024;
char* data_stm32_recive;
char* data_k210_recive;
char* receive_point_list;
uint8_t* receive_point_list_uint8;
uint8_t* output_movepath;
uint8_t* deposits_list;
uint8_t deposits_list_new[8]={0};
uint8_t output_movepath_len;
char* output_movepath_char;
uint8_t stay_min_path=0;
TaskHandle_t rx_task_handle;
TaskHandle_t tx_task_handle;
TaskHandle_t rx_task_handle_stm32;
TaskHandle_t tx_task_handle_stm32;
TaskHandle_t get_past_task_handle;
TaskHandle_t tx_task_handle_stm32_true_treasure;
TaskHandle_t tx_task_handle_stm32_fake_treasure;

const char true_tresure[]={124,251,0};
const char fake_tresure[]={123,251,0};
const char enable_ai_sensor[]={126,0};
//与K210通信所用串口
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)
#define blue_team_pin (GPIO_NUM_8)
const int uart_num = UART_NUM_1;
uint8_t is_red_team=0;
uint8_t is_blue_team=0;
//与STM32通信所用串口
#define TXD_PIN_stm32 (GPIO_NUM_6)
#define RXD_PIN_stm32 (GPIO_NUM_7)
const int uart_num_stm32 = UART_NUM_2;
//保存所有点状态的二维数组
//【0-9存顺序】
//【0-5存状态】
//第0位点数
//第1位红蓝，1红2蓝0未知
//第2位真假，1真2假0未知
//第3位是否经过，1经过0没经过
//第4位象限，1234对应1234象限
//第5位保留
//第6位是否确认，1确认，0未确认，当真假和红蓝都确认了就是确认，经过也是确认
uint8_t simple_path_all_state[10][7]={0};
//已经到达的宝藏数
uint8_t found_count=0;
uint8_t point_now=0;
uint8_t blue_true_treasure=0;
uint8_t red_true_treasure=0;

void calulate_simple_path_all_state(void);
void print_simple_path_all_state(void);
void tx_task_stm32_true_treasure(void *arg);
void tx_task_stm32_true_treasure(void *arg);
static void merge(int16_T idx[8], uint8_T x[8], int16_T iwork[8], uint8_T xwork[8])
{
    int16_T iout;
    int16_T j;
    int16_T p;
    int16_T q;
    for (j = 0; j < 8; j++) {
        iwork[j] = idx[j];
        xwork[j] = x[j];
    }
    p = 0;
    q = 4;
    iout = -1;
    int32_T exitg1;
    do {
        exitg1 = 0L;
        iout++;
        if (xwork[p] <= xwork[q]) {
            idx[iout] = iwork[p];
            x[iout] = xwork[p];
            if ((p + 1) < 4) {
                p++;
            }
            else {
                exitg1 = 1L;
            }
        }
        else {
            idx[iout] = iwork[q];
            x[iout] = xwork[q];
            if ((q + 1) < 8) {
                q++;
            }
            else {
                q = iout - p;
                for (j = p + 1; j < 5; j++) {
                    iout = q + j;
                    idx[iout] = iwork[j - 1];
                    x[iout] = xwork[j - 1];
                }
                exitg1 = 1L;
            }
        }
    } while (exitg1 == 0L);
}
static void sort(uint8_T x[8])
{
  int16_T idx[8];
  int16_T iwork[8];
  int16_t b_i;
  int16_t b_i1;
  int16_t i1;
  int16_t i2;
  int16_t i3;
  int16_t i4;
  int8_T idx4[4];
  int8_T b_i2;
  int8_T b_i3;
  int8_T b_i4;
  int8_T i5;
  uint8_T xwork[8];
  uint8_T x4[4];
  uint8_T u;
  uint8_T u1;
  uint8_T u2;
  uint8_T u3;
  idx4[0] = 1;
  idx4[1] = 2;
  idx4[2] = 3;
  idx4[3] = 4;
  u = x[0];
  x4[0] = u;
  u1 = x[1];
  x4[1] = u1;
  u2 = x[2];
  x4[2] = u2;
  u3 = x[3];
  x4[3] = u3;
  if (u <= u1) {
    i1 = 1;
    i2 = 2;
  } else {
    i1 = 2;
    i2 = 1;
  }
  if (u2 <= u3) {
    i3 = 3;
    i4 = 4;
  } else {
    i3 = 4;
    i4 = 3;
  }
  b_i = (int16_t)x4[i1 - 1];
  b_i1 = (int16_t)x4[i3 - 1];
  if (b_i <= b_i1) {
    b_i = (int16_t)x4[i2 - 1];
    if (b_i <= b_i1) {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i2;
      b_i4 = (int8_T)i3;
      i5 = (int8_T)i4;
    } else if (b_i <= ((int16_t)x4[i4 - 1])) {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i3;
      b_i4 = (int8_T)i2;
      i5 = (int8_T)i4;
    } else {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i3;
      b_i4 = (int8_T)i4;
      i5 = (int8_T)i2;
    }
  } else {
    b_i1 = (int16_t)x4[i4 - 1];
    if (b_i <= b_i1) {
      if (((int16_t)x4[i2 - 1]) <= b_i1) {
        b_i2 = (int8_T)i3;
        b_i3 = (int8_T)i1;
        b_i4 = (int8_T)i2;
        i5 = (int8_T)i4;
      } else {
        b_i2 = (int8_T)i3;
        b_i3 = (int8_T)i1;
        b_i4 = (int8_T)i4;
        i5 = (int8_T)i2;
      }
    } else {
      b_i2 = (int8_T)i3;
      b_i3 = (int8_T)i4;
      b_i4 = (int8_T)i1;
      i5 = (int8_T)i2;
    }
  }
  idx[0] = (int16_t)idx4[b_i2 - 1];
  idx[1] = (int16_t)idx4[b_i3 - 1];
  idx[2] = (int16_t)idx4[b_i4 - 1];
  idx[3] = (int16_t)idx4[i5 - 1];
  x[0] = x4[b_i2 - 1];
  x[1] = x4[b_i3 - 1];
  x[2] = x4[b_i4 - 1];
  x[3] = x4[i5 - 1];
  idx4[0] = 5;
  idx4[1] = 6;
  idx4[2] = 7;
  idx4[3] = 8;
  u = x[4];
  x4[0] = u;
  u1 = x[5];
  x4[1] = u1;
  u2 = x[6];
  x4[2] = u2;
  u3 = x[7];
  x4[3] = u3;
  if (u <= u1) {
    i1 = 1;
    i2 = 2;
  } else {
    i1 = 2;
    i2 = 1;
  }
  if (u2 <= u3) {
    i3 = 3;
    i4 = 4;
  } else {
    i3 = 4;
    i4 = 3;
  }
  b_i = (int16_t)x4[i1 - 1];
  b_i1 = (int16_t)x4[i3 - 1];
  if (b_i <= b_i1) {
    b_i = (int16_t)x4[i2 - 1];
    if (b_i <= b_i1) {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i2;
      b_i4 = (int8_T)i3;
      i5 = (int8_T)i4;
    } else if (b_i <= ((int16_t)x4[i4 - 1])) {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i3;
      b_i4 = (int8_T)i2;
      i5 = (int8_T)i4;
    } else {
      b_i2 = (int8_T)i1;
      b_i3 = (int8_T)i3;
      b_i4 = (int8_T)i4;
      i5 = (int8_T)i2;
    }
  } else {
    b_i1 = (int16_t)x4[i4 - 1];
    if (b_i <= b_i1) {
      if (((int16_t)x4[i2 - 1]) <= b_i1) {
        b_i2 = (int8_T)i3;
        b_i3 = (int8_T)i1;
        b_i4 = (int8_T)i2;
        i5 = (int8_T)i4;
      } else {
        b_i2 = (int8_T)i3;
        b_i3 = (int8_T)i1;
        b_i4 = (int8_T)i4;
        i5 = (int8_T)i2;
      }
    } else {
      b_i2 = (int8_T)i3;
      b_i3 = (int8_T)i4;
      b_i4 = (int8_T)i1;
      i5 = (int8_T)i2;
    }
  }
  idx[4] = (int16_t)idx4[b_i2 - 1];
  idx[5] = (int16_t)idx4[b_i3 - 1];
  idx[6] = (int16_t)idx4[b_i4 - 1];
  idx[7] = (int16_t)idx4[i5 - 1];
  x[4] = x4[b_i2 - 1];
  x[5] = x4[b_i3 - 1];
  x[6] = x4[b_i4 - 1];
  x[7] = x4[i5 - 1];
  for (i1 = 0; i1 < 8; i1++) {
    iwork[i1] = 0;
    xwork[i1] = 0U;
  }
  merge(idx, x, iwork, xwork);
}
void tx_task(void *arg);
void tx_task_stm32(void *arg);
int sendData_stm32(const char* logName, const char* data);
void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    const gpio_config_t blue_or_red={
        .pin_bit_mask=(1ULL<<blue_team_pin),
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_DISABLE,
    };
    data_k210_recive = (char*) malloc(RX_BUF_SIZE+1);
    data_stm32_recive = (char*) malloc(RX_BUF_SIZE+1);
    receive_point_list=(char*)malloc(8);
    receive_point_list_uint8=(uint8_t*)malloc(8);
    deposits_list=(uint8_t*)malloc(8);
    output_movepath=(uint8_t*)malloc(1000);
    output_movepath_char=(char*)malloc(1001);
    output_movepath_len=0;
    // We won't use a buffer for sending data.
    gpio_config(&blue_or_red);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num_stm32, &uart_config);
    uart_set_pin(uart_num_stm32, TXD_PIN_stm32, RXD_PIN_stm32, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num_stm32, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}
real_T rtNaN = (real_T)NAN;
real_T rtInf = (real_T)INFINITY;
real_T rtMinusInf = -(real_T)INFINITY;
real32_T rtNaNF = (real32_T)NAN;
real32_T rtInfF = (real32_T)INFINITY;
real32_T rtMinusInfF = -(real32_T)INFINITY;
typedef struct {
    int16_T heap[100];
    int16_T indexToHeap[100];
    int16_T len;
} c_matlab_internal_coder_minPrio;
boolean_T rtIsInf(real_T value)
{
    return (isinf(value) != 0U);
}
boolean_T rtIsInfF(real32_T value)
{
    return (isinf((real_T)value) != 0U);
}
boolean_T rtIsNaN(real_T value)
{
    return (isnan(value) != 0U);
}
boolean_T rtIsNaNF(real32_T value)
{
    return (isnan((real_T)value) != 0U);
}
static int16_T asr_s16(int16_T u, uint16_T n)
{
    int16_T y;
    if (u >= 0) {
        y = (int16_T)((uint16_T)(((uint16_T)u) >> ((uint32_T)n)));
    }
    else {
        y = (-((int16_T)((uint16_T)(((uint16_T)((int16_T)(-1 - u))) >>
            ((uint32_T)n))))) -
            1;
    }
    return y;
}
static void minPriorityQueue_percUp(c_matlab_internal_coder_minPrio* obj,int16_T b_i, const real_T dist[100]){
    int16_T iparent;
    boolean_T exitg1;
    iparent = (b_i / 2) - 1;
    exitg1 = false;
    while ((!exitg1) && ((iparent + 1) > 0)) {
        real_T b_d1;
        real_T d;
        int16_T obj_idx_1;
        obj_idx_1 = obj->heap[b_i - 1];
        d = dist[obj_idx_1 - 1];
        b_d1 = dist[obj->heap[iparent] - 1];
        if ((d < b_d1) || ((d == b_d1) && (obj_idx_1 <= obj->heap[iparent]))) {
            obj->heap[b_i - 1] = obj->heap[iparent];
            obj->heap[iparent] = obj_idx_1;
            obj_idx_1 = obj->indexToHeap[obj->heap[b_i - 1] - 1];
            obj->indexToHeap[obj->heap[b_i - 1] - 1] =
                obj->indexToHeap[obj->heap[iparent] - 1];
            obj->indexToHeap[obj->heap[iparent] - 1] = obj_idx_1;
            b_i = iparent + 1;
            iparent = ((iparent + 1) / 2) - 1;
        }
        else {
            exitg1 = true;
        }
    }
}
static const uint8_T uv[101] = {
    0U,   1U,   3U,   5U,   7U,   9U,   11U,  14U,  16U,  19U,  21U,  23U,
    25U,  27U,  29U,  32U,  35U,  37U,  39U,  40U,  42U,  44U,  46U,  47U,
    50U,  52U,  54U,  56U,  60U,  62U,  63U,  64U,  66U,  69U,  71U,  72U,
    74U,  77U,  79U,  81U,  83U,  86U,  88U,  91U,  94U,  96U,  98U,  101U,
    103U, 104U, 106U, 108U, 109U, 111U, 114U, 116U, 118U, 121U, 124U, 126U,
    129U, 131U, 133U, 135U, 138U, 140U, 141U, 143U, 146U, 148U, 149U, 150U,
    152U, 156U, 158U, 160U, 162U, 165U, 166U, 168U, 170U, 172U, 173U, 175U,
    177U, 180U, 183U, 185U, 187U, 189U, 191U, 193U, 196U, 198U, 201U, 203U,
    205U, 207U, 209U, 211U, 212U };
static real_T rt_roundd_snf(real_T u){
    real_T y;
    if (fabs(u) < 4.503599627370496E+15) {
        if (u >= 0.5) {
            y = floor(u + 0.5);
        }
        else if (u > -0.5) {
            y = u * 0.0;
        }
        else {
            y = ceil(u - 0.5);
        }
    }
    else {
        y = u;
    }
    return y;
}
static void graphBase_shortestpath(uint8_T s, uint8_T t, emxArray_real_T* path){
    static const int8_T ir[212] = {
        2,  1,  12, 4,  13, 3,  5,  4,  15, 7,  16, 6,  8,   17, 7,  9,  8,  10,
        19, 9,  20, 12, 21, 2,  11, 3,  23, 15, 24, 5,  14,  16, 6,  15, 26, 7,
        18, 17, 28, 9,  10, 30, 11, 22, 21, 32, 13, 14, 25,  34, 24, 26, 16, 25,
        28, 37, 18, 27, 29, 38, 28, 39, 20, 41, 22, 33, 32,  34, 43, 24, 33, 36,
        35, 37, 27, 36, 47, 28, 48, 29, 40, 39, 50, 31, 42,  51, 41, 43, 33, 42,
        44, 43, 45, 54, 44, 46, 45, 47, 37, 46, 57, 38, 49,  48, 40, 60, 41, 61,
        53, 52, 63, 44, 55, 64, 54, 56, 55, 57, 47, 56, 58,  57, 59, 68, 58, 60,
        50, 59, 70, 51, 62, 61, 72, 53, 73, 54, 65, 74, 64,  66, 65, 68, 77, 58,
        67, 69, 68, 79, 60, 81, 62, 73, 63, 72, 74, 83, 64,  73, 76, 85, 75, 77,
        67, 76, 87, 88, 69, 80, 79, 90, 71, 91, 92, 73, 84,  83, 94, 75, 86, 95,
        85, 87, 96, 77, 86, 78, 98, 90, 99, 80, 89, 81, 92,  82, 91, 93, 92, 94,
        84, 93, 95, 85, 94, 86, 97, 96, 98, 88, 97, 89, 100, 99 };
    c_matlab_internal_coder_minPrio queue;
    real_T dist[100];
    real_T pred[100];
    real_T tmp;
    real_T tnext;
    real_T* path_data;
    int16_T b_i;
    int16_T c_i;
    int16_T ichild;
    int16_T n;
    int16_T nd2;
    char_T colors[100];
    boolean_T inTargetSubset[100];
    boolean_T exitg1;
    for (b_i = 0; b_i < 100; b_i++) {
        inTargetSubset[b_i] = false;
        pred[b_i] = rtNaN;
        dist[b_i] = rtInf;
        colors[b_i] = '\x00';
        queue.heap[b_i] = 0;
        queue.indexToHeap[b_i] = 0;
    }
    inTargetSubset[((int16_T)t) - 1] = true;
    pred[((int16_T)s) - 1] = 0.0;
    dist[((int16_T)s) - 1] = 0.0;
    colors[((int16_T)s) - 1] = '\x01';
    queue.len = 1;
    queue.heap[0] = (int16_T)s;
    queue.indexToHeap[((int16_T)s) - 1] = 1;
    minPriorityQueue_percUp(&queue, 1, dist);
    exitg1 = false;
    while ((!exitg1) && (queue.len != 0)) {
        int16_T u;
        u = queue.heap[0] - 1;
        c_i = queue.len - 1;
        queue.heap[0] = queue.heap[queue.len - 1];
        queue.indexToHeap[0] = queue.heap[queue.len - 1];
        queue.len--;
        b_i = 0;
        int32_T exitg2;
        do {
            exitg2 = 0L;
            ichild = (b_i + 1) * 2;
            if (ichild <= c_i) {
                if ((ichild + 1) > c_i) {
                    ichild--;
                }
                else {
                    nd2 = queue.heap[ichild - 1];
                    tmp = dist[nd2 - 1];
                    tnext = dist[queue.heap[ichild] - 1];
                    if ((tmp < tnext) ||
                        ((tmp == tnext) && (nd2 <= queue.heap[ichild]))) {
                        ichild--;
                    }
                }
                tmp = dist[queue.heap[b_i] - 1];
                tnext = dist[queue.heap[ichild] - 1];
                if ((tmp < tnext) ||
                    ((tmp == tnext) && (queue.heap[b_i] <= queue.heap[ichild]))) {
                    exitg2 = 1L;
                }
                else {
                    n = queue.heap[b_i];
                    queue.heap[b_i] = queue.heap[ichild];
                    queue.heap[ichild] = n;
                    n = queue.indexToHeap[queue.heap[b_i] - 1];
                    queue.indexToHeap[queue.heap[b_i] - 1] =
                        queue.indexToHeap[queue.heap[ichild] - 1];
                    queue.indexToHeap[queue.heap[ichild] - 1] = n;
                    b_i = ichild;
                }
            }
            else {
                exitg2 = 1L;
            }
        } while (exitg2 == 0L);
        colors[u] = '\x02';
        if (inTargetSubset[u]) {
            for (nd2 = 0; nd2 < c_i; nd2++) {
                pred[queue.heap[nd2] - 1] = rtNaN;
            }
            exitg1 = true;
        }
        else {
            int8_T outNodes_data[212];
            c_i = (int16_T)uv[u + 1];
            if ((((int16_T)uv[u]) + 1) > c_i) {
                nd2 = 0;
                c_i = 0;
            }
            else {
                nd2 = (int16_T)uv[u];
            }
            n = c_i - nd2;
            for (c_i = 0; c_i < n; c_i++) {
                outNodes_data[c_i] = ir[nd2 + c_i];
            }
            for (nd2 = 0; nd2 < n; nd2++) {
                char_T c;
                int8_T i1;
                i1 = outNodes_data[nd2];
                c = colors[i1 - 1];
                if (((int8_T)c) == '\x00') {
                    colors[i1 - 1] = '\x01';
                    dist[i1 - 1] = dist[u] + 1.0;
                    pred[i1 - 1] = (real_T)((int16_T)(u + 1));
                    queue.len++;
                    queue.heap[queue.len - 1] = (int16_T)i1;
                    queue.indexToHeap[i1 - 1] = queue.len;
                    minPriorityQueue_percUp(&queue, queue.len, dist);
                }
                else if (((int8_T)c) == '\x01') {
                    tmp = dist[i1 - 1];
                    if (((dist[u] + 1.0) < tmp) ||
                        (((dist[u] + 1.0) == tmp) &&
                            ((((real_T)((int16_T)(u + 1))) + 1.0) < pred[i1 - 1]))) {
                        dist[i1 - 1] = dist[u] + 1.0;
                        pred[i1 - 1] = (real_T)((int16_T)(u + 1));
                        minPriorityQueue_percUp(&queue, queue.indexToHeap[i1 - 1], dist);
                    }
                }
                else {
                    /* no actions */
                }
            }
        }
    }
    tmp = (real_T)t;
    path->size[0] = 1;
    path->size[1] = 0;
    tnext = pred[((int16_T)t) - 1];
    if (!rtIsNaN(tnext)) {
        while (tnext != 0.0) {
            c_i = path->size[1];
            nd2 = path->size[0] * path->size[1];
            path->size[1]++;
            emxEnsureCapacity_real_T(path, nd2);
            path_data = path->data;
            path_data[c_i] = tmp;
            tmp = tnext;
            tnext = pred[((int16_T)tnext) - 1];
        }
        c_i = path->size[1];
        nd2 = path->size[0] * path->size[1];
        path->size[1]++;
        emxEnsureCapacity_real_T(path, nd2);
        path_data = path->data;
        path_data[c_i] = tmp;
        if (path->size[1] > 1) {
            n = path->size[1] - 1;
            nd2 = asr_s16(path->size[1], 1U);
            for (b_i = 0; b_i < nd2; b_i++) {
                tmp = path_data[b_i];
                ichild = n - b_i;
                path_data[b_i] = path_data[ichild];
                path_data[ichild] = tmp;
            }
        }
    }
}
void Output_path_allpoint(void *arg){
    static const uint8_t b_uv[10000] = {
        0U,  1U,  14U, 13U, 12U, 13U, 14U, 15U, 16U, 17U, 3U,  2U,  15U, 10U, 11U,
        12U, 15U, 16U, 17U, 18U, 4U,  5U,  16U, 9U,  10U, 11U, 14U, 15U, 16U, 19U,
        11U, 6U,  7U,  8U,  15U, 14U, 13U, 16U, 17U, 18U, 10U, 9U,  8U,  9U,  10U,
        11U, 12U, 17U, 18U, 17U, 11U, 16U, 15U, 10U, 11U, 12U, 13U, 14U, 15U, 16U,
        12U, 13U, 14U, 11U, 12U, 13U, 16U, 15U, 16U, 17U, 21U, 14U, 13U, 12U, 19U,
        18U, 17U, 24U, 17U, 18U, 20U, 19U, 14U, 15U, 18U, 19U, 18U, 23U, 20U, 19U,
        19U, 18U, 17U, 16U, 17U, 20U, 21U, 22U, 21U, 22U, 1U,  0U,  13U, 12U, 11U,
        12U, 13U, 14U, 15U, 16U, 2U,  1U,  14U, 9U,  10U, 11U, 14U, 15U, 16U, 17U,
        3U,  4U,  15U, 8U,  9U,  10U, 13U, 14U, 15U, 18U, 10U, 5U,  6U,  7U,  14U,
        13U, 12U, 15U, 16U, 17U, 9U,  8U,  7U,  8U,  9U,  10U, 11U, 16U, 17U, 16U,
        10U, 15U, 14U, 9U,  10U, 11U, 12U, 13U, 14U, 15U, 11U, 12U, 13U, 10U, 11U,
        12U, 15U, 14U, 15U, 16U, 20U, 13U, 12U, 11U, 18U, 17U, 16U, 23U, 16U, 17U,
        19U, 18U, 13U, 14U, 17U, 18U, 17U, 22U, 19U, 18U, 18U, 17U, 16U, 15U, 16U,
        19U, 20U, 21U, 20U, 21U, 14U, 13U, 0U,  1U,  2U,  5U,  6U,  7U,  8U,  9U,
        11U, 12U, 1U,  4U,  3U,  4U,  7U,  8U,  9U,  10U, 10U, 9U,  2U,  5U,  6U,
        5U,  10U, 9U,  10U, 11U, 11U, 8U,  7U,  6U,  13U, 12U, 11U, 10U, 11U, 12U,
        10U, 9U,  8U,  9U,  10U, 11U, 12U, 11U, 12U, 13U, 11U, 16U, 15U, 10U, 11U,
        12U, 13U, 14U, 15U, 14U, 12U, 13U, 14U, 11U, 12U, 13U, 16U, 15U, 16U, 15U,
        21U, 14U, 13U, 12U, 19U, 18U, 17U, 24U, 17U, 18U, 20U, 19U, 14U, 15U, 18U,
        19U, 18U, 23U, 20U, 19U, 19U, 18U, 17U, 16U, 17U, 20U, 21U, 22U, 21U, 22U,
        13U, 12U, 1U,  0U,  1U,  4U,  5U,  6U,  7U,  8U,  10U, 11U, 2U,  3U,  2U,
        3U,  6U,  7U,  8U,  9U,  9U,  8U,  3U,  4U,  5U,  4U,  9U,  8U,  9U,  10U,
        10U, 7U,  6U,  5U,  12U, 11U, 10U, 9U,  10U, 11U, 9U,  8U,  7U,  8U,  9U,
        10U, 11U, 10U, 11U, 12U, 10U, 15U, 14U, 9U,  10U, 11U, 12U, 13U, 14U, 13U,
        11U, 12U, 13U, 10U, 11U, 12U, 15U, 14U, 15U, 14U, 20U, 13U, 12U, 11U, 18U,
        17U, 16U, 23U, 16U, 17U, 19U, 18U, 13U, 14U, 17U, 18U, 17U, 22U, 19U, 18U,
        18U, 17U, 16U, 15U, 16U, 19U, 20U, 21U, 20U, 21U, 12U, 11U, 2U,  1U,  0U,
        3U,  4U,  5U,  6U,  7U,  9U,  10U, 3U,  2U,  1U,  2U,  5U,  6U,  7U,  8U,
        8U,  7U,  4U,  3U,  4U,  3U,  8U,  7U,  8U,  9U,  9U,  6U,  5U,  4U,  11U,
        10U, 9U,  8U,  9U,  10U, 8U,  7U,  6U,  7U,  8U,  9U,  10U, 9U,  10U, 11U,
        9U,  14U, 13U, 8U,  9U,  10U, 11U, 12U, 13U, 12U, 10U, 11U, 12U, 9U,  10U,
        11U, 14U, 13U, 14U, 13U, 19U, 12U, 11U, 10U, 17U, 16U, 15U, 22U, 15U, 16U,
        18U, 17U, 12U, 13U, 16U, 17U, 16U, 21U, 18U, 17U, 17U, 16U, 15U, 14U, 15U,
        18U, 19U, 20U, 19U, 20U, 13U, 12U, 5U,  4U,  3U,  0U,  1U,  2U,  3U,  4U,
        10U, 11U, 6U,  3U,  2U,  1U,  2U,  3U,  4U,  5U,  9U,  8U,  7U,  4U,  3U,
        2U,  5U,  4U,  5U,  6U,  10U, 7U,  6U,  5U,  8U,  7U,  6U,  5U,  6U,  7U,
        9U,  8U,  7U,  8U,  9U,  8U,  7U,  6U,  7U,  8U,  10U, 15U, 14U, 9U,  10U,
        9U,  8U,  9U,  10U, 9U,  11U, 12U, 13U, 10U, 11U, 12U, 11U, 10U, 11U, 10U,
        20U, 13U, 12U, 11U, 14U, 13U, 12U, 19U, 12U, 13U, 19U, 18U, 13U, 14U, 15U,
        14U, 13U, 18U, 15U, 14U, 18U, 17U, 16U, 15U, 16U, 15U, 16U, 17U, 16U, 17U,
        14U, 13U, 6U,  5U,  4U,  1U,  0U,  1U,  2U,  3U,  11U, 12U, 7U,  4U,  3U,
        2U,  1U,  2U,  3U,  4U,  10U, 9U,  8U,  5U,  4U,  3U,  4U,  3U,  4U,  5U,
        11U, 8U,  7U,  6U,  7U,  6U,  5U,  4U,  5U,  6U,  10U, 9U,  8U,  9U,  8U,
        7U,  6U,  5U,  6U,  7U,  11U, 16U, 15U, 10U, 9U,  8U,  7U,  8U,  9U,  8U,
        12U, 13U, 14U, 11U, 12U, 13U, 10U, 9U,  10U, 9U,  21U, 14U, 13U, 12U, 13U,
        12U, 11U, 18U, 11U, 12U, 20U, 19U, 14U, 15U, 14U, 13U, 12U, 17U, 14U, 13U,
        19U, 18U, 17U, 16U, 15U, 14U, 15U, 16U, 15U, 16U, 15U, 14U, 7U,  6U,  5U,
        2U,  1U,  0U,  1U,  2U,  12U, 13U, 8U,  5U,  4U,  3U,  2U,  3U,  2U,  3U,
        11U, 10U, 9U,  6U,  5U,  4U,  5U,  4U,  5U,  4U,  12U, 9U,  8U,  7U,  8U,
        7U,  6U,  5U,  6U,  7U,  11U, 10U, 9U,  10U, 9U,  8U,  7U,  6U,  7U,  8U,
        12U, 17U, 16U, 11U, 10U, 9U,  8U,  9U,  10U, 9U,  13U, 14U, 15U, 12U, 13U,
        14U, 11U, 10U, 11U, 10U, 22U, 15U, 14U, 13U, 14U, 13U, 12U, 19U, 12U, 13U,
        21U, 20U, 15U, 16U, 15U, 14U, 13U, 18U, 15U, 14U, 20U, 19U, 18U, 17U, 16U,
        15U, 16U, 17U, 16U, 17U, 16U, 15U, 8U,  7U,  6U,  3U,  2U,  1U,  0U,  1U,
        13U, 14U, 9U,  6U,  5U,  4U,  3U,  4U,  1U,  2U,  12U, 11U, 10U, 7U,  6U,
        5U,  6U,  5U,  6U,  3U,  13U, 10U, 9U,  8U,  9U,  8U,  7U,  6U,  7U,  8U,
        12U, 11U, 10U, 11U, 10U, 9U,  8U,  7U,  8U,  9U,  13U, 18U, 17U, 12U, 11U,
        10U, 9U,  10U, 11U, 10U, 14U, 15U, 16U, 13U, 14U, 15U, 12U, 11U, 12U, 11U,
        23U, 16U, 15U, 14U, 15U, 14U, 13U, 20U, 13U, 14U, 22U, 21U, 16U, 17U, 16U,
        15U, 14U, 19U, 16U, 15U, 21U, 20U, 19U, 18U, 17U, 16U, 17U, 18U, 17U, 18U,
        17U, 16U, 9U,  8U,  7U,  4U,  3U,  2U,  1U,  0U,  14U, 15U, 10U, 7U,  6U,
        5U,  4U,  5U,  2U,  1U,  13U, 12U, 11U, 8U,  7U,  6U,  7U,  6U,  7U,  2U,
        14U, 11U, 10U, 9U,  10U, 9U,  8U,  7U,  8U,  9U,  13U, 12U, 11U, 12U, 11U,
        10U, 9U,  8U,  9U,  10U, 14U, 19U, 18U, 13U, 12U, 11U, 10U, 11U, 12U, 11U,
        15U, 16U, 17U, 14U, 15U, 16U, 13U, 12U, 13U, 12U, 24U, 17U, 16U, 15U, 16U,
        15U, 14U, 21U, 14U, 15U, 23U, 22U, 17U, 18U, 17U, 16U, 15U, 20U, 17U, 16U,
        22U, 21U, 20U, 19U, 18U, 17U, 18U, 19U, 18U, 19U, 3U,  2U,  11U, 10U, 9U,
        10U, 11U, 12U, 13U, 14U, 0U,  1U,  12U, 7U,  8U,  9U,  12U, 13U, 14U, 15U,
        1U,  2U,  13U, 6U,  7U,  8U,  11U, 12U, 13U, 16U, 8U,  3U,  4U,  5U,  12U,
        11U, 10U, 13U, 14U, 15U, 7U,  6U,  5U,  6U,  7U,  8U,  9U,  14U, 15U, 14U,
        8U,  13U, 12U, 7U,  8U,  9U,  10U, 11U, 12U, 13U, 9U,  10U, 11U, 8U,  9U,
        10U, 13U, 12U, 13U, 14U, 18U, 11U, 10U, 9U,  16U, 15U, 14U, 21U, 14U, 15U,
        17U, 16U, 11U, 12U, 15U, 16U, 15U, 20U, 17U, 16U, 16U, 15U, 14U, 13U, 14U,
        17U, 18U, 19U, 18U, 19U, 2U,  1U,  12U, 11U, 10U, 11U, 12U, 13U, 14U, 15U,
        1U,  0U,  13U, 8U,  9U,  10U, 13U, 14U, 15U, 16U, 2U,  3U,  14U, 7U,  8U,
        9U,  12U, 13U, 14U, 17U, 9U,  4U,  5U,  6U,  13U, 12U, 11U, 14U, 15U, 16U,
        8U,  7U,  6U,  7U,  8U,  9U,  10U, 15U, 16U, 15U, 9U,  14U, 13U, 8U,  9U,
        10U, 11U, 12U, 13U, 14U, 10U, 11U, 12U, 9U,  10U, 11U, 14U, 13U, 14U, 15U,
        19U, 12U, 11U, 10U, 17U, 16U, 15U, 22U, 15U, 16U, 18U, 17U, 12U, 13U, 16U,
        17U, 16U, 21U, 18U, 17U, 17U, 16U, 15U, 14U, 15U, 18U, 19U, 20U, 19U, 20U,
        15U, 14U, 1U,  2U,  3U,  6U,  7U,  8U,  9U,  10U, 12U, 13U, 0U,  5U,  4U,
        5U,  8U,  9U,  10U, 11U, 11U, 10U, 1U,  6U,  7U,  6U,  11U, 10U, 11U, 12U,
        12U, 9U,  8U,  7U,  14U, 13U, 12U, 11U, 12U, 13U, 11U, 10U, 9U,  10U, 11U,
        12U, 13U, 12U, 13U, 14U, 12U, 17U, 16U, 11U, 12U, 13U, 14U, 15U, 16U, 15U,
        13U, 14U, 15U, 12U, 13U, 14U, 17U, 16U, 17U, 16U, 22U, 15U, 14U, 13U, 20U,
        19U, 18U, 25U, 18U, 19U, 21U, 20U, 15U, 16U, 19U, 20U, 19U, 24U, 21U, 20U,
        20U, 19U, 18U, 17U, 18U, 21U, 22U, 23U, 22U, 23U, 10U, 9U,  4U,  3U,  2U,
        3U,  4U,  5U,  6U,  7U,  7U,  8U,  5U,  0U,  1U,  2U,  5U,  6U,  7U,  8U,
        6U,  5U,  6U,  1U,  2U,  3U,  8U,  7U,  8U,  9U,  7U,  4U,  3U,  2U,  11U,
        10U, 9U,  8U,  9U,  10U, 6U,  5U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U,
        7U,  12U, 11U, 6U,  7U,  8U,  9U,  10U, 11U, 12U, 8U,  9U,  10U, 7U,  8U,
        9U,  12U, 11U, 12U, 13U, 17U, 10U, 9U,  8U,  15U, 14U, 13U, 20U, 13U, 14U,
        16U, 15U, 10U, 11U, 14U, 15U, 14U, 19U, 16U, 15U, 15U, 14U, 13U, 12U, 13U,
        16U, 17U, 18U, 17U, 18U, 11U, 10U, 3U,  2U,  1U,  2U,  3U,  4U,  5U,  6U,
        8U,  9U,  4U,  1U,  0U,  1U,  4U,  5U,  6U,  7U,  7U,  6U,  5U,  2U,  3U,
        2U,  7U,  6U,  7U,  8U,  8U,  5U,  4U,  3U,  10U, 9U,  8U,  7U,  8U,  9U,
        7U,  6U,  5U,  6U,  7U,  8U,  9U,  8U,  9U,  10U, 8U,  13U, 12U, 7U,  8U,
        9U,  10U, 11U, 12U, 11U, 9U,  10U, 11U, 8U,  9U,  10U, 13U, 12U, 13U, 12U,
        18U, 11U, 10U, 9U,  16U, 15U, 14U, 21U, 14U, 15U, 17U, 16U, 11U, 12U, 15U,
        16U, 15U, 20U, 17U, 16U, 16U, 15U, 14U, 13U, 14U, 17U, 18U, 19U, 18U, 19U,
        12U, 11U, 4U,  3U,  2U,  1U,  2U,  3U,  4U,  5U,  9U,  10U, 5U,  2U,  1U,
        0U,  3U,  4U,  5U,  6U,  8U,  7U,  6U,  3U,  2U,  1U,  6U,  5U,  6U,  7U,
        9U,  6U,  5U,  4U,  9U,  8U,  7U,  6U,  7U,  8U,  8U,  7U,  6U,  7U,  8U,
        9U,  8U,  7U,  8U,  9U,  9U,  14U, 13U, 8U,  9U,  10U, 9U,  10U, 11U, 10U,
        10U, 11U, 12U, 9U,  10U, 11U, 12U, 11U, 12U, 11U, 19U, 12U, 11U, 10U, 15U,
        14U, 13U, 20U, 13U, 14U, 18U, 17U, 12U, 13U, 16U, 15U, 14U, 19U, 16U, 15U,
        17U, 16U, 15U, 14U, 15U, 16U, 17U, 18U, 17U, 18U, 15U, 14U, 7U,  6U,  5U,
        2U,  1U,  2U,  3U,  4U,  12U, 13U, 8U,  5U,  4U,  3U,  0U,  1U,  4U,  5U,
        11U, 10U, 9U,  6U,  5U,  4U,  3U,  2U,  3U,  6U,  12U, 9U,  8U,  7U,  6U,
        5U,  4U,  3U,  4U,  5U,  11U, 10U, 9U,  8U,  7U,  6U,  5U,  4U,  5U,  6U,
        12U, 15U, 14U, 9U,  8U,  7U,  6U,  7U,  8U,  7U,  13U, 14U, 13U, 10U, 11U,
        12U, 9U,  8U,  9U,  8U,  20U, 13U, 12U, 11U, 12U, 11U, 10U, 17U, 10U, 11U,
        19U, 18U, 13U, 14U, 13U, 12U, 11U, 16U, 13U, 12U, 18U, 17U, 16U, 15U, 14U,
        13U, 14U, 15U, 14U, 15U, 16U, 15U, 8U,  7U,  6U,  3U,  2U,  3U,  4U,  5U,
        13U, 14U, 9U,  6U,  5U,  4U,  1U,  0U,  5U,  6U,  12U, 11U, 10U, 7U,  6U,
        5U,  2U,  1U,  2U,  7U,  11U, 10U, 9U,  8U,  5U,  4U,  3U,  2U,  3U,  4U,
        10U, 9U,  8U,  7U,  6U,  5U,  4U,  3U,  4U,  5U,  11U, 14U, 13U, 8U,  7U,
        6U,  5U,  6U,  7U,  6U,  12U, 13U, 12U, 9U,  10U, 11U, 8U,  7U,  8U,  7U,
        19U, 12U, 11U, 10U, 11U, 10U, 9U,  16U, 9U,  10U, 18U, 17U, 12U, 13U, 12U,
        11U, 10U, 15U, 12U, 11U, 17U, 16U, 15U, 14U, 13U, 12U, 13U, 14U, 13U, 14U,
        17U, 16U, 9U,  8U,  7U,  4U,  3U,  2U,  1U,  2U,  14U, 15U, 10U, 7U,  6U,
        5U,  4U,  5U,  0U,  3U,  13U, 12U, 11U, 8U,  7U,  6U,  7U,  6U,  7U,  4U,
        14U, 11U, 10U, 9U,  10U, 9U,  8U,  7U,  8U,  9U,  13U, 12U, 11U, 12U, 11U,
        10U, 9U,  8U,  9U,  10U, 14U, 19U, 18U, 13U, 12U, 11U, 10U, 11U, 12U, 11U,
        15U, 16U, 17U, 14U, 15U, 16U, 13U, 12U, 13U, 12U, 24U, 17U, 16U, 15U, 16U,
        15U, 14U, 21U, 14U, 15U, 23U, 22U, 17U, 18U, 17U, 16U, 15U, 20U, 17U, 16U,
        22U, 21U, 20U, 19U, 18U, 17U, 18U, 19U, 18U, 19U, 18U, 17U, 10U, 9U,  8U,
        5U,  4U,  3U,  2U,  1U,  15U, 16U, 11U, 8U,  7U,  6U,  5U,  6U,  3U,  0U,
        14U, 13U, 12U, 9U,  8U,  7U,  8U,  7U,  8U,  1U,  15U, 12U, 11U, 10U, 11U,
        10U, 9U,  8U,  9U,  10U, 14U, 13U, 12U, 13U, 12U, 11U, 10U, 9U,  10U, 11U,
        15U, 20U, 19U, 14U, 13U, 12U, 11U, 12U, 13U, 12U, 16U, 17U, 18U, 15U, 16U,
        17U, 14U, 13U, 14U, 13U, 25U, 18U, 17U, 16U, 17U, 16U, 15U, 22U, 15U, 16U,
        24U, 23U, 18U, 19U, 18U, 17U, 16U, 21U, 18U, 17U, 23U, 22U, 21U, 20U, 19U,
        18U, 19U, 20U, 19U, 20U, 4U,  3U,  10U, 9U,  8U,  9U,  10U, 11U, 12U, 13U,
        1U,  2U,  11U, 6U,  7U,  8U,  11U, 12U, 13U, 14U, 0U,  1U,  12U, 5U,  6U,
        7U,  10U, 11U, 12U, 15U, 7U,  2U,  3U,  4U,  11U, 10U, 9U,  12U, 13U, 14U,
        6U,  5U,  4U,  5U,  6U,  7U,  8U,  13U, 14U, 13U, 7U,  12U, 11U, 6U,  7U,
        8U,  9U,  10U, 11U, 12U, 8U,  9U,  10U, 7U,  8U,  9U,  12U, 11U, 12U, 13U,
        17U, 10U, 9U,  8U,  15U, 14U, 13U, 20U, 13U, 14U, 16U, 15U, 10U, 11U, 14U,
        15U, 14U, 19U, 16U, 15U, 15U, 14U, 13U, 12U, 13U, 16U, 17U, 18U, 17U, 18U,
        5U,  4U,  9U,  8U,  7U,  8U,  9U,  10U, 11U, 12U, 2U,  3U,  10U, 5U,  6U,
        7U,  10U, 11U, 12U, 13U, 1U,  0U,  11U, 4U,  5U,  6U,  9U,  10U, 11U, 14U,
        6U,  1U,  2U,  3U,  10U, 9U,  8U,  11U, 12U, 13U, 5U,  4U,  3U,  4U,  5U,
        6U,  7U,  12U, 13U, 12U, 6U,  11U, 10U, 5U,  6U,  7U,  8U,  9U,  10U, 11U,
        7U,  8U,  9U,  6U,  7U,  8U,  11U, 10U, 11U, 12U, 16U, 9U,  8U,  7U,  14U,
        13U, 12U, 19U, 12U, 13U, 15U, 14U, 9U,  10U, 13U, 14U, 13U, 18U, 15U, 14U,
        14U, 13U, 12U, 11U, 12U, 15U, 16U, 17U, 16U, 17U, 16U, 15U, 2U,  3U,  4U,
        7U,  8U,  9U,  10U, 11U, 13U, 14U, 1U,  6U,  5U,  6U,  9U,  10U, 11U, 12U,
        12U, 11U, 0U,  7U,  8U,  7U,  12U, 11U, 12U, 13U, 13U, 10U, 9U,  8U,  15U,
        14U, 13U, 12U, 13U, 14U, 12U, 11U, 10U, 11U, 12U, 13U, 14U, 13U, 14U, 15U,
        13U, 18U, 17U, 12U, 13U, 14U, 15U, 16U, 17U, 16U, 14U, 15U, 16U, 13U, 14U,
        15U, 18U, 17U, 18U, 17U, 23U, 16U, 15U, 14U, 21U, 20U, 19U, 26U, 19U, 20U,
        22U, 21U, 16U, 17U, 20U, 21U, 20U, 25U, 22U, 21U, 21U, 20U, 19U, 18U, 19U,
        22U, 23U, 24U, 23U, 24U, 9U,  8U,  5U,  4U,  3U,  4U,  5U,  6U,  7U,  8U,
        6U,  7U,  6U,  1U,  2U,  3U,  6U,  7U,  8U,  9U,  5U,  4U,  7U,  0U,  1U,
        2U,  9U,  8U,  9U,  10U, 6U,  3U,  2U,  1U,  10U, 9U,  8U,  9U,  10U, 11U,
        5U,  4U,  3U,  4U,  5U,  6U,  7U,  10U, 11U, 12U, 6U,  11U, 10U, 5U,  6U,
        7U,  8U,  9U,  10U, 11U, 7U,  8U,  9U,  6U,  7U,  8U,  11U, 10U, 11U, 12U,
        16U, 9U,  8U,  7U,  14U, 13U, 12U, 19U, 12U, 13U, 15U, 14U, 9U,  10U, 13U,
        14U, 13U, 18U, 15U, 14U, 14U, 13U, 12U, 11U, 12U, 15U, 16U, 17U, 16U, 17U,
        10U, 9U,  6U,  5U,  4U,  3U,  4U,  5U,  6U,  7U,  7U,  8U,  7U,  2U,  3U,
        2U,  5U,  6U,  7U,  8U,  6U,  5U,  8U,  1U,  0U,  1U,  8U,  7U,  8U,  9U,
        7U,  4U,  3U,  2U,  11U, 10U, 9U,  8U,  9U,  10U, 6U,  5U,  4U,  5U,  6U,
        7U,  8U,  9U,  10U, 11U, 7U,  12U, 11U, 6U,  7U,  8U,  9U,  10U, 11U, 12U,
        8U,  9U,  10U, 7U,  8U,  9U,  12U, 11U, 12U, 13U, 17U, 10U, 9U,  8U,  15U,
        14U, 13U, 20U, 13U, 14U, 16U, 15U, 10U, 11U, 14U, 15U, 14U, 19U, 16U, 15U,
        15U, 14U, 13U, 12U, 13U, 16U, 17U, 18U, 17U, 18U, 11U, 10U, 5U,  4U,  3U,
        2U,  3U,  4U,  5U,  6U,  8U,  9U,  6U,  3U,  2U,  1U,  4U,  5U,  6U,  7U,
        7U,  6U,  7U,  2U,  1U,  0U,  7U,  6U,  7U,  8U,  8U,  5U,  4U,  3U,  10U,
        9U,  8U,  7U,  8U,  9U,  7U,  6U,  5U,  6U,  7U,  8U,  9U,  8U,  9U,  10U,
        8U,  13U, 12U, 7U,  8U,  9U,  10U, 11U, 12U, 11U, 9U,  10U, 11U, 8U,  9U,
        10U, 13U, 12U, 13U, 12U, 18U, 11U, 10U, 9U,  16U, 15U, 14U, 21U, 14U, 15U,
        17U, 16U, 11U, 12U, 15U, 16U, 15U, 20U, 17U, 16U, 16U, 15U, 14U, 13U, 14U,
        17U, 18U, 19U, 18U, 19U, 14U, 13U, 10U, 9U,  8U,  5U,  4U,  5U,  6U,  7U,
        11U, 12U, 11U, 8U,  7U,  6U,  3U,  2U,  7U,  8U,  10U, 9U,  12U, 9U,  8U,
        7U,  0U,  1U,  2U,  9U,  9U,  8U,  7U,  8U,  3U,  2U,  1U,  2U,  3U,  4U,
        8U,  7U,  6U,  5U,  4U,  3U,  2U,  3U,  4U,  5U,  9U,  12U, 11U, 6U,  5U,
        4U,  3U,  4U,  5U,  6U,  10U, 11U, 10U, 7U,  8U,  9U,  6U,  5U,  6U,  7U,
        17U, 10U, 9U,  8U,  9U,  8U,  7U,  14U, 7U,  8U,  16U, 15U, 10U, 11U, 10U,
        9U,  8U,  13U, 10U, 9U,  15U, 14U, 13U, 12U, 11U, 10U, 11U, 12U, 11U, 12U,
        15U, 14U, 9U,  8U,  7U,  4U,  3U,  4U,  5U,  6U,  12U, 13U, 10U, 7U,  6U,
        5U,  2U,  1U,  6U,  7U,  11U, 10U, 11U, 8U,  7U,  6U,  1U,  0U,  1U,  8U,
        10U, 9U,  8U,  9U,  4U,  3U,  2U,  1U,  2U,  3U,  9U,  8U,  7U,  6U,  5U,
        4U,  3U,  2U,  3U,  4U,  10U, 13U, 12U, 7U,  6U,  5U,  4U,  5U,  6U,  5U,
        11U, 12U, 11U, 8U,  9U,  10U, 7U,  6U,  7U,  6U,  18U, 11U, 10U, 9U,  10U,
        9U,  8U,  15U, 8U,  9U,  17U, 16U, 11U, 12U, 11U, 10U, 9U,  14U, 11U, 10U,
        16U, 15U, 14U, 13U, 12U, 11U, 12U, 13U, 12U, 13U, 16U, 15U, 10U, 9U,  8U,
        5U,  4U,  5U,  6U,  7U,  13U, 14U, 11U, 8U,  7U,  6U,  3U,  2U,  7U,  8U,
        12U, 11U, 12U, 9U,  8U,  7U,  2U,  1U,  0U,  9U,  11U, 10U, 9U,  10U, 5U,
        4U,  3U,  2U,  1U,  2U,  10U, 9U,  8U,  7U,  6U,  5U,  4U,  3U,  4U,  3U,
        11U, 14U, 13U, 8U,  7U,  6U,  5U,  6U,  5U,  4U,  12U, 13U, 12U, 9U,  10U,
        11U, 8U,  7U,  8U,  5U,  19U, 12U, 11U, 10U, 11U, 10U, 9U,  16U, 9U,  10U,
        18U, 17U, 12U, 13U, 12U, 11U, 10U, 15U, 12U, 11U, 17U, 16U, 15U, 14U, 13U,
        12U, 13U, 14U, 13U, 14U, 19U, 18U, 11U, 10U, 9U,  6U,  5U,  4U,  3U,  2U,
        16U, 17U, 12U, 9U,  8U,  7U,  6U,  7U,  4U,  1U,  15U, 14U, 13U, 10U, 9U,
        8U,  9U,  8U,  9U,  0U,  16U, 13U, 12U, 11U, 12U, 11U, 10U, 9U,  10U, 11U,
        15U, 14U, 13U, 14U, 13U, 12U, 11U, 10U, 11U, 12U, 16U, 21U, 20U, 15U, 14U,
        13U, 12U, 13U, 14U, 13U, 17U, 18U, 19U, 16U, 17U, 18U, 15U, 14U, 15U, 14U,
        26U, 19U, 18U, 17U, 18U, 17U, 16U, 23U, 16U, 17U, 25U, 24U, 19U, 20U, 19U,
        18U, 17U, 22U, 19U, 18U, 24U, 23U, 22U, 21U, 20U, 19U, 20U, 21U, 20U, 21U,
        11U, 10U, 11U, 10U, 9U,  10U, 11U, 12U, 13U, 14U, 8U,  9U,  12U, 7U,  8U,
        9U,  12U, 11U, 14U, 15U, 7U,  6U,  13U, 6U,  7U,  8U,  9U,  10U, 11U, 16U,
        0U,  5U,  4U,  5U,  10U, 9U,  8U,  11U, 12U, 13U, 1U,  2U,  3U,  4U,  5U,
        6U,  7U,  12U, 13U, 12U, 2U,  9U,  8U,  5U,  6U,  7U,  8U,  9U,  10U, 11U,
        3U,  4U,  7U,  6U,  7U,  8U,  11U, 10U, 11U, 12U, 14U, 5U,  6U,  7U,  12U,
        13U, 12U, 17U, 12U, 13U, 13U, 12U, 7U,  8U,  11U, 12U, 13U, 16U, 15U, 14U,
        12U, 11U, 10U, 9U,  10U, 13U, 14U, 15U, 16U, 17U, 6U,  5U,  8U,  7U,  6U,
        7U,  8U,  9U,  10U, 11U, 3U,  4U,  9U,  4U,  5U,  6U,  9U,  10U, 11U, 12U,
        2U,  1U,  10U, 3U,  4U,  5U,  8U,  9U,  10U, 13U, 5U,  0U,  1U,  2U,  9U,
        8U,  7U,  10U, 11U, 12U, 4U,  3U,  2U,  3U,  4U,  5U,  6U,  11U, 12U, 11U,
        5U,  10U, 9U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 6U,  7U,  8U,  5U,  6U,
        7U,  10U, 9U,  10U, 11U, 15U, 8U,  7U,  6U,  13U, 12U, 11U, 18U, 11U, 12U,
        14U, 13U, 8U,  9U,  12U, 13U, 12U, 17U, 14U, 13U, 13U, 12U, 11U, 10U, 11U,
        14U, 15U, 16U, 15U, 16U, 7U,  6U,  7U,  6U,  5U,  6U,  7U,  8U,  9U,  10U,
        4U,  5U,  8U,  3U,  4U,  5U,  8U,  9U,  10U, 11U, 3U,  2U,  9U,  2U,  3U,
        4U,  7U,  8U,  9U,  12U, 4U,  1U,  0U,  1U,  8U,  7U,  6U,  9U,  10U, 11U,
        3U,  2U,  1U,  2U,  3U,  4U,  5U,  10U, 11U, 10U, 4U,  9U,  8U,  3U,  4U,
        5U,  6U,  7U,  8U,  9U,  5U,  6U,  7U,  4U,  5U,  6U,  9U,  8U,  9U,  10U,
        14U, 7U,  6U,  5U,  12U, 11U, 10U, 17U, 10U, 11U, 13U, 12U, 7U,  8U,  11U,
        12U, 11U, 16U, 13U, 12U, 12U, 11U, 10U, 9U,  10U, 13U, 14U, 15U, 14U, 15U,
        8U,  7U,  6U,  5U,  4U,  5U,  6U,  7U,  8U,  9U,  5U,  6U,  7U,  2U,  3U,
        4U,  7U,  8U,  9U,  10U, 4U,  3U,  8U,  1U,  2U,  3U,  8U,  9U,  10U, 11U,
        5U,  2U,  1U,  0U,  9U,  8U,  7U,  10U, 11U, 12U, 4U,  3U,  2U,  3U,  4U,
        5U,  6U,  11U, 12U, 11U, 5U,  10U, 9U,  4U,  5U,  6U,  7U,  8U,  9U,  10U,
        6U,  7U,  8U,  5U,  6U,  7U,  10U, 9U,  10U, 11U, 15U, 8U,  7U,  6U,  13U,
        12U, 11U, 18U, 11U, 12U, 14U, 13U, 8U,  9U,  12U, 13U, 12U, 17U, 14U, 13U,
        13U, 12U, 11U, 10U, 11U, 14U, 15U, 16U, 15U, 16U, 15U, 14U, 13U, 12U, 11U,
        8U,  7U,  8U,  9U,  10U, 12U, 13U, 14U, 11U, 10U, 9U,  6U,  5U,  10U, 11U,
        11U, 10U, 15U, 10U, 11U, 10U, 3U,  4U,  5U,  12U, 10U, 9U,  8U,  9U,  0U,
        1U,  2U,  5U,  6U,  7U,  9U,  8U,  7U,  6U,  5U,  4U,  3U,  6U,  7U,  8U,
        10U, 13U, 12U, 7U,  6U,  5U,  4U,  5U,  6U,  7U,  11U, 12U, 11U, 8U,  9U,
        10U, 7U,  6U,  7U,  8U,  18U, 11U, 10U, 9U,  10U, 9U,  8U,  15U, 8U,  9U,
        17U, 16U, 11U, 12U, 11U, 10U, 9U,  14U, 11U, 10U, 16U, 15U, 14U, 13U, 12U,
        11U, 12U, 13U, 12U, 13U, 14U, 13U, 12U, 11U, 10U, 7U,  6U,  7U,  8U,  9U,
        11U, 12U, 13U, 10U, 9U,  8U,  5U,  4U,  9U,  10U, 10U, 9U,  14U, 9U,  10U,
        9U,  2U,  3U,  4U,  11U, 9U,  8U,  7U,  8U,  1U,  0U,  1U,  4U,  5U,  6U,
        8U,  7U,  6U,  5U,  4U,  3U,  2U,  5U,  6U,  7U,  9U,  12U, 11U, 6U,  5U,
        4U,  3U,  4U,  5U,  6U,  10U, 11U, 10U, 7U,  8U,  9U,  6U,  5U,  6U,  7U,
        17U, 10U, 9U,  8U,  9U,  8U,  7U,  14U, 7U,  8U,  16U, 15U, 10U, 11U, 10U,
        9U,  8U,  13U, 10U, 9U,  15U, 14U, 13U, 12U, 11U, 10U, 11U, 12U, 11U, 12U,
        13U, 12U, 11U, 10U, 9U,  6U,  5U,  6U,  7U,  8U,  10U, 11U, 12U, 9U,  8U,
        7U,  4U,  3U,  8U,  9U,  9U,  8U,  13U, 8U,  9U,  8U,  1U,  2U,  3U,  10U,
        8U,  7U,  6U,  7U,  2U,  1U,  0U,  3U,  4U,  5U,  7U,  6U,  5U,  4U,  3U,
        2U,  1U,  4U,  5U,  6U,  8U,  11U, 10U, 5U,  4U,  3U,  2U,  3U,  4U,  5U,
        9U,  10U, 9U,  6U,  7U,  8U,  5U,  4U,  5U,  6U,  16U, 9U,  8U,  7U,  8U,
        7U,  6U,  13U, 6U,  7U,  15U, 14U, 9U,  10U, 9U,  8U,  7U,  12U, 9U,  8U,
        14U, 13U, 12U, 11U, 10U, 9U,  10U, 11U, 10U, 11U, 16U, 15U, 10U, 9U,  8U,
        5U,  4U,  5U,  6U,  7U,  13U, 14U, 11U, 8U,  7U,  6U,  3U,  2U,  7U,  8U,
        12U, 11U, 12U, 9U,  8U,  7U,  2U,  1U,  2U,  9U,  11U, 10U, 9U,  10U, 5U,
        4U,  3U,  0U,  3U,  4U,  10U, 9U,  8U,  7U,  6U,  5U,  4U,  1U,  2U,  5U,
        11U, 14U, 13U, 8U,  7U,  6U,  5U,  6U,  7U,  6U,  12U, 13U, 12U, 9U,  10U,
        11U, 8U,  7U,  8U,  7U,  19U, 12U, 11U, 10U, 11U, 10U, 9U,  16U, 9U,  10U,
        18U, 17U, 12U, 13U, 12U, 11U, 10U, 15U, 12U, 11U, 17U, 16U, 15U, 14U, 13U,
        12U, 13U, 14U, 13U, 14U, 17U, 16U, 11U, 10U, 9U,  6U,  5U,  6U,  7U,  8U,
        14U, 15U, 12U, 9U,  8U,  7U,  4U,  3U,  8U,  9U,  13U, 12U, 13U, 10U, 9U,
        8U,  3U,  2U,  1U,  10U, 12U, 11U, 10U, 11U, 6U,  5U,  4U,  3U,  0U,  1U,
        11U, 10U, 9U,  8U,  7U,  6U,  5U,  4U,  5U,  2U,  12U, 15U, 14U, 9U,  8U,
        7U,  6U,  5U,  4U,  3U,  13U, 14U, 13U, 10U, 11U, 12U, 7U,  6U,  7U,  4U,
        18U, 13U, 12U, 11U, 10U, 9U,  8U,  15U, 8U,  9U,  17U, 16U, 13U, 14U, 11U,
        10U, 9U,  14U, 11U, 10U, 16U, 15U, 14U, 13U, 12U, 11U, 12U, 13U, 12U, 13U,
        18U, 17U, 12U, 11U, 10U, 7U,  6U,  7U,  8U,  9U,  15U, 16U, 13U, 10U, 9U,
        8U,  5U,  4U,  9U,  10U, 14U, 13U, 14U, 11U, 10U, 9U,  4U,  3U,  2U,  11U,
        13U, 12U, 11U, 12U, 7U,  6U,  5U,  4U,  1U,  0U,  12U, 11U, 10U, 9U,  8U,
        7U,  6U,  5U,  6U,  1U,  13U, 14U, 13U, 8U,  7U,  6U,  5U,  4U,  3U,  2U,
        14U, 13U, 12U, 9U,  10U, 11U, 6U,  5U,  6U,  3U,  17U, 12U, 11U, 10U, 9U,
        8U,  7U,  14U, 7U,  8U,  16U, 15U, 12U, 13U, 10U, 9U,  8U,  13U, 10U, 9U,
        15U, 14U, 13U, 12U, 11U, 10U, 11U, 12U, 11U, 12U, 10U, 9U,  10U, 9U,  8U,
        9U,  10U, 11U, 12U, 13U, 7U,  8U,  11U, 6U,  7U,  8U,  11U, 10U, 13U, 14U,
        6U,  5U,  12U, 5U,  6U,  7U,  8U,  9U,  10U, 15U, 1U,  4U,  3U,  4U,  9U,
        8U,  7U,  10U, 11U, 12U, 0U,  1U,  2U,  3U,  4U,  5U,  6U,  11U, 12U, 11U,
        1U,  8U,  7U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 2U,  3U,  6U,  5U,  6U,
        7U,  10U, 9U,  10U, 11U, 13U, 4U,  5U,  6U,  11U, 12U, 11U, 16U, 11U, 12U,
        12U, 11U, 6U,  7U,  10U, 11U, 12U, 15U, 14U, 13U, 11U, 10U, 9U,  8U,  9U,
        12U, 13U, 14U, 15U, 16U, 9U,  8U,  9U,  8U,  7U,  8U,  9U,  10U, 11U, 12U,
        6U,  7U,  10U, 5U,  6U,  7U,  10U, 9U,  12U, 13U, 5U,  4U,  11U, 4U,  5U,
        6U,  7U,  8U,  9U,  14U, 2U,  3U,  2U,  3U,  8U,  7U,  6U,  9U,  10U, 11U,
        1U,  0U,  1U,  2U,  3U,  4U,  5U,  10U, 11U, 10U, 2U,  9U,  8U,  3U,  4U,
        5U,  6U,  7U,  8U,  9U,  3U,  4U,  7U,  4U,  5U,  6U,  9U,  8U,  9U,  10U,
        14U, 5U,  6U,  5U,  12U, 11U, 10U, 17U, 10U, 11U, 13U, 12U, 7U,  8U,  11U,
        12U, 11U, 16U, 13U, 12U, 12U, 11U, 10U, 9U,  10U, 13U, 14U, 15U, 14U, 15U,
        8U,  7U,  8U,  7U,  6U,  7U,  8U,  9U,  10U, 11U, 5U,  6U,  9U,  4U,  5U,
        6U,  9U,  8U,  11U, 12U, 4U,  3U,  10U, 3U,  4U,  5U,  6U,  7U,  8U,  13U,
        3U,  2U,  1U,  2U,  7U,  6U,  5U,  8U,  9U,  10U, 2U,  1U,  0U,  1U,  2U,
        3U,  4U,  9U,  10U, 9U,  3U,  8U,  7U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,
        4U,  5U,  6U,  3U,  4U,  5U,  8U,  7U,  8U,  9U,  13U, 6U,  5U,  4U,  11U,
        10U, 9U,  16U, 9U,  10U, 12U, 11U, 6U,  7U,  10U, 11U, 10U, 15U, 12U, 11U,
        11U, 10U, 9U,  8U,  9U,  12U, 13U, 14U, 13U, 14U, 9U,  8U,  9U,  8U,  7U,
        8U,  9U,  10U, 11U, 12U, 6U,  7U,  10U, 5U,  6U,  7U,  8U,  7U,  12U, 13U,
        5U,  4U,  11U, 4U,  5U,  6U,  5U,  6U,  7U,  14U, 4U,  3U,  2U,  3U,  6U,
        5U,  4U,  7U,  8U,  9U,  3U,  2U,  1U,  0U,  1U,  2U,  3U,  8U,  9U,  8U,
        4U,  7U,  6U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  5U,  6U,  5U,  2U,  3U,
        4U,  7U,  6U,  7U,  8U,  12U, 5U,  4U,  3U,  10U, 9U,  8U,  15U, 8U,  9U,
        11U, 10U, 5U,  6U,  9U,  10U, 9U,  14U, 11U, 10U, 10U, 9U,  8U,  7U,  8U,
        11U, 12U, 13U, 12U, 13U, 10U, 9U,  10U, 9U,  8U,  9U,  8U,  9U,  10U, 11U,
        7U,  8U,  11U, 6U,  7U,  8U,  7U,  6U,  11U, 12U, 6U,  5U,  12U, 5U,  6U,
        7U,  4U,  5U,  6U,  13U, 5U,  4U,  3U,  4U,  5U,  4U,  3U,  6U,  7U,  8U,
        4U,  3U,  2U,  1U,  0U,  1U,  2U,  7U,  8U,  7U,  5U,  8U,  7U,  2U,  3U,
        4U,  3U,  4U,  5U,  6U,  6U,  7U,  6U,  3U,  4U,  5U,  6U,  5U,  6U,  7U,
        13U, 6U,  5U,  4U,  9U,  8U,  7U,  14U, 7U,  8U,  12U, 11U, 6U,  7U,  10U,
        9U,  8U,  13U, 10U, 9U,  11U, 10U, 9U,  8U,  9U,  10U, 11U, 12U, 11U, 12U,
        11U, 10U, 11U, 10U, 9U,  8U,  7U,  8U,  9U,  10U, 8U,  9U,  12U, 7U,  8U,
        9U,  6U,  5U,  10U, 11U, 7U,  6U,  13U, 6U,  7U,  8U,  3U,  4U,  5U,  12U,
        6U,  5U,  4U,  5U,  4U,  3U,  2U,  5U,  6U,  7U,  5U,  4U,  3U,  2U,  1U,
        0U,  1U,  6U,  7U,  6U,  6U,  9U,  8U,  3U,  4U,  3U,  2U,  3U,  4U,  5U,
        7U,  8U,  7U,  4U,  5U,  6U,  5U,  4U,  5U,  6U,  14U, 7U,  6U,  5U,  8U,
        7U,  6U,  13U, 6U,  7U,  13U, 12U, 7U,  8U,  9U,  8U,  7U,  12U, 9U,  8U,
        12U, 11U, 10U, 9U,  10U, 9U,  10U, 11U, 10U, 11U, 12U, 11U, 12U, 11U, 10U,
        7U,  6U,  7U,  8U,  9U,  9U,  10U, 13U, 8U,  9U,  8U,  5U,  4U,  9U,  10U,
        8U,  7U,  14U, 7U,  8U,  9U,  2U,  3U,  4U,  11U, 7U,  6U,  5U,  6U,  3U,
        2U,  1U,  4U,  5U,  6U,  6U,  5U,  4U,  3U,  2U,  1U,  0U,  5U,  6U,  5U,
        7U,  10U, 9U,  4U,  3U,  2U,  1U,  2U,  3U,  4U,  8U,  9U,  8U,  5U,  6U,
        7U,  4U,  3U,  4U,  5U,  15U, 8U,  7U,  6U,  7U,  6U,  5U,  12U, 5U,  6U,
        14U, 13U, 8U,  9U,  8U,  7U,  6U,  11U, 8U,  7U,  13U, 12U, 11U, 10U, 9U,
        8U,  9U,  10U, 9U,  10U, 17U, 16U, 11U, 10U, 9U,  6U,  5U,  6U,  7U,  8U,
        14U, 15U, 12U, 9U,  8U,  7U,  4U,  3U,  8U,  9U,  13U, 12U, 13U, 10U, 9U,
        8U,  3U,  2U,  3U,  10U, 12U, 11U, 10U, 11U, 6U,  5U,  4U,  1U,  4U,  5U,
        11U, 10U, 9U,  8U,  7U,  6U,  5U,  0U,  1U,  6U,  12U, 15U, 14U, 9U,  8U,
        7U,  6U,  7U,  8U,  7U,  13U, 14U, 13U, 10U, 11U, 12U, 9U,  8U,  9U,  8U,
        20U, 13U, 12U, 11U, 12U, 11U, 10U, 17U, 10U, 11U, 19U, 18U, 13U, 14U, 13U,
        12U, 11U, 16U, 13U, 12U, 18U, 17U, 16U, 15U, 14U, 13U, 14U, 15U, 14U, 15U,
        18U, 17U, 12U, 11U, 10U, 7U,  6U,  7U,  8U,  9U,  15U, 16U, 13U, 10U, 9U,
        8U,  5U,  4U,  9U,  10U, 14U, 13U, 14U, 11U, 10U, 9U,  4U,  3U,  4U,  11U,
        13U, 12U, 11U, 12U, 7U,  6U,  5U,  2U,  5U,  6U,  12U, 11U, 10U, 9U,  8U,
        7U,  6U,  1U,  0U,  7U,  13U, 16U, 15U, 10U, 9U,  8U,  7U,  8U,  9U,  8U,
        14U, 15U, 14U, 11U, 12U, 13U, 10U, 9U,  10U, 9U,  21U, 14U, 13U, 12U, 13U,
        12U, 11U, 18U, 11U, 12U, 20U, 19U, 14U, 15U, 14U, 13U, 12U, 17U, 14U, 13U,
        19U, 18U, 17U, 16U, 15U, 14U, 15U, 16U, 15U, 16U, 17U, 16U, 13U, 12U, 11U,
        8U,  7U,  8U,  9U,  10U, 14U, 15U, 14U, 11U, 10U, 9U,  6U,  5U,  10U, 11U,
        13U, 12U, 15U, 12U, 11U, 10U, 5U,  4U,  3U,  12U, 12U, 11U, 10U, 11U, 8U,
        7U,  6U,  5U,  2U,  1U,  11U, 10U, 9U,  8U,  7U,  6U,  5U,  6U,  7U,  0U,
        12U, 13U, 12U, 7U,  6U,  5U,  4U,  3U,  2U,  1U,  13U, 12U, 11U, 8U,  9U,
        10U, 5U,  4U,  5U,  2U,  16U, 11U, 10U, 9U,  8U,  7U,  6U,  13U, 6U,  7U,
        15U, 14U, 11U, 12U, 9U,  8U,  7U,  12U, 9U,  8U,  14U, 13U, 12U, 11U, 10U,
        9U,  10U, 11U, 10U, 11U, 11U, 10U, 11U, 10U, 9U,  10U, 11U, 12U, 13U, 14U,
        8U,  9U,  12U, 7U,  8U,  9U,  12U, 11U, 14U, 15U, 7U,  6U,  13U, 6U,  7U,
        8U,  9U,  10U, 11U, 16U, 2U,  5U,  4U,  5U,  10U, 9U,  8U,  11U, 12U, 13U,
        1U,  2U,  3U,  4U,  5U,  6U,  7U,  12U, 13U, 12U, 0U,  7U,  6U,  5U,  6U,
        7U,  8U,  9U,  10U, 11U, 1U,  2U,  5U,  6U,  7U,  8U,  11U, 10U, 11U, 12U,
        12U, 3U,  4U,  5U,  10U, 11U, 12U, 15U, 12U, 13U, 11U, 10U, 5U,  6U,  9U,
        10U, 11U, 14U, 15U, 14U, 10U, 9U,  8U,  7U,  8U,  11U, 12U, 13U, 16U, 17U,
        16U, 15U, 16U, 15U, 14U, 15U, 16U, 17U, 18U, 19U, 13U, 14U, 17U, 12U, 13U,
        14U, 15U, 14U, 19U, 20U, 12U, 11U, 18U, 11U, 12U, 13U, 12U, 13U, 14U, 21U,
        9U,  10U, 9U,  10U, 13U, 12U, 11U, 14U, 15U, 14U, 8U,  9U,  8U,  7U,  8U,
        9U,  10U, 15U, 16U, 13U, 7U,  0U,  1U,  6U,  7U,  8U,  9U,  10U, 11U, 12U,
        6U,  5U,  2U,  5U,  6U,  7U,  12U, 11U, 12U, 13U, 11U, 4U,  3U,  4U,  9U,
        10U, 11U, 14U, 13U, 14U, 10U, 9U,  4U,  5U,  8U,  9U,  10U, 13U, 16U, 15U,
        9U,  8U,  7U,  6U,  7U,  10U, 11U, 12U, 17U, 18U, 15U, 14U, 15U, 14U, 13U,
        14U, 15U, 16U, 17U, 18U, 12U, 13U, 16U, 11U, 12U, 13U, 14U, 13U, 18U, 19U,
        11U, 10U, 17U, 10U, 11U, 12U, 11U, 12U, 13U, 20U, 8U,  9U,  8U,  9U,  12U,
        11U, 10U, 13U, 14U, 13U, 7U,  8U,  7U,  6U,  7U,  8U,  9U,  14U, 15U, 12U,
        6U,  1U,  0U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 5U,  4U,  1U,  4U,  5U,
        6U,  11U, 10U, 11U, 12U, 10U, 3U,  2U,  3U,  8U,  9U,  10U, 13U, 12U, 13U,
        9U,  8U,  3U,  4U,  7U,  8U,  9U,  12U, 15U, 14U, 8U,  7U,  6U,  5U,  6U,
        9U,  10U, 11U, 16U, 17U, 10U, 9U,  10U, 9U,  8U,  9U,  10U, 11U, 12U, 13U,
        7U,  8U,  11U, 6U,  7U,  8U,  9U,  8U,  13U, 14U, 6U,  5U,  12U, 5U,  6U,
        7U,  6U,  7U,  8U,  15U, 5U,  4U,  3U,  4U,  7U,  6U,  5U,  8U,  9U,  8U,
        4U,  3U,  2U,  1U,  2U,  3U,  4U,  9U,  10U, 7U,  5U,  6U,  5U,  0U,  1U,
        2U,  3U,  4U,  5U,  6U,  6U,  5U,  4U,  1U,  2U,  3U,  6U,  5U,  6U,  7U,
        11U, 4U,  3U,  2U,  9U,  8U,  7U,  14U, 7U,  8U,  10U, 9U,  4U,  5U,  8U,
        9U,  8U,  13U, 10U, 9U,  9U,  8U,  7U,  6U,  7U,  10U, 11U, 12U, 11U, 12U,
        11U, 10U, 11U, 10U, 9U,  10U, 9U,  10U, 11U, 12U, 8U,  9U,  12U, 7U,  8U,
        9U,  8U,  7U,  12U, 13U, 7U,  6U,  13U, 6U,  7U,  8U,  5U,  6U,  7U,  14U,
        6U,  5U,  4U,  5U,  6U,  5U,  4U,  7U,  8U,  7U,  5U,  4U,  3U,  2U,  3U,
        4U,  3U,  8U,  9U,  6U,  6U,  7U,  6U,  1U,  0U,  1U,  2U,  3U,  4U,  5U,
        7U,  6U,  5U,  2U,  3U,  4U,  5U,  4U,  5U,  6U,  12U, 5U,  4U,  3U,  8U,
        7U,  6U,  13U, 6U,  7U,  11U, 10U, 5U,  6U,  9U,  8U,  7U,  12U, 9U,  8U,
        10U, 9U,  8U,  7U,  8U,  9U,  10U, 11U, 10U, 11U, 12U, 11U, 12U, 11U, 10U,
        9U,  8U,  9U,  10U, 11U, 9U,  10U, 13U, 8U,  9U,  10U, 7U,  6U,  11U, 12U,
        8U,  7U,  14U, 7U,  8U,  9U,  4U,  5U,  6U,  13U, 7U,  6U,  5U,  6U,  5U,
        4U,  3U,  6U,  7U,  6U,  6U,  5U,  4U,  3U,  4U,  3U,  2U,  7U,  8U,  5U,
        7U,  8U,  7U,  2U,  1U,  0U,  1U,  2U,  3U,  4U,  8U,  7U,  6U,  3U,  4U,
        5U,  4U,  3U,  4U,  5U,  13U, 6U,  5U,  4U,  7U,  6U,  5U,  12U, 5U,  6U,
        12U, 11U, 6U,  7U,  8U,  7U,  6U,  11U, 8U,  7U,  11U, 10U, 9U,  8U,  9U,
        8U,  9U,  10U, 9U,  10U, 13U, 12U, 13U, 12U, 11U, 8U,  7U,  8U,  9U,  10U,
        10U, 11U, 14U, 9U,  10U, 9U,  6U,  5U,  10U, 11U, 9U,  8U,  15U, 8U,  9U,
        10U, 3U,  4U,  5U,  12U, 8U,  7U,  6U,  7U,  4U,  3U,  2U,  5U,  6U,  5U,
        7U,  6U,  5U,  4U,  3U,  2U,  1U,  6U,  7U,  4U,  8U,  9U,  8U,  3U,  2U,
        1U,  0U,  1U,  2U,  3U,  9U,  8U,  7U,  4U,  5U,  6U,  3U,  2U,  3U,  4U,
        14U, 7U,  6U,  5U,  6U,  5U,  4U,  11U, 4U,  5U,  13U, 12U, 7U,  8U,  7U,
        6U,  5U,  10U, 7U,  6U,  12U, 11U, 10U, 9U,  8U,  7U,  8U,  9U,  8U,  9U,
        14U, 13U, 14U, 13U, 12U, 9U,  8U,  9U,  10U, 11U, 11U, 12U, 15U, 10U, 11U,
        10U, 7U,  6U,  11U, 12U, 10U, 9U,  16U, 9U,  10U, 11U, 4U,  5U,  6U,  13U,
        9U,  8U,  7U,  8U,  5U,  4U,  3U,  6U,  5U,  4U,  8U,  7U,  6U,  5U,  4U,
        3U,  2U,  7U,  8U,  3U,  9U,  10U, 9U,  4U,  3U,  2U,  1U,  0U,  1U,  2U,
        10U, 9U,  8U,  5U,  6U,  7U,  2U,  1U,  2U,  3U,  13U, 8U,  7U,  6U,  5U,
        4U,  3U,  10U, 3U,  4U,  12U, 11U, 8U,  9U,  6U,  5U,  4U,  9U,  6U,  5U,
        11U, 10U, 9U,  8U,  7U,  6U,  7U,  8U,  7U,  8U,  15U, 14U, 15U, 14U, 13U,
        10U, 9U,  10U, 11U, 12U, 12U, 13U, 16U, 11U, 12U, 11U, 8U,  7U,  12U, 13U,
        11U, 10U, 17U, 10U, 11U, 12U, 5U,  6U,  5U,  14U, 10U, 9U,  8U,  9U,  6U,
        5U,  4U,  7U,  4U,  3U,  9U,  8U,  7U,  6U,  5U,  4U,  3U,  8U,  9U,  2U,
        10U, 11U, 10U, 5U,  4U,  3U,  2U,  1U,  0U,  1U,  11U, 10U, 9U,  6U,  7U,
        8U,  3U,  2U,  3U,  2U,  14U, 9U,  8U,  7U,  6U,  5U,  4U,  11U, 4U,  5U,
        13U, 12U, 9U,  10U, 7U,  6U,  5U,  10U, 7U,  6U,  12U, 11U, 10U, 9U,  8U,
        7U,  8U,  9U,  8U,  9U,  16U, 15U, 14U, 13U, 12U, 9U,  8U,  9U,  10U, 11U,
        13U, 14U, 15U, 12U, 11U, 10U, 7U,  6U,  11U, 12U, 12U, 11U, 16U, 11U, 12U,
        11U, 6U,  5U,  4U,  13U, 11U, 10U, 9U,  10U, 7U,  6U,  5U,  6U,  3U,  2U,
        10U, 9U,  8U,  7U,  6U,  5U,  4U,  7U,  8U,  1U,  11U, 12U, 11U, 6U,  5U,
        4U,  3U,  2U,  1U,  0U,  12U, 11U, 10U, 7U,  8U,  9U,  4U,  3U,  4U,  1U,
        15U, 10U, 9U,  8U,  7U,  6U,  5U,  12U, 5U,  6U,  14U, 13U, 10U, 11U, 8U,
        7U,  6U,  11U, 8U,  7U,  13U, 12U, 11U, 10U, 9U,  8U,  9U,  10U, 9U,  10U,
        12U, 11U, 12U, 11U, 10U, 11U, 12U, 13U, 14U, 15U, 9U,  10U, 13U, 8U,  9U,
        10U, 13U, 12U, 15U, 16U, 8U,  7U,  14U, 7U,  8U,  9U,  10U, 11U, 12U, 17U,
        3U,  6U,  5U,  6U,  11U, 10U, 9U,  12U, 13U, 14U, 2U,  3U,  4U,  5U,  6U,
        7U,  8U,  13U, 14U, 13U, 1U,  6U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 12U,
        0U,  1U,  4U,  5U,  6U,  7U,  12U, 11U, 12U, 13U, 11U, 2U,  3U,  4U,  9U,
        10U, 11U, 14U, 13U, 14U, 10U, 9U,  4U,  5U,  8U,  9U,  10U, 13U, 16U, 15U,
        9U,  8U,  7U,  6U,  7U,  10U, 11U, 12U, 17U, 18U, 13U, 12U, 13U, 12U, 11U,
        12U, 13U, 14U, 15U, 16U, 10U, 11U, 14U, 9U,  10U, 11U, 14U, 13U, 16U, 17U,
        9U,  8U,  15U, 8U,  9U,  10U, 11U, 12U, 13U, 18U, 4U,  7U,  6U,  7U,  12U,
        11U, 10U, 13U, 14U, 13U, 3U,  4U,  5U,  6U,  7U,  8U,  9U,  14U, 15U, 12U,
        2U,  5U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 1U,  0U,  3U,  4U,  5U,
        6U,  11U, 10U, 11U, 12U, 10U, 1U,  2U,  3U,  8U,  9U,  10U, 13U, 12U, 13U,
        9U,  8U,  3U,  4U,  7U,  8U,  9U,  12U, 15U, 14U, 8U,  7U,  6U,  5U,  6U,
        9U,  10U, 11U, 16U, 17U, 14U, 13U, 14U, 13U, 12U, 13U, 14U, 15U, 16U, 17U,
        11U, 12U, 15U, 10U, 11U, 12U, 13U, 12U, 17U, 18U, 10U, 9U,  16U, 9U,  10U,
        11U, 10U, 11U, 12U, 19U, 7U,  8U,  7U,  8U,  11U, 10U, 9U,  12U, 13U, 12U,
        6U,  7U,  6U,  5U,  6U,  7U,  8U,  13U, 14U, 11U, 5U,  2U,  1U,  4U,  5U,
        6U,  7U,  8U,  9U,  10U, 4U,  3U,  0U,  3U,  4U,  5U,  10U, 9U,  10U, 11U,
        9U,  2U,  1U,  2U,  7U,  8U,  9U,  12U, 11U, 12U, 8U,  7U,  2U,  3U,  6U,
        7U,  8U,  11U, 14U, 13U, 7U,  6U,  5U,  4U,  5U,  8U,  9U,  10U, 15U, 16U,
        11U, 10U, 11U, 10U, 9U,  10U, 11U, 12U, 13U, 14U, 8U,  9U,  12U, 7U,  8U,
        9U,  10U, 9U,  14U, 15U, 7U,  6U,  13U, 6U,  7U,  8U,  7U,  8U,  9U,  16U,
        6U,  5U,  4U,  5U,  8U,  7U,  6U,  9U,  10U, 9U,  5U,  4U,  3U,  2U,  3U,
        4U,  5U,  10U, 11U, 8U,  6U,  5U,  4U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,
        5U,  4U,  3U,  0U,  1U,  2U,  7U,  6U,  7U,  8U,  10U, 3U,  2U,  1U,  8U,
        9U,  8U,  13U, 8U,  9U,  9U,  8U,  3U,  4U,  7U,  8U,  9U,  12U, 11U, 10U,
        8U,  7U,  6U,  5U,  6U,  9U,  10U, 11U, 12U, 13U, 12U, 11U, 12U, 11U, 10U,
        11U, 12U, 13U, 14U, 15U, 9U,  10U, 13U, 8U,  9U,  10U, 11U, 10U, 15U, 16U,
        8U,  7U,  14U, 7U,  8U,  9U,  8U,  9U,  10U, 17U, 7U,  6U,  5U,  6U,  9U,
        8U,  7U,  10U, 11U, 10U, 6U,  5U,  4U,  3U,  4U,  5U,  6U,  11U, 12U, 9U,
        7U,  6U,  5U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  6U,  5U,  4U,  1U,  0U,
        1U,  8U,  7U,  8U,  9U,  11U, 4U,  3U,  2U,  9U,  10U, 9U,  14U, 9U,  10U,
        10U, 9U,  4U,  5U,  8U,  9U,  10U, 13U, 12U, 11U, 9U,  8U,  7U,  6U,  7U,
        10U, 11U, 12U, 13U, 14U, 13U, 12U, 13U, 12U, 11U, 12U, 13U, 14U, 15U, 16U,
        10U, 11U, 14U, 9U,  10U, 11U, 12U, 11U, 16U, 17U, 9U,  8U,  15U, 8U,  9U,
        10U, 9U,  10U, 11U, 18U, 8U,  7U,  6U,  7U,  10U, 9U,  8U,  11U, 12U, 11U,
        7U,  6U,  5U,  4U,  5U,  6U,  7U,  12U, 13U, 10U, 8U,  7U,  6U,  3U,  4U,
        5U,  6U,  7U,  8U,  9U,  7U,  6U,  5U,  2U,  1U,  0U,  9U,  8U,  9U,  10U,
        12U, 5U,  4U,  3U,  10U, 11U, 10U, 15U, 10U, 11U, 11U, 10U, 5U,  6U,  9U,
        10U, 11U, 14U, 13U, 12U, 10U, 9U,  8U,  7U,  8U,  11U, 12U, 13U, 14U, 15U,
        16U, 15U, 16U, 15U, 14U, 11U, 10U, 11U, 12U, 13U, 13U, 14U, 17U, 12U, 13U,
        12U, 9U,  8U,  13U, 14U, 12U, 11U, 18U, 11U, 12U, 13U, 6U,  7U,  8U,  15U,
        11U, 10U, 9U,  10U, 7U,  6U,  5U,  8U,  7U,  6U,  10U, 9U,  8U,  7U,  6U,
        5U,  4U,  9U,  10U, 5U,  11U, 12U, 11U, 6U,  5U,  4U,  3U,  2U,  3U,  4U,
        12U, 11U, 10U, 7U,  8U,  9U,  0U,  1U,  2U,  5U,  11U, 10U, 9U,  8U,  3U,
        2U,  1U,  8U,  3U,  4U,  10U, 9U,  8U,  7U,  4U,  3U,  2U,  7U,  6U,  5U,
        9U,  8U,  7U,  6U,  5U,  4U,  5U,  6U,  7U,  8U,  15U, 14U, 15U, 14U, 13U,
        10U, 9U,  10U, 11U, 12U, 12U, 13U, 16U, 11U, 12U, 11U, 8U,  7U,  12U, 13U,
        11U, 10U, 17U, 10U, 11U, 12U, 5U,  6U,  7U,  14U, 10U, 9U,  8U,  9U,  6U,
        5U,  4U,  7U,  6U,  5U,  9U,  8U,  7U,  6U,  5U,  4U,  3U,  8U,  9U,  4U,
        10U, 11U, 10U, 5U,  4U,  3U,  2U,  1U,  2U,  3U,  11U, 10U, 9U,  6U,  7U,
        8U,  1U,  0U,  1U,  4U,  12U, 9U,  8U,  7U,  4U,  3U,  2U,  9U,  2U,  3U,
        11U, 10U, 9U,  8U,  5U,  4U,  3U,  8U,  5U,  4U,  10U, 9U,  8U,  7U,  6U,
        5U,  6U,  7U,  6U,  7U,  16U, 15U, 16U, 15U, 14U, 11U, 10U, 11U, 12U, 13U,
        13U, 14U, 17U, 12U, 13U, 12U, 9U,  8U,  13U, 14U, 12U, 11U, 18U, 11U, 12U,
        13U, 6U,  7U,  8U,  15U, 11U, 10U, 9U,  10U, 7U,  6U,  5U,  8U,  7U,  6U,
        10U, 9U,  8U,  7U,  6U,  5U,  4U,  9U,  10U, 5U,  11U, 12U, 11U, 6U,  5U,
        4U,  3U,  2U,  3U,  4U,  12U, 11U, 10U, 7U,  8U,  9U,  2U,  1U,  0U,  5U,
        13U, 10U, 9U,  8U,  5U,  4U,  3U,  10U, 1U,  2U,  12U, 11U, 10U, 9U,  6U,
        5U,  4U,  9U,  4U,  3U,  11U, 10U, 9U,  8U,  7U,  6U,  7U,  8U,  5U,  6U,
        17U, 16U, 15U, 14U, 13U, 10U, 9U,  10U, 11U, 12U, 14U, 15U, 16U, 13U, 12U,
        11U, 8U,  7U,  12U, 13U, 13U, 12U, 17U, 12U, 13U, 12U, 7U,  6U,  5U,  14U,
        12U, 11U, 10U, 11U, 8U,  7U,  6U,  7U,  4U,  3U,  11U, 10U, 9U,  8U,  7U,
        6U,  5U,  8U,  9U,  2U,  12U, 13U, 12U, 7U,  6U,  5U,  4U,  3U,  2U,  1U,
        13U, 12U, 11U, 8U,  9U,  10U, 5U,  4U,  5U,  0U,  16U, 11U, 10U, 9U,  8U,
        7U,  6U,  13U, 6U,  7U,  15U, 14U, 11U, 12U, 9U,  8U,  7U,  12U, 9U,  8U,
        14U, 13U, 12U, 11U, 10U, 9U,  10U, 11U, 10U, 11U, 21U, 20U, 21U, 20U, 19U,
        20U, 21U, 22U, 23U, 24U, 18U, 19U, 22U, 17U, 18U, 19U, 20U, 19U, 24U, 25U,
        17U, 16U, 23U, 16U, 17U, 18U, 17U, 18U, 19U, 26U, 14U, 15U, 14U, 15U, 18U,
        17U, 16U, 19U, 18U, 17U, 13U, 14U, 13U, 12U, 13U, 14U, 15U, 20U, 21U, 16U,
        12U, 11U, 10U, 11U, 12U, 13U, 14U, 13U, 14U, 15U, 11U, 10U, 9U,  10U, 11U,
        12U, 11U, 12U, 13U, 16U, 0U,  9U,  8U,  9U,  8U,  9U,  10U, 13U, 14U, 15U,
        1U,  4U,  7U,  6U,  7U,  8U,  9U,  12U, 17U, 16U, 2U,  3U,  4U,  5U,  6U,
        9U,  10U, 11U, 18U, 19U, 14U, 13U, 14U, 13U, 12U, 13U, 14U, 15U, 16U, 17U,
        11U, 12U, 15U, 10U, 11U, 12U, 13U, 12U, 17U, 18U, 10U, 9U,  16U, 9U,  10U,
        11U, 10U, 11U, 12U, 19U, 5U,  8U,  7U,  8U,  11U, 10U, 9U,  12U, 13U, 12U,
        4U,  5U,  6U,  5U,  6U,  7U,  8U,  13U, 14U, 11U, 3U,  4U,  3U,  4U,  5U,
        6U,  7U,  8U,  9U,  10U, 2U,  1U,  2U,  3U,  4U,  5U,  10U, 9U,  10U, 11U,
        9U,  0U,  1U,  2U,  7U,  8U,  9U,  12U, 11U, 12U, 8U,  7U,  2U,  3U,  6U,
        7U,  8U,  11U, 14U, 13U, 7U,  6U,  5U,  4U,  5U,  8U,  9U,  10U, 15U, 16U,
        13U, 12U, 13U, 12U, 11U, 12U, 13U, 14U, 15U, 16U, 10U, 11U, 14U, 9U,  10U,
        11U, 12U, 11U, 16U, 17U, 9U,  8U,  15U, 8U,  9U,  10U, 9U,  10U, 11U, 18U,
        6U,  7U,  6U,  7U,  10U, 9U,  8U,  11U, 12U, 11U, 5U,  6U,  5U,  4U,  5U,
        6U,  7U,  12U, 13U, 10U, 4U,  3U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,
        3U,  2U,  1U,  2U,  3U,  4U,  9U,  8U,  9U,  10U, 8U,  1U,  0U,  1U,  6U,
        7U,  8U,  11U, 10U, 11U, 7U,  6U,  1U,  2U,  5U,  6U,  7U,  10U, 13U, 12U,
        6U,  5U,  4U,  3U,  4U,  7U,  8U,  9U,  14U, 15U, 12U, 11U, 12U, 11U, 10U,
        11U, 12U, 13U, 14U, 15U, 9U,  10U, 13U, 8U,  9U,  10U, 11U, 10U, 15U, 16U,
        8U,  7U,  14U, 7U,  8U,  9U,  8U,  9U,  10U, 17U, 7U,  6U,  5U,  6U,  9U,
        8U,  7U,  10U, 11U, 10U, 6U,  5U,  4U,  3U,  4U,  5U,  6U,  11U, 12U, 9U,
        5U,  4U,  3U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  4U,  3U,  2U,  1U,  2U,
        3U,  8U,  7U,  8U,  9U,  9U,  2U,  1U,  0U,  7U,  8U,  9U,  12U, 9U,  10U,
        8U,  7U,  2U,  3U,  6U,  7U,  8U,  11U, 12U, 11U, 7U,  6U,  5U,  4U,  5U,
        8U,  9U,  10U, 13U, 14U, 19U, 18U, 19U, 18U, 17U, 14U, 13U, 14U, 15U, 16U,
        16U, 17U, 20U, 15U, 16U, 15U, 12U, 11U, 16U, 17U, 15U, 14U, 21U, 14U, 15U,
        16U, 9U,  10U, 11U, 18U, 12U, 13U, 12U, 13U, 10U, 9U,  8U,  11U, 10U, 9U,
        11U, 12U, 11U, 10U, 9U,  8U,  7U,  12U, 13U, 8U,  10U, 9U,  8U,  9U,  8U,
        7U,  6U,  5U,  6U,  7U,  9U,  8U,  7U,  8U,  9U,  10U, 3U,  4U,  5U,  8U,
        8U,  7U,  6U,  7U,  0U,  1U,  2U,  7U,  6U,  7U,  7U,  6U,  5U,  4U,  1U,
        2U,  3U,  6U,  9U,  8U,  6U,  5U,  4U,  3U,  2U,  3U,  4U,  5U,  10U, 11U,
        18U, 17U, 18U, 17U, 16U, 13U, 12U, 13U, 14U, 15U, 15U, 16U, 19U, 14U, 15U,
        14U, 11U, 10U, 15U, 16U, 14U, 13U, 20U, 13U, 14U, 15U, 8U,  9U,  10U, 17U,
        13U, 12U, 11U, 12U, 9U,  8U,  7U,  10U, 9U,  8U,  12U, 11U, 10U, 9U,  8U,
        7U,  6U,  11U, 12U, 7U,  11U, 10U, 9U,  8U,  7U,  6U,  5U,  4U,  5U,  6U,
        10U, 9U,  8U,  9U,  10U, 11U, 2U,  3U,  4U,  7U,  9U,  8U,  7U,  8U,  1U,
        0U,  1U,  8U,  5U,  6U,  8U,  7U,  6U,  5U,  2U,  3U,  2U,  7U,  8U,  7U,
        7U,  6U,  5U,  4U,  3U,  4U,  5U,  6U,  9U,  10U, 17U, 16U, 17U, 16U, 15U,
        12U, 11U, 12U, 13U, 14U, 14U, 15U, 18U, 13U, 14U, 13U, 10U, 9U,  14U, 15U,
        13U, 12U, 19U, 12U, 13U, 14U, 7U,  8U,  9U,  16U, 12U, 11U, 10U, 11U, 8U,
        7U,  6U,  9U,  8U,  7U,  11U, 10U, 9U,  8U,  7U,  6U,  5U,  10U, 11U, 6U,
        12U, 11U, 10U, 7U,  6U,  5U,  4U,  3U,  4U,  5U,  11U, 10U, 9U,  8U,  9U,
        10U, 1U,  2U,  3U,  6U,  10U, 9U,  8U,  9U,  2U,  1U,  0U,  7U,  4U,  5U,
        9U,  8U,  7U,  6U,  3U,  2U,  1U,  6U,  7U,  6U,  8U,  7U,  6U,  5U,  4U,
        3U,  4U,  5U,  8U,  9U,  24U, 23U, 24U, 23U, 22U, 19U, 18U, 19U, 20U, 21U,
        21U, 22U, 25U, 20U, 21U, 20U, 17U, 16U, 21U, 22U, 20U, 19U, 26U, 19U, 20U,
        21U, 14U, 15U, 16U, 23U, 17U, 18U, 17U, 18U, 15U, 14U, 13U, 16U, 15U, 14U,
        16U, 17U, 16U, 15U, 14U, 13U, 12U, 17U, 18U, 13U, 15U, 14U, 13U, 14U, 13U,
        12U, 11U, 10U, 11U, 12U, 14U, 13U, 12U, 13U, 14U, 15U, 8U,  9U,  10U, 13U,
        13U, 12U, 11U, 12U, 7U,  8U,  7U,  0U,  11U, 12U, 12U, 11U, 10U, 9U,  6U,
        5U,  6U,  1U,  14U, 13U, 11U, 10U, 9U,  8U,  7U,  4U,  3U,  2U,  15U, 16U,
        17U, 16U, 17U, 16U, 15U, 12U, 11U, 12U, 13U, 14U, 14U, 15U, 18U, 13U, 14U,
        13U, 10U, 9U,  14U, 15U, 13U, 12U, 19U, 12U, 13U, 14U, 7U,  8U,  9U,  16U,
        12U, 11U, 10U, 11U, 8U,  7U,  6U,  9U,  8U,  7U,  11U, 10U, 9U,  8U,  7U,
        6U,  5U,  10U, 11U, 6U,  12U, 13U, 12U, 7U,  6U,  5U,  4U,  3U,  4U,  5U,
        13U, 12U, 11U, 8U,  9U,  10U, 3U,  2U,  1U,  6U,  14U, 11U, 10U, 9U,  6U,
        5U,  4U,  11U, 0U,  1U,  13U, 12U, 11U, 10U, 7U,  6U,  5U,  10U, 3U,  2U,
        12U, 11U, 10U, 9U,  8U,  7U,  8U,  9U,  4U,  5U,  18U, 17U, 18U, 17U, 16U,
        13U, 12U, 13U, 14U, 15U, 15U, 16U, 19U, 14U, 15U, 14U, 11U, 10U, 15U, 16U,
        14U, 13U, 20U, 13U, 14U, 15U, 8U,  9U,  10U, 17U, 13U, 12U, 11U, 12U, 9U,
        8U,  7U,  10U, 9U,  8U,  12U, 11U, 10U, 9U,  8U,  7U,  6U,  11U, 12U, 7U,
        13U, 14U, 13U, 8U,  7U,  6U,  5U,  4U,  5U,  6U,  14U, 13U, 12U, 9U,  10U,
        11U, 4U,  3U,  2U,  7U,  15U, 12U, 11U, 10U, 7U,  6U,  5U,  12U, 1U,  0U,
        14U, 13U, 12U, 11U, 8U,  7U,  6U,  11U, 2U,  1U,  13U, 12U, 11U, 10U, 9U,
        8U,  9U,  10U, 3U,  4U,  20U, 19U, 20U, 19U, 18U, 19U, 20U, 21U, 22U, 23U,
        17U, 18U, 21U, 16U, 17U, 18U, 19U, 18U, 23U, 24U, 16U, 15U, 22U, 15U, 16U,
        17U, 16U, 17U, 18U, 25U, 13U, 14U, 13U, 14U, 17U, 16U, 15U, 18U, 17U, 16U,
        12U, 13U, 12U, 11U, 12U, 13U, 14U, 19U, 20U, 15U, 11U, 10U, 9U,  10U, 11U,
        12U, 13U, 12U, 13U, 14U, 10U, 9U,  8U,  9U,  10U, 11U, 10U, 11U, 12U, 15U,
        1U,  8U,  7U,  8U,  7U,  8U,  9U,  12U, 13U, 14U, 0U,  3U,  6U,  5U,  6U,
        7U,  8U,  11U, 16U, 15U, 1U,  2U,  3U,  4U,  5U,  8U,  9U,  10U, 17U, 18U,
        19U, 18U, 19U, 18U, 17U, 18U, 19U, 20U, 21U, 22U, 16U, 17U, 20U, 15U, 16U,
        17U, 18U, 17U, 22U, 23U, 15U, 14U, 21U, 14U, 15U, 16U, 15U, 16U, 17U, 24U,
        12U, 13U, 12U, 13U, 16U, 15U, 14U, 17U, 16U, 15U, 11U, 12U, 11U, 10U, 11U,
        12U, 13U, 18U, 19U, 14U, 10U, 9U,  8U,  9U,  10U, 11U, 12U, 11U, 12U, 13U,
        9U,  8U,  7U,  8U,  9U,  10U, 9U,  10U, 11U, 14U, 4U,  7U,  6U,  7U,  6U,
        7U,  8U,  11U, 12U, 13U, 3U,  0U,  5U,  4U,  5U,  6U,  7U,  10U, 15U, 14U,
        2U,  1U,  2U,  3U,  4U,  7U,  8U,  9U,  16U, 17U, 14U, 13U, 14U, 13U, 12U,
        13U, 14U, 15U, 16U, 17U, 11U, 12U, 15U, 10U, 11U, 12U, 13U, 12U, 17U, 18U,
        10U, 9U,  16U, 9U,  10U, 11U, 10U, 11U, 12U, 19U, 7U,  8U,  7U,  8U,  11U,
        10U, 9U,  12U, 13U, 12U, 6U,  7U,  6U,  5U,  6U,  7U,  8U,  13U, 14U, 11U,
        5U,  4U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 4U,  3U,  2U,  3U,  4U,
        5U,  8U,  9U,  10U, 11U, 7U,  2U,  1U,  2U,  5U,  6U,  7U,  10U, 11U, 12U,
        6U,  5U,  0U,  1U,  4U,  5U,  6U,  9U,  14U, 13U, 5U,  4U,  3U,  2U,  3U,
        6U,  7U,  8U,  15U, 16U, 15U, 14U, 15U, 14U, 13U, 14U, 15U, 16U, 17U, 18U,
        12U, 13U, 16U, 11U, 12U, 13U, 14U, 13U, 18U, 19U, 11U, 10U, 17U, 10U, 11U,
        12U, 11U, 12U, 13U, 20U, 8U,  9U,  8U,  9U,  12U, 11U, 10U, 13U, 14U, 13U,
        7U,  8U,  7U,  6U,  7U,  8U,  9U,  14U, 15U, 12U, 6U,  5U,  4U,  5U,  6U,
        7U,  8U,  9U,  10U, 11U, 5U,  4U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  12U,
        6U,  3U,  2U,  3U,  4U,  5U,  6U,  9U,  10U, 11U, 5U,  4U,  1U,  0U,  3U,
        4U,  5U,  8U,  13U, 12U, 4U,  3U,  2U,  1U,  2U,  5U,  6U,  7U,  14U, 15U,
        18U, 17U, 18U, 17U, 16U, 15U, 14U, 15U, 16U, 17U, 15U, 16U, 19U, 14U, 15U,
        16U, 13U, 12U, 17U, 18U, 14U, 13U, 20U, 13U, 14U, 15U, 10U, 11U, 12U, 19U,
        11U, 12U, 11U, 12U, 11U, 10U, 9U,  12U, 11U, 10U, 10U, 11U, 10U, 9U,  10U,
        9U,  8U,  13U, 14U, 9U,  9U,  8U,  7U,  8U,  9U,  8U,  7U,  6U,  7U,  8U,
        8U,  7U,  6U,  7U,  8U,  9U,  4U,  5U,  6U,  9U,  7U,  6U,  5U,  6U,  1U,
        2U,  3U,  6U,  7U,  8U,  6U,  5U,  4U,  3U,  0U,  1U,  2U,  5U,  10U, 9U,
        5U,  4U,  3U,  2U,  1U,  2U,  3U,  4U,  11U, 12U, 19U, 18U, 19U, 18U, 17U,
        14U, 13U, 14U, 15U, 16U, 16U, 17U, 20U, 15U, 16U, 15U, 12U, 11U, 16U, 17U,
        15U, 14U, 21U, 14U, 15U, 16U, 9U,  10U, 11U, 18U, 12U, 13U, 12U, 13U, 10U,
        9U,  8U,  11U, 10U, 9U,  11U, 12U, 11U, 10U, 9U,  8U,  7U,  12U, 13U, 8U,
        10U, 9U,  8U,  9U,  8U,  7U,  6U,  5U,  6U,  7U,  9U,  8U,  7U,  8U,  9U,
        10U, 3U,  4U,  5U,  8U,  8U,  7U,  6U,  7U,  2U,  3U,  2U,  5U,  6U,  7U,
        7U,  6U,  5U,  4U,  1U,  0U,  1U,  4U,  9U,  8U,  6U,  5U,  4U,  3U,  2U,
        1U,  2U,  3U,  10U, 11U, 18U, 17U, 18U, 17U, 16U, 13U, 12U, 13U, 14U, 15U,
        15U, 16U, 19U, 14U, 15U, 14U, 11U, 10U, 15U, 16U, 14U, 13U, 20U, 13U, 14U,
        15U, 8U,  9U,  10U, 17U, 13U, 12U, 11U, 12U, 9U,  8U,  7U,  10U, 9U,  8U,
        12U, 11U, 10U, 9U,  8U,  7U,  6U,  11U, 12U, 7U,  11U, 10U, 9U,  8U,  7U,
        6U,  5U,  4U,  5U,  6U,  10U, 9U,  8U,  9U,  10U, 11U, 2U,  3U,  4U,  7U,
        9U,  8U,  7U,  8U,  3U,  2U,  1U,  6U,  5U,  6U,  8U,  7U,  6U,  5U,  2U,
        1U,  0U,  5U,  8U,  7U,  7U,  6U,  5U,  4U,  3U,  2U,  3U,  4U,  9U,  10U,
        23U, 22U, 23U, 22U, 21U, 18U, 17U, 18U, 19U, 20U, 20U, 21U, 24U, 19U, 20U,
        19U, 16U, 15U, 20U, 21U, 19U, 18U, 25U, 18U, 19U, 20U, 13U, 14U, 15U, 22U,
        16U, 17U, 16U, 17U, 14U, 13U, 12U, 15U, 14U, 13U, 15U, 16U, 15U, 14U, 13U,
        12U, 11U, 16U, 17U, 12U, 14U, 13U, 12U, 13U, 12U, 11U, 10U, 9U,  10U, 11U,
        13U, 12U, 11U, 12U, 13U, 14U, 7U,  8U,  9U,  12U, 12U, 11U, 10U, 11U, 6U,
        7U,  6U,  1U,  10U, 11U, 11U, 10U, 9U,  8U,  5U,  4U,  5U,  0U,  13U, 12U,
        10U, 9U,  8U,  7U,  6U,  3U,  2U,  1U,  14U, 15U, 20U, 19U, 20U, 19U, 18U,
        15U, 14U, 15U, 16U, 17U, 17U, 18U, 21U, 16U, 17U, 16U, 13U, 12U, 17U, 18U,
        16U, 15U, 22U, 15U, 16U, 17U, 10U, 11U, 12U, 19U, 15U, 14U, 13U, 14U, 11U,
        10U, 9U,  12U, 11U, 10U, 14U, 13U, 12U, 11U, 10U, 9U,  8U,  13U, 14U, 9U,
        15U, 16U, 15U, 10U, 9U,  8U,  7U,  6U,  7U,  8U,  16U, 15U, 14U, 11U, 12U,
        13U, 6U,  5U,  4U,  9U,  17U, 14U, 13U, 12U, 9U,  8U,  7U,  14U, 3U,  2U,
        16U, 15U, 14U, 13U, 10U, 9U,  8U,  13U, 0U,  1U,  15U, 14U, 13U, 12U, 11U,
        10U, 11U, 12U, 1U,  2U,  19U, 18U, 19U, 18U, 17U, 14U, 13U, 14U, 15U, 16U,
        16U, 17U, 20U, 15U, 16U, 15U, 12U, 11U, 16U, 17U, 15U, 14U, 21U, 14U, 15U,
        16U, 9U,  10U, 11U, 18U, 14U, 13U, 12U, 13U, 10U, 9U,  8U,  11U, 10U, 9U,
        13U, 12U, 11U, 10U, 9U,  8U,  7U,  12U, 13U, 8U,  14U, 15U, 14U, 9U,  8U,
        7U,  6U,  5U,  6U,  7U,  15U, 14U, 13U, 10U, 11U, 12U, 5U,  4U,  3U,  8U,
        16U, 13U, 12U, 11U, 8U,  7U,  6U,  13U, 2U,  1U,  15U, 14U, 13U, 12U, 9U,
        8U,  7U,  12U, 1U,  0U,  14U, 13U, 12U, 11U, 10U, 9U,  10U, 11U, 2U,  3U,
        19U, 18U, 19U, 18U, 17U, 18U, 19U, 20U, 21U, 22U, 16U, 17U, 20U, 15U, 16U,
        17U, 18U, 17U, 22U, 23U, 15U, 14U, 21U, 14U, 15U, 16U, 15U, 16U, 17U, 24U,
        12U, 13U, 12U, 13U, 16U, 15U, 14U, 17U, 16U, 15U, 11U, 12U, 11U, 10U, 11U,
        12U, 13U, 18U, 19U, 14U, 10U, 9U,  8U,  9U,  10U, 11U, 12U, 11U, 12U, 13U,
        9U,  8U,  7U,  8U,  9U,  10U, 9U,  10U, 11U, 14U, 2U,  7U,  6U,  7U,  6U,
        7U,  8U,  11U, 12U, 13U, 1U,  2U,  5U,  4U,  5U,  6U,  7U,  10U, 15U, 14U,
        0U,  1U,  2U,  3U,  4U,  7U,  8U,  9U,  16U, 17U, 18U, 17U, 18U, 17U, 16U,
        17U, 18U, 19U, 20U, 21U, 15U, 16U, 19U, 14U, 15U, 16U, 17U, 16U, 21U, 22U,
        14U, 13U, 20U, 13U, 14U, 15U, 14U, 15U, 16U, 23U, 11U, 12U, 11U, 12U, 15U,
        14U, 13U, 16U, 15U, 14U, 10U, 11U, 10U, 9U,  10U, 11U, 12U, 17U, 18U, 13U,
        9U,  8U,  7U,  8U,  9U,  10U, 11U, 10U, 11U, 12U, 8U,  7U,  6U,  7U,  8U,
        9U,  8U,  9U,  10U, 13U, 3U,  6U,  5U,  6U,  5U,  6U,  7U,  10U, 11U, 12U,
        2U,  1U,  4U,  3U,  4U,  5U,  6U,  9U,  14U, 13U, 1U,  0U,  1U,  2U,  3U,
        6U,  7U,  8U,  15U, 16U, 17U, 16U, 17U, 16U, 15U, 16U, 17U, 18U, 19U, 20U,
        14U, 15U, 18U, 13U, 14U, 15U, 16U, 15U, 20U, 21U, 13U, 12U, 19U, 12U, 13U,
        14U, 13U, 14U, 15U, 22U, 10U, 11U, 10U, 11U, 14U, 13U, 12U, 15U, 14U, 13U,
        9U,  10U, 9U,  8U,  9U,  10U, 11U, 16U, 17U, 12U, 8U,  7U,  6U,  7U,  8U,
        9U,  10U, 9U,  10U, 11U, 7U,  6U,  5U,  6U,  7U,  8U,  7U,  8U,  9U,  12U,
        4U,  5U,  4U,  5U,  4U,  5U,  6U,  9U,  10U, 11U, 3U,  2U,  3U,  2U,  3U,
        4U,  5U,  8U,  13U, 12U, 2U,  1U,  0U,  1U,  2U,  5U,  6U,  7U,  14U, 15U,
        16U, 15U, 16U, 15U, 14U, 15U, 16U, 17U, 18U, 19U, 13U, 14U, 17U, 12U, 13U,
        14U, 15U, 14U, 19U, 20U, 12U, 11U, 18U, 11U, 12U, 13U, 12U, 13U, 14U, 21U,
        9U,  10U, 9U,  10U, 13U, 12U, 11U, 14U, 13U, 12U, 8U,  9U,  8U,  7U,  8U,
        9U,  10U, 15U, 16U, 11U, 7U,  6U,  5U,  6U,  7U,  8U,  9U,  8U,  9U,  10U,
        6U,  5U,  4U,  5U,  6U,  7U,  6U,  7U,  8U,  11U, 5U,  4U,  3U,  4U,  3U,
        4U,  5U,  8U,  9U,  10U, 4U,  3U,  2U,  1U,  2U,  3U,  4U,  7U,  12U, 11U,
        3U,  2U,  1U,  0U,  1U,  4U,  5U,  6U,  13U, 14U, 17U, 16U, 17U, 16U, 15U,
        16U, 15U, 16U, 17U, 18U, 14U, 15U, 18U, 13U, 14U, 15U, 14U, 13U, 18U, 19U,
        13U, 12U, 19U, 12U, 13U, 14U, 11U, 12U, 13U, 20U, 10U, 11U, 10U, 11U, 12U,
        11U, 10U, 13U, 12U, 11U, 9U,  10U, 9U,  8U,  9U,  10U, 9U,  14U, 15U, 10U,
        8U,  7U,  6U,  7U,  8U,  9U,  8U,  7U,  8U,  9U,  7U,  6U,  5U,  6U,  7U,
        8U,  5U,  6U,  7U,  10U, 6U,  5U,  4U,  5U,  2U,  3U,  4U,  7U,  8U,  9U,
        5U,  4U,  3U,  2U,  1U,  2U,  3U,  6U,  11U, 10U, 4U,  3U,  2U,  1U,  0U,
        3U,  4U,  5U,  12U, 13U, 20U, 19U, 20U, 19U, 18U, 15U, 14U, 15U, 16U, 17U,
        17U, 18U, 21U, 16U, 17U, 16U, 13U, 12U, 17U, 18U, 16U, 15U, 22U, 15U, 16U,
        17U, 10U, 11U, 12U, 19U, 13U, 14U, 13U, 14U, 11U, 10U, 9U,  12U, 11U, 10U,
        12U, 13U, 12U, 11U, 10U, 9U,  8U,  13U, 14U, 9U,  11U, 10U, 9U,  10U, 9U,
        8U,  7U,  6U,  7U,  8U,  10U, 9U,  8U,  9U,  10U, 11U, 4U,  5U,  6U,  9U,
        9U,  8U,  7U,  8U,  3U,  4U,  3U,  4U,  7U,  8U,  8U,  7U,  6U,  5U,  2U,
        1U,  2U,  3U,  10U, 9U,  7U,  6U,  5U,  4U,  3U,  0U,  1U,  2U,  11U, 12U,
        21U, 20U, 21U, 20U, 19U, 16U, 15U, 16U, 17U, 18U, 18U, 19U, 22U, 17U, 18U,
        17U, 14U, 13U, 18U, 19U, 17U, 16U, 23U, 16U, 17U, 18U, 11U, 12U, 13U, 20U,
        14U, 15U, 14U, 15U, 12U, 11U, 10U, 13U, 12U, 11U, 13U, 14U, 13U, 12U, 11U,
        10U, 9U,  14U, 15U, 10U, 12U, 11U, 10U, 11U, 10U, 9U,  8U,  7U,  8U,  9U,
        11U, 10U, 9U,  10U, 11U, 12U, 5U,  6U,  7U,  10U, 10U, 9U,  8U,  9U,  4U,
        5U,  4U,  3U,  8U,  9U,  9U,  8U,  7U,  6U,  3U,  2U,  3U,  2U,  11U, 10U,
        8U,  7U,  6U,  5U,  4U,  1U,  0U,  1U,  12U, 13U, 22U, 21U, 22U, 21U, 20U,
        17U, 16U, 17U, 18U, 19U, 19U, 20U, 23U, 18U, 19U, 18U, 15U, 14U, 19U, 20U,
        18U, 17U, 24U, 17U, 18U, 19U, 12U, 13U, 14U, 21U, 15U, 16U, 15U, 16U, 13U,
        12U, 11U, 14U, 13U, 12U, 14U, 15U, 14U, 13U, 12U, 11U, 10U, 15U, 16U, 11U,
        13U, 12U, 11U, 12U, 11U, 10U, 9U,  8U,  9U,  10U, 12U, 11U, 10U, 11U, 12U,
        13U, 6U,  7U,  8U,  11U, 11U, 10U, 9U,  10U, 5U,  6U,  5U,  2U,  9U,  10U,
        10U, 9U,  8U,  7U,  4U,  3U,  4U,  1U,  12U, 11U, 9U,  8U,  7U,  6U,  5U,
        2U,  1U,  0U,  13U, 14U, 21U, 20U, 21U, 20U, 19U, 16U, 15U, 16U, 17U, 18U,
        18U, 19U, 22U, 17U, 18U, 17U, 14U, 13U, 18U, 19U, 17U, 16U, 23U, 16U, 17U,
        18U, 11U, 12U, 13U, 20U, 16U, 15U, 14U, 15U, 12U, 11U, 10U, 13U, 12U, 11U,
        15U, 14U, 13U, 12U, 11U, 10U, 9U,  14U, 15U, 10U, 16U, 17U, 16U, 11U, 10U,
        9U,  8U,  7U,  8U,  9U,  17U, 16U, 15U, 12U, 13U, 14U, 7U,  6U,  5U,  10U,
        18U, 15U, 14U, 13U, 10U, 9U,  8U,  15U, 4U,  3U,  17U, 16U, 15U, 14U, 11U,
        10U, 9U,  14U, 1U,  2U,  16U, 15U, 14U, 13U, 12U, 11U, 12U, 13U, 0U,  1U,
        22U, 21U, 22U, 21U, 20U, 17U, 16U, 17U, 18U, 19U, 19U, 20U, 23U, 18U, 19U,
        18U, 15U, 14U, 19U, 20U, 18U, 17U, 24U, 17U, 18U, 19U, 12U, 13U, 14U, 21U,
        17U, 16U, 15U, 16U, 13U, 12U, 11U, 14U, 13U, 12U, 16U, 15U, 14U, 13U, 12U,
        11U, 10U, 15U, 16U, 11U, 17U, 18U, 17U, 12U, 11U, 10U, 9U,  8U,  9U,  10U,
        18U, 17U, 16U, 13U, 14U, 15U, 8U,  7U,  6U,  11U, 19U, 16U, 15U, 14U, 11U,
        10U, 9U,  16U, 5U,  4U,  18U, 17U, 16U, 15U, 12U, 11U, 10U, 15U, 2U,  3U,
        17U, 16U, 15U, 14U, 13U, 12U, 13U, 14U, 1U,  0U };
    for(uint16_t i=0;i<1000;i++){
      output_movepath[i]=0;
      output_movepath_char[i]=0;
    }
    output_movepath_char[1001]=0;
    output_movepath_len=0;
    emxArray_real_T* b_min_path;
    emxArray_real_T* r;
    emxArray_uint8_T* move_path;
    real_T c_i;
    real_T* r1;
    int16_t d_i;
    int16_t q0;
    uint16_t qq;
    uint16_t temp;
    int8_t b_i;
    int8_t j;
    //下一�??点是否为宝藏标志�??
    int8_t is_treasure=0;
    //下一�??点和上一�??点的距�??
    int16_t distance2=0;
    uint8_t min_path[10];
    uint8_t node_list[10];
    uint8_t distance_min=255;
    uint8_t distance_temp;
    uint8_t* move_path_data;
    distance_temp = 0U;

    sort(deposits_list);
    node_list[0] = 1U;
    for (temp = 0; temp < 8; temp++) {
        node_list[temp + 1] = deposits_list[temp];
    }
    node_list[9] = 100U;
    for (q0 = 0; q0 < 9; q0++) {
        /* 计算最短距离*/
        distance_temp += b_uv[(((uint16_t)node_list[q0])+(100*(((uint16_t)node_list[q0 + 1]) - 1))) -1];
    }
    distance_min = distance_temp;
    for (temp = 0; temp < 10; temp++) {
        min_path[temp] = node_list[temp];
    }
    int32_T exitg1;
    do {
        exitg1 = 0L;
        b_i = 7;
        while ((b_i >= 1) && (deposits_list[b_i - 1] >= deposits_list[b_i])) {
            b_i--;
        }
        if (b_i < 1) {
            exitg1 = 1L;
        }
        else {
            j = 8;
            while ((j > b_i) && (deposits_list[j - 1] <= deposits_list[b_i - 1])) {
                j--;
            }
            distance_temp = deposits_list[b_i - 1];
            deposits_list[b_i - 1] = deposits_list[j - 1];
            deposits_list[j - 1] = distance_temp;
            b_i++;
            j = 8;
            while (b_i < j) {
                distance_temp = deposits_list[b_i - 1];
                deposits_list[b_i - 1] = deposits_list[j - 1];
                deposits_list[j - 1] = distance_temp;
                b_i++;
                j--;
            }
            node_list[0] = 1U;
            for (temp = 0; temp < 8; temp++) {
                node_list[temp + 1] = deposits_list[temp];
            }
            node_list[9] = 100U;
            distance_temp = 0U;
            for (q0 = 0; q0 < 9; q0++) {
                /* 计算最�????�????�???? */
                distance_temp += b_uv[(((uint16_t)node_list[q0]) +
                    (100 * (((uint16_t)node_list[q0 + 1]) - 1))) -
                    1];
            }
            if (distance_temp < distance_min) {
                distance_min = distance_temp;
                for (temp = 0; temp < 10; temp++) {
                    min_path[temp] = node_list[temp];
                }
            }
        }
    } while (exitg1 == 0L);

    if(stay_min_path==1){
        for(uint8_t i=1;i<9;i++){
            min_path[i]=deposits_list_new[i-1];
        }
    }

    printf("minpath=");
    for(uint8_t i=0;i<10;i++){
        printf("%d ",min_path[i]);
    }
    if(stay_min_path==0){
        for(uint8_t i=0;i<10;i++){
            simple_path_all_state[i][0]=min_path[i];
        }
    }
    for(uint8_t i=0;i<8;i++){
        deposits_list_new[i]=min_path[i+1];
    }
    printf("\n");
    printf("distance_min=%d\n",distance_min);
    /*
    * TEST
    printf("%d\n", distance_min);
    for (temp = 0; temp < 10; temp++) {
        printf("%d\n", min_path[temp]);
    }
    */
    emxInit_uint8_T(&move_path);
    temp = move_path->size[0] * move_path->size[1];
    move_path->size[0] = 1;
    move_path->size[1] = 200;
    emxEnsureCapacity_uint8_T(move_path, temp);
    move_path_data = move_path->data;
    for (temp = 0; temp < 200; temp++) {
        move_path_data[temp] = 0U;
    }
    emxInit_real_T(&r);
    emxInit_real_T(&b_min_path);
    for (qq = 0; qq < 9; qq++) {
        distance_temp = min_path[qq];
        distance_min = min_path[qq + 1];
        if ((qq + 1) == 1) {
            graphBase_shortestpath((uint8_T)1U, min_path[1], r);
            r1 = r->data;
            temp = move_path->size[0] * move_path->size[1];
            move_path->size[0] = 1;
            move_path->size[1] = r->size[1];
            emxEnsureCapacity_uint8_T(move_path, temp);
            move_path_data = move_path->data;
            q0 = r->size[1];
            for (temp = 0; temp < q0; temp++) {
                c_i = rt_roundd_snf(r1[temp]);
                if (c_i < 256.0) {
                    if (c_i >= 0.0) {
                        distance_temp = (uint8_T)c_i;
                    }
                    else {
                        distance_temp = 0U;
                    }
                }
                else if (c_i >= 256.0) {
                    distance_temp = MAX_uint8_T;
                }
                else {
                    distance_temp = 0U;
                }
                move_path_data[temp] = distance_temp;
            }
        }
        else {
            graphBase_shortestpath(distance_temp, distance_min, r);
            r1 = r->data;
            temp = move_path->size[1];
            q0 = r->size[1];
            d_i = move_path->size[0] * move_path->size[1];
            move_path->size[1] += r->size[1];
            emxEnsureCapacity_uint8_T(move_path, d_i);
            move_path_data = move_path->data;
            for (d_i = 0; d_i < q0; d_i++) {
                c_i = rt_roundd_snf(r1[d_i]);
                if (c_i < 256.0) {
                    if (c_i >= 0.0) {
                        distance_temp = (uint8_T)c_i;
                    }
                    else {
                        distance_temp = 0U;
                    }
                }
                else if (c_i >= 256.0) {
                    distance_temp = MAX_uint8_T;
                }
                else {
                    distance_temp = 0U;
                }
                move_path_data[temp + d_i -qq] = distance_temp;
            }
        }
    }

    emxFree_real_T(&b_min_path);
    emxFree_real_T(&r);
    /* node_now为小车当前�?�在的位�????，direction_now为方向，右下左上对应1234 */
    /* node_now=1; */

    b_i = 1;
    //(void)memset(&output_movepath[0], 0, 1000U * (sizeof(uint8_T)));
    c_i = 1.0;
    temp = move_path->size[1];

    //move_path_data储存经历的所有点
    printf("min_path_allpoints=");
    uint8_t enable_75=0;
    uint8_t enable_26=0;
    uint8_t enable_14=0;
    uint8_t enable_87=0;    
    //如果新地图存在26或75
    for(int i=0;i<8;i++){
        if(deposits_list_new[i]==75){
            enable_75=1;
        }
        if(deposits_list_new[i]==26){
            enable_26=1;
        }
        if(deposits_list_new[i]==14){
            enable_14=1;
        }
        if(deposits_list_new[i]==87){
            enable_87=1;
        }       
    }
    //寻宝操作只进行一次，0可进行，1不可进行
    uint8_t once_26=0;
    uint8_t once_75=0;
    uint8_t once_14=0;
    uint8_t once_87=0;
    for(int i=1;i<9;i++){
        if(simple_path_all_state[i][0]==26||simple_path_all_state[i][0]==75){
            for(int k=5;k<temp-8;k++){
                //26特殊操作，第一次寻宝，只寻一次
                // 25 26 16 -> 25 26 25 24 14 15 16
                if(move_path_data[k]==16&&move_path_data[k-1]==26&&move_path_data[k-2]==25&&once_26==0&&enable_26==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+4;j--){
                        move_path_data[j]=move_path_data[j-4];
                    }
                    temp=temp+4;
                    once_26=1;
                    move_path_data[k]=25;
                    move_path_data[k+1]=24;
                    move_path_data[k+2]=14;
                    move_path_data[k+3]=15;
                }
                //不寻宝就绕开26点
                else if(move_path_data[k]==16&&move_path_data[k-1]==26&&move_path_data[k-2]==25){
                    move_path_data[k-1]=15;
                    move_path_data[k-2]=14;
                }
                // 26 25 24 -> 26 16 15 14 24
                if(move_path_data[k]==25&&move_path_data[k-1]==26&&move_path_data[k-2]==16&&once_26==0&&enable_26==1){
                    for(int j=temp-7;j>=k+3;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    temp=temp+2;
                    once_26=1;
                    move_path_data[k]=16;
                    move_path_data[k+1]=15;
                    move_path_data[k+2]=14;
                }
                else if(move_path_data[k]==25&&move_path_data[k-1]==26&&move_path_data[k-2]==16){
                    move_path_data[k]=14;
                    move_path_data[k-1]=15;
                }    
                //75特殊操作
                if(move_path_data[k]==85&&move_path_data[k-1]==75&&move_path_data[k-2]==76&&once_75==0&&enable_75==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+4;j--){
                        move_path_data[j]=move_path_data[j-4];
                    }
                    temp=temp+4;
                    once_75=1;
                    move_path_data[k]=76;
                    move_path_data[k+1]=77;
                    move_path_data[k+2]=87;
                    move_path_data[k+3]=86;
                }
                //不寻宝就绕开75点
                else if(move_path_data[k]==85&&move_path_data[k-1]==75&&move_path_data[k-2]==76){
                    move_path_data[k-1]=86;
                    move_path_data[k-2]=87;
                }
                // 75 76 77 -> 75 85 86 87 77
                if(move_path_data[k]==76&&move_path_data[k-1]==75&&move_path_data[k-2]==85&&once_75==0&&enable_75==1){
                    for(int j=temp-7;j>=k+3;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    temp=temp+2;
                    once_75=1;
                    move_path_data[k]=85;
                    move_path_data[k+1]=86;
                    move_path_data[k+2]=87;
                }
                else if(move_path_data[k]==76&&move_path_data[k-1]==75&&move_path_data[k-2]==85){
                    move_path_data[k]=87;
                    move_path_data[k-1]=86;
                }
            }
            break;
        }
        if(simple_path_all_state[i][0]==14||simple_path_all_state[i][0]==87){
            for(int k=5;k<temp-8;k++){
                //14特殊操作，第一次寻宝，只寻一次
                //24 14 15 5 -> 24 14 24 25 26 16 15 5
                if(move_path_data[k]==5&&move_path_data[k-1]==15&&move_path_data[k-2]==14&&move_path_data[k-3]==24&&once_14==0&&enable_14==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+4;j--){
                        move_path_data[j]=move_path_data[j-4];
                    }
                    temp=temp+4;
                    once_14=1;
                    move_path_data[k-1]=24;
                    move_path_data[k]=25;
                    move_path_data[k+1]=26;
                    move_path_data[k+2]=16;
                    move_path_data[k+3]=15;
                }
                //24 14 15 16-> 24 14 24 25 26 16
                else if(move_path_data[k]==16&&move_path_data[k-1]==15&&move_path_data[k-2]==14&&move_path_data[k-3]==24&&once_14==0&&enable_14==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+2;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    temp=temp+2;
                    once_14=1;
                    move_path_data[k-1]=24;
                    move_path_data[k]=25;
                    move_path_data[k+1]=26;
                    move_path_data[k+2]=16;
                }
                //不寻宝就绕开14点
                else if(move_path_data[k]==16&&move_path_data[k-1]==15&&move_path_data[k-2]==14&&move_path_data[k-3]==24){
                    move_path_data[k-1]=26;
                    move_path_data[k-2]=25;
                }
                else if(move_path_data[k]==24&&move_path_data[k-1]==14&&move_path_data[k-2]==15&&move_path_data[k-3]==16){
                    move_path_data[k-1]=25;
                    move_path_data[k-2]=26;
                }
                // 26 25 24 -> 26 16 15 14 24


                //87特殊操作
                if(move_path_data[k]==77&&move_path_data[k-1]==87&&move_path_data[k-2]==86&&once_87==0&&enable_87==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+4;j--){
                        move_path_data[j]=move_path_data[j-4];
                    }
                    temp=temp+4;
                    once_87=1;
                    move_path_data[k]=86;
                    move_path_data[k+1]=85;
                    move_path_data[k+2]=75;
                    move_path_data[k+3]=76;
                }
                else if(move_path_data[k]==96&&move_path_data[k-1]==86&&move_path_data[k-2]==87&&move_path_data[k-2]==77&&once_87==0&&enable_87==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+4;j--){
                        move_path_data[j]=move_path_data[j-4];
                    }
                    temp=temp+4;
                    once_87=1;
                    move_path_data[k-1]=77;
                    move_path_data[k]=76;
                    move_path_data[k+1]=75;
                    move_path_data[k+2]=85;
                    move_path_data[k+3]=86;
                }
                else if(move_path_data[k]==85&&move_path_data[k-1]==86&&move_path_data[k-2]==87&&move_path_data[k-2]==77&&once_87==0&&enable_87==1){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+2;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    temp=temp+2;
                    once_87=1;
                    move_path_data[k-1]=77;
                    move_path_data[k]=76;
                    move_path_data[k+1]=75;
                    move_path_data[k+2]=85;
                }
                //不寻宝就绕开87点
                /*
                else if(move_path_data[k]==77&&move_path_data[k-1]==87&&move_path_data[k-2]==86){
                    move_path_data[k-1]=76;
                    move_path_data[k-2]=75;
                }
                */
                //不寻宝就绕开87点
                else if(move_path_data[k]==96&&move_path_data[k-1]==86&&move_path_data[k-2]==87&&move_path_data[k-3]==77){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+2;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    move_path_data[k+2]=96;
                    move_path_data[k+1]=86;
                    move_path_data[k]=85;
                    move_path_data[k-1]=75;
                    move_path_data[k-2]=76;
                }
                //不寻宝就绕开87点
                else if(move_path_data[k]==85&&move_path_data[k-1]==86&&move_path_data[k-2]==87&&move_path_data[k-3]==77){
                    move_path_data[k]=85;
                    move_path_data[k-1]=75;
                    move_path_data[k-2]=76;
                }

                else if(move_path_data[k]==77&&move_path_data[k-1]==87&&move_path_data[k-2]==86&&move_path_data[k-3]==96){
                    //最后一位为temp-7
                    for(int j=temp-7;j>=k+2;j--){
                        move_path_data[j]=move_path_data[j-2];
                    }
                    move_path_data[k+2]=77;
                    move_path_data[k+1]=76;
                    move_path_data[k]=75;
                    move_path_data[k-1]=85;
                    move_path_data[k-2]=86;
                }
                //不寻宝就绕开87点
                else if(move_path_data[k]==77&&move_path_data[k-1]==87&&move_path_data[k-2]==86&&move_path_data[k-3]==85){
                    move_path_data[k]=77;
                    move_path_data[k-1]=76;
                    move_path_data[k-2]=75;
                }            
            }
            break;
        }
    }
    for(int i=0;i<temp-8;i++){
        printf("%d ",move_path_data[i]);
    }

    for (qq = 0; qq <= (temp - 2); qq++) {
        uint8_T u;
        /* node_now=qq; */
        
        distance_temp = move_path_data[qq];
        distance_min = move_path_data[qq + 1];
        if (distance_min > 127UL) {
            distance_min = 127U;
        }
        u = distance_temp;
        if (distance_temp > 127UL) {
            u = 127U;
        }
        j = (int8_T)((int16_T)(((int16_T)distance_min) - ((int16_T)u)));
        /* 获得接下来的方向向量 */
        if (j != 1) {
            if (j == -10) {
                /* 向下 */
                j = 2;
            }
            else if (j == 10) {
                /* 向上 */
                j = 4;
            }
            else if (j == -1) {
                /* 向左 */
                j = 3;
            }
            else {
                /* no actions */
            }
        }
        else {
            /* 向右 */
        }
        /* 右转1掉头2左转3直�??4 */
        if (j != b_i) {
            /* 如果方向改变 */
            d_i = ((int16_T)j) - ((int16_T)b_i);
            if (d_i > 127) {
                d_i = 127;
            }
            else if (d_i < -128) {
                d_i = -128;
            }
            else {
                /* no actions */
            }
            if (d_i == 1) {
                /* 右转 */
                output_movepath[((int16_T)c_i) - 1] = 1U;
            }
            else if (d_i == -1) {
                /* 左转 */
                output_movepath[((int16_T)c_i) - 1] = 3U;
            }
            else {
                q0 = -d_i;
                if ((-d_i) > 127) {
                    q0 = 127;
                }
                if (d_i < 0) {
                    q0 = (int16_T)((int8_T)q0);
                }
                else {
                    q0 = d_i;
                }
                if (q0 == 2) {
                    /* 掉头 */
                    output_movepath[((int16_T)c_i) - 1] = 2U;
                }
                else if (d_i == 3) {
                    /* 左转 */
                    output_movepath[((int16_T)c_i) - 1] = 3U;
                }
                else if (d_i == -3) {
                    /* 右转 */
                    output_movepath[((int16_T)c_i) - 1] = 1U;
                }
            }
            c_i++;
        }
        else if ((((int16_T)uv[distance_temp]) -((int16_T)uv[((int16_T)distance_temp) - 1])) >= 3) {
          //直�??
          output_movepath[((int16_T)c_i) - 1] = 4U;
          c_i++;
        }
        is_treasure=0;
        for(int8_t i=0;i<8;i++){
            if(move_path_data[qq+1]==deposits_list[i]){
                is_treasure=1;
            }
        }
        if(is_treasure==1){
            output_movepath[(int16_T)c_i-1]=5;
            c_i++;
            distance2=(int16_T)(move_path_data[qq+1]-move_path_data[qq-1]);
            if((distance2==2||distance2==-2||distance2==-20||distance2==20)&&output_movepath[(int16_T)c_i-3]!=4){
                output_movepath[(int16_T)c_i-1]=7;
                c_i++;
            }
            else{
                output_movepath[(int16_T)c_i-1]=6;
                c_i++;
            }
        }
        b_i = j;
    }
    //�???一�???数为标志�???252
    output_movepath_len=(int16_T)c_i;
    printf("\noutput_movepath_len=%d",output_movepath_len);
    output_movepath_char[0]=252;
    for(int i=0;i<output_movepath_len;i++){
      output_movepath_char[i+1]=output_movepath[i];
    }
    output_movepath_char[output_movepath_len-2]=251;
    output_movepath_char[output_movepath_len-1]=0;
    printf("\noutput_move_direction=");

    int i=0;
    while(output_movepath_char[i]!=0){
        printf("%d ",output_movepath_char[i]);
        i++;
    }
    printf("\n");

    //emxFree_uint8_T(&move_path);
    xTaskCreate(tx_task_stm32, "uart_tx_task", 1024*4, NULL, configMAX_PRIORITIES-1, &tx_task_handle_stm32);
    stay_min_path=0;
    vTaskDelete(get_past_task_handle);
}
int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(uart_num, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}
void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    sendData(TX_TASK_TAG, output_movepath_char);
    vTaskDelete(tx_task_handle);
}

void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    //uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(uart_num, data_k210_recive, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {//接受到数�????
            data_k210_recive[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data_k210_recive);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data_k210_recive, rxBytes, ESP_LOG_INFO);
            if(data_k210_recive[0]==127){
                //strncpy(receive_point_list,data,8);
                for(int i=0;i<8;i++){
                    receive_point_list_uint8[i]=data_k210_recive[i+1];
                    deposits_list[i]=data_k210_recive[i+1];
                    //printf("%d\n",receive_point_list_uint8[i]);
                }
                found_count=0;
                point_now=0;
                xTaskCreate(Output_path_allpoint, "Output_path_allpoint", 1024*24, NULL, configMAX_PRIORITIES-1, &get_past_task_handle);
            }
            /*
            if(is_blue_team==1){
                if(data_k210_recive[0]==103){
                    sendData_stm32("true_trasure",true_tresure);
                    printf("true_tresure\n");
                }
                else if(data_k210_recive[0]==100||data_k210_recive[0]==101||data_k210_recive[0]==102){
                    sendData_stm32("true_trasure",fake_tresure);
                    printf("fake_tresure\n");
                }
            }
            else if(is_red_team==1){
                if(data_k210_recive[0]==100){
                    sendData_stm32("true_trasure",true_tresure);
                    printf("true_tresure\n");
                }
                else if(data_k210_recive[0]==103||data_k210_recive[0]==101||data_k210_recive[0]==102){
                    sendData_stm32("true_trasure",fake_tresure);
                    printf("fake_tresure\n");
                }
            }
            */
            uint8_t i=0;
            uint8_t j=0;
            //int8_t temp=0;
            //不一致，找到最后一个设为point_now
            if(simple_path_all_state[point_now+1][0]!=deposits_list_new[point_now]){
                i=point_now;
                while(deposits_list_new[point_now]==deposits_list_new[i]){
                    i++;
                }
                j=i;
                while(deposits_list_new[i]==deposits_list_new[j]){
                    j++;
                }
                j--;
                point_now=j;

            }
            if(is_blue_team==1){
                if(data_k210_recive[0]==103){
                    point_now++;
                    sendData_stm32("true_trasure",true_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("true_tresure\n");
                }
                else if(data_k210_recive[0]==100){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_k210_recive[0]==101){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_k210_recive[0]==102){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
            }
            else if(is_red_team==1){
                if(data_k210_recive[0]==100){
                    point_now++;
                    sendData_stm32("true_trasure",true_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("true_tresure\n");
                }
                else if(data_k210_recive[0]==101){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_k210_recive[0]==102){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_k210_recive[0]==103){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }                
            }
            printf("point_now=%d\n",point_now);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    free(data_k210_recive);
}
int sendData_stm32(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(uart_num_stm32, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}
void tx_task_stm32(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK_stm32";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    sendData_stm32(TX_TASK_TAG, output_movepath_char);
    //while(1){
     //   sendData_stm32(TX_TASK_TAG, output_movepath_char);
      //  vTaskDelay(2000 / portTICK_PERIOD_MS);
    //}
    vTaskDelete(tx_task_handle_stm32);
}
void tx_task_stm32_true_treasure(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK_stm32_true_treasure";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    //sendData_stm32(TX_TASK_TAG, output_movepath_char);
    while(1){
        sendData_stm32(TX_TASK_TAG, true_tresure);
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(tx_task_handle_stm32);
}
void tx_task_stm32_fake_treasure(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK_stm32_fake_treasure";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    //sendData_stm32(TX_TASK_TAG, output_movepath_char);
    while(1){
        sendData_stm32(TX_TASK_TAG, fake_tresure);
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(tx_task_handle_stm32);
}
void rx_task_stm32(void *arg)
{

    static const char *RX_TASK_TAG = "RX_TASK_stm32";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    //uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(uart_num_stm32, data_stm32_recive, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {//接受到数�??
            data_stm32_recive[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data_stm32_recive);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data_stm32_recive, rxBytes, ESP_LOG_INFO);
            if(data_stm32_recive[0]==126){
                //strncpy(receive_point_list,data,8);
                sendData("enable_ai_sonser",enable_ai_sensor);
                printf("enable_ai_sensor\n");
            }
            else if(data_stm32_recive[0]==111){
                //vTaskDelete(tx_task_handle_stm32);
            }
            //调试用
            uint8_t i=0;
            uint8_t j=0;
            //int8_t temp=0;
            //不一致，找到最后一个设为point_now
            if(simple_path_all_state[point_now+1][0]!=deposits_list_new[point_now]){
                i=point_now;
                while(deposits_list_new[point_now]==deposits_list_new[i]){
                    i++;
                }
                j=i;
                while(deposits_list_new[i]==deposits_list_new[j]){
                    j++;
                }
                j--;
                point_now=j;

            }
            if(data_stm32_recive[0]==127){
                //strncpy(receive_point_list,data,8);
                for(int i=0;i<8;i++){
                    receive_point_list_uint8[i]=data_stm32_recive[i+1];
                    deposits_list[i]=data_stm32_recive[i+1];
                    //printf("%d\n",receive_point_list_uint8[i]);
                }
                found_count=0;
                xTaskCreate(Output_path_allpoint, "Output_path_allpoint", 1024*24, NULL, configMAX_PRIORITIES-1, &get_past_task_handle);
            }
            if(is_blue_team==1){
                if(data_stm32_recive[0]==103){
                    point_now++;
                    sendData_stm32("true_trasure",true_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("true_tresure\n");
                }
                else if(data_stm32_recive[0]==100){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_stm32_recive[0]==101){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_stm32_recive[0]==102){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
            }
            else if(is_red_team==1){
                if(data_stm32_recive[0]==100){
                    sendData_stm32("true_trasure",true_tresure);
                    printf("true_tresure\n");
                }
                else if(data_stm32_recive[0]==101){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_stm32_recive[0]==102){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=1;
                    simple_path_all_state[point_now][2]=2;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }
                else if(data_stm32_recive[0]==103){
                    point_now++;
                    sendData_stm32("fake_tresure",fake_tresure);
                    simple_path_all_state[point_now][1]=2;
                    simple_path_all_state[point_now][2]=1;
                    simple_path_all_state[point_now][3]=1;
                    simple_path_all_state[point_now][6]=1;
                    calulate_simple_path_all_state();
                    printf("fake_tresure\n");
                }                
            }
            printf("point_now=%d\n",point_now);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    free(data_stm32_recive);
}
void calulate_simple_path_all_state(){
    uint8_t i=0;
    uint8_t j=0;
    //uint8_t k=0;
    simple_path_all_state[0][0]=1;
    simple_path_all_state[9][0]=100;
    //计算象限
    for(i=1;i<9;i++){
        if(simple_path_all_state[i][0]%10>5||simple_path_all_state[i][0]%10==0){
            //第一象限
            if(simple_path_all_state[i][0]/10>=5){
                simple_path_all_state[i][4]=1;
            }
            //第4象限
            else if(simple_path_all_state[i][0]/10<5){
                simple_path_all_state[i][4]=4;
            }
        }
        else{
            //第2象限
            if(simple_path_all_state[i][0]/10>=5){
                simple_path_all_state[i][4]=2;
            }
            //第3象限
            else if(simple_path_all_state[i][0]/10<5){
                simple_path_all_state[i][4]=3;
            }
        }
    }
    //关系计算
    for(i=1;i<9;i++){
        //同一象限颜色关系,颜色相反
        //红色
        if(simple_path_all_state[i][1]==1){
            //同一象限另一个为蓝色，真假未知
            for(j=1;j<9;j++){
                if(j==i){
                    continue;
                }
                if(simple_path_all_state[j][4]==simple_path_all_state[i][4]){
                    simple_path_all_state[j][1]=2;
                    break;
                }
            }
        }
        //蓝色
        else if(simple_path_all_state[i][1]==2){
            //同一象限另一个为红色
            for(j=1;j<9;j++){
                if(j==i){
                    continue;
                }
                if(simple_path_all_state[j][4]==simple_path_all_state[i][4]){
                    simple_path_all_state[j][1]=1;
                    break;
                }
            }
        }
        //对角线关系，颜色相反，真假相同
        if(simple_path_all_state[i][6]==1){
            for(j=1;j<9;j++){
                if(j==i){
                    continue;
                }
                if(simple_path_all_state[j][0]==101-simple_path_all_state[i][0]){
                    if(simple_path_all_state[i][1]==1&&simple_path_all_state[i][2]==1){
                        simple_path_all_state[j][1]=2;
                        simple_path_all_state[j][2]=1;
                    }
                    else if(simple_path_all_state[i][1]==1&&simple_path_all_state[i][2]==2){
                        simple_path_all_state[j][1]=2;
                        simple_path_all_state[j][2]=2;
                    }
                    else if(simple_path_all_state[i][1]==2&&simple_path_all_state[i][2]==2){
                        simple_path_all_state[j][1]=1;
                        simple_path_all_state[j][2]=2;
                    }
                    else if(simple_path_all_state[i][1]==2&&simple_path_all_state[i][2]==1){
                        simple_path_all_state[j][1]=1;
                        simple_path_all_state[j][2]=1;
                    }                                        
                }
            }

        }
        //在运行一次
        //同一象限颜色关系,颜色相反
        //红色
        if(simple_path_all_state[i][1]==1){
            //同一象限另一个为蓝色，真假未知
            for(j=1;j<9;j++){
                if(j==i){
                    continue;
                }
                if(simple_path_all_state[j][4]==simple_path_all_state[i][4]){
                    simple_path_all_state[j][1]=2;
                    break;
                }
            }
        }
        //蓝色
        else if(simple_path_all_state[i][1]==2){
            //同一象限另一个为红色
            for(j=1;j<9;j++){
                if(j==i){
                    continue;
                }
                if(simple_path_all_state[j][4]==simple_path_all_state[i][4]){
                    simple_path_all_state[j][1]=1;
                    break;
                }
            }
        }
    }
    
    //是否已经确认
    for(i=1;i<9;i++){
        if(simple_path_all_state[i][1]!=0&&simple_path_all_state[i][2]!=0){
            simple_path_all_state[i][6]=1;
        }
    }
    red_true_treasure=0;
    blue_true_treasure=0;
    //数量关系
    for(i=1;i<9;i++){
        if(simple_path_all_state[i][1]==1&&simple_path_all_state[i][2]==1&&simple_path_all_state[i][3]==1){
            red_true_treasure++;
        }
        else if(simple_path_all_state[i][1]==2&&simple_path_all_state[i][2]==1&&simple_path_all_state[i][3]==1){
            blue_true_treasure++;
        }
    }
    //下一个去的点
    for(i=0;i<10;i++){
        printf("%d %d %d %d %d %d %d %d\n",i,simple_path_all_state[i][0],simple_path_all_state[i][1],simple_path_all_state[i][2],simple_path_all_state[i][3],simple_path_all_state[i][4],simple_path_all_state[i][5],simple_path_all_state[i][6]);
    }
    
    printf("blue_true_number:%d\n",blue_true_treasure);
    printf("red_true_number:%d\n",red_true_treasure);

    //deposits_list
    for(i=point_now+1;i<9;i++){
        if(is_blue_team==1){
            if(simple_path_all_state[i][1]==1||simple_path_all_state[i][2]==2){
                deposits_list_new[i-1]=deposits_list_new[i-2];
            }
        }
        else if(is_red_team==1){
            if(simple_path_all_state[i][1]==2||simple_path_all_state[i][2]==2){
                deposits_list_new[i-1]=deposits_list_new[i-2];
            }
        }
    }

    for(i=0;i<8;i++){
        printf("%d ",deposits_list_new[i]);
        deposits_list[i]=deposits_list_new[i];
    }
    printf("\n");
    stay_min_path=1;
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    xTaskCreate(Output_path_allpoint, "Output_path_allpoint", 1024*24, NULL, configMAX_PRIORITIES-1, &get_past_task_handle);
}
void app_main(void)
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 2048*2, NULL, configMAX_PRIORITIES,&rx_task_handle);
    xTaskCreate(rx_task_stm32, "uart_rx_task_stm32", 2048*2, NULL, configMAX_PRIORITIES,&rx_task_handle_stm32);
    //xTaskCreate(tx_task, "uart_tx_task", 2048*2, NULL, configMAX_PRIORITIES,&tx_task_handle);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //printf("%d",gpio_get_level(blue_team_pin));
    if(gpio_get_level(blue_team_pin)==0){
        is_blue_team=1;
        is_red_team=0;
        printf("blue_team\n");
    }
    else{
        is_blue_team=0;
        is_red_team=1;
        printf("red_team\n");
    }
}