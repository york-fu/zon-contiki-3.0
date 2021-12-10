/*********************************************************************************************
* �ļ���zxbee-inf.c
* ���ߣ�xuzhy 
* ˵����
*       
*       
*      
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include "contiki.h"
#include <string.h>
#include "zxbee.h"
#include "zxbee-inf.h"
#include "dev/leds.h"
#include "config.h"

int zhiyun_send(char *pkg);
void clock_delay_ms(unsigned int ms);

int zhiyun_send_coap(char *pkg);
int zhiyun_send_udp(char *pkg);

#define COAP    0
#define UDP     1


PROCESS_NAME(u_zhiyun_coap);
PROCESS_NAME(u_zhiyun_udp);

/*********************************************************************************************
* ���ƣ�ZXBeeInfInit
* ���ܣ�ZXBee�ӿڵײ��ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeInfInit(void)
{
  config_init();

  if (nbConfig.mode == 0) {
    process_start(&u_zhiyun_coap, NULL);
  } else {
     process_start(&u_zhiyun_udp, NULL);
  }
}

/*********************************************************************************************
* ���ƣ�ZXBeeInfSend
* ���ܣ�ZXBee�ײ㷢�ͽӿ�
* ������p: ZXBee��ʽ����
       len�����ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void  ZXBeeInfSend(char *p, int len)
{
  leds_on(1);
  clock_delay_ms(50);
  if (nbConfig.mode == COAP) {
    zhiyun_send_coap(p);
  } else {
    zhiyun_send_udp(p);
  }
  leds_off(1);
}
/*********************************************************************************************
* ���ƣ�ZXBeeInfRecv
* ���ܣ����յ�ZXbee���ݱ�������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeInfRecv(char *buf, int len)
{
  leds_on(1);
  clock_delay_ms(50);
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
  leds_off(1);
}