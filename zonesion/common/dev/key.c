/*********************************************************************************************
*�ļ�: key.c
*����: zonesion   2015.12.18
*˵��: �����������
*�޸�:
*ע��:
*********************************************************************************************/
#include "key.h"
#include "stdio.h"
#include "delay.h"
/*********************************************************************************************
����ȫ�ֱ���
*********************************************************************************************/
unsigned int key_status = 0;
void key_adc_init(void)
{
  GPIO_InitTypeDef      GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);         //ʹ��GPIOCʱ��

  //�ȳ�ʼ��ADC2ͨ��0 IO��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;        //PC2/PC3 ADCͨ��12/13
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;                  //ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;             //����������
  GPIO_Init(GPIOC, &GPIO_InitStructure);                        //��ʼ��
}

u16 get_key_status(u8 key)
{
  //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
  //ADC2,ADCͨ��12/13,480������,��߲���ʱ�������߾�ȷ��
  ADC_RegularChannelConfig(ADC1, key, 1, ADC_SampleTime_480Cycles );
  ADC_SoftwareStartConv(ADC1);		                        //ʹ��ָ����ADC2�����ת����������
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));               //�ȴ�ת������
  return ADC_GetConversionValue(ADC1);	                        //�������һ��ADC2�������ת�����
}

#if     SYSTEM_SUPPORT_CONTIKI
/*********************************************************************************************
���������
*********************************************************************************************/
PROCESS(key, "key");
extern process_event_t process_event_key;
extern struct process lcd;
/*********************************************************************************************
* ����: PROCESS_THREAD()
* ����: �����������
* ����: sensors, ev, data
* ����: ��
* �޸�:
* ע��:
*********************************************************************************************/
PROCESS_THREAD(key, ev, data)
{
  static struct etimer key_timer;
  static unsigned char key_status_tmp = 0;
  PROCESS_BEGIN();
  key_adc_init();
  while (1) {
    if(get_key_status(KEY1_2) > 2000){
      key_status_tmp = 0;
      for(;;){
        etimer_set(&key_timer, 30*CLOCK_SECOND/1000);
        PROCESS_YIELD();
        if(get_key_status(KEY1_2) > 2000)
          key_status_tmp++;
        else break;
      }
      if(key_status_tmp >= 18){
        key_status = KEY1_LONG_PRESSED;
        printf("KEY1_LONG_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
      else{
        key_status = KEY1_PRESSED;
        printf("KEY1_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
    }

    else if(get_key_status(KEY1_2) > 1000){
      key_status_tmp = 0;
      for(;;){
        etimer_set(&key_timer, 30*CLOCK_SECOND/1000);
        PROCESS_YIELD();
        if((get_key_status(KEY1_2) > 1000) && (get_key_status(KEY1_2) < 2000))
          key_status_tmp++;
        else break;
      }
      if(key_status_tmp >= 18){
        key_status = KEY2_LONG_PRESSED;
        printf("KEY2_LONG_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
      else{
        key_status = KEY2_PRESSED;
        printf("KEY2_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
    }

    if(get_key_status(KEY3_4) > 2000){
      key_status_tmp = 0;
      for(;;){
        etimer_set(&key_timer, 30*CLOCK_SECOND/1000);
        PROCESS_YIELD();
        if(get_key_status(KEY3_4) > 2000)
          key_status_tmp++;
        else break;
      }
      if(key_status_tmp >= 18){
        key_status = KEY3_LONG_PRESSED;
        printf("KEY3_LONG_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
      else{
        key_status = KEY3_PRESSED;
        printf("KEY3_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
    }

    else if(get_key_status(KEY3_4) > 1000){
      key_status_tmp = 0;
      for(;;){
        etimer_set(&key_timer, 30*CLOCK_SECOND/1000);
        PROCESS_YIELD();
        if((get_key_status(KEY3_4) > 1000) && (get_key_status(KEY3_4) < 2000))
          key_status_tmp++;
        else break;
      }
      if(key_status_tmp >= 18){
        key_status = KEY4_LONG_PRESSED;
        printf("KEY4_LONG_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
      else{
        key_status = KEY4_PRESSED;
        printf("KEY4_PRESSED\r\n");
        process_post(&lcd,process_event_key,&key_status);
      }
    }
    etimer_set(&key_timer,20*CLOCK_SECOND/1000);
    PROCESS_YIELD();
  }
  PROCESS_END();
}
#endif