#include <stdio.h>
#include "stm32f4xx.h"

void uart3_init(unsigned int bpr)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  
    //����1��Ӧ���Ÿ���ӳ��
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3); //GPIOA9����ΪUSART1
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3); //GPIOA10����ΪUSART1
  
  /* Configure USARTx_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�10MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate = bpr;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART3, &USART_InitStructure);
  
    /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* Enable USARTy Receive and Transmit interrupts */
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  
   /* Enable the USARTy */
  USART_Cmd(USART3, ENABLE);
  
}

void uart3_putc(unsigned char x)
{
  USART_SendData(USART3, x);
    /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

static int (*input_call)(unsigned char )  = NULL;
void uart3_set_input(int(*fun)(unsigned char))
{
  input_call = fun;
}

void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
    unsigned char x;
    x = USART_ReceiveData(USART3);
    if(input_call != NULL) input_call(x);
  }
}