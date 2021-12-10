#include <contiki.h>

#include <contiki.h>
PROCESS_NAME(stepcount);
PROCESS_NAME(gps);
PROCESS_NAME(sx1278);
PROCESS_NAME(sensor);
PROCESS_NAME(at);
struct process * const autostart_processes[] = {
  &sx1278,
  &stepcount,
  &gps,
  &at,
  &sensor,
  NULL
};

