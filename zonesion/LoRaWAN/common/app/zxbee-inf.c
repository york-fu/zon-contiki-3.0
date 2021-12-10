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
#include <string.h>
#include <stdint.h>
#include "zxbee.h"
#include "zxbee-inf.h"

void at_notify_data(char *buf, int len);
int loraWanRegisterOnPortCall(int port, void (*fun)(uint8_t*, int));
void loraWanSendData(int AppPort, uint8_t *AppData, int AppDataSize);

static void onAppData(uint8_t *dat, int len)
{
  ZXBeeInfRecv((char*)dat, len);
}
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
  loraWanRegisterOnPortCall(2, onAppData);      //zhiyun���ݶ˿�
  loraWanRegisterOnPortCall(200, onAppData);    //�����ƾݶ˿�
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
  loraWanSendData(2, (uint8_t*)p, len); //���ƺ����׷�������õ���ͬһ�˿�
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
  at_notify_data(buf, len);
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}