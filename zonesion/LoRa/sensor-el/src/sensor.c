/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-EL����������
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
#include "oled.h"
#include "relay.h"
#include "key.h"
#include "buzzer.h"
#include "sensor.h"
#include "7941w.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define KEY     "\xFF\xFF\xFF\xFF\xFF\xFF"

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static unsigned char  D1 = 2;                                   // �̵�����ʼ״̬Ϊȫ��
static char A0[16];                                              // A0�洢����
static unsigned char A1 = 0;                                    //������ 0:id�� 1����ֵ��
static int A2 = 0;                                             //�����
static int A3 = 10000;                                           // �豸����ʼֵΪ100Ԫ 
static int A4 = 0;                                               //�豸��ǰ����
static int A5 = 0;                                                //�豸�ۼ�����
static  int V1 = 0; 
static  int V2 = 0; 

static int work_mod = 2; //1 �豸��ֵģʽ, 2 ����ģʽ,��λ�����Ը���Ƭ��ֵ
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
  RFID7941Init();
  
  key_init();
 
  relay_init();
  relay_control(2);;                                                  // �����̵���2��ģ�⿪������

  buzzer_init();
  OLED_Init();
  OLED_Clear();
  OLED_ShowCHinese(21,2,4);
  OLED_ShowCHinese(21+13,2,5);
  OLED_ShowCHinese(21+13+13,2,12);
  
  OLED_ShowCHinese(21,0,8);
  OLED_ShowCHinese(21+13,0,9);
  OLED_ShowCHinese(21+13+13,0,12);
  if(work_mod == 1){
    OLED_ShowCHinese(21+13*3,0,6);
    OLED_ShowCHinese(21+13*4,0,7);
  }
  else{
    OLED_ShowCHinese(21+13*3,0,10);
    OLED_ShowCHinese(21+13*4,0,11);
  }
  char buf[16];
  sprintf(buf, "%d.%d", A3/100, A3%100);
  OLED_ShowString(21+13*3,2,(unsigned char *)buf,12); 
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
  
  if (A3 > 0) {
    A4 = 1 + rand()%500;                                              // �豸�������ѿۿ��ͨ�����������
    if (A4 > A3) A4 = A3;
    A3 = (A3 - A4);                                             // �豸���
    A5 = (A5 + A4);                                             // �豸�ۼ����ѽ��
    
    ZXBeeBegin();
    sprintf(p, "%d.%d", A3/100, A3%100);                      // �������
    ZXBeeAdd("A3", p);
    sprintf(p, "%d.%d", A4/100, A4%100);                      // ���������ѿۿ���
    ZXBeeAdd("A4", p);
    sprintf(p, "%d.%d", A5/100, A5%100);                      // �����ۻ����ѽ��
    ZXBeeAdd("A5", p);
	  
    p = ZXBeeEnd();                                             // ��������֡��ʽ��β
    if (p != NULL) {												
      ZXBeeInfSend(p, strlen(p));	                            // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
    }
    sprintf(p, "%d.%d  ", A3/100, A3%100);
    OLED_ShowString(21+13*3,2,(unsigned char *)p,12);           // oLED��ʾ�������
    if(A3 == 0) {   //����۷ѣ��̵����Ͽ���ģ�������豸�Ͽ���
      D1 &= ~2;
      relay_control(D1);
    }
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
  char buff[16];
  
  ZXBeeBegin();
  /* �������� */
  uint8_t k = get_key_status();
  static uint8_t last_key = 0;
  if (k & 0x01 == 0x01) {
    if ((last_key & 0x01) == 0) {
      if (work_mod == 1) {
          OLED_ShowCHinese(21+13*3,0,10);
          OLED_ShowCHinese(21+13*4,0,11);
          work_mod = 2;
      } else {
        OLED_ShowCHinese(21+13*3,0,6);
        OLED_ShowCHinese(21+13*4,0,7);
        work_mod = 1;
      }
    }
  } else if ((k & 0x02) == 0x02) {
    if ((last_key & 0x02) == 0) {
      //ģ������
      if (A3 > 0) {
        A4 = rand()%500;                                          // �豸�������ѿۿ��ͨ�����������
        if (A4 > A3) A4 = A3;
        A3 = A3 - A4;
        A5 = A5 + A4;
        sprintf(buff, "%d.%d", A3/100,A3%100);
        ZXBeeAdd("A3", buff);
        sprintf(buff, "%d.%d", A4/100,A4%100);
        ZXBeeAdd("A4", buff);
        sprintf(buff, "%d.%d", A5/100,A5%100);
        ZXBeeAdd("A5", buff);
        sprintf(buff, "%d.%d   ", A3/100,A3%100);
        OLED_ShowString(21+13*3,2,(unsigned char *)buff,12);
        if(A3 == 0) {   //����۷ѣ��̵����Ͽ���ģ�������豸�Ͽ���
          D1 &= ~2;
          relay_control(D1);
        }
      }
    }
  }
  last_key = k;
  //  �����������
  
  // ��������
  static char status = 1;
  static char last_cid[7];
  static char card_cnt = 0;                                     // ��Ƭ������
  static char cid[7];
  static char write = 0;
  
  if (status == 1) {                                            // ���125k�������
    buzzer_off();
    if (RFID7941CheckCard125kHzRsp(cid) > 0) {
      if (memcmp(last_cid, cid, 5) != 0) {
        sprintf(A0, "%02X%02X%02X%02X%02X", cid[0],cid[1],cid[2],cid[3],cid[4]);
        ZXBeeAdd("A0", A0);
        A1 = 0;
        sprintf(buff, "%d", A1);
        ZXBeeAdd("A1", buff);
        memcpy(last_cid, cid, 5);
        card_cnt = 5;
        buzzer_on(); 
      } else { //ͬһ�ſ�Ƭ
        card_cnt = 5;
      }
      RFID7941CheckCard125kHzReq();
    } else {
      if (card_cnt > 0) {
        card_cnt -= 1;
      }
      if (card_cnt == 0) {
        memset(last_cid, 0, sizeof last_cid);
        RFID7941CheckCard1356MHzReq();                          // ���13.56M��Ƭ
        status = 2;
      } else {
        RFID7941CheckCard125kHzReq();
      }
    }
  } else if (status == 2) {                                     // 13.56��Ƭ��⴦��
    buzzer_off();
    if (RFID7941CheckCard1356MHzRsp(cid) > 0) {
      if (memcmp(last_cid, cid, 4) != 0) {
        RFID7941ReadCard1356MHzReq(8, KEY);
        status = 3;
      } else {
        card_cnt = 5;                                           // ˢ�¿�Ƭ����
        if (work_mod == 1) {                                    // ��ֵ�豸
          if (write == 3 && A2 > 0) {
            memset(buff, 0, 16);
            RFID7941WriteCard1356MHzReq(8, KEY, buff);
            status = 4;
          } else {
            RFID7941CheckCard1356MHzReq();
          }
        } else
        if (work_mod == 2) {                                    // ����ģʽ
          int money;
          if (V1 != 0) {                                        // ��ֵ
            money = A2 + V1;    
            write = 1;
          } else if (V2 != 0) {
            money = A2 - V2;
            write = 2;
          }
          if (V1 != 0 || V2 != 0) {
            memset(buff, 0, 16);
            buff[12] = (money>>24) & 0xff;
            buff[13] = (money>>16) & 0xff;
            buff[14] = (money>>8) & 0xff;
            buff[15] = money&0xff;
            RFID7941WriteCard1356MHzReq(8, KEY, buff);
            status = 5;
          } else {
            RFID7941CheckCard1356MHzReq();
          }
        } else {
          RFID7941CheckCard1356MHzReq();
        }
      }
    } else {
      if (card_cnt > 0) {
        card_cnt -= 1;
      }
      if (card_cnt == 0) {
        memset(last_cid, 0, sizeof last_cid);
        RFID7941CheckCard125kHzReq();
        status = 1;
        A2 = 0;                                                 // �������
      } else {
        RFID7941CheckCard1356MHzReq();
      }
    }
  } else if (status == 3) {                                     // ����Ƭ��ȡ���
    if (RFID7941ReadCard1356MHzRsp(buff) > 0) {                 // ��ȡ�������浱ǰ��Ƭid
      memcpy(last_cid, cid, 4);
      card_cnt = 5;      
      sprintf(A0, "%02X%02X%02X%02X", cid[0],cid[1],cid[2],cid[3]);  
      int money = ((uint32_t)buff[12]<<24) | ((uint32_t)buff[13]<<16) | (buff[14]<<8) | (buff[15]);
      A2 = money;
      if (work_mod == 1) {                                      // �豸��ֵģʽ
        if (money > 0) {
          buff[12] = buff[13] = buff[14] = buff[15] = 0;
          write = 3;                                            // �ٴμ�⿨Ƭ��ͨ��write = 3 ��д��
          RFID7941CheckCard1356MHzReq();
          status = 2;
        } else {
          buzzer_on();                          
          RFID7941CheckCard1356MHzReq();
          status = 2;
        }
      } else {  //work_mod == 2;
        ZXBeeAdd("A0", A0);
        ZXBeeAdd("A1", "1");
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        V1 = 0;
        V2 = 0;
        buzzer_on();                          //
        RFID7941CheckCard1356MHzReq();
        status = 2;
      }
    } else {
      RFID7941CheckCard1356MHzReq();                            // ���¼�⿨Ƭ
      status = 2;
    }
  } else if (status == 4) {                                     // �����ֵ�豸д�����
 
      if (RFID7941WriteCard1356MHzRsp() > 0) {
        //��ֵ�豸�ɹ�
        A3 += A2;
        A2 = 0;
        sprintf(buff, "%d.%d   ", A3/100,A3%100);
        OLED_ShowString(21+13*3,2,(unsigned char *)buff,12);
        
        write = 0;
        D1 |= 2;
        relay_control(D1);
        
        buzzer_on();
      } else { //д��ʧ��
        //memset(last_cid, 0, sizeof last_cid);
        //card_cnt = 0;
      }
      RFID7941CheckCard1356MHzReq();
      status = 2;
  } else if (status == 5) {                                     // ��Ƭ��ֵ�۷ѽ�����
    if (RFID7941WriteCard1356MHzRsp() > 0) {
      if (write == 1) {
        ZXBeeAdd("V1", "1");
        A2 += V1;
        V1 = 0;
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        write = 0;
      } else if (write == 2) {
        ZXBeeAdd("V2", "1");
        A2 -= V2;
        V2 = 0;
        sprintf(buff, "%d.%d", A2/100,A2%100);
        ZXBeeAdd("A2", buff);
        write  = 0;
      }
      buzzer_on();
    }
    RFID7941CheckCard1356MHzReq();
    status = 2;
  }
  
  char *p = ZXBeeEnd();
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
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
  relay_control(cmd);
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
  
  if (0 == strcmp("V1", ptag)) {
    float v = atof(pval);
    V1 = (int)(v * 100);
    
  }
  if (0 == strcmp("V2", ptag)) {
    float v = atof(pval);
    V2 = (int)(v * 100);
    if (V2 > A2) {
      V2 = 0;
      ZXBeeAdd("V2", "0");
    }
  }
  if (0 == strcmp("V3", ptag)) {
    float v = atof(pval); 
    A3 =  A3 + (int)(v * 100);
    ZXBeeAdd("V3", "1");  
    D1 |= 2;
    relay_control(D1);
  }
  if (0 == strcmp("V4", ptag)) {
    float v = atof(pval);
    int x = (int)(v * 1000);
    if (A3 >= x) {
        A3 =  A3 - (int)(v * 100);
        ZXBeeAdd("V4", "1");
        if (A3 == 0) {
                D1 &= ~2;
                relay_control(D1);
        }
    } else {
        ZXBeeAdd("V4", "0");
    }
   
  }

  // �����������
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