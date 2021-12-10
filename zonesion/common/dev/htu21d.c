/*********************************************************************************************
* �ļ�: htu21d.c
* ���ߣ�zonesion 2016.12.22
* ˵����������ʪ����غ���  
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include <sys/clock.h>

//�������� 1:ʹ�����ģ��I2C  
#define I2C_GPIO  GPIOA
#define PIN_SCL   GPIO_Pin_1 
#define PIN_SDA   GPIO_Pin_0  

#define SDA_IN     do{I2C_GPIO->MODER &= ~(3<<(0*2)); I2C_GPIO->MODER |= (0<<(0*2));}while(0)
#define SDA_OUT    do{I2C_GPIO->MODER &= ~(3<<(0*2)); I2C_GPIO->MODER |= (1<<(0*2));}while(0)

#define   SCL_L         (I2C_GPIO->BSRRH=PIN_SCL)
#define   SCL_H         (I2C_GPIO->BSRRL=PIN_SCL)
#define   SDA_L         (I2C_GPIO->BSRRH=PIN_SDA)
#define   SDA_H         (I2C_GPIO->BSRRL=PIN_SDA)

#define   SDA_R         (I2C_GPIO->IDR&PIN_SDA)

void clock_delay_us(unsigned int ut);
void clock_delay_ms(unsigned int ut);  
#define delay_us(x) clock_delay_us(x)
#define delay_ms(x) clock_delay_ms(x)

/*********************************************************************************************
* ����:HTU21DGPIOInit()
* ����:��ʼ��HTU21D
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void HTU21DGPIOInit(void)
{

  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
    
  SDA_H;  
  SCL_H;    
}
/*********************************************************************************************
* ����:I2C_Start()
* ����:I2C��ʼ�ź�
* ����:��
* ����:0
* �޸�:
* ע��:
*********************************************************************************************/
static int I2C_Start(void)
{   
        SDA_OUT;
	SDA_H;
	SCL_H;
	delay_us(2);
	SDA_L;
	delay_us(2);
	SCL_L;
	return 0;
}

/*********************************************************************************************
* ����:I2C_Stop()
* ����:I2Cֹͣ�ź�
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
static void I2C_Stop(void)
{
        SDA_OUT;
        SCL_L;
	SDA_L;
	delay_us(2);
	SCL_H;
	SDA_H;
	//delay_us(2);
}

/*********************************************************************************************
* ����:I2C_Ack()
* ����:I2CӦ���ź�
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
static void I2C_Ack(void)
{	
        SCL_L;
        SDA_OUT;
	SDA_L;
	delay_us(2);
	SCL_H;
	delay_us(2);
	SCL_L;
}

/*********************************************************************************************
* ����:I2C_NoAck()
* ����:I2C��Ӧ���ź�
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
static void I2C_NoAck(void)
{	
        SCL_L;
        SDA_OUT;
	SDA_H;
	delay_us(2);
	SCL_H;
	delay_us(2);
	SCL_L;
}

/*********************************************************************************************
* ����:I2C_WaitAck()
* ����:I2C�ȴ�Ӧ���ź�
* ����:��
* ����:����Ϊ:=1��ACK,=0��ACK
* �޸�:
* ע��:
*********************************************************************************************/
static int I2C_WaitAck(void)
{
	//int r = 0;
       // int t = 0;
        SDA_IN;
	SDA_H;
        delay_us(2);   
	SCL_H;
        delay_us(2);
	if (SDA_R) {
            I2C_Stop();
            return 1;
        }
	SCL_L;
	return 0;
}

/*********************************************************************************************
* ����:I2C_SendByte()
* ����:I2C�����ֽ�����
* ����:char SendByte -- ��������
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
static void I2C_SendByte(char SendByte) //���ݴӸ�λ����λ//
{
    u8 i=8;
    SDA_OUT;
    while(i--)
    {
      
      if(SendByte&0x80) SDA_H;  
      else SDA_L;   
      SendByte<<=1;
      delay_us(1);
      SCL_H;
      delay_us(2);
      SCL_L;
      delay_us(1);
    }
}

/*********************************************************************************************
* ����:I2C_ReceiveByte()
* ����:I2C�����ֽ����ݣ����ݴӸ�λ����λ
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
static int I2C_ReceiveByte(void)  
{
  u8 i=8;
  u8 ReceiveByte=0;

    SDA_IN;
    SDA_H;
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      delay_us(2);
      SCL_H;
      delay_us(2);
      if(SDA_R)
      {
        ReceiveByte|=0x01;
      }
      //SCL_L;
      //delay_us(1);
    }
    SCL_L;
    return (ReceiveByte&0xff);
}

/*********************************************************************************************
* ����:bh1750_i2c_write()
* ����:I2Cд
* ����:char addr -- ��ַ, char *buf -- ��������, int len -- �������ݳ���
* ����:0/-1
* �޸�:
* ע��:
*********************************************************************************************/

static int i2c_write(char addr, char *buf, int len)
{
  if (I2C_Start() < 0) {
    I2C_Stop(); 
    return -1;
  }
  I2C_SendByte(addr<<1);
  if (I2C_WaitAck()){ 
    I2C_Stop(); 
    return -1;
  }
  for (int i=0; i<len; i++) {
    I2C_SendByte(buf[i]);
    if (I2C_WaitAck()) {
      I2C_Stop(); 
      return -1;
    }
  }
  I2C_Stop();
  return 0;
}

/*********************************************************************************************
* ����:i2c_read()
* ����:I2C��
* ����:char addr -- ��ַ, char *buf -- ��������, int len -- �������ݳ���
* ����:���ݳ���/-1
* �޸�:
* ע��:
*********************************************************************************************/
static int i2c_read(char addr, char *buf, int len)
{
  int i;
  
  if (I2C_Start() < 0) {
    I2C_Stop(); 
    return -1;
  }
  I2C_SendByte((addr<<1)|1);
  if (I2C_WaitAck()) {
    I2C_Stop();
    return -1;
  }

  for (i=0; i<len-1; i++) {
    buf[i] = I2C_ReceiveByte();
    I2C_Ack();
  }
  buf[i] = I2C_ReceiveByte();
  I2C_NoAck();
  I2C_Stop();
  return len;
}
#define HTU21D_ADDR 0x40

/*********************************************************************************************
* ����:htu21d_reset()
* ����:htu21d����
* ����:��
* ����:
* �޸�:
* ע��:
*********************************************************************************************/
void htu21d_reset(void)
{
  char cmd = 0xfe;
  i2c_write(HTU21D_ADDR, &cmd, 1); //reset
}
/*********************************************************************************************
* ����:htu21d_mesure_t()
* ����:���Ͷ�ȡ�¶�ָ��
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void htu21d_mesure_t(void)
{
   char cmd = 0xf3;
  i2c_write(HTU21D_ADDR, &cmd, 1);
}

/*********************************************************************************************
* ����:htu21d_mesure_h()
* ����:���Ͷ�ȡʪ������
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void htu21d_mesure_h(void)
{
   char cmd = 0xf5;
  i2c_write(HTU21D_ADDR, &cmd, 1);
}

/*********************************************************************************************
* ����:htu21d_init()
* ����:htu21d��ʼ��
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void htu21d_init(void)
{
  char cmd = 0xfe;
  HTU21DGPIOInit();
  i2c_write(HTU21D_ADDR, &cmd, 1); //reset
  delay_ms(20);
}

/*********************************************************************************************
* ����:htu21d_t()
* ����:��ȡ�¶�
* ����:��
* ����:-1/float t -- �������¶�ֵ
* �޸�:
* ע��:
*********************************************************************************************/
float htu21d_t(void)
{
  char cmd = 0xf3;
  char dat[4];
  i2c_write(HTU21D_ADDR, &cmd, 1);
  delay_ms(50);
  if (i2c_read(HTU21D_ADDR, dat, 2) == 2) {
    if ((dat[1]&0x02) == 0) {
      float t = -46.85f + 175.72f * ((dat[0]<<8 | dat[1])&0xfffc) / (1<<16);
      return t;
    }
  }
  return -1;
}

/*********************************************************************************************
* ����:htu21d_h()
* ����:��ȡʪ��
* ����:��
* ����:-1/float h -- ������ʪ��ֵ
* �޸�:
* ע��:
*********************************************************************************************/
float htu21d_h(void)
{
  char cmd = 0xf5;
  char dat[4];
  
  i2c_write(HTU21D_ADDR, &cmd, 1);
  delay_ms(50);
  if (i2c_read(HTU21D_ADDR, dat, 2) == 2) {
    if ((dat[1]&0x02) == 0x02) {
      float h = -6 + 125 * ((dat[0]<<8 | dat[1])&0xfffc) / (1<<16);
      return h;
    }
  }
  return -1;
}

float htu21d_read_t(void)
{
  char dat[4];
  if (i2c_read(HTU21D_ADDR, dat, 2) == 2) {
    if ((dat[1]&0x02) == 0) {
      float t = -46.85f + 175.72f * ((dat[0]<<8 | dat[1])&0xfffc) / (1<<16);
      return t;
    }
  }
  return -1;
}
float htu21d_read_h(void)
{
  char dat[4];
  if (i2c_read(HTU21D_ADDR, dat, 2) == 2) {
    if ((dat[1]&0x02) == 0x02) {
      float h = -6 + 125 * ((dat[0]<<8 | dat[1])&0xfffc) / (1<<16);
      return h;
    }
  }
  return -1;
}
