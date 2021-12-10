#ifndef _delay_h_
#define _delay_h_

#include <sys/clock.h>
#include "stm32f10x.h"

#define delay_us  clock_delay_us
#define delay_ms  clock_delay_ms

#endif