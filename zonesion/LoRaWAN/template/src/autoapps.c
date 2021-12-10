#include <contiki.h>

PROCESS_NAME(at);
PROCESS_NAME(sensor);
PROCESS_NAME(lorawan);
PROCESS_NAME(loraMacTimer);
struct process * const autostart_processes[] = {
  &loraMacTimer,
  &lorawan,
  &at,
  &sensor,
  NULL
};
