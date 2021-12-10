/*********************************************************************************************
* �ļ���rgb.c
* ���ߣ�Meixin 2017.09.15
* ˵����rgb����������     
* �޸ģ�liutong 20171027 �޸���rgb�������ų�ʼ���������ơ��Ż���ע�͡�������rgb�򿪺͹رպ���
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "rgb.h"
#include "stm32f10x.h"
/*********************************************************************************************
* ���ƣ�rgb_init()
* ���ܣ�rgb�������ų�ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void rgb_init(void)
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
  
  rgb_off(0x01);                                                //��ʼ״̬Ϊ�ر�
  rgb_off(0x02);    
  rgb_off(0x04);    
}
  
/*********************************************************************************************
* ���ƣ�rgb_on()
* ���ܣ�rgb���ƴ򿪺���
* ������rgb�ţ���rgb.h�к궨��ΪRGB_R��RGB_G��RGB_B
* ���أ�0����rgb�ɹ���-1����������
* �޸ģ�
* ע�ͣ�����ֻ������RGB_R��RGB_G��RGB_B����᷵��-1
*********************************************************************************************/
signed char rgb_on(unsigned char rgb)
{
  if(rgb & 0x01){                                                //���Ҫ��RGB_R
    GPIO_ResetBits(RGB_R_port,RGB_R);
    return 0;
  }
    
  if(rgb & 0x02){                                                //���Ҫ��RGB_G
    GPIO_ResetBits(RGB_G_port,RGB_G);
    return 0;
  }
  
  if(rgb & 0x04){                                                //���Ҫ��RGB_B
    GPIO_ResetBits(RGB_B_port,RGB_B);
    return 0;
  }
   
  return -1;                                                    //�������󣬷���-1
}

/*********************************************************************************************
* ���ƣ�rgb_off()
* ���ܣ�rgb���ƹرպ���
* ������rgb�ţ���rgb.h�к궨��ΪRGB_R��RGB_G��RGB_B
* ���أ�0���ر�rgb�ɹ���-1����������
* �޸ģ�
* ע�ͣ�����ֻ������RGB_R��RGB_G��RGB_B����᷵��-1
*********************************************************************************************/
signed char rgb_off(unsigned char rgb)
{
  if(rgb & 0x01){                                                //���Ҫ�ر�RGB_R
    GPIO_SetBits(RGB_R_port,RGB_R);
    return 0;
  }
    
  if(rgb & 0x02){                                                //���Ҫ�ر�RGB_G
    GPIO_SetBits(RGB_G_port,RGB_G);
    return 0;
  }

  if(rgb & 0x04){                                                //���Ҫ�ر�RGB_B
    GPIO_SetBits(RGB_B_port,RGB_B);
    return 0;
  }
  
  return -1;                                                    //�������󣬷���-1
}