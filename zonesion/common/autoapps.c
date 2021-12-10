#include <contiki.h>

PROCESS_NAME(zigbee_uart_process);
#if (DEV_TYPE==3)
PROCESS_NAME(wifi_config);
#endif
PROCESS_NAME(lcd);
PROCESS_NAME(sensors);
PROCESS_NAME(helloworld);
PROCESS_NAME(zxbee);
PROCESS_NAME(key);

struct process * const autostart_processes[] = {
  &zigbee_uart_process,
#if (DEV_TYPE==3)
  &wifi_config,
#endif
  &helloworld,
  &sensors,
  &lcd,
  &zxbee,
  &key,
  NULL
};
