#ifndef _DELAY_H_
#define _DELAY_H_
#include "stm32f4xx.h"
//0,��֧��contiki
//1,֧��contiki
#define SYSTEM_SUPPORT_CONTIKI		1				//����ϵͳ�ļ����Ƿ�֧��CONTIKI

#if SYSTEM_SUPPORT_CONTIKI
#include "contiki.h"
void clock_delay_ms(unsigned int ut);
#define delay_ms(x) clock_delay_ms(x)
void clock_delay_us(unsigned int ut);
#define delay_us(x) clock_delay_us(x)

#else
void delay_init(unsigned char SYSCLK);
void delay_ms(unsigned short nms);
void delay_us(unsigned int nus);
void delay_xms(unsigned short nms);
#endif
#endif





























