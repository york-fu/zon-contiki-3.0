#include "stm32f10x.h"
#include "stdio.h"

unsigned int clock_time(void);

unsigned int t1, t2;

void timer1Init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  TIM_DeInit(TIM1);	/*��λTIM1��ʱ��*/

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	/*��ʱ��*/
  
  TIM_TimeBaseStructure.TIM_Period = 0xffff;	///     

  TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;	/* ��Ƶ*/  
  
  TIM_TimeBaseStructure.TIM_RepetitionCounter=0; 

  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  	 

  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	/*�����������ϼ���*/

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Clear TIM1 update pending flag  ���TIM1����жϱ�־]  */

  TIM_ClearFlag(TIM1, TIM_FLAG_Update);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;	/*����ж�*/

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;

  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  

  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);	/* Enable TIM1 Update interrupt TIM1����ж�����*/

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