#include "stm32f10x.h"
#include "stdio.h"

unsigned int clock_time(void);

unsigned int t1, t2;

void timer1Init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  TIM_DeInit(TIM1);	/*复位TIM1定时器*/

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	/*开时钟*/
  
  TIM_TimeBaseStructure.TIM_Period = 0xffff;	///     

  TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;	/* 分频*/  
  
  TIM_TimeBaseStructure.TIM_RepetitionCounter=0; 

  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  	 

  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	/*计数方向向上计数*/

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Clear TIM1 update pending flag  清除TIM1溢出中断标志]  */

  TIM_ClearFlag(TIM1, TIM_FLAG_Update);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;	/*溢出中断*/

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;

  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  

  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);	/* Enable TIM1 Update interrupt TIM1溢出中断允许*/

  TIM_Cmd(TIM1, DISABLE);
  //t1 = clock_time();
 
}



static unsigned int flag  = 0;
unsigned int timer1Tick(void)
{
  
  unsigned int t;
  if (flag == 0) {
    t = TIM1->CNT;
      return t/10;
  } else {
    t = TIM1->ARR;
    t += TIM1->CNT;
    return t/10;
  }
}
void timer1SetTimeout(unsigned int t)
{
  if (t > 65535/10) {
    printf("error: timer1 timeout value is too lagger\r\n");
    t = 65535/10;
  }
  
  flag = 0;
  TIM1->CR1 = TIM1->CR1 & ~0x81; //ARPE = 0 EN=0

  TIM1->CNT = 0;
  TIM1->ARR = t*10;
  TIM1->CR1 = TIM1->CR1 |  0x01;
}
void TimerIrqHandler( void );

void TIM1_UP_IRQHandler(void)
{
  TIM_ClearFlag(TIM1, TIM_FLAG_Update);

  flag = 1;
  TIM1->CR1 = TIM1->CR1 & ~0x01;
  TIM_ClearFlag(TIM1, TIM_FLAG_Update);
  TimerIrqHandler();
}