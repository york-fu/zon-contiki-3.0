#include "stm32f4xx.h"
#include "tim.h"

void time_init(void)
  {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ;
    
    NVIC_InitTypeDef NVIC_InitStructure;
    
      //��TIM2����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
                          
   /*//��TIM3����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
   //��TIM4����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);*/

   //**************************************************************************
   //     ��ʱ��2���ã� 7��Ƶ��20ms�ж�һ�Σ����ϼ���
   //**************************************************************************
    TIM_TimeBaseStructure.TIM_Period = 1999;
    TIM_TimeBaseStructure.TIM_Prescaler = 7;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //��ʼ����ʱ��
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //����ʱ���ж�
    
    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //��ʱ�� 2 �ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ� 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //��Ӧ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);// �ܳ�ʼ�� NVIC
    
    TIM_Cmd(TIM2, ENABLE); //ʹ�ܶ�ʱ��
    
}