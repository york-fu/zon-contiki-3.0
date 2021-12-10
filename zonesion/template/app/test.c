
#include <contiki.h>
#include <stdio.h>
#include "stdlib.h"
#include "../dev/lis3dh.h"
#include "arm_math.h"
#include "pcf8563.h"
#include "M25Pxx.h"
//#include "arm_const_structs.h"

PROCESS(test, "test");

#define SYS_MODE_AUTO       0
#define SYS_MODE_MANUAL     1

extern char sys_mode ;
extern char sleep_begin_time[2];
extern char sleep_end_time[2];

extern char sport_begin_time[2];
extern char sport_end_time[2];

#define SENSOR_MODE_STEP    1
#define SENSOR_MODE_SLEEP   0
#define SENSOR_MODE_IDLE   2
extern char sensor_mode;

extern unsigned int step_cnt[1+7]; 
unsigned int total_step_cnt = 0;

int stepcounting(float32_t* test_f32);
void mpu9250_init(void);
void config_init(void);
void config_save_step(int w, int ts);
void config_save_sleepdata();
int mpu9250_accel(float *x, float *y, float *z);
void sleep_enter();
void sleep_leave();
void leds_on(unsigned char ledv);
void leds_off(unsigned char ledv);
void pluse_on(void);
void pluse_off(void);
void fader_on(void);
void fader_off(void);
void sleepalg(float32_t* test_f32);


void set_sys_mode(int m)
{
  sys_mode = !!m;
}

static int time_in_duration(char *tm, char *tb, char *te)
{
  if (tb[0]<te[0] || (tb[0]==te[0] && tb[1]<te[1])) {
     /* 当前时间段在一天以内*/
    if (tm[0]>tb[0] || (tm[0]==tb[0] && tm[1]>tb[1])) {
      if (tm[0]<te[0] || (tm[0]==te[0] && tm[1]<te[1])) {
        return 1;
      }
    }
  } else {
    /* 时间段跨2天*/
    if (tm[0]>tb[0] || (tm[0]==tb[0] && tm[1]>tb[1])) return 1;
    if (tm[0]<te[0] || (tm[0]==te[0] && tm[1]<te[1])) return 1;
  }
  return 0;
}


PROCESS_THREAD(test, ev, data)
{
  static struct etimer t_acc;
  static float acc_input[64*2];
  static uint32_t acc_len = 0;

  PROCESS_BEGIN();
  etimer_set(&t_acc, CLOCK_SECOND/10);
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_TIMER);
  
  PCF8563_init();
  
  mpu9250_init();
  
  M25Pxx_init();
  
  unsigned int id;
  id = M25Pxx_Read_ID();
  if (id != 0x00202015) {
    printf("error can't find ic m25p16 \r\n");
  } else {
    printf("m25p16 id %08X\r\n", id);
  }
  config_init();
  
  total_step_cnt = step_cnt[step_cnt[0]+1];
  
  
  
  etimer_set(&t_acc, CLOCK_SECOND/10);
  while (1) {
    PROCESS_YIELD();
    if (etimer_expired(&t_acc)) {
      etimer_set(&t_acc, CLOCK_SECOND/10);
      static char tick = 9;
      if (++tick == 10) {
          tick = 0;
           char t[8];
         
           PCF8563_gettime(t);
         
          int h = ((t[2] >> 4)&0x03) * 10 + (t[2]&0x0f);
          int m = ((t[1] >> 4)&0x07) * 10 + (t[1]&0x0f);
          //int s = ((t[0] >> 4)&0x07) * 10 + (t[0]&0x0f);
          int xq = (t[4] & 0x07);
          //int d = ((t[3]>>4)&0x03) * 10 + (t[3]&0x0f);
          //int M = ((t[5]>>4)&0x01) * 10 + (t[5]&0x0f);
          //int y = ((t[6]>>4)&0x0f) * 10 + (t[6]&0x0f);
          
          /* 计步数据存储*/
          if (xq != step_cnt[0]) {
              total_step_cnt = 0;
              config_save_step(xq, total_step_cnt);
          } else if (abs(total_step_cnt - step_cnt[xq+1])>10) {
            config_save_step(xq, total_step_cnt);
          }
          /* 每5分钟存储睡眠数据 1 次*/
          static unsigned int time_tick = 0;
          if (sensor_mode == SENSOR_MODE_SLEEP && ++time_tick == 5*60) {
            time_tick = 0;
            config_save_sleepdata();
          }
          
          /* 检测当前工作模式*/
          if (sys_mode == SYS_MODE_AUTO) {
            t[0] = h; t[1] = m;
            if (time_in_duration(t, sleep_begin_time, sleep_end_time)) {
              sensor_mode = SENSOR_MODE_SLEEP;
            } else 
            if (time_in_duration(t, sport_begin_time, sport_end_time)) {
              sensor_mode = SENSOR_MODE_STEP;
            } else {
              sensor_mode = SENSOR_MODE_IDLE;
            }
          }
      }
      float x, y, z;
      mpu9250_accel(&x, &y, &z);
      //printf("(%.1f, %.1f, %.1f)\r\n", x, y, z);
      float a = sqrt(x*x + y*y + z*z);
      acc_input[acc_len*2] = a;
      acc_input[acc_len*2+1] = 0;
      acc_len ++;
      if (acc_len == 64) acc_len = 0;
      static char last_mode = SENSOR_MODE_IDLE;
      
      if (last_mode != SENSOR_MODE_SLEEP && sensor_mode == SENSOR_MODE_SLEEP) {
        sleep_enter();
      }
      if (last_mode == SENSOR_MODE_SLEEP && sensor_mode != SENSOR_MODE_SLEEP) {
        sleep_leave();
      }
      if (sensor_mode == SENSOR_MODE_STEP) {
          leds_off(0x40);
          fader_off();
          pluse_on();
          if (acc_len == 0) {
            total_step_cnt += stepcounting(acc_input);   
          }
      } else if (sensor_mode == SENSOR_MODE_SLEEP) {
          leds_off(0x40);
          pluse_off();
          fader_on();
          if (acc_len == 0) sleepalg(acc_input);   
      } else {
          pluse_off();
          fader_off();
          leds_on(0x40);
      }
      last_mode = sensor_mode;
    }
  }
  
  PROCESS_END();
}