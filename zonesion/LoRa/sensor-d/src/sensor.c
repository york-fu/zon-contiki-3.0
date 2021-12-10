/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-D����������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "stm32f10x.h"
#include "relay.h"
#include "zlg7290.h"
#include "oled.h"
#include "sensor.h"
#include "zxbee.h"
#include "zxbee-inf.h"

PROCESS(sensor, "sensor");

extern void EXTI9_5_CallSet_2(void (*func)(void));

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8_t  D1 = 1;                                           // ���ӳ�ʼ״̬Ϊ��
static uint8_t  V1 = 0;                                           // ����Ƶ��
static uint8_t  V2 = 0;                                           // ��������
static unsigned char key_val = 0;
/*********************************************************************************************
* ���ƣ�EXTI2_IRQHandler()
* ���ܣ������ж�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zlg7290KeyIRQ(void)
{
    key_val = zlg7290_get_keyval();
}

/*********************************************************************************************
* ���ƣ�sensorInit()
* ���ܣ�������Ӳ����ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorInit(void)
{
  // ��ʼ������������
  OLED_Init();
  oled_turn_on();
  char buff[64];
  sprintf(buff,"%d ",V1);
  OLED_ShowString(8,1,(unsigned char*)buff,16);
  sprintf(buff,"%2d",V2);
  OLED_ShowString(104,5,(unsigned char*)buff,16);OLED_ShowString(56,3,"TV",16);
  zlg7290_init();
  segment_display(V1*100+V2);
  key_val = zlg7290_read_reg(SYETEM_REG);
  
  EXTI9_5_CallSet_2(zlg7290KeyIRQ);
}
/*********************************************************************************************
* ���ƣ�sensorLinkOn()
* ���ܣ��������ڵ������ɹ����ú���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
}
/*********************************************************************************************
* ���ƣ�sensorUpdate()
* ���ܣ����������ϱ�������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  ZXBeeBegin();                                                 // ��������֡��ʽ��ͷ
  
  sprintf(p, "%d", V1);
  ZXBeeAdd("V1", p);
  
  sprintf(p, "%d", V2);
  ZXBeeAdd("V2", p);
  
  sprintf(p, "%u", D1);                                  // �ϱ����Ʊ��� 
  ZXBeeAdd("D1", p);
  
  p = ZXBeeEnd();                                               // ��������֡��ʽ��β
  if (p != NULL) {												
    ZXBeeInfSend(p, strlen(p));	                                // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
  }
}
/*********************************************************************************************
* ���ƣ�sensorCheck()
* ���ܣ����������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorCheck(void)
{
  char buf[32];
  if (key_val != 0) {
    ZXBeeBegin();                                               //��ʼ����������״̬���������ͻ�����
    sprintf(buf, "%d", key_val);
    ZXBeeAdd("A0", buf);
    if (D1 & 0x01) {
      if(key_val == RIGHT){
        V2++;
        if(V2 > 99)
          V2 = 0;
      }
      if(key_val == LEFT){
        if(V2 == 0)
          V2 = 99;
        else
          V2--;
      }
      if(key_val == UP){
        V1++;
        if(V1 > 19)
          V1 = 0;
      }
      if(key_val == DOWN){
        if(V1 == 0)
          V1 = 19;
        else
          V1--;
      }
    }
    if(key_val == CENTER){
      if(D1 & 0X01){
        D1 &= ~0X01;
      }
      else
      {
        D1 |= 0X01;
      }
      sprintf(buf, "%u", D1);
      ZXBeeAdd("D1", buf);
    }
    if(D1 & 0X01){
      sprintf(buf, "%d", V1);
      ZXBeeAdd("V1", buf);
      sprintf(buf, "%d", V2);
      ZXBeeAdd("V2", buf);
    }
    char *p = ZXBeeEnd();                                       // ��������֡��ʽ��β
    if (p != NULL) {												
      ZXBeeInfSend(p, strlen(p));	                         // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
    }
    key_val = 0;
  }
  static uint8_t lastV1 = 0, lastV2 = 0, lastDis = 1;
  if (lastDis != (D1&0x01)) {
    lastDis = D1 & 0x01;
    if (lastDis == 0) {
       oled_turn_off();
       display_off();
    } else {
   
      oled_turn_on();                                          //������ʾ��
      OLED_DisFill(0,0,127,63,0);
      OLED_DisFill(0,0,127,7,1);
      OLED_DisFill(0,0,7,63,1);
      OLED_DisFill(0,56,127,63,1);
      OLED_DisFill(120,0,127,63,1);
      OLED_Refresh_Gram();
      OLED_DisFill(0,0,127,63,1);
        
      char buff[64];
      sprintf(buff,"%d ",V1);
      OLED_ShowString(8,1,(unsigned char*)buff,16);
      sprintf(buff,"%2d",V2);
      OLED_ShowString(104,5,(unsigned char*)buff,16);OLED_ShowString(56,3,"TV",16);
      segment_display(V1*100+V2);
    }
  }
  if (V1 != lastV1 ||  V2 != lastV2) {
    char buff[64];  
    segment_display(V1*100+V2);
      
    sprintf(buff,"%d ",V1);
    OLED_ShowString(8,1,(unsigned char*)buff,16);
    sprintf(buff,"%2d",V2);
    OLED_ShowString(104,5,(unsigned char*)buff,16);OLED_ShowString(56,3,"TV",16);
    lastV1 = V1;
    lastV2 = V2;
  }   
}
/*********************************************************************************************
* ���ƣ�sensorControl()
* ���ܣ�����������
* ������cmd - ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorControl(uint8_t cmd)
{
	//����LEDˢ����ռ�ô���cpuֱ�ӣ��˴�ˢ���ᵼ�º����ȡ���ʱ��
	//���D1����ͳһ�ŵ�sensorCheck�д���
}
/*********************************************************************************************
* ���ƣ�ZXBeeUserProcess()
* ���ܣ������յ��Ŀ�������
* ������*ptag -- ������������
*       *pval -- �����������
* ���أ�<0:��֧��ָ�>=0 ָ���Ѵ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  int ret = 0;	
  char pData[16];
  char *p = pData;
  
  // ���ַ�������pval����ת��Ϊ���ͱ�����ֵ
  val = atoi(pval);	
  // �����������
  if (0 == strcmp("CD1", ptag)){                                // ��D1��λ���в�����CD1��ʾλ�������
    D1 &= ~val;
    sensorControl(D1);                                          // ����ִ������
  }
  if (0 == strcmp("OD1", ptag)){                                // ��D1��λ���в�����OD1��ʾλ��һ����
    D1 |= val;
    sensorControl(D1);                                          // ����ִ������
  }
  if (0 == strcmp("D1", ptag)){                                 // ��ѯִ�����������
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D1);
      ZXBeeAdd("D1", p);
    } 
  }
  
  if (0 == strcmp("V1", ptag)){ 
    if (0 == strcmp("?", pval)){   
      sprintf(p, "%d", V1);
      ZXBeeAdd("V1", p);
    }
    else {   
    if (val >= 0 && val <= 19) {
      V1 = val;
      }
    } 
  }  
  if (0 == strcmp("V2", ptag)){ 
    if (0 == strcmp("?", pval)){   
      sprintf(p, "%d", V2); 
      ZXBeeAdd("V2", p);
    } 
    else {   
      if (val >= 0 && val <= 99) {
        V2 = val;
      }
    }
  }
  return ret;
}
/*********************************************************************************************
* ���ƣ�sensor()
* ���ܣ��������ɼ��߳�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
PROCESS_THREAD(sensor, ev, data)
{
  static struct etimer et_update;
  static struct etimer et_check;
  
  PROCESS_BEGIN();
  
  ZXBeeInfInit();
  
  sensorInit();
  
  etimer_set(&et_update, CLOCK_SECOND*30); 
  etimer_set(&et_check, CLOCK_SECOND/10); //100 hz
  
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
    if (etimer_expired(&et_check)) {
      sensorCheck();
      etimer_set(&et_check, CLOCK_SECOND/10);
    }
    if (etimer_expired(&et_update)) {
      sensorUpdate();
      etimer_set(&et_update, CLOCK_SECOND*30); 
    } 
  }
  PROCESS_END();
}
