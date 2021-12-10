/*********************************************************************************************
* �ļ���zlg7290.c
* ���ߣ�zonesion
* ˵����zlg7290��������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/

#include <math.h>
#include <stdio.h>
#include "zlg7290.h"
#include "iic2.h"
#include "delay.h"



/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
//unsigned char key_flag = 0;
/*********************************************************************************************
* ���ƣ�zlg7290_init()
* ���ܣ�zlg7290��ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zlg7290_init(void)
{
  iic2_init();		                                        //I2C��ʼ��
  zlg7290_write_data(SCANNUM, 3); 
  GPIO_InitTypeDef   GPIO_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  
    /* ��ʼ��AFIOʱ�� */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  /* ���ⲿ�ж�2���ӵ� PA.2 pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);

  /* �����ⲿ�ж�0���� */
  EXTI_InitStructure.EXTI_Line = EXTI_Line7;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* �����ⲿ�ж�0Ϊ������ȼ� */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*********************************************************************************************
* ���ƣ�zlg7290_read_reg()
* ���ܣ�zlg7290��ȡ�Ĵ���
* ������cmd -- �Ĵ�����ַ
* ���أ�data �Ĵ�������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char zlg7290_read_reg(unsigned char cmd)
{
  unsigned char data = 0; 				        //��������
  
  
  iic2_read_buf(ZLG7290ADDR>>1, cmd, (char*)&data, 1);
  delay_ms(1);
  return data;						        //��������
}

/*********************************************************************************************
* ���ƣ�zlg7290_write_data()
* ���ܣ�zlg7290д�Ĵ���
* ������cmd -- �Ĵ�����ַ��data �Ĵ�������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zlg7290_write_data(unsigned char cmd, unsigned char data)
{
  
  iic2_write_buf(ZLG7290ADDR>>1, cmd, (char*)&data, 1);
  delay_ms(1);
}


/*********************************************************************************************
* ���ƣ�get_keyval()
* ���ܣ���ȡ������ֵ
* ������
* ���أ�������ֵ
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char zlg7290_get_keyval(void)
{
  unsigned char key_num = 0;
  key_num = zlg7290_read_reg(KEY_REG);

  if(key_num == 5)
    return UP;
  if(key_num == 13)
    return LEFT;
  if(key_num == 21)
    return DOWN;
  if(key_num == 29)
    return RIGHT;
  if(key_num == 37)
    return CENTER;
  return 0;
}


/*********************************************************************************************
* ���ƣ�zlg7290_download()
* ���ܣ��������ݲ�����
* ������addr    ��ȡֵ0��7����ʾ����DpRam0��DpRam7�ı��
*       dp      ���Ƿ������λ��С���㣬0��Ϩ��1������
*       flash   �����Ƹ�λ�Ƿ���˸��0������˸��1����˸, 2-������
*       dat     ��ȡֵ0��31����ʾҪ��ʾ������ 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zlg7290_download(unsigned char addr, unsigned dp, unsigned char flash, unsigned char dat)
{
  unsigned char cmd0;
  unsigned char cmd1;
  cmd0 = addr & 0x0F;
  cmd0 |= 0x60; 
  cmd1 = dat & 0x1F;
  if ( dp == 1 )
    cmd1 |= 0x80;
  if ( flash == 1 )
    cmd1 |= 0x40;
  //zlg7290_set_smd(cmd0,cmd1);
  char cmd[2];
  cmd[0] = cmd0;
  cmd[1] = cmd1;
  iic2_write_buf(ZLG7290ADDR>>1, CMDBUF0, cmd, 2);
  delay_ms(1);
}



/*********************************************************************************************
* ���ƣ�segment_display()
* ���ܣ��������ʾ����
* ������num -- ���ݣ����9999��
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void segment_display(unsigned int num)
{
  static unsigned char h = 0,j = 0,k = 0,l = 0;
  if(num > 9999)
    num = 0;
  h = num % 10;
  j = num % 100 /10;
  k = num % 1000 / 100;
  l = num /1000;
  zlg7290_download(0,0,0,l);
  zlg7290_download(1,0,0,k);
  zlg7290_download(2,0,0,j);
  zlg7290_download(3,0,0,h);
}



/*********************************************************************************************
* ���ƣ�displa_off()
* ���ܣ��������ʾ����
* ������num -- ���ݣ����9999��
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void display_off(void)
{
  zlg7290_download(0,0,0,31);  
  zlg7290_download(1,0,0,31);
  zlg7290_download(2,0,0,31);
  zlg7290_download(3,0,0,31);
}
