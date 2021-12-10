#include "stm32f4xx.h"

static unsigned char led_status = 0;
void leds_arch_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  GPIO_Init(GPIOE, &GPIO_InitStructure);  
  GPIOE->BSRRL = GPIO_Pin_2 | GPIO_Pin_3;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  GPIOB->BSRRL = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
}
unsigned char leds_arch_get(void)
{
  return led_status;
}
void leds_arch_set(unsigned char leds)
{
  unsigned int pin_on = 0;
  unsigned int pin_off = 0;
  led_status = leds;
  if (leds & 0x01) pin_on |= GPIO_Pin_2;
  else pin_off |= GPIO_Pin_2;
  if (leds & 0x02) pin_on |= GPIO_Pin_3;
  else pin_off |= GPIO_Pin_3;
  GPIO_ResetBits(GPIOE, pin_on); //on
  GPIO_SetBits(GPIOE, pin_off); //off
  
  pin_on = pin_off = 0;
  if (leds & 0x04) pin_on |= GPIO_Pin_0;
  else pin_off |= GPIO_Pin_0;
  if (leds & 0x08) pin_on |= GPIO_Pin_1;
  else pin_off |= GPIO_Pin_1;
  if (leds & 0x10) pin_on |= GPIO_Pin_2;
  else pin_off |= GPIO_Pin_2;  
  GPIO_ResetBits(GPIOB, pin_on); //on
  GPIO_SetBits(GPIOB, pin_off); //off  
}