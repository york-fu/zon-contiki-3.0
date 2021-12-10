#include <contiki.h>

#include <contiki.h>
PROCESS_NAME(stepcount);
PROCESS_NAME(gps);
PROCESS_NAME(gsm);
PROCESS_NAME(u_zhiyun);
PROCESS_NAME(sensor);
PROCESS_NAME(at);
struct process * const autostart_processes[] = {
  &gsm,
  &u_zhiyun,
  &stepcount,
  &gps,
  &at,
  &sensor,
  NULL
};

