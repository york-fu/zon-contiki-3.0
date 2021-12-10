#include "stm32f4xx.h"

static unsigned char led_status = 0;
void rgb_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  GPIO_Init(GPIOD, &GPIO_InitStructure);  
  GPIOE->BSRRL = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  /*
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  GPIOB->BSRRL = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;*/
}
unsigned char rgb_get(void)
{
  return led_status;
}
void rgb_set(unsigned char leds)
{
  unsigned int pin_on = 0;
  unsigned int pin_off = 0;
  led_status = leds;
  if (leds & 0x01) pin_on |= GPIO_Pin_3;
  else pin_off |= GPIO_Pin_3;
  if (leds & 0x02) pin_on |= GPIO_Pin_4;
  else pin_off |= GPIO_Pin_4;
  if (leds & 0x04) pin_on |= GPIO_Pin_5;
  else pin_off |= GPIO_Pin_5;

  GPIO_ResetBits(GPIOD, pin_on); //on
  GPIO_SetBits(GPIOD, pin_off); //off

}