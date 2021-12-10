#include <stdio.h>
#include "zxbee.h"
#include "string.h"
#include "pcf8563.h"

static char mac[18];

extern char alarms[2][3];

void config_save_alarm();
void config_save_mode(void);


int z_process_command_call(char *tag, char *val)
{
  char buf[32];
  if (memcmp(tag, "MAC", 3) == 0) {
    strcpy(mac, val);
  }
  if (memcmp(tag, "ECHO", 4) == 0) {
    zxbeeAdd("ECHO", val);
  }
  if (strcmp(tag, "DT") == 0) {
    if (*val == '?') {
      char t[8];
      
      PCF8563_gettime(t);
      int h = ((t[2] >> 4)&0x03) * 10 + (t[2]&0x0f);
      int m = ((t[1] >> 4)&0x07) * 10 + (t[1]&0x0f);
      int s = ((t[0] >> 4)&0x07) * 10 + (t[0]&0x0f);
      int xq = (t[4] & 0x07);
      int d = ((t[3]>>4)&0x03) * 10 + (t[3]&0x0f);
      int M = ((t[5]>>4)&0x01) * 10 + (t[5]&0x0f);
      int y = ((t[6]>>4)&0x0f) * 10 + (t[6]&0x0f);
      sprintf(buf, "%02d%02d%02d%02d%02d%02d%d", y,M,d, h,m,s,xq);
      zxbeeAdd("DT", buf);
    } else {
      //160227221800
      char t[8];
      char *v = val;
      t[0] = 0x00;
      t[1] = ((v[8]-'0')<<4) | (v[9]-'0');
      t[2] = ((v[6]-'0')<<4) | (v[7]-'0');
      t[3] = ((v[4]-'0')<<4) | (v[5]-'0');
      t[4] = v[12]-'0'; // ÐÇÆÚ
      t[5] = ((v[2]-'0')<<4) | (v[3]-'0');
      t[6] = ((v[0]-'0')<<4) | (v[1]-'0');
      PCF8563_settime((u8 *)t);
    }
  }
  
  if (strcmp(tag, "A1") == 0) {
    
    if (*val == '?') {   
      sprintf(buf, "%d%02d%02d",alarms[0][0],
              alarms[0][1],alarms[0][2]);
      zxbeeAdd("A1", buf);
    } else {
      
      char type = val[0] - '0';
      char h = (val[1]-'0')*10 + (val[2] - '0');
      char m = (val[3]-'0')*10 + (val[4] - '0');

      alarms[0][0] = !!type;
      alarms[0][1] = h;
      alarms[0][2] = m;
      config_save_alarm();
    }
  }
  if (strcmp(tag, "A2") == 0) {
    if (*val == '?') {   
      sprintf(buf, "%d%02d%02d",alarms[1][0],
              alarms[1][1],alarms[1][2]);
      zxbeeAdd("A2", buf);
    } else {
      
      char type = val[0] - '0';
      char h = (val[1]-'0')*10 + (val[2] - '0');
      char m = (val[3]-'0')*10 + (val[4] - '0');

      alarms[1][0] = !!type;
      alarms[1][1] = h;
      alarms[1][2] = m;
      config_save_alarm();
    }
  }
  if (strcmp(tag, "MD") == 0) {
    extern char sys_mode;
    extern char sensor_mode;
    extern char sleep_begin_time[2];
    extern char sleep_end_time[2];
    extern char sport_begin_time[2];
    extern char sport_end_time[2];

    if (*val == '?') {
      if (sys_mode == 0) {
        sprintf(buf, "0%02u%02u%02u%02u%02u%02u%02u%02u",
                sport_begin_time[0],sport_begin_time[1],sport_end_time[0],sport_end_time[1],
                sleep_begin_time[0],sleep_begin_time[1],sleep_end_time[0],sleep_end_time[1]);
        zxbeeAdd("MD", buf);
      } else {
        sprintf(buf, "1%u", sensor_mode);
        sprintf(&buf[2], "%02u%02u%02u%02u%02u%02u%02u%02u",
                sport_begin_time[0],sport_begin_time[1],sport_end_time[0],sport_end_time[1],
                sleep_begin_time[0],sleep_begin_time[1],sleep_end_time[0],sleep_end_time[1]);
        
        zxbeeAdd("MD", buf);
      }
    } else {
      if (*val == '0') {
        sys_mode = 0;
        sport_begin_time[0] = (val[1]-'0')*10+(val[2]-'0');
        sport_begin_time[1] = (val[3]-'0')*10+(val[4]-'0');
        sport_end_time[0] = (val[5]-'0')*10+(val[6]-'0');
        sport_end_time[1] = (val[7]-'0')*10+(val[8]-'0');
        sleep_begin_time[0] = (val[9]-'0')*10+(val[10]-'0');
        sleep_begin_time[1] = (val[11]-'0')*10+(val[12]-'0');
        sleep_end_time[1] = (val[13]-'0')*10+(val[14]-'0');
        sleep_end_time[1] = (val[15]-'0')*10+(val[16]-'0');
     
      } else {
        sys_mode = 1;
        if (val[1] == '0') {
          sensor_mode = 0;
        } else {
          sensor_mode = 1;
        }
      }
      config_save_mode();
    }
  }
  if (strcmp(tag, "ST") == 0) {
    if (*val == '?') {
      extern unsigned int total_step_cnt;
      sprintf(buf, "%u", total_step_cnt);
      zxbeeAdd("ST", buf);
    }
  }
  if (strcmp(tag, "SP") == 0) {
    extern unsigned int sleep_time;
    extern unsigned int sleep_deep;
    extern unsigned int turnover_cnt;
    if (*val == '?') {
        sprintf(buf, "%u&%u&%u", sleep_time/60, sleep_deep/60, turnover_cnt);
    }
    zxbeeAdd("SP", buf);
  }
  return 0;
}