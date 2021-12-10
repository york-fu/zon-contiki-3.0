/*********************************************************************************************
* �ļ���iic.c
* ���ߣ�zonesion
* ˵����iic��������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "iic.h"
#include <math.h>
#include <stdio.h>

/*********************************************************************************************
* �궨��
*********************************************************************************************/

/*********************************************************************************************
* ���ƣ�iic_delay_us()
* ���ܣ���ʱ����
* ������i -- ��ʱ����
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void  iic_delay_us(unsigned int i)
{
  while(i--)
  {
    asm("nop");
  }
}

/*********************************************************************************************
* ���ƣ�iic_init()
* ���ܣ�I2C��ʼ������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //����������
//  iic_delay_us(2);                                             //��ʱ10us
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //����ʱ����
//  iic_delay_us(2);                                             //��ʱ10us
}

void sda_in(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void sda_out(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*********************************************************************************************
* ���ƣ�iic_start()
* ���ܣ�I2C��ʼ�ź�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic_start(void)
{
  sda_out(); 
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //����������
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //����ʱ����
  iic_delay_us(2);                                              //��ʱ
  GPIO_ResetBits(GPIOB,  GPIO_Pin_0);                                                      //�����½���
  iic_delay_us(2);                                              //��ʱ
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                      //����ʱ����
}

/*********************************************************************************************
* ���ƣ�iic_stop()
* ���ܣ�I2Cֹͣ�ź�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic_stop(void)
{
  sda_out(); 
  GPIO_ResetBits(GPIOB,  GPIO_Pin_0);                                                       //����������
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                       //����ʱ����
  iic_delay_us(2);                                              //��ʱ5us
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                        //����������
  iic_delay_us(2);                                              //��ʱ5us
}


/*********************************************************************************************
* ���ƣ�iic_write_byte()
* ���ܣ�I2Cдһ���ֽ����ݣ�����ACK����NACK���Ӹߵ��ͣ����η���
* ������data -- Ҫд������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char iic_write_byte(unsigned char data)
{
  unsigned char i;
  sda_out();                                                //����P0_4/P0_5Ϊ���ģʽ
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                      //����ʱ����
  iic_delay_us(2);                                              //��ʱ2us
  for(i = 0;i < 8;i++){                                         
    if(data & 0x80){                                            //�ж��������λ�Ƿ�Ϊ1
      GPIO_SetBits(GPIOB,  GPIO_Pin_0);
      //iic_delay_us(2);                                            //��ʱ5us
    }
    else{
      GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
      //iic_delay_us(2);                                            //��ʱ5us
    }
   // iic_delay_us(2);                                            //��ʱ5us
    GPIO_SetBits(GPIOB,  GPIO_Pin_1);	                                                //���SDA�ȶ�������SCL���������أ��ӻ���⵽��������ݲ���
    iic_delay_us(2);                                            //��ʱ5us
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                    //����ʱ����
    //iic_delay_us(2);                                            //��ʱ5us
    data <<= 1;                                                 //��������һλ
  } 
  //iic_delay_us(2);                                              //��ʱ2us
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //����������
  //iic_delay_us(2);                                              //��ʱ2us
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //����ʱ����
  sda_in();
  //iic_delay_us(2);                                              //��ʱ2us���ȴ��ӻ�Ӧ��
  if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 1){			                                //SDAΪ�ߣ��յ�NACK
    return 1;	
  }else{ 				                        //SDAΪ�ͣ��յ�ACK
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
    //iic_delay_us(2);
    return 0;
  }
}

/*********************************************************************************************
* ���ƣ�iic_read_byte()
* ���ܣ�I2Cдһ���ֽ����ݣ�����ACK����NACK���Ӹߵ��ͣ����η���
* ������data -- Ҫд������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char iic_read_byte(unsigned char ack)
{
  unsigned char i,data = 0;
  sda_out();                                                //����P0_4/P0_5Ϊ���ģʽ
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);			                                //�ͷ�����	
  sda_in();
  for(i = 0;i < 8;i++){
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);		                                        //����������
//    iic_delay_us(2);	                                        //��ʱ�ȴ��ź��ȶ�
    GPIO_SetBits(GPIOB,  GPIO_Pin_1);		                                        //����������
//    iic_delay_us(2);	                                        //��ʱ�ȴ��ź��ȶ�
    data <<= 1;	
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 1){ 		                                //������ȡ����
      data |= 0x01;
    }else{
      data &= 0xfe;
    }
    iic_delay_us(2);
  }   
  sda_out();                                                //����P0_4/P0_5Ϊ���ģʽ
  if(ack == 0)
    GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
  else
    GPIO_SetBits(GPIOB,  GPIO_Pin_0);
//  iic_delay_us(2);
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                         
//  iic_delay_us(2);          
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
//  iic_delay_us(2);
  return data;
}

/*********************************************************************************************
* ���ƣ�delay()
* ���ܣ���ʱ
* ������t -- ����ʱ��
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void delay(unsigned int t)
{
  unsigned char i;
  while(t--){
    for(i = 0;i < 200;i++);
  }					   
}
