
#include <contiki.h>
#include <stdio.h>
#include "string.h"
#include "stm32f4xx.h"

PROCESS(vibrator, "vibrator");

void vibrator_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
  
  GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void vibrator_onoff(int st)
{
  if (st) {
     GPIO_SetBits(GPIOB, GPIO_Pin_0);
  } else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
  }
}
static char vibrator_param[64] ;//= {5,2,3,10,6,10};
/* vibrator_set
 * buf : 振动参数， 第1个字节，振动强度，第二个字节，振动时间个数
 *      振动时间2个字节，第1个开启时间，第二个关闭时间，单位毫秒
 */
void vibrator_set(char *buf, char len)
{
  if (len < sizeof vibrator_param) {
    memcpy(vibrator_param, buf, len);
  }
}

PROCESS_THREAD(vibrator, ev, data)
{
  static struct etimer et;
  static unsigned char strength = 0;  //取值0--10
  
  PROCESS_BEGIN();
  
  vibrator_init();
  etimer_set(&et, CLOCK_SECOND/100); // 10 ms
  while (1) {
    PROCESS_YIELD();
    if (etimer_expired(&et)) {
      static char tick = 0;
      tick ++;
      if (tick == 10) {  //一个周期
        tick = 0;
      } 
      if (strength > 0) {
        if (tick < strength) {//振动强度 > 0 开启马达
          GPIO_SetBits(GPIOB, GPIO_Pin_0);
        } else {
          GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        }
      } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
      }
      
      /** vibrator_param **/
      static int ttick = 0;
      static int offset = 0;
      if (vibrator_param[1] != 0) {
        if (ttick == 0 && offset == 0) {
          offset = 2;
          ttick = vibrator_param[offset]*10;
        } else {
          ttick --;
          if (ttick <= 0) {
            offset += 1;
            if (offset >= 2 + vibrator_param[1]*2) offset = 2;
            ttick = vibrator_param[offset]*10;
          }
        }
        if (offset % 2 == 0) {
          strength = vibrator_param[0];
        } else {
          strength = 0;
        }
          
      } else {
        strength = vibrator_param[0];
        ttick = 0;
        offset = 0;
      }      
      etimer_set(&et, CLOCK_SECOND/100); // 10 ms
    }
  }
  PROCESS_END();
}