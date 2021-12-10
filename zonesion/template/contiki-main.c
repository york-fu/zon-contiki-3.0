#include <stdio.h>
#include <contiki.h>
#include <contiki-net.h>
#include <dev/leds.h>
#include "usart.h"
unsigned int idle_count = 0;

void relay_init(void);
void uart1_init(unsigned int bpr);
void debug_init(void (*f)(char));

void main(void)
{
   clock_init();
  
  leds_init();
  leds_on(1);
 
  uart3_init(115200);
  debug_init(uart3_putc);
  int debug_input(char c);
  uart3_set_input(debug_input);
  
  printf("\r\nStarting ");
  printf(CONTIKI_VERSION_STRING);
  printf(" on STM32F40x\r\n"); 
  
  process_start(&etimer_process, NULL);
  
  ctimer_init();


  
  //IIC3_Init();
  //fsmc_init();
  //EXTI15_10_nvic_init();
  
  //serial_shell_init();
  
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