/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-C����������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*
 * mode��������ѡ���ʶ���������ߵĲ�ͬmode
 *  1: ѡ��������⴫��������&����&���洫����
 *  2��ѡ�����������������ϳɴ�����
 *
 */
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "stm32f10x.h"
#include "delay.h"
#include "Flame.h"
#include "grating.h"
#include "hall.h"
#include "relay.h"
#include "infrared.h"
#include "MP-4.h"
#include "Touch.h"
#include "vibration.h"
#include "syn6288.h"

#include "zxbee.h"
#include "zxbee-inf.h"
#include "sensor.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8_t  D0 = 0xFF;                                        // Ĭ�ϴ������ϱ�����
static uint8_t  D1 = 0;                                           // �̵�����ʼ״̬Ϊȫ��
static uint8_t  A0 = 0;                                           // mode=1 Ϊ��������⣬mode=2 Ϊ�������
static uint8_t  A1 = 0;                                           // mode=1 Ϊ�𶯼�⣬mode=2 û�ж�Ӧ������
static uint8_t  A2 = 0;                                           // mode=1 Ϊ������⣬mode=2 û�ж�Ӧ������
static uint8_t  A3 = 0;                                           // mode=1 Ϊ�����⣬mode=2 û�ж�Ӧ������
static uint8_t  A4 = 0;                                           // ��ȼ������
static uint8_t  A5 = 0;                                           // �����դ���
static uint16_t V0 = 30;                                          // V0����Ϊ�ϱ�ʱ������Ĭ��Ϊ30s
static char*  V1;                                                 // ����ת�١�����������Ƶ��
static uint8_t mode = 0;  
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
  V0 = atoi(val);                                               // ��ȡ�����ϱ�ʱ�����ֵ
}
/*********************************************************************************************
* ���ƣ�updateA0()
* ���ܣ�����A0��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA0(void)
{
   static uint32_t ct = 0;
   
  if (mode == 1) {                                               // ����������⴫������ֵ
    A0 = get_infrared_status();
    if (A0 != 0) {
      ct = clock_time();
    } else if (clock_time() > ct+1000) {
      ct = 0;
      A0 = 0;
    } else {
      A0 = 1;
    }
  } else {
    A0 = get_touch_status();                                     // ���´�����������ֵ
  }
}
/*********************************************************************************************
* ���ƣ�updateA1()
* ���ܣ�����A1��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA1(void)
{
   static uint32_t ct = 0;
  
  if (mode == 1) {                                              // �����񶯴�������ֵ
    A1 = get_vibration_status();
    if (A1 != 0) {
      ct = clock_time();
    } else if (clock_time() > ct+2000) {
      ct = 0;
      A1 = 0;
    } else {
      A1 = 1;
    }
  }
}
/*********************************************************************************************
* ���ƣ�updateA2()
* ���ܣ�����A2��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA2(void)
{
  if (mode == 1) {
    A2 = get_hall_status();                                     // ���»�����������ֵ
  }
}
/*********************************************************************************************
* ���ƣ�updateA3()
* ���ܣ�����A3��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA3(void)
{
  static uint32_t ct = 0;
  
  if (mode == 1) {
    A3 = get_flame_status();                                    // ���»��洫������ֵ
    if (A3 != 0) {
      ct = clock_time();
    } else if (clock_time() > ct+2000) {
      ct = 0;
      A3 = 0;
    } else {
      A3 = 1;
    }
  }
}
/*********************************************************************************************
* ���ƣ�updateA4()
* ���ܣ�����A4��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA4(void)
{
  A4 = get_combustiblegas_data();                               // ����ȼ����������ֵ
}
/*********************************************************************************************
* ���ƣ�updateA5()
* ���ܣ�����A5��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA5(void)
{
  A5 = get_grating_status();                                    // ���¹�դ��������ֵ
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
  clock_delay_ms(300);  //�ȴ�����оƬ��ʼ������ټ��io��
  vibration_init();
  if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)){
    mode = 1;
    infrared_init();                                            // ������⴫������ʼ��
    flame_init();                                               // ���洫������ʼ��
    hall_init();                                                // ������������ʼ��
  }
  else{                                                         // ����������ģʽ2
    mode = 2;
    touch_init();                                               // ������������ʼ��
    syn6288_init();                                             // �����ϳɳ�ʼ��    
    syn6288_play("���");
  }
  combustiblegas_init();                                        // ȼ����������ʼ��
  grating_init();                                               // ��դ��������ʼ��
  relay_init();                                                 // �̵�����ʼ��
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
  
  // ����D0��λ״̬�ж���Ҫ�����ϱ�����ֵ
  if ((D0 & 0x01) == 0x01){                                     // ���¶��ϱ�������pData�����ݰ�������¶�����
    updateA0();
    sprintf(p, "%u", A0); 
    ZXBeeAdd("A0", p);
  }
  if (mode == 1) {
    if ((D0 & 0x02) == 0x02){                                     // ��ʪ���ϱ�������pData�����ݰ������ʪ������
      updateA1();
      sprintf(p, "%u", A1); 
      ZXBeeAdd("A1", p);
    }   
    if ((D0 & 0x04) == 0x04){                                     // �������ϱ�������pData�����ݰ�����ӹ�������
      updateA2();
      sprintf(p, "%u", A2);  
      ZXBeeAdd("A2", p);
    }
    if ((D0 & 0x08) == 0x08){                                     // �����������ϱ�������pData�����ݰ�����ӿ�����������
      updateA3();
      sprintf(p, "%u", A3);  
      ZXBeeAdd("A3", p);
    }
  }
  if ((D0 & 0x10) == 0x10){                                     // ������ѹ���ϱ�������pData�����ݰ�����Ӵ���ѹ������
    updateA4();
    sprintf(p, "%u", A4);
    ZXBeeAdd("A4", p);
  }
  if ((D0 & 0x10) == 0x10){                                     // ������״̬�ϱ�������pData�����ݰ�����ӵ���״̬����
    updateA5();
    sprintf(p, "%u", A5);
    ZXBeeAdd("A5", p);
  }
  
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
  static char lastA0 = 0,lastA1=0,lastA2=0,lastA3=0,lastA4=0,lastA5=0;
  static uint32_t ct0=0, ct1=0, ct2=0, ct3=0, ct4=0, ct5=0;
  char pData[16];
  char *p = pData;
  
  updateA0();
  updateA1();
  updateA2();
  updateA3();
  updateA4();
  updateA5();
  
  ZXBeeBegin();
  
  if (lastA0 != A0 
      || ( //ģʽ1��������⴫����ѡͨ��������ѭ��3S�ϱ���ģʽ2������������ѡͨ��״̬��ת���ϱ�1�Σ�  
         (mode == 1)&& ct0 != 0 && clock_time() > (ct0+3000)) //����3���ϱ�
      ) {
    if (D0 & 0x01) {
      sprintf(p, "%u", A0);
      ZXBeeAdd("A0", p);
      ct0 = clock_time();
    }
    if (A0 == 0) {
        ct0 = 0;
    }
    lastA0 = A0;
  }
  if (mode == 1) {   // ģʽ1���񶯡����������洫����ѡͨ
    if (lastA1 != A1 || (ct1 != 0 && clock_time() > (ct1+3000))) {  // �񶯴��������
      if (D0 & 0x02) {
        sprintf(p, "%u", A1);
        ZXBeeAdd("A1", p);
        ct1 = clock_time();
      }
      if (A1 == 0) {
        ct1 = 0;
      }
      lastA1 = A1;
    }
    if (lastA2 != A2 || (ct2 != 0 && clock_time() > (ct2+3000))) {  // �������������
      if (D0 & 0x04) {
        sprintf(p, "%u", A2);
        ZXBeeAdd("A2", p);
        ct2 = clock_time();
      }
      if (A2 == 0) {
        ct2 = 0;
      }
      lastA2 = A2;
    }
    
    if (lastA3 != A3 || (ct3 != 0 && clock_time() > (ct3+3000))) {  // ���洫�������
      if (D0 & 0x08) {
        sprintf(p, "%u", A3);
        ZXBeeAdd("A3", p);
        ct3 = clock_time();
      }
      if (A3 == 0) {
        ct3 = 0;
      }
      lastA3 = A3;
    }
  }
  if (lastA4 != A4 || (ct4 != 0 && clock_time() > (ct4+3000))) {  // ȼ�����������
    if (D0 & 0x10) {
      sprintf(p, "%u", A4);
      ZXBeeAdd("A4", p);
      ct4 = clock_time();
    }
    if (A4 == 0) {
      ct4 = 0;
    }
    lastA4 = A4;
  }
  if (lastA5 != A5 || (ct5 != 0 && clock_time() > (ct5+3000))) {  // ��դ���������
    if (D0 & 0x01) {
      sprintf(p, "%u", A5);
      ZXBeeAdd("A5", p);
      ct5 = clock_time();
    }
    if (A5 == 0) {
      ct5 = 0;
    }
    lastA5 = A5;
  }

  p = ZXBeeEnd();
  if (p != NULL) {
    int len = strlen(p); 
    ZXBeeInfSend(p, len);
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
  // ����cmd���������Ӧ�Ŀ��Ƴ���
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
  if (0 == strcmp("A0", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA0();
      ret = sprintf(p, "%u", A0);   
      ZXBeeAdd("A0", p);
    } 
  }
  if (0 == strcmp("A1", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA1();
      ret = sprintf(p, "%u", A1);  
      ZXBeeAdd("A1", p);
    } 
  }
  if (0 == strcmp("A2", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA2();
      ret = sprintf(p, "%u", A2); 
      ZXBeeAdd("A2", p);
    } 
  }
  if (0 == strcmp("A3", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA3();
      ret = sprintf(p, "%u", A3); 
      ZXBeeAdd("A3", p);
    } 
  }
  if (0 == strcmp("A4", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA4();
      ret = sprintf(p, "%u", A4);
      ZXBeeAdd("A4", p);
    } 
  }  
  if (0 == strcmp("A5", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA5();
      ret = sprintf(p, "%u", A5);   
      ZXBeeAdd("A5", p);
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
  if (0 == strcmp("V1", ptag)){
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%s", V1);                
      ZXBeeAdd("V1", p);
    }else{
      if(mode == 2){
        int n = strlen(pval)/2;
        syn6288_play_unicode(hex2unicode(pval),n);
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