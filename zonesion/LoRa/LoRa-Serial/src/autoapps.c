#include <contiki.h>

PROCESS_NAME(sx1278);
PROCESS_NAME(sensor);
PROCESS_NAME(at);
struct process * const autostart_processes[] = {
  &sx1278,
  &at,
  NULL
};
