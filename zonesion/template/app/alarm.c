#include <contiki.h>
#include <stdio.h>
#include "pcf8563.h"
PROCESS(alarm, "alarm");

void vibrator_init(void);
void vibrator_onoff(int st);


extern char sys_mode;
extern char sport_begin_time[2];
extern char sport_end_time[2];
extern char sleep_begin_time[2];
extern char sleep_end_time[2];

extern char alarms[2][3];

static struct etimer et;


PROCESS_THREAD(alarm, ev, data)
{ 
  static unsigned int tick = 0;
  static char wday = 0, hour = 0, mini = 0, sec = 0;
  char t[8];
  
  PROCESS_BEGIN();
  
  vibrator_init();
  
  if (PCF8563_gettime(t) < 0) {
    printf(" error : read rtc time\r\n");
  }
  hour = ((t[2] >> 4)&0x03) * 10 + (t[2]&0x0f);
  mini = ((t[1] >> 4)&0x07) * 10 + (t[1]&0x0f);
  sec = ((t[0] >> 4)&0x07) * 10 + (t[0]&0x0f);
  wday = (t[4])&0x07;
  
  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    tick ++;
    if (tick == 30) {    
      tick = 0;
      if (PCF8563_gettime(t) < 0) {
        printf("error : read rtc time\r\n");
      }
      hour = ((t[2] >> 4)&0x03) * 10 + (t[2]&0x0f);
      mini = ((t[1] >> 4)&0x07) * 10 + (t[1]&0x0f);
      sec = ((t[0] >> 4)&0x07) * 10 + (t[0]&0x0f);
      wday = t[4] & 0x07;
    } else {
      sec ++;
      if (sec == 60) {
        sec = 0;
        mini ++;
        if (mini == 60) {
          mini = 0;
          hour ++;
          if (hour == 24) {
            hour = 0;
            wday ++;
            if (wday == 7) {
              wday = 0;
            }
          }
        }
      }
    }
    char a = 0;
    if (sys_mode == 0) {  //auto
      if (hour == sport_begin_time[0] && mini==sport_begin_time[1]) a = 1;
      if (hour == sleep_begin_time[0] && mini==sleep_begin_time[1]) a = 1;
    }
    if (alarms[0][0] != 0 && hour==alarms[0][1] && mini==alarms[0][2]) a = 1;
    if (alarms[1][0] != 0 && hour==alarms[1][1] && mini==alarms[1][2]) a = 1;
    if (a != 0) {
      vibrator_onoff(1);
    } else {
      vibrator_onoff(0);
    }
  }
  
  PROCESS_END();
}