#include "flamanage.h"
#include "string.h"
char sys_mode = 0;
char sport_begin_time[2] = {8, 0};
char sport_end_time[2] = {22, 0};
char sleep_begin_time[2] = {22, 0};
char sleep_end_time[2] = {8, 0};
char sensor_mode = 1;


// step_cnt 第1个单元存储最后存储时的星期，后面7个单元存储星期对应的步数
unsigned int step_cnt[1+7]; 

extern unsigned int sleep_time;
/* 记录深度随眠时长 单位s */
extern unsigned int sleep_deep;
/* 记录翻身次数*/
extern unsigned int turnover_cnt;

/* 记录2个闹钟*/
char alarms[2][3];

void config_save_mode(void)
{
  char buf[10];
  buf[0] = sys_mode;
  buf[1] = sport_begin_time[0];
  buf[2] = sport_begin_time[1];
  buf[3] = sport_end_time[0];
  buf[4] = sport_end_time[1];
  buf[5] = sleep_begin_time[0];
  buf[6] = sleep_begin_time[1];
  buf[7] = sleep_end_time[0];
  buf[8] = sleep_end_time[1];
  buf[9] = sensor_mode;
  
  FlaVPageWrite(2, 0, buf, 10);
}
void config_read_mode(void)
{
  char buf[10];
  FlaVPageRead(2, 0, buf, 10);
  sys_mode = buf[0];
  sport_begin_time[0] = buf[1];
  sport_begin_time[1] = buf[2];
  sport_end_time[0] = buf[3];
  sport_end_time[1] = buf[4];
  sleep_begin_time[0] = buf[5];
  sleep_begin_time[1] = buf[6];
  sleep_end_time[0] = buf[7];
  sleep_end_time[1] = buf[8];
  sensor_mode = buf[9];
}
void config_save_step(int w, int ts)
{
   w = w % 7;
   step_cnt[0] = w;
   step_cnt[w+1] = ts;
   FlaVPageWrite(3, 0, step_cnt, sizeof step_cnt);
}
void config_read_step(void)
{
  FlaVPageRead(3, 0, step_cnt, sizeof step_cnt);
}

void config_save_sleepdata()
{
  unsigned int buf[3];
  buf[0] = sleep_time;
  buf[1] = sleep_deep;
  buf[2] = turnover_cnt;
  FlaVPageWrite(4, 0, buf, sizeof buf);
}
void config_read_sleepdata()
{
  unsigned int buf[3];
  FlaVPageRead(4, 0, buf, sizeof buf);
  sleep_time = buf[0];
  sleep_deep = buf[1];
  turnover_cnt = buf[2];
}

void config_save_alarm()
{
  FlaVPageWrite(5, 0, alarms, sizeof alarms);
}
void config_read_alarm()
{
  FlaVPageRead(5, 0, alarms, sizeof alarms);
}


void config_init(void)
{
  FlaManageInit();
  int flag = 0;
  FlaVPageRead(1, 0, &flag, 4);
  if (flag != 0x5a5a5a5b) {
    FlaFormat();
    flag = 0x5a5a5a5b;
    FlaVPageWrite(1, 0, &flag, 4);
    config_save_mode();
    memset(step_cnt, 0, sizeof step_cnt);
    FlaVPageWrite(3, 0, step_cnt, sizeof step_cnt);
    config_save_sleepdata();
    config_save_alarm();
  } else {
      config_read_mode();
      config_read_step();
      config_read_sleepdata();
      config_read_alarm();
  }
}