
#include <contiki.h>
#include <stdio.h>

PROCESS(helloworld, "helloworld");

PROCESS_THREAD(helloworld, ev, data)
{
  PROCESS_BEGIN();
  //while (1) {
    printf("helloworld\r\n");
  //  clock_delay_ms(100);
  //}
  PROCESS_END();
}