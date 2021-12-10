
#include <contiki.h>
#include <stdio.h>
#include "dev/leds.h"

PROCESS(led, "led");
PROCESS_THREAD(led, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  
  leds_init();
  etimer_set(&et, CLOCK_SECOND/100); // 100 ms
  while (1) {
    static char led = 1;
    static char flag = 0;
    PROCESS_YIELD();
    if (etimer_expired(&et)) {
      leds_off(0x0f);
      leds_on(led);
      if (flag == 0) {
        led <<= 1;
        if (led & 0x10) {
          led = 0x08;
          flag = 1;
        }
      } else {
        led >>= 1;
        if (led == 0) {
          led = 0x01;
          flag = 0;
        }
      }
      
      etimer_set(&et, CLOCK_SECOND/30); // 100 ms
    }
  }
  PROCESS_END();
}