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
#include "zxbee.h"
#include "zxbee-inf.h"

extern char* _zxbee_onrecv_fun(char *pkg, int len);
extern int LoraSendPackage(char *buf, int len);
extern void LoraNetInit(void);
extern int LoraNetSend(unsigned short da, char *buf, int len);
extern void LoraNetSetOnRecv(void (*_fun)(char *pkg, int len));

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
  LoraNetInit();
  LoraNetSetOnRecv(ZXBeeInfRecv);
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
  LoraNetSend(0, p, len);
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
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}