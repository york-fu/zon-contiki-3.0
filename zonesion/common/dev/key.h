#ifndef _KEY_H_
#define _KEY_H_
#include "stm32f4xx.h"

#define         KEY1_2                  ADC_Channel_12
#define         KEY3_4                  ADC_Channel_13
#define         KEY1_PRESSED            0X01
#define         KEY2_PRESSED            0X02
#define         KEY3_PRESSED            0X04
#define         KEY4_PRESSED            0X08
#define         KEY1_LONG_PRESSED       0X10
#define         KEY2_LONG_PRESSED       0X20
#define         KEY3_LONG_PRESSED       0X40
#define         KEY4_LONG_PRESSED       0X80

extern unsigned int key_status;

void key_adc_init(void);
u16 get_key_status(u8 key);

#if     SYSTEM_SUPPORT_CONTIKI
PROCESS_THREAD(key, ev, data);
#endif

#endif