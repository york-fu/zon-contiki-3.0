#include <stdio.h>
#include "stm32f4xx.h"

void EXTI15_10_nvic_init(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;
  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置
   
   
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置    
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置        
    
}

void EXTI15_4_nvic_init(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;
  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;//抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置
   
   
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置    
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断1
    
    NVIC_Init(&NVIC_InitStructure);//配置        
    
}

static void (*_call_4)(void);
void EXTI4_Register_Call(void (*fun)(void)) 
{
    _call_4 = fun;
}
void EXTI4_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line4)) {
    if (_call_4 != NULL) _call_4 ();
    EXTI_ClearITPendingBit(EXTI_Line4);
  }
}
static void (*_call_5_9[5])(void);
void EXTI9_5_Register_Call(unsigned int line, void (*fun)(void))
{
  if (line & EXTI_Line5) _call_5_9[0] = fun;
  if (line & EXTI_Line6) _call_5_9[1] = fun;
  if (line & EXTI_Line7) _call_5_9[2] = fun;
  if (line & EXTI_Line8) _call_5_9[3] = fun;
  if (line & EXTI_Line9) _call_5_9[4] = fun;
}
void EXTI9_5_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXTI_Line5)) {
    if (_call_5_9[0] != NULL) _call_5_9[0] ();
    EXTI_ClearITPendingBit(EXTI_Line5);
  }
  if (EXTI_GetITStatus(EXTI_Line6)) {
    if (_call_5_9[1] != NULL) _call_5_9[1] ();
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
  if (EXTI_GetITStatus(EXTI_Line7)) {
    if (_call_5_9[2] != NULL) _call_5_9[2] ();
    EXTI_ClearITPendingBit(EXTI_Line7);
  }
  if (EXTI_GetITStatus(EXTI_Line8)) {
    if (_call_5_9[3] != NULL) _call_5_9[3] ();
    EXTI_ClearITPendingBit(EXTI_Line8);
  }
  if (EXTI_GetITStatus(EXTI_Line9)) {
    if (_call_5_9[4] != NULL) _call_5_9[4] ();
    EXTI_ClearITPendingBit(EXTI_Line9);
  }

}

static void (*_call[6])(void);
void EXTI15_10_Register_Call(unsigned int line, void (*fun)(void))
{
  if (line & EXTI_Line10) _call[0] = fun;
  if (line & EXTI_Line11) _call[1] = fun;
  if (line & EXTI_Line12) _call[2] = fun;
  if (line & EXTI_Line13) _call[3] = fun;
  if (line & EXTI_Line14) _call[4] = fun;
  if (line & EXTI_Line15) _call[5] = fun;
}
void EXTI15_10_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line10)) {
    if (_call[0] != NULL) _call[0] ();
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
  if (EXTI_GetITStatus(EXTI_Line11)) {
    if (_call[1] != NULL) _call[1] ();
    EXTI_ClearITPendingBit(EXTI_Line11);
  }
  if (EXTI_GetITStatus(EXTI_Line12)) {
    if (_call[2] != NULL) _call[2] ();
    EXTI_ClearITPendingBit(EXTI_Line12);
  }
  if (EXTI_GetITStatus(EXTI_Line13)) {
    if (_call[3] != NULL) _call[3] ();
    EXTI_ClearITPendingBit(EXTI_Line13);
  }
  if (EXTI_GetITStatus(EXTI_Line14)) {
    if (_call[4] != NULL) _call[4] ();
    EXTI_ClearITPendingBit(EXTI_Line14);
  }
  if (EXTI_GetITStatus(EXTI_Line15)) {
    if (_call[5] != NULL) _call[5] ();
    EXTI_ClearITPendingBit(EXTI_Line15);
  }  
}
static void (*_call_2)(void) = NULL;
void EXTI2_Register_Call(unsigned int line, void (*fun)(void))
{
   if (line & EXTI_Line2) _call_2 = fun;
}
void EXTI2_IRQHandler(void)
{
  if (_call_2 != NULL) {
    _call_2();
  }
  EXTI_ClearITPendingBit(EXTI_Line2);
}