/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-B����������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "led.h"
#include "stepmotor.h"
#include "rgb.h"
#include "relay.h"
#include "FAN.h"
#include "beep.h"
#include "mode.h"

#include "zxbee.h"
#include "zxbee-inf.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8_t  D0 = 0xFF;                                        // Ĭ�ϴ������ϱ�����
static uint8_t  D1 = 0;                                           // �̵�����ʼ״̬Ϊȫ��
static uint16_t V0 = 30;                                          // V0����Ϊ�ϱ�ʱ������Ĭ��Ϊ30s
static uint8_t  mode = 0;                                         // ���Ʊ�ʶ��
/*********************************************************************************************
* ���ƣ�updateV0()
* ���ܣ�����V0��ֵ
* ������*val -- �����µı���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateV0(char *val)
{
  //���ַ�������val����ת��Ϊ���ͱ�����ֵ
  V0 = atoi(val);                                 // ��ȡ�����ϱ�ʱ�����ֵ
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
  if(mode_read() == 0){
    mode = 1;
    stepmotor_init(); 
    FAN_init();
  }else{
    mode = 2;
    rgb_init();
    Beep_init();
  }
  led_init();
  relay_init();
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
  
  ZXBeeBegin();
  
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
  static uint8_t stepmotor_flag = 0;
  if(mode == 1){                                                  // ������ģʽһ����
    if ((cmd & 0x04) == 0x04) {                                   // �����������λ��bit2
      if(stepmotor_flag != 1) {                                   // ���������ת
        stepmotor_flag = 1; 
        forward(5000);
      }
    } else {                                                      // ���������ת
      if(stepmotor_flag != 0) {
        stepmotor_flag = 0; 
        reversion(5000);
      }
    }
    if(cmd & 0x08){                                   // ����cmd���������Ӧ�Ŀ��Ƴ���
      FAN_on(0x01);                                             // ��������
    }
    else{
      FAN_off(0x01);                                            // �رշ���       
    }
  }

  if(mode == 2){                                                  // ������ģʽ������
    if ((cmd & 0x01) == 0x01){                                    // RGB�ƿ���λ��bit0~bit1
      if ((cmd & 0x02) == 0x02){                                  // cmdΪ3��������
        rgb_off(0x01);                                          
        rgb_off(0x02);                                         
        rgb_on(0x04);                                          
      }
      else{                                                       // cmdΪ1�������
        rgb_on(0x01);                                         
        rgb_off(0x02);                                          
        rgb_off(0x04);                                        
        
      }
    }
    else if ((cmd & 0x02) == 0x02){                               // cmdΪ2�����̵�
      rgb_off(0x01);                                           
      rgb_on(0x02);                                            
      rgb_off(0x04);                                           
    }
    else{                                                         // cmdΪ1����ȫ��
      rgb_off(0x01);                                           
      rgb_off(0x02);                                           
      rgb_off(0x04);                                           
    }
    if(cmd & 0x08){                                             // ����������λ��bit3
      Beep_on(0x01);                                            // ����������
    }
    else{
      Beep_off(0x01);                                           // �رշ�����       
    }
  }
  if(cmd & 0x20){                                                // LED2�ƿ���λ��bit5
    led_on(0x02);                                                //����LED2
  }
  else{
    led_off(0x02);                                               //�ر�LED2        
  }
  if(cmd & 0x10){                                                // LED1�ƿ���λ��bit4
    led_on(0x01);                                                //����LED1
  }
  else{
    led_off(0x01);                                               //�ر�LED1
  }
  
  relay_control(cmd>>6);
}
/*********************************************************************************************
* ���ƣ�ZXBeeUserProcess()
* ���ܣ������յ��Ŀ�������
* ������*ptag -- ������������
*       *pval -- �����������
* ���أ�ret -- p�ַ�������
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
  if (0 == strcmp("CD0", ptag)){                                // ��D0��λ���в�����CD0��ʾλ�������
    D0 &= ~val;
  }
  if (0 == strcmp("OD0", ptag)){                                // ��D0��λ���в�����OD0��ʾλ��һ����
    D0 |= val;
  }
  if (0 == strcmp("D0", ptag)){                                 // ��ѯ�ϱ�ʹ�ܱ���
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D0);
      ZXBeeAdd("D0", p);
    } 
  }
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
  if (0 == strcmp("V0", ptag)){
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", V0);                         	// �ϱ�ʱ����
      ZXBeeAdd("V0", p);
    }else{
      updateV0(pval);
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
  
  etimer_set(&et_update, CLOCK_SECOND*V0); 
  etimer_set(&et_check, CLOCK_SECOND/10); //100 hz
  
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
    if (etimer_expired(&et_check)) {
      sensorCheck();
      etimer_set(&et_check, CLOCK_SECOND/10);
    }
    if (etimer_expired(&et_update)) {
      sensorUpdate();
      etimer_set(&et_update, CLOCK_SECOND*V0); 
    } 
  }
  PROCESS_END();
}