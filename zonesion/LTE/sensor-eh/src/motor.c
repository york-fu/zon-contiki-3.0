#include "stm32f10x.h"
void motorInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  GPIO_ResetBits(GPIOB,  GPIO_Pin_0 | GPIO_Pin_1);
}

void motorSet(int st)
{
  if (st == 0) {
    GPIO_ResetBits(GPIOB,  GPIO_Pin_0 | GPIO_Pin_1);
  } 
  if (st == 1) {
    GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
    GPIO_SetBits(GPIOB,  GPIO_Pin_1);
  }
  if (st == 2) {
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
    GPIO_SetBits(GPIOB,  GPIO_Pin_0);
  }
}

