#include "contiki.h"
#include "sys/process.h"
#include <stdio.h>
#include "zxbee.h"
#include "string.h"
#include "usart.h"
#include "stdlib.h"

#define BUFSIZE         64
static char buf[BUFSIZE];

static process_event_t evt_data;

PROCESS(ble_recv, "ble_recv");

static int ble_input_byte(char ch)
{
  static int rlen = 0;
  if (/*recv_flag == 0 && rlen == 0 && */ch == '{') {
    rlen = 0;
    buf[rlen++] = ch;
  } else if (rlen > 0) {
    if (rlen < sizeof buf) {
      buf[rlen++] = ch;
      if (ch == '}') {
        char *p = (char *)malloc(rlen+1);
        memcpy(p, buf, rlen);
        p[rlen] = 0;
        if (0 != process_post(&ble_recv, evt_data, p)) {
          free(p);
        }
        rlen = 0;
      }
    } else rlen = 0;
  }
  return 0;
}

void ble_write(char *buf)
{
  int i;
  int len = strlen(buf);
  printf("ble <<< %s\r\n", buf);
  for (i=0; i<len; i++) {
    uart1_putc(buf[i]);
  }
}


PROCESS_THREAD(ble_recv, ev, data)
{
  PROCESS_BEGIN();
  
  evt_data = process_alloc_event();
  
  uart1_init(115200);
  uart1_set_input(ble_input_byte);
  
  while (1) {
    PROCESS_WAIT_EVENT();
    //PROCESS_WAIT_UNTIL(ev == evt_data);
    if (ev == evt_data) {
      char *p = data;
      printf("ble >>> %s\r\n", p);
      zxbeeBegin();
      zxbee_onrecv_fun(p, strlen(p));
      free(p);
      char *x = zxbeeEnd();
      if (x) {
        ble_write(x);
      }
    }
  }
  PROCESS_END();
}