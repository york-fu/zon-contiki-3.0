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
//#include "delay.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define I2C_GPIO                GPIOB
#define I2C_CLK                 RCC_APB2Periph_GPIOB
#define PIN_SCL                 GPIO_Pin_1 
#define PIN_SDA                 GPIO_Pin_0  

#define SDA_R                   GPIO_ReadInputDataBit(I2C_GPIO,PIN_SDA)
void clock_delay_us(unsigned int ut);	  
static void delay_us(int us)
{
  clock_delay_us(us*4);
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
void iic_start(void)
{
  sda_out();
  GPIO_SetBits(I2C_GPIO,PIN_SDA);                               //����������
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(1);                                                  //��ʱ
  GPIO_ResetBits(I2C_GPIO,PIN_SDA);                             //�����½���
  delay_us(1);                                                  //��ʱ
  GPIO_ResetBits(I2C_GPIO,PIN_SCL);                             //����ʱ����
  delay_us(1); 
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
  GPIO_ResetBits(I2C_GPIO,PIN_SDA);                             //����������
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(1);                                                  //��ʱ5us
  GPIO_SetBits(I2C_GPIO,PIN_SDA);                               //����������
  delay_us(1);                                                  //��ʱ5us
}

/*********************************************************************************************
* ���ƣ�iic_send_ack()
* ���ܣ�I2C����Ӧ��
* ������ack -- Ӧ���ź�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void iic_send_ack(int ack)
{
  sda_out();
  if(ack)
    GPIO_SetBits(I2C_GPIO,PIN_SDA);                             //дӦ���ź�
  else
    GPIO_ResetBits(I2C_GPIO,PIN_SCL); 
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(1);                                                  //��ʱ
  GPIO_ResetBits(I2C_GPIO,PIN_SCL);                             //����ʱ����
  delay_us(1);                                                  //��ʱ
}

/*********************************************************************************************
* ���ƣ�iic_recv_ack()
* ���ܣ�I2C����Ӧ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int iic_recv_ack(void)
{
  int CY = 0;
  sda_in();
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(1);                                                  //��ʱ
  CY = SDA_R;                                                   //��Ӧ���ź�
  GPIO_ResetBits(I2C_GPIO,PIN_SDA);                             //����ʱ����
  delay_us(1);                                                  //��ʱ
  return CY;
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
  sda_out();
  GPIO_ResetBits(I2C_GPIO,PIN_SCL);                             //����ʱ����
  for(i = 0;i < 8;i++){                                         
    if(data & 0x80){                                            //�ж��������λ�Ƿ�Ϊ1
      GPIO_SetBits(I2C_GPIO,PIN_SDA);  	                                                
    }
    else
      GPIO_ResetBits(I2C_GPIO,PIN_SDA); 
    //delay_us(1);                                                //��ʱ5us
    GPIO_SetBits(I2C_GPIO,PIN_SCL); 	                        //���SDA�ȶ�������SCL���������أ��ӻ���⵽��������ݲ���
    delay_us(1);                                                //��ʱ5us
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //����ʱ����
    delay_us(1);                                                //��ʱ5us
    data <<= 1;                                                 //��������һλ
  } 
  //delay_us(1);                                                  //��ʱ2us
  sda_in();
  GPIO_SetBits(I2C_GPIO,PIN_SDA);                               //����������
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                               //����ʱ����
  delay_us(1);                                                  //��ʱ2us���ȴ��ӻ�Ӧ��
  if(SDA_R){			                                //SDAΪ�ߣ��յ�NACK
   GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //�ͷ�����  
    delay_us(1);     
    return 1;	
  }else{ 				                        //SDAΪ�ͣ��յ�ACK
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);                           //�ͷ�����                  
    delay_us(1);                                                //��ʱ2us���ȴ��ӻ�Ӧ��
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
  sda_in();
  GPIO_ResetBits(I2C_GPIO,PIN_SCL); 
  GPIO_SetBits(I2C_GPIO,PIN_SDA);  			        //�ͷ�����	
  for(i = 0;i < 8;i++){
    GPIO_SetBits(I2C_GPIO,PIN_SCL);  		                //����������
    delay_us(1);	                                        //��ʱ�ȴ��ź��ȶ�
    data <<= 1;	
    if(SDA_R){ 		                                        //������ȡ����
      data |= 0x01;
    }else{
      data &= 0xfe;
    }
    //delay_us(1);
    GPIO_ResetBits(I2C_GPIO,PIN_SCL);    		        //�½��أ��ӻ�������һλֵ
    delay_us(1);
  }
  sda_out();
  if(ack)
    GPIO_SetBits(I2C_GPIO,PIN_SDA);	                        //Ӧ��״̬
  else
    GPIO_ResetBits(I2C_GPIO,PIN_SDA);  
  //delay_us(1);
  GPIO_SetBits(I2C_GPIO,PIN_SCL);                        
  delay_us(1);          
  GPIO_ResetBits(I2C_GPIO,PIN_SCL); 
  delay_us(1);
  return data;
}

int iic_read_buf(char addr, char r, char *buf, int len)
{
    int i;
    
    iic_start();
    if (iic_write_byte(addr<<1)) return -1;

    if (iic_write_byte(r)) return -1;
   
    iic_start();
    if (iic_write_byte((addr<<1)|1)) return -1;
   
    for (i=0; i<len-1; i++) {
      buf[i] = iic_read_byte(0);
    }
    buf[i++] = iic_read_byte(1);
    iic_stop();
    return i;
}
int iic_write_buf(char addr, char r, char *buf, int len)
{
  int i;
  
  iic_start();
  if (iic_write_byte(addr<<1)) return -1;
  //if (iic_recv_ack()) return -1;
  if (iic_write_byte(r)) return -1;
  //if (iic_recv_ack()) return -1;
  for (i=0; i<len; i++) {
    if (iic_write_byte(buf[i])) return -1;
    //if (iic_recv_ack()) return -1;
  }
  iic_stop();
  return i;
}
