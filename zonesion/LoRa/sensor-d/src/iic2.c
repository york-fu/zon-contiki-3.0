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
#include "stm32f10x.h"
#include "delay.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define I2C_GPIO                GPIOA
#define I2C_CLK                 RCC_APB2Periph_GPIOA
#define PIN_SCL                 GPIO_Pin_2 
#define PIN_SDA                 GPIO_Pin_3 

#define SDA_R                   GPIO_ReadInputDataBit(I2C_GPIO,PIN_SDA)
                 

/*********************************************************************************************
* ���ƣ�iic_init()
* ���ܣ�I2C��ʼ������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic2_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(I2C_CLK, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = PIN_SCL | PIN_SDA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);  
}

/*********************************************************************************************
* ���ƣ�sda_out()
* ���ܣ�����SDAΪ���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void sda_out(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PIN_SDA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);  
}

/*********************************************************************************************
* ���ƣ�sda_in()
* ���ܣ�����SDAΪ����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void sda_in(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PIN_SDA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);  
}

/*********************************************************************************************
* ���ƣ�iic_start()
* ���ܣ�I2C��ʼ�ź�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic2_start(void)
{

  GPIO_SetBits(I2C_GPIO,PIN_SDA);                               //����������
  delay_us(5); 
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(5);                                                  //��ʱ
  GPIO_ResetBits(I2C_GPIO,PIN_SDA);                             //�����½���
  delay_us(5);                                                  //��ʱ
  GPIO_ResetBits(I2C_GPIO,PIN_SCL);                             //����ʱ����
}

/*********************************************************************************************
* ���ƣ�iic_stop()
* ���ܣ�I2Cֹͣ�ź�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic2_stop(void)
{
  
  GPIO_ResetBits(I2C_GPIO,PIN_SDA);                             //����������
  delay_us(5); 
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(5);                                                  //��ʱ5us
  GPIO_SetBits(I2C_GPIO,PIN_SDA);                               //����������
}


/*********************************************************************************************
* ���ƣ�iic_write_byte()
* ���ܣ�I2Cдһ���ֽ����ݣ�����ACK����NACK���Ӹߵ��ͣ����η���
* ������data -- Ҫд������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char iic2_write_byte(unsigned char data)
{
  unsigned char i;
 
  GPIO_ResetBits(I2C_GPIO,PIN_SCL);                             //����ʱ����
  for(i = 0;i < 8;i++){                                         
    if(data & 0x80){                                            //�ж��������λ�Ƿ�Ϊ1
      GPIO_SetBits(I2C_GPIO,PIN_SDA);  	                                                
    }
    else
      GPIO_ResetBits(I2C_GPIO,PIN_SDA); 
    delay_us(5);                                                //��ʱ5us
    GPIO_SetBits(I2C_GPIO,PIN_SCL); 	                        //���SDA�ȶ�������SCL���������أ��ӻ���⵽��������ݲ���
    delay_us(5);                                                //��ʱ5us
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //����ʱ����
    
    data <<= 1;                                                 //��������һλ
  } 
  sda_in();
  delay_us(5);                                                  //��ʱ2us
 
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  if(SDA_R){			                                //SDAΪ�ߣ��յ�NACK
     GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //�ͷ����� 
    sda_out();
    return 1;	
  }else{ 				                        //SDAΪ�ͣ��յ�ACK
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //�ͷ�����                  
    sda_out();
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
unsigned char iic2_read_byte(unsigned char ack)
{
  unsigned char i,data = 0;
  
  GPIO_ResetBits(I2C_GPIO,PIN_SCL); 
  sda_in();
  for(i = 0;i < 8;i++){
    delay_us(5);
    GPIO_SetBits(I2C_GPIO,PIN_SCL);  		                //����������
    	                                        
    data <<= 1;	
    if(SDA_R){ 		                                        //������ȡ����
      data |= 0x01;
    }else{
      data &= 0xfe;
    }
    delay_us(5);
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);    		        //�½��أ��ӻ�������һλֵ
  }
  sda_out();
  if(ack)
    GPIO_SetBits(I2C_GPIO,PIN_SDA);	                        //Ӧ��״̬
  else
    GPIO_ResetBits(I2C_GPIO,PIN_SDA);  
  delay_us(5);
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                        
  delay_us(5);          
  GPIO_ResetBits(I2C_GPIO,PIN_SCL); 

  return data;
}

int iic2_read_buf(char addr, char r, char *buf, int len)
{
    int i;
    
    iic2_start();
    if (iic2_write_byte(addr<<1)) {
      iic2_stop();
      return -1;
    }
    if (iic2_write_byte(r)){
     iic2_stop();
      return -1;
    }
    iic2_start();
    if (iic2_write_byte((addr<<1)|1)) {
      iic2_stop();
      return -1;
    }
    for (i=0; i<len-1; i++) {
      buf[i] = iic2_read_byte(0);
    }
    buf[i++] = iic2_read_byte(1);
    iic2_stop();
    return i;
}
int iic2_write_buf(char addr, char r, char *buf, int len)
{
  int i;
  
  iic2_start();
  if (iic2_write_byte(addr<<1)) {
    iic2_stop();
    return -1;
  }

  if (iic2_write_byte(r)) {
    iic2_stop();
    return -1;
  }
 
  for (i=0; i<len; i++) {
    if (iic2_write_byte(buf[i])){
     iic2_stop();
     return -1;
    }
  }
  iic2_stop();
  return i;
}