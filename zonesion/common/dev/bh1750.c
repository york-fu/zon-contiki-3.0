/*********************************************************************************************
* �ļ�: bh1750.c
* ���ߣ�zonesion 2016.12.22
* ˵�������ع��ն���غ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"

/*********************************************************************************************
* �궨��
*********************************************************************************************/
//�������� 1:ʹ�����ģ��I2C
#define I2C_GPIO  GPIOE
#define PIN_SCL   GPIO_Pin_0
#define PIN_SDA   GPIO_Pin_1
#define SDA_IN     do{I2C_GPIO->MODER &= ~(3<<(1*2)); I2C_GPIO->MODER |= (0<<(1*2));}while(0)
#define SDA_OUT    do{I2C_GPIO->MODER &= ~(3<<(1*2)); I2C_GPIO->MODER |= (1<<(1*2));}while(0)
#define   SCL_L         (I2C_GPIO->BSRRH=PIN_SCL)
#define   SCL_H         (I2C_GPIO->BSRRL=PIN_SCL)
#define   SDA_L         (I2C_GPIO->BSRRH=PIN_SDA)
#define   SDA_H         (I2C_GPIO->BSRRL=PIN_SDA)
#define   SDA_R         (I2C_GPIO->IDR&PIN_SDA)

/*********************************************************************************************
* ����:BH1750GPIOInit()
* ����:��ʼ��BH1750
* ����:��
* ����:��
* �޸�:
* ע��:
*********************************************************************************************/
void BH1750GPIOInit(void)
{

  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin = PIN_SCL | PIN_SDA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);

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
  SDA_IN;
  SDA_H;
  delay_us(1);
  SCL_H;
  if (SDA_R) {
    I2C_Stop();
    return 1;
  }
  delay_us(1);
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

static int bh1750_i2c_write(char addr, char *buf, int len)
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
* ����:bh1750_i2c_read()
* ����:I2C��
* ����:char addr -- ��ַ, char *buf -- ��������, int len -- �������ݳ���
* ����:���ݳ���/-1
* �޸�:
* ע��:
*********************************************************************************************/
static int bh1750_i2c_read(char addr, unsigned char *buf, int len)
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

#define BH1750_ADDR 0x23

/*********************************************************************************************
* ����:bh1750_write_cmd()
* ����:��bh1750��������
* ����:char cmd -- bh1750����
* ����:
* �޸�:
* ע��:
*********************************************************************************************/
int  bh1750_write_cmd(char cmd)
{
  return bh1750_i2c_write(BH1750_ADDR, &cmd, 1);
}

/*********************************************************************************************
* ����:bh1750_init()
* ����:bh1750��ʼ��
* ����:��
* ����:0
* �޸�:
* ע��:
*********************************************************************************************/
int bh1750_init(void)
{
  BH1750GPIOInit();

  if (bh1750_write_cmd(0x01)<0)return -1; //power on
  delay_ms(10);
  if (bh1750_write_cmd(0x07)<0)return -1; //reset
  delay_ms(10);
  if (bh1750_write_cmd(0x10)<0)return -1; //H- resolution mode
  delay_ms(180); //180ms
  return 0;
}
float bh1750_get(void)
{
  unsigned char dat[4] = {0, 0};

  if (bh1750_i2c_read(BH1750_ADDR, dat, 2) == 2) {
    unsigned short x = dat[0]<<8 | dat[1];
    //printf("%02X %02X %02X", dat[0], dat[1], dat[2]);
    return x/1.2;
  }
  return -1;
}