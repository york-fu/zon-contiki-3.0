#include "stm32f10x.h"
#include "spi.h"
#include "stdio.h"

extern void EXTI9_5_CallSet_3(void (*func)(void));
extern void sx1278Irq(void);

void clock_delay_ms(unsigned int ms);

void sx1278_reset()
{
  //GPIO_ResetBits(GPIOA, GPIO_Pin_11);
  //clock_delay_ms(10);
  //GPIO_SetBits(GPIOA, GPIO_Pin_11);
  //clock_delay_ms(10);
}
void sx1278_cs(int s)
{
  if (s) {
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
  }else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_12); 
  }
}
int sx1278RegRead(int reg)
{
  int r;
  
  sx1278_cs(0);
  SPI2_ReadWriteByte(reg&0x7f);
  r = SPI2_ReadWriteByte(0xff);
  sx1278_cs(1);
  
  return r;
}
void sx1278RegWrite(int reg, int val)
{
  sx1278_cs(0);
  SPI2_ReadWriteByte(reg | 0x80);
  SPI2_ReadWriteByte(val&0xff);
  sx1278_cs(1);
}

void sx1278BufWrite(char *buf, int len)
{
  for (int i=0; i<len; i++) {
    SPI2_ReadWriteByte(buf[i]);
  }
}
int sx1278BufRead(char *buf, int len)
{
  char r;
  for (int i=0; i<len; i++) {
    r = SPI2_ReadWriteByte(0xff);
    buf[i] = r;
  } 
  return len;
}

void sx1278_IO_Init(void)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOB时钟
  
     /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  
  /* sx1278 cs config*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* sx1278 reset  --- connect ot dio1 irq */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  /*  sx1278 irq config*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  /* Connect EXTI0 Line to PA.00 pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
  
  /* Configure EXTI0 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line8 | EXTI_Line4 | EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //lora采用较高优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
    /* Enable and set EXTI0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //lora采用较高优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
      /* Enable and set EXTI0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //lora采用较高优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable and set EXTI0 Interrupt to the lowest priority */
 /* NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //lora采用较高优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  */
  SPI2_Init();
    
    EXTI9_5_CallSet_3(sx1278Irq);                               //设置sx1278中断函数
}

void (*EXTI9_5_Call[5])(void) = {NULL};

/*
中断回调函数设置函数，1-4对应5-9线中断
*/
void EXTI9_5_CallSet_0(void (*func)(void))
{
    EXTI9_5_Call[0] = func;
}

void EXTI9_5_CallSet_1(void (*func)(void))
{
    EXTI9_5_Call[1] = func;
}

void EXTI9_5_CallSet_2(void (*func)(void))
{
    EXTI9_5_Call[2] = func;
}

void EXTI9_5_CallSet_3(void (*func)(void))
{
    EXTI9_5_Call[3] = func;
}

void EXTI9_5_CallSet_4(void (*func)(void))
{
    EXTI9_5_Call[4] = func;
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
        if(EXTI9_5_Call[0] != NULL)
        {
            EXTI9_5_Call[0]();
        }
    }
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
        if(EXTI9_5_Call[1] != NULL)
        {
            EXTI9_5_Call[1]();
        }
    }
    if(EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line7);
        if(EXTI9_5_Call[2] != NULL)
        {
            EXTI9_5_Call[2]();
        }
    }
    if(EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        if(EXTI9_5_Call[3] != NULL)
        {
            EXTI9_5_Call[3]();
        }
    }
    if(EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9);
        if(EXTI9_5_Call[4] != NULL)
        {
            EXTI9_5_Call[4]();
        }
    }
}

void EXTI4_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line4);
  sx1278Irq(); 
}
void EXTI3_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line3);
  sx1278Irq();
}
