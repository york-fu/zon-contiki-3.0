#include "stm32f10x.h"

void RTC_Configuration(void)

{
    /* 使能PWR和BKP时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP,ENABLE);
    /* 使能对后备寄存器的访问 */ 
    PWR_BackupAccessCmd(ENABLE);
    /* 复位BKP寄存器 */ 
    BKP_DeInit();
    /* 使能LSE */ 
    //RCC_LSEConfig(RCC_LSE_ON);
    /*等待启动完成 */
   // while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}
    
    //RCC_LSICmd(ENABLE);
    //while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}
     
    /* 将 RTC时钟设置为LSE这个32.768KHZ的晶振*/ 
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    
    /* 使能RTC Clock */ 
    RCC_RTCCLKCmd(ENABLE);
    /* 等待同步 */ 
    RTC_WaitForSynchro();
    /* 等待对RTC寄存器最后的写操作完成*/             
    RTC_WaitForLastTask();
    /* 配置了预分频值: 设置RTC时钟周期为1s */
    RTC_SetPrescaler(61); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)*/
    /* 等待对RTC寄存器最后的写操作完成 */
    RTC_WaitForLastTask();
    /* 使能RTC秒中断 */ 
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    /* 等待对RTC寄存器最后的写操作完成 */         
    RTC_WaitForLastTask();
    
    //RTC_EnterConfigMode();
    RTC_SetCounter(0);
    RTC_WaitForLastTask();
    //RTC_ExitConfigMode();
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;      //RTC全局中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //先占优先级1位,从优先级3位
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //先占优先级0位,从优先级4位
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     //使能该通道中断
    NVIC_Init(&NVIC_InitStructure);     //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器


}
void RtcSetTimeout(unsigned int to)
{
  RTC_SetAlarm(to);
  RTC_WaitForLastTask();
}
unsigned int RtcGetTimerValue(void)
{
  return RTC_GetCounter();
}

void TimerIrqHandler( void );
void RTC_IRQHandler(void)
{
  TimerIrqHandler();
  RTC_ClearITPendingBit(RTC_IT_ALR); //清闹钟中断
}