#include <stdio.h>
#include <contiki.h>
#include <contiki-net.h>
#include <dev/leds.h>
#include "usart.h"

#define DEBUG 0

#if DEBUG
#define Debug      printf
#else
#define Debug(...)
#endif

unsigned int idle_count = 0;

void relay_init(void);
void uart1_init(unsigned int bpr);
void debug_init(void (*f)(char));

void main(void)
{
    clock_init();
    
    leds_init();
    leds_on(1);
    
#if defined(WITH_xLab_AP) || defined(LORA_Serial)
    uart2_init(38400);
    debug_init(uart2_putc);
    int debug_input(char c);
    uart2_set_input(debug_input); 
#else
    uart3_init(38400);
    debug_init(uart3_putc);
    int debug_input(char c);
    uart3_set_input(debug_input);
#endif
    
    Debug("\r\nStarting ");
    Debug(CONTIKI_VERSION_STRING);
    Debug(" on STM32F103x\r\n"); 
    
    process_start(&etimer_process, NULL);
    
    ctimer_init();
    
#if AUTOSTART_ENABLE    
    autostart_start(autostart_processes);
#endif
    
    while(1) {
        do {
        } while(process_run() > 0);
        idle_count++;
        /* Idle! */
        /* Stop processor clock */
        /* asm("wfi"::); */ 
    }
}