#include <contiki.h>

//PROCESS_NAME(zigbee_uart_process);

PROCESS_NAME(led);
PROCESS_NAME(fader_process);
PROCESS_NAME(helloworld);
PROCESS_NAME(test);
PROCESS_NAME(ble_recv);
PROCESS_NAME(alarm);

struct process * const autostart_processes[] = {
  //&zigbee_uart_process,
  &helloworld,
  &led,
  &test,
  &ble_recv,
  &fader_process,
  &alarm,
  NULL
};
