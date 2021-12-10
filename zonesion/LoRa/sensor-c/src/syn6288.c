/*********************************************************************************************
* �ļ���syn6288.c
* ���ߣ�zonesion
* ˵����syn6288��������
* �޸ģ�Chenkm 2017.01.04 ������ע��
* ע�ͣ�
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "syn6288.h"
#include "usart.h"
#include "string.h"
/*********************************************************************************************
* ���ƣ�uart_send_char()
* ���ܣ����ڷ����ֽں���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart_send_char(unsigned char ch)
{
  uart2_putc(ch);
}

/*********************************************************************************************
* ���ƣ�syn6288_init()
* ���ܣ�syn6288��ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void syn6288_init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
 	
  uart2_init(9600); 
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PA�˿�ʱ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

int syn6288_busy(void)
{ 
  if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))                                                      //æ�������
    return 0;                                                   //û�м�⵽�źŷ��� 0
  else
    return 1;                                                   //��⵽�źŷ��� 1
}
/*********************************************************************************************
* ���ƣ�syn6288_play()
* ���ܣ�
* ������s -- ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void syn6288_play(char *s)
{
  int i;
  int len = strlen(s);
  unsigned char c = 0;  
  unsigned char head[] = {0xFD,0x00,0x00,0x01,0x00};            //���ݰ�ͷ  
  head[1] = (len+3)>>8;
  head[2] = (len+3)&0xff;
  for (i=0; i<5; i++){
    uart_send_char(head[i]);
    c ^= head[i];
  }
  for (i=0; i<len; i++){
    uart_send_char(s[i]);
    c ^= s[i];
  }
  uart_send_char(c);
}

/*******************************************************************************
* ����: hex2unicode()
* ����: ��16����unicode�ַ���ת����bin��ʽ
* ����: 
* ����: 
* �޸�:
* ע��: 
*******************************************************************************/
char *hex2unicode(char *str)
{       
  static char uni[64];
  int n = strlen(str)/2;
  if (n > 64) n = 64;
  
  for (int i=0; i<n; i++) {
    unsigned int x = 0;
    for (int j=0; j<2; j++) {
      char c = str[2*i+j];
      char o;
      if (c>='0' && c<='9') o = c - '0';
      else if (c>='A' && c<='F') o = 10+(c-'A');
      else if (c>='a' && c<='f') o = 10+(c-'a');
      else o = 0;
      x = (x<<4) | (o&0x0f);
    }
    uni[i] = x;
  }
  uni[n] = 0;
  return uni;
}


/*******************************************************************************
* syn6288_play_unicode()
* ���ܣ�
* ������s -- ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*******************************************************************************/
void syn6288_play_unicode(char *s, int len)
{
  int i;
  char c = 0;  
  unsigned char head[] = {0xFD,0x00,0x00,0x01,0x03};            //���ݰ�ͷ  
  head[1] = (len+3)>>8;
  head[2] = (len+3)&0xff;
  for (i=0; i<5; i++){
    uart_send_char(head[i]);
    c ^= head[i];
  }
  for (i=0; i<len; i++){
    uart_send_char(s[i]);
    c ^= s[i];
  }
  uart_send_char(c);
}
