#include "usart.h"	
#include "string.h"
#include "stm32f4xx.h"


/*********************************************************************************************
* ���ƣ�usart5_init
* ���ܣ�usart5��ʼ��
* ������bound������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart5_init(unsigned int bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);          //ʹ��GPIOCʱ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);          //ʹ��GPIODʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);          //ʹ��UART5ʱ��
 
  //����5��Ӧ���Ÿ���ӳ��
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_UART5);       //GPIOC12����ΪUART5
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_UART5);        //GPIOD2����ΪUART5
	
  //UART5�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                    //GPIOC12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                  //���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	        //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //����
  GPIO_Init(GPIOC,&GPIO_InitStructure);                         //��ʼ��PC12
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                     //GPIOD2
  GPIO_Init(GPIOD,&GPIO_InitStructure);                         //��ʼ��PD2(����ͬ��)
  
  //UART5 ��ʼ������
  USART_InitStructure.USART_BaudRate = bound;                   //����������
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //�ֳ�Ϊ8λ���ݸ�ʽ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;        //һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;           //����żУ��λ
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
  USART_Init(UART5, &USART_InitStructure);                      //�����������ó�ʼ������5
	
  //Uart5 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;              //����5�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;       //��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);	                        //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

  USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);                 //��������5�����ж�
  USART_Cmd(UART5, ENABLE);                                     //ʹ�ܴ���5 	
}



void uart5_putc(unsigned char x)
{
  USART_SendData(UART5, x);
    /* Loop until the end of transmission */
  while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}

static int (*input_call)(unsigned char )  = NULL;
void uart5_set_input(int(*fun)(unsigned char))
{
  input_call = fun;
}

/*********************************************************************************************
* ���ƣ�UART5_IRQHandler
* ���ܣ������жϴ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void UART5_IRQHandler(void)                	           
{
  if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) {
    unsigned char x;
    x = USART_ReceiveData(UART5);
    if(input_call != NULL) input_call(x);
  }
}
