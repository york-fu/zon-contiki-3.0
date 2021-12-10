
#include <contiki.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "key.h"


#define K1_Active()  (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
#define K2_Active()  (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
#define K3_Active()  (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
#define K4_Active()  (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5))


void key_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOB时钟
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3| GPIO_Pin_4 | GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
  
}

process_event_t process_event_keydown;
process_event_t process_event_keyup;


PROCESS(key, "key");
PROCESS_THREAD(key, ev, data)
{ 
  static struct etimer et;
  static char key_status = 0;
  static char key1_status = 0;
  static char key2_status = 0;
  PROCESS_BEGIN();
  
  process_event_keydown = process_alloc_event();
  process_event_keyup = process_alloc_event();
  
  key_init();
  
  etimer_set(&et, CLOCK_SECOND);
  
  while (1) { 
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    if (K1_Active() && ((key_status & KEY1)==0)) {
      key1_status++;
    }
    if ((key1_status > 0) && !K1_Active()) {
      if(key1_status >= 50){
        process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY1LONG);
        printf("key1 longdown\r\n");
      }
      else{
        process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY1);
        printf("key1 down\r\n");
      }
      key1_status = 0;
    }
    if (K2_Active() && ((key_status & KEY2)==0)) {
      key2_status++;
    }
    if ((key2_status > 0) && !K2_Active()) {
      if(key2_status >= 50){
        process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY2LONG);
        printf("key2 longdown\r\n");
      }
      else{
        process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY2);
        printf("key2 down\r\n");
      }
      key2_status = 0;
    }
    
    if (K3_Active() && ((key_status & KEY3)==0)) {
      process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY3);
      key_status |= KEY3;
    }
    if (((key_status & KEY3)==KEY3) && !K3_Active()) {
      process_post(PROCESS_BROADCAST, process_event_keyup, (void*)KEY3);
      key_status &= ~KEY3;
    }
    
    
    if (K4_Active() && ((key_status & KEY4)==0)) {
      process_post(PROCESS_BROADCAST, process_event_keydown, (void*)KEY4);
      key_status |= KEY4;
    }
    if (((key_status & KEY4)==KEY4) && !K4_Active()) {
      process_post(PROCESS_BROADCAST, process_event_keyup, (void*)KEY4);
      key_status &= ~KEY4;
    }
    etimer_set(&et, CLOCK_SECOND/100);
  }
  
  PROCESS_END();
}