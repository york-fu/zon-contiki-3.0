#include "pcf8563.h"
#include <sys/clock.h>
#include "stdio.h"
void clock_delay_us(unsigned int ut);
#define RTC_I2C_delay(x) clock_delay_us(x)
//#define RTC_I2C_delay(x) do{int i,a=x; for(i=0;i<a; i++){for(int j=0;j<702;j++);}}while(0)

/*********************************************************************************************
* ���ƣ�ģ��IIC����io������
* ���ܣ���ʼ��IO(PF0��PF1,�������������)
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void RTC_I2C_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                 //���ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;              //����
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  RTC_I2C_SDA = 1;
  RTC_I2C_SCL = 1;
}


/***********************************************************
* ����: RTC_I2C_Start()
* ����: IIC��ʼ����
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

int RTC_I2C_Start(void)
{
  RTC_SDA_OUT();                                                //SDA���
  RTC_I2C_SDA = 1;
  RTC_I2C_SCL = 1;
  RTC_I2C_delay(5);
  RTC_I2C_SDA = 0;
  RTC_I2C_delay(5);
  RTC_I2C_SCL = 0;
  return 0;
}

/***********************************************************
* ����: Stop()
* ����: IICֹͣ����
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

void RTC_I2C_Stop(void)
{
  RTC_SDA_OUT();                                                //SDA���
  RTC_I2C_SCL = 0;
  RTC_I2C_SDA = 0;
  RTC_I2C_delay(5);
  RTC_I2C_SCL = 1;
   RTC_I2C_delay(5);
  RTC_I2C_SDA = 1;
  RTC_I2C_delay(5);
}

/***********************************************************
* ����: RTC_I2C_Ack()
* ����: ��������Ӧ���ź�
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

void RTC_I2C_Ack(void)
{
  RTC_SDA_OUT();
  RTC_I2C_SCL = 0;
  RTC_I2C_delay(2);
  RTC_I2C_SDA = 0;   
  RTC_I2C_delay(3);
  RTC_I2C_SCL = 1; 
  RTC_I2C_delay(5);
  RTC_I2C_SCL = 0;

}

/***********************************************************
* ����: RTC_I2C_NoAck()
* ����: ������ȷ���ź�
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

void RTC_I2C_NoAck(void)
{
  RTC_SDA_OUT();
  RTC_I2C_SCL = 0;
  RTC_I2C_delay(2);
  RTC_I2C_SDA = 1;
  RTC_I2C_delay(3);
  RTC_I2C_SCL = 1;
  RTC_I2C_delay(5);
  RTC_I2C_SCL = 0;
}
/***********************************************************
* ����: RTC_I2C_WaitAck()
* ����: �ȴ����豸����Ӧ���ź�
* ����: ��
* ����: 1Ϊ��ACK,0Ϊ��ACK
* �޸�:
* ע��: 
***********************************************************/

int RTC_I2C_WaitAck(void) 	
{
  unsigned char ucErrTime=0;
  
  RTC_I2C_SDA=1;
  RTC_SDA_IN();
  //RTC_I2C_SCL=0;
  RTC_I2C_delay(5);	   
  RTC_I2C_SCL=1;
  RTC_I2C_delay(2);	
  while(RTC_SDA_read)
   {
    ucErrTime++;
	if(ucErrTime>250){
	 RTC_I2C_Stop();
	 return 1;
        }
   }
  RTC_I2C_SCL=0;//����?����?3?0 	   
  return 0;  
} 

/***********************************************************
* ����: RTC_I2C_SendByte()
* ����: ���Ͱ�λ����
* ����: ���� SendByte ��Ҫ���͵��ֽ�
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

void RTC_I2C_SendByte(unsigned char SendByte)                   //���ݴӸ�λ����λ
{ 
    RTC_SDA_OUT(); 	    
    //RTC_I2C_SCL=0;
    for(unsigned char t=0;t<8;t++){              
        	
        RTC_I2C_delay(2);   
        RTC_I2C_SDA=(SendByte&0x80)>>7;
        SendByte<<=1; 	  
	RTC_I2C_delay(3);   
	RTC_I2C_SCL=1;
	RTC_I2C_delay(5); 
	RTC_I2C_SCL=0;
	//RTC_I2C_delay(2);
    }	 
}

/***********************************************************
* ����: RTC_I2C_ReceiveByte()
* ����: ��ȡ��λ����
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/

unsigned char RTC_I2C_ReceiveByte(void)                         //���ݴӸ�λ����λ//
{ 
unsigned char i,receive=0;
      RTC_I2C_SDA=1;
	RTC_SDA_IN();
    for(i=0;i<8;i++ )
	{
        
        RTC_I2C_delay(5);
	RTC_I2C_SCL=1;
        RTC_I2C_delay(3);         
        receive<<=1;
        if(RTC_SDA_read)receive++;   
        RTC_I2C_delay(2);
	 RTC_I2C_SCL=0;	
    }					 
    return receive;
}



/***********************************************************
* ����: PCF8563_write_byte()
* ����: ��8563ʱ��оƬд��һ���ֽ�
* ����: ��
* ����: ��
* �޸�:
* ע��: addrΪд�ļĴ�����ַ;dat�Ĵ���Ҫд���ֵ
***********************************************************/
void PCF8563_write_byte(unsigned char addr,unsigned char dat){
   RTC_I2C_Start();                                                 //����iic
   RTC_I2C_SendByte(0xA2);                                          //ѡ��PCF8563�����������ñ�д���׼��(PCF8563��д��ַΪ0xA2)
   RTC_I2C_WaitAck();           
   RTC_I2C_SendByte(addr);                                          //д��Ҫд�ļĴ�����ַ
   RTC_I2C_WaitAck();           
   RTC_I2C_SendByte(dat);                                           //д��Ҫд������
   RTC_I2C_WaitAck();           
   RTC_I2C_Stop();                                                  //ֹͣiic
}

/***********************************************************
* ����: PCF8563_init()
* ����: ��ʼ��ʱ��оƬ
* ����: ��
* ����: ��
* �޸�:
* ע��: 
***********************************************************/
void PCF8563_init(void){
 RTC_I2C_GPIO_Config();
 PCF8563_write_byte(0x00,0x00);                                 //0x00,0x01Ϊ����/״̬�Ĵ�����ַ
 PCF8563_write_byte(0x01,0x12);                                 //����,�����ж���Ч
 PCF8563_write_byte(0x09,0x80);                                 //�رշ��ӱ���//  
 PCF8563_write_byte(0x0A,0x80);                                 //�ر�Сʱ����
 PCF8563_write_byte(0x0B,0x80);                                 //�ر��ձ���
 PCF8563_write_byte(0x0C,0x80);                                 //�ر����ڱ���
 PCF8563_write_byte(0x0D,0x00);                                 //�ر�CLKOUT���
 PCF8563_write_byte(0x0E,0x03);                                 //�رն�ʱ��
}
/***********************************************************
* ����: PCF8563_read_byte()
* ����: ��PCF8563��ȡһ���ֽ�
* ����: ��
* ����: ����������
* �޸�:
* ע��: addrΪҪ���ĵ�ַ���������ض���������
***********************************************************/
unsigned char PCF8563_read_byte(unsigned char addr){
 unsigned char dat;
 RTC_I2C_Start();                                               //����iic
 RTC_I2C_SendByte(0xA2);                                        //ѡ��PCF8563�����������ñ�д���׼��(PCF8563��д��ַΪ0xA2)
 RTC_I2C_WaitAck();           
 RTC_I2C_SendByte(addr);                                        //д��Ҫ���ĵ�ַ
 RTC_I2C_WaitAck();           
 RTC_I2C_Start();                                               //���¿�ʼ��ͨѶ
 RTC_I2C_SendByte(0xA3);                                        //PCF8563�Ķ���ַΪ0xA3,׼����ʼ��ȡ
 RTC_I2C_WaitAck();           	
 dat=RTC_I2C_ReceiveByte();                                     //����������ֵ��������
 RTC_I2C_NoAck();                                               //��Ϊֻ��һ���ֽڣ����Բ��÷���Ӧ���ź�
 RTC_I2C_Stop();                                                //ֹͣiic
 return(dat);
}

/*********************************************************************************************
* ����: PCF8563_Read_NByte()
* ����: ��PCF8563��ȡN���ֽ�
* ����: 
* ����: 
* �޸�:
* ע��: 
*********************************************************************************************/
unsigned char * PCF8563_Read_NByte(unsigned char addr ,unsigned char len,unsigned char *buf){
 unsigned char i;
 RTC_I2C_Start();                
 RTC_I2C_SendByte(0xA2);       
 if (RTC_I2C_WaitAck()) {
   return NULL;
 }
 RTC_I2C_SendByte(addr);        
 if ( RTC_I2C_WaitAck()) {
   return NULL;
 }
 RTC_I2C_Start();                
 RTC_I2C_SendByte(0xA3); 
 if (RTC_I2C_WaitAck()) {
   return NULL;
 }
 for(i=0;i<len;i++){
   buf[i]=RTC_I2C_ReceiveByte();
   if(i<len-1)             
    RTC_I2C_Ack();
 }
 RTC_I2C_NoAck();          
 RTC_I2C_Stop();              
 return buf;
}

int PCF8563_gettime(char *buf)
{
  if (PCF8563_Read_NByte(0x02, 7, (unsigned char *)buf)) return 0;
  
  return -1;
}

/*********************************************************************************************
* ����: PCF8563_settime()
* ����: ����ʱ��
* ����: 
* ����: 
* �޸�:
* ע��: ʱ��ĸ�ʽ�����ʱ�������� BCD��
* �磺unsigned char Time[7]={0x55,0x59,0x23,0x28,0x01,0x02,0x03};
* PCF8563_settime(Time);
*********************************************************************************************/
void PCF8563_settime(u8*buf){
  for(unsigned char i=0;i<7;i++){
  PCF8563_write_byte(0x02+i,buf[i]);
  }
}

