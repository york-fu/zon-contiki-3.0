#include "stm32f10x.h"

void RTC_Configuration(void)

{
    /* ʹ��PWR��BKPʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP,ENABLE);
    /* ʹ�ܶԺ󱸼Ĵ����ķ��� */ 
    PWR_BackupAccessCmd(ENABLE);
    /* ��λBKP�Ĵ��� */ 
    BKP_DeInit();
    /* ʹ��LSE */ 
    //RCC_LSEConfig(RCC_LSE_ON);
    /*�ȴ�������� */
   // while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}
    
    //RCC_LSICmd(ENABLE);
    //while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}
     
    /* �� RTCʱ������ΪLSE���32.768KHZ�ľ���*/ 
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    
    /* ʹ��RTC Clock */ 
    RCC_RTCCLKCmd(ENABLE);
    /* �ȴ�ͬ�� */ 
    RTC_WaitForSynchro();
    /* �ȴ���RTC�Ĵ�������д�������*/             
    RTC_WaitForLastTask();
    /* ������Ԥ��Ƶֵ: ����RTCʱ������Ϊ1s */
    RTC_SetPrescaler(61); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)*/
    /* �ȴ���RTC�Ĵ�������д������� */
    RTC_WaitForLastTask();
    /* ʹ��RTC���ж� */ 
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    /* �ȴ���RTC�Ĵ�������д������� */         
    RTC_WaitForLastTask();
    
    //RTC_EnterConfigMode();
    RTC_SetCounter(0);
    RTC_WaitForLastTask();
    //RTC_ExitConfigMode();
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;      //RTCȫ���ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //��ռ���ȼ�1λ,�����ȼ�3λ
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //��ռ���ȼ�0λ,�����ȼ�4λ
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     //ʹ�ܸ�ͨ���ж�
    NVIC_Init(&NVIC_InitStructure);     //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���


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
  RTC_ClearITPendingBit(RTC_IT_ALR); //�������ж�
}