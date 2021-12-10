//#include "mma7660.h"
//#include "hw_types.h"
#include "IIC.h"


//ģ��IIC���ų�ʼ��
void I2C_GPIO_Config(void)
{
  PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);      //ʹ��ʱ��
  PinTypeGPIO(PIN_58,PIN_MODE_0,false);                         //ѡ������ΪGPIOģʽ��gpio14��
  GPIODirModeSet(GPIOA0_BASE, G03_UCPINS, GPIO_DIR_MODE_OUT);   //����GPIO14Ϊ���ģʽ

  PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);      //ʹ��ʱ��
  PinTypeGPIO(PIN_08,PIN_MODE_0,false);                         //ѡ������ΪGPIOģʽ��gpio17��
  GPIODirModeSet(GPIOA2_BASE, G17_UCPINS, GPIO_DIR_MODE_OUT);   //����GPIO16Ϊ���ģʽ

  PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);      //ʹ��ʱ��
  PinTypeGPIO(PIN_07,PIN_MODE_0,false);                         //ѡ������ΪGPIOģʽ��gpio16��
  GPIODirModeSet(GPIOA2_BASE, G16_UCPINS, GPIO_DIR_MODE_OUT);   //����GPIO16Ϊ���ģʽ
  GPIOPinWrite(GPIOA2_BASE, G16_UCPINS, 0x0);                  //ʹ��
} 




void I2C_Start(void)
{       SDA_OUT();     //sda�����
	SDA(H);
        delay_us(1);
	SCL(H);
	delay_us(4);
	
	SDA(L);
	delay_us(4);
	SCL(L);
        delay_us(4);
}

void RepeatStart()                //without stop signal before
{
  SDA_OUT();
  SDA(H);
  delay_us(1);
  SCL(H);
  delay_us(4);                   //delay about 5us
  SDA(L);
  delay_us(4); 
  SCL(L);
  delay_us(4); 
} 


void I2C_Stop(void)
{
        SDA_OUT();
        SDA(L);
        delay_us(1);
	SCL(H);
	delay_us(4);
	SDA(H);
	delay_us(4);
        SCL(L);
        delay_us(4);
}


unsigned char ChkACK(void)        //����0��ʾ��ack��1��ʾ��ack
{ 
  SDA_OUT();
  SDA(H);
  delay_us(4);
  SCL(H);
  delay_us(4);
  SDA_IN();
  if(!SDA_read)               //���SDA�е͵�ƽ
  {
    SCL(L);
    delay_us(4);
    return 0;
  }else{
    SCL(L);
   delay_us(4);
    return 1;
  }
}

void SendAck(void)
{ 
  SDA_OUT();
  SDA(L);     
  delay_us(1);
  SCL(H);
  delay_us(4);
  SCL(L);
  delay_us(4);
}

void ReadSDA(void)
{
     
}

void SendNoAck(void)
{
  SDA_OUT();
  SCL(L);
  delay_us(4);
  SDA(H);
  delay_us(1);
  SCL(H);
  delay_us(4);
}

void Send8bit(unsigned char AData)//���Ͱ�λ�ֽڣ��ȷ��͸�λ���ڴ����λ
{
  unsigned char i=8;
  SDA_OUT();
  for(i=8; i>0;i--)                //��ϸ�о�
  {
    if(AData&0x80)
      SDA(H);
    else
     SDA(L);
    delay_us(1);
    SCL(H);
    delay_us(4);
    SCL(L);
    AData<<=1; 
    delay_us(4);                   //mannual
  }
}

unsigned char Read8bit(unsigned char ack)        //ack=1��ʾ��Ҫ��MMA����ֹͣλ��Ϊ0���ʾ����Ҫ��MMA����ֹͣλ
{
  unsigned char temp=0;
  unsigned char  i;
  unsigned char label;
  label=ack;
  SDA_OUT();
  SDA(H);
  delay_us(4);
  for(i=8;i>0;i--)
  {
     //ActiveTime();                 //�����������ͷ����ɾ��
    SDA_OUT();
    SCL(H);
    delay_us(4);
    SDA_IN();
    if(SDA_read)
      temp+=0x01;
    else
      temp&=0xfe;
    SCL(L);
    delay_us(4);
    if(i>1)
      temp<<=1;
  }
  if(label)
  { 
    SendAck();
  }
  else
  {
    SendNoAck();
  }                               //�ĳ����ģʽ
  return temp;
}


#define ERR      1
#define SUCCESS  0
unsigned char RegWrite(unsigned char address, unsigned char reg,unsigned char val)
{
  I2C_Start();                        //Send Start
  Send8bit(address);              //Send IIC "Write" Address
  if(ChkACK())
    return ERR;
  Send8bit(reg);                  // Send Register
  if(ChkACK())
   return ERR;
  Send8bit(val);                  // Send Value
  if(ChkACK())
   return ERR;
  I2C_Stop();                         // Send Stop
  return SUCCESS;
}

/***********************************************************
�ظ�������������������豸����豸�ڲ�ͬģʽ֮���л��ĳ���
����ǰ��һ�δ��������������stop�źţ���ת���������״̬
�����Ӳ����Ļ������ͷ����ߣ�һֱ����busy״̬
**************************************************************/

unsigned char RegRead(unsigned char address, unsigned char reg)             
{
  unsigned char  b=0;
  I2C_Start();                              
  Send8bit(address);    
  if(ChkACK())
   return ERR;
  Send8bit(reg);        
  if(ChkACK())
    return ERR;
  RepeatStart();                  //��MMA�����ȡ���ݱ���������ַ�ʽ               
  Send8bit(address+1);            //���·����豸��ַλ�����Ҹö�д����Ϊ��ȡ
  if(ChkACK())
    return ERR;
  b=Read8bit(0);                  //����Ӧ���ź�                         
  I2C_Stop();                            
    return b;
}

/*********************************************************************************************
* ���ƣ�MMA7660_GetResult()
* ���ܣ���ȡ������
* ������Regs_Addr
* ���أ�ret
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*uint8 MMA7660_GetResult(uint8 Regs_Addr) 
{
  uint8 ret ;  
  if(Regs_Addr>MMA7660_ZOUT)
    return 0;
   ret=RegRead(IIC_Read,Regs_Addr);
  while(ret&0x40) 
  {
    RegRead(IIC_Read,Regs_Addr);
  } 
  return ret;

}*/