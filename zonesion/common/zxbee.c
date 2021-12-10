#include "contiki.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stimer.h>
#include "rf/zigbee-net.h"

PROCESS(zxbee, "zxbee");

extern float gLight, gTemp, gHumi;
extern uint16_t gADC1, gADC2, gADC3, gADC4;
extern unsigned int PM1_0, PM2_5, PM10;
extern process_event_t process_event_key;

extern int z_process_command_call(char* ptag, char* pval, char* obuf);
extern void sensor_init(void);
extern void sensor_poll(unsigned int);
extern char *sensor_type(void);

//static float A0, A1, A2, A3, A4, A5, A6, A7;
static char wbuf[96];
static void map(void)
{
}

void zxbeeBegin(void)
{
  wbuf[0] = '{';
  wbuf[1] = '\0';
}
int zxbeeAdd(char* tag, char* val)
{
  int offset = strlen(wbuf);
  sprintf(&wbuf[offset],"%s=%s,", tag, val);
  return 0;
}
char* zxbeeEnd(void)
{
  int offset = strlen(wbuf);
  wbuf[offset-1] = '}';
  wbuf[offset] = '\0';
  if (offset > 2) return wbuf;
  return NULL;
}

static int _process_command_call(char* ptag, char* pval, char* obuf)
{
  int ret = -1;
 if (memcmp(ptag, "TYPE", 4) == 0) {
    if (pval[0] == '?') {
      extern int zigbee_type(void);
      int t = zigbee_type();
      if (t < 0)  t = 2;
      ret = sprintf(obuf, "TYPE=%d%d%s", DEV_TYPE, t, sensor_type());
    }
  }
  return ret;
}
void _zxbee_onrecv_fun(char *pkg, int len)
{
   char *p;
  char *ptag = NULL;
  char *pval = NULL;
  //int len = strlen(pkg);
  char *pwbuf = wbuf+1;


  if (pkg[0] != '{' || pkg[len-1] != '}') return;

  pkg[len-1] = 0;
  p = pkg+1;
  do {
    ptag = p;
    p = strchr(p, '=');
    if (p != NULL) {
      *p++ = 0;
      pval = p;
      p = strchr(p, ',');
      if (p != NULL) *p++ = 0;
      /*if (process_command_call != NULL) */{
        int ret;
        ret = _process_command_call(ptag, pval, pwbuf);
        if (ret < 0) {
           ret = z_process_command_call(ptag, pval, pwbuf);
        }
        if (ret > 0) {
          pwbuf += ret;
          *pwbuf++ = ',';
        }
      }
    }
  } while (p != NULL);
  if (pwbuf - wbuf > 1) {
    wbuf[0] = '{';
    pwbuf[0] = 0;
    pwbuf[-1] = '}';
    zigbee_send(wbuf);
  }
}

PROCESS_THREAD(zxbee, ev, data)
{
  static struct etimer _timer;
  static struct etimer _map_timer;
  static unsigned int tick = 0;
  PROCESS_BEGIN();

  //relay_on(2);

  zigbee_set_onrecv(&_zxbee_onrecv_fun);
  sensor_init();
  etimer_set(&_timer, 5+(rand()%25) * CLOCK_SECOND);
  etimer_set(&_map_timer, CLOCK_SECOND);
  while(1) {
    PROCESS_YIELD();
    if (etimer_expired(&_map_timer)) {
      map();
      sensor_poll(++tick);
      etimer_set(&_map_timer, CLOCK_SECOND);
    }
    if (ev==process_event_key) {
    }
  }
  PROCESS_END();
}