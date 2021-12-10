#include "usart.h"	
#include "string.h"
#include "stm32f4xx.h"
#include <stdio.h>





/*********************************************************************************************
* ���ƣ�usart3_init
* ���ܣ�usart3��ʼ��
* ������bound������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart3_init(unsigned int bound){
   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);     //ʹ��GPIOBʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);    //ʹ��USART1ʱ��
 
  //����3��Ӧ���Ÿ���ӳ��
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_USART3);  //GPIOB10����ΪUSART3
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_USART3);  //GPIOB11����ΪUSART3
	
  //USART3�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;  //GPIOB10��GPIOB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             //���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             //����
  GPIO_Init(GPIOC,&GPIO_InitStructure);                    //��ʼ��PB10 PB11

  //USART3 ��ʼ������
  USART_InitStructure.USART_BaudRate = bound;                      //����������
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;      //�ֳ�Ϊ8λ���ݸ�ʽ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;           //һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;              //����żУ��λ
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //�շ�ģʽ
  USART_Init(USART3, &USART_InitStructure);                        //�����������ó�ʼ������3
	
	//Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;               //����3�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;         //��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		  //�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		  //IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);	                          //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);                  //��������3�����ж�
  USART_Cmd(USART3, ENABLE);                                      //ʹ�ܴ���3 	
}

static int (*input_call)( char x) = NULL;
void  uart3_set_input(int (*fun)( char ) )
{
 
  input_call = fun;
}

/*********************************************************************************************
* ���ƣ�USART3_IRQHandler
* ���ܣ������жϴ�����
* ������ 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void USART3_IRQHandler(void)                	           
{
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){         //����յ�����(�����ж�) 
       unsigned char x;
      x = USART_ReceiveData(USART3);
      if(input_call != NULL) input_call(x);   
  } 
}



void uart3_putc( char x)
{
  USART_SendData(USART3, x);
    /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}
