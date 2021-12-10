#include "usart.h"	
#include "string.h"
#include "stm32f4xx.h"



/*********************************************************************************************
* ���ƣ�usart1_init
* ���ܣ�usart1��ʼ��
* ������bound������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart6_init(unsigned int bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);     //ʹ��GPIOCʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);    //ʹ��USART6ʱ��
 
  //����6��Ӧ���Ÿ���ӳ��
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6);  //GPIOC6����ΪUSART6
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); //GPIOAC7����ΪUSART6
	
  //USART6�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  //GPIOA9��GPIOA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             //���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   //�ٶ�50MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             //����
  GPIO_Init(GPIOC,&GPIO_InitStructure);                    //��ʼ��PC6��PC7

  //USART6 ��ʼ������
  USART_InitStructure.USART_BaudRate = bound;                      //����������
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;      //�ֳ�Ϊ8λ���ݸ�ʽ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;           //һ��ֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;              //����żУ��λ
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //�շ�ģʽ
  USART_Init(USART6, &USART_InitStructure);                        //�����������ó�ʼ������6
	
  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;               //����1�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;         //��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		  //�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		  //IRQͨ��ʹ��
  NVIC_Init(&NVIC_InitStructure);	                          //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

  USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);                  //��������6�����ж�
  USART_Cmd(USART6, ENABLE);                                      //ʹ�ܴ���6	
}


static int (*input_call)( char )  = NULL;
void uart6_set_input(int(*fun)( char))
{
  input_call = fun;
}
/*********************************************************************************************
* ���ƣ�USART6_IRQHandler
* ���ܣ������жϴ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void USART6_IRQHandler(void)                	           
{
  char c;
  if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET){  
    c = USART_ReceiveData(USART6);
    if (input_call != NULL) {
      input_call(c);
    }
  }
}


/*********************************************************************************************
* ���ƣ�usart6_send
* ���ܣ�����6��������
* ������s�����͵�����ָ�룬len�����͵����ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/ 
void uart6_putc( char x)
{
  USART_SendData(USART6, x);
  // Loop until the end of transmission
  while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
}


