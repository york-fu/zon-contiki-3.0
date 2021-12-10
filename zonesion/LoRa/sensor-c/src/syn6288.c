/*********************************************************************************************
* 文件：syn6288.c
* 作者：zonesion
* 说明：syn6288驱动程序
* 修改：Chenkm 2017.01.04 增加了注释
* 注释：
*********************************************************************************************/
/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "syn6288.h"
#include "usart.h"
#include "string.h"
/*********************************************************************************************
* 名称：uart_send_char()
* 功能：串口发送字节函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart_send_char(unsigned char ch)
{
  uart2_putc(ch);
}

/*********************************************************************************************
* 名称：syn6288_init()
* 功能：syn6288初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void syn6288_init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
 	
  uart2_init(9600); 
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA端口时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

int syn6288_busy(void)
{ 
  if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))                                                      //忙检测引脚
    return 0;                                                   //没有检测到信号返回 0
  else
    return 1;                                                   //检测到信号返回 1
}
/*********************************************************************************************
* 名称：syn6288_play()
* 功能：
* 参数：s -- 数组名
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void syn6288_play(char *s)
{
  int i;
  int len = strlen(s);
  unsigned char c = 0;  
  unsigned char head[] = {0xFD,0x00,0x00,0x01,0x00};            //数据包头  
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
* 名称: hex2unicode()
* 功能: 将16进制unicode字符串转换成bin格式
* 参数: 
* 返回: 
* 修改:
* 注释: 
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
* 功能：
* 参数：s -- 数组名
* 返回：
* 修改：
* 注释：
*******************************************************************************/
void syn6288_play_unicode(char *s, int len)
{
  int i;
  char c = 0;  
  unsigned char head[] = {0xFD,0x00,0x00,0x01,0x03};            //数据包头  
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
