/*********************************************************************************************
* �ļ���stepmotor.c
* ���ߣ�zonesion 2016.4.13
* ˵���������������
*       ͨ��P0_0��P0_1�˿ڿ��Ƶ���Ĺ���
* �޸ģ�Chenkm 2017.01.10 �޸Ĵ����ʽ�����Ӵ���ע�ͺ��ļ�˵��
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stepmotor.h"
#include "delay.h"
#include "stm32f10x.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/

extern void delay_us(u32 nus);
/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static unsigned int dir = 0;

void stepmotor_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	        //ʹ��PA�˿�ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	        //ʹ��PB�˿�ʱ��
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(PIN_EN_port,PIN_EN);
}

/*********************************************************************************************
* ���ƣ�step(int dir,int steps)
* ���ܣ��������
* ������int dir,int steps
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void step(int dir,int steps)
{
  int i;
  if (dir) 
    GPIO_SetBits(PIN_DIR_port,PIN_DIR);											//���������������
  else 
    GPIO_ResetBits(PIN_DIR_port,PIN_DIR);	
  delay_us(5);						        //��ʱ5us
  for (i=0; i<steps; i++){				        //���������ת
   GPIO_ResetBits(PIN_STEP_port,PIN_STEP);
   delay_us(80);
   GPIO_SetBits(PIN_STEP_port,PIN_STEP);
   delay_us(80);
  }
}

/*********************************************************************************************
* ���ƣ�forward()
* ���ܣ������ת
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void forward(int data)
{
  dir = 0;
  GPIO_ResetBits(PIN_EN_port,PIN_EN);                           //���������������
  step(dir, data);                              	        //�����������
  GPIO_SetBits(PIN_EN_port,PIN_EN);
}

/*********************************************************************************************
* ���ƣ�reversion()
* ���ܣ������ת
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void reversion(int data)
{
  dir = 1;														//���������������
  GPIO_ResetBits(PIN_EN_port,PIN_EN); 
  step(dir, data);												//�����������

  GPIO_SetBits(PIN_EN_port,PIN_EN);

}