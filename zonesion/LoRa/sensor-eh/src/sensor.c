/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-EH����������
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
#include "contiki.h"
#include "zxbee.h"
#include "zxbee-inf.h"
#include "relay.h"
#include "key.h"
#include "sensor.h"
#include "rfid900m.h"
#include "motor.h"


PROCESS(sensor, "sensor");

/*********************************************************************************************
* �궨��
*********************************************************************************************/

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static unsigned char  D1 = 0;                                   // �̵�����ʼ״̬Ϊȫ��
static char A0[32];                                              // A0�洢����
static int A2 = 0;                                             //�����
static int V1;
static int V2;

static int motor = 0;
/*********************************************************************************************
* ����ԭ��
*********************************************************************************************/
void clock_delay_ms(unsigned int ms);
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
  motorInit();
  RFID900MInit();
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
  static char last_epc[12];
  static int new_tag = 0;                       //��⵽��Ƭ����ȡ���ɹ����
  static char epc[12];                                    //��¼��ǰ��⵽��Ƭepc
  static char write = 0;
  static char status = 1;                              //״̬ת����ʶ
  
  if (status == 1) {
    if (RFID900MCheckCardRsp(epc) > 0) {
      if (memcmp(last_epc, epc, 12) != 0)  {
        RFID900MReadReq(NULL, epc, BLK_USER, 0, 2); //���Ͷ�������
        status = 2;       
      } else {
        
        new_tag = 8;
        if (V1 != 0) {
          long money = A2 + V1;
          RFID900MWriteReq(NULL, epc, BLK_USER, 0, 2, (char*)&money);
          write = 1;
          status = 3;
        } else if (V2 != 0) {
          long money = A2 - V2;
          RFID900MWriteReq(NULL, epc, BLK_USER, 0, 2, (char*)&money);
          write = 2;
          status = 3;
        } else {
          RFID900MCheckCardReq();
          status = 1;
        }
      }
    } else {
      //û�м�⵽��Ƭ
      if (new_tag > 0) {
        new_tag -= 1;
        if (new_tag == 0) {
          memset(last_epc, 0, 12);
        }
      }
      RFID900MCheckCardReq();
      status = 1;
    }
  } else if (status == 2) {
    int money;
    if (RFID900MReadRsp((char*)&money) > 0) {  
      //��ȡ������ϱ�
      for (int j=0; j<12; j++) sprintf(&A0[j*2], "%02X", epc[j]);
      char buf[16];
      if (money < 0) money = 0;
      ZXBeeBegin();
      ZXBeeAdd("A0", A0);
      A2 = money;
      sprintf(buf, "%ld.%ld", A2/100, A2%100);
      ZXBeeAdd("A2", buf);
      char *p = ZXBeeEnd();
      if (p != NULL) {
        ZXBeeInfSend(p, strlen(p));
      }
      memcpy(last_epc, epc, 12);  //���浱ǰ��Ƭid
      new_tag = 8;
      V1 = 0;  //��ʼ���۽��
      V2 = 0;
    }

    RFID900MCheckCardReq();
    status = 1;
    
  } else if (status == 3) {
    if (RFID900MWriteRsp() > 0) {
      char buf[16];
      ZXBeeBegin();
      if (write == 1) {
        A2 = A2 + V1;
        V1 = 0;
        ZXBeeAdd("V1", "1"); 
      } else if (write == 2) {
        A2 = A2 - V2;
        V2 = 0;
        ZXBeeAdd("V2", "1");
      }
      sprintf(buf, "%d.%d", A2/100, A2%100);
      ZXBeeAdd("A2", buf);
      char *p = ZXBeeEnd();
      if (p != NULL) {
        ZXBeeInfSend(p, strlen(p));
      }
    } 
    RFID900MCheckCardReq();
    status = 1;
  }
  
    /*�������״̬���*/
  static int tick = 0;
  if (tick == motor){
    motorSet(0);                //�ر����
  }else if (tick > motor) {
    motorSet(2);                //����բ��
    tick -= 1;
  } else if (tick < motor) {
    motorSet(1);                //�ر�բ��
    tick += 1;
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
   if (cmd & 0x01) {
    motor = 3; //բ������ʱ��300ms
  } else {
    motor = 0;
  }
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
  char buf[32];
  // ���ַ�������pval����ת��Ϊ���ͱ�����ֵ
  val = atoi(pval);	
  // �����������
  if (0 == strcmp("A2", ptag)) {
    if (pval[0] == '?'){
      sprintf(buf, "%d.%d", A2/100,A2%100);
      ZXBeeAdd("A2", buf);
    }
  }
  if (0 == strcmp("V1", ptag)) {
    float v = atof(pval);
     V1 = (long) (v *100);
  }
  if (0 == strcmp("V2", ptag)) {
     float v = atof(pval);
     V2 = (long) (v *100);
  }
  if (0 == strcmp("CD1", ptag)){                                // ��D1��λ���в�����CD1��ʾλ�������
    D1 &= ~val;
    sensorControl(D1);
  }
  if (0 == strcmp("OD1", ptag)){                                // ��D1��λ���в�����OD1��ʾλ��һ����
    D1 |= val;
    sensorControl(D1);
  }
  if (0 == strcmp("D1", ptag)){                                 
    if (0 == strcmp("?", pval)){                                
      sprintf(buf, "%u", D1);                         
      ZXBeeAdd("D1", buf);
    } 
  }
  return 0;
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