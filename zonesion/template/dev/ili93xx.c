/*********************************************************************************************
* �ļ�: ili93xx.c
* ���ߣ�zonesion 2016.12.22
* ˵����LCD��ʾ��غ���  
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include "font.h"
#include "ili93xx.h"

static char CMD_WR_RAM = 0x22;
unsigned int LCD_ID = 0x9325;

/*********************************************************************************************
* ���ƣ�LCD_WR_REG
* ���ܣ�д�Ĵ�������
* ������regval:�Ĵ���ֵ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCD_WR_REG(vu16 regval)
{   
  regval=regval;		                                //ʹ��-O2�Ż���ʱ��,����������ʱ
  ILI93xx_REG=regval;                                           //д��Ҫд�ļĴ������	 
}

/*********************************************************************************************
* ���ƣ�LCD_WR_DATA
* ���ܣ�дLCD����
* ������data:Ҫд���ֵ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCD_WR_DATA(vu16 data)
{	  
  data=data;			                                //ʹ��-O2�Ż���ʱ��,����������ʱ
  ILI93xx_DAT=data;		 
}

/*********************************************************************************************
* ���ƣ�LCD_RD_DATA
* ���ܣ���LCD����
* ��������
* ���أ�������ֵ
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
u16 LCD_RD_DATA(void)
{
  vu16 ram;			                                //��ֹ���Ż�
  ram=ILI93xx_DAT;	
  return ram;	 
}

/*********************************************************************************************
* ���ƣ�ILI93xx_WriteReg
* ���ܣ���LCDָ���Ĵ���д������
* ������r:�Ĵ�����ַ; d:Ҫд���ֵ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ILI93xx_WriteReg(uint16_t r, uint16_t d)
{
  ILI93xx_REG = r;
  ILI93xx_DAT = d;
}

/*********************************************************************************************
* ���ƣ�ILI93xx_ReadReg
* ���ܣ���ȡ�Ĵ�����ֵ
* ������r:�Ĵ�����ַ
* ���أ�������ֵ
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint16_t ILI93xx_ReadReg(uint16_t r)
{
  uint16_t v;
  ILI93xx_REG = r;
  v = ILI93xx_DAT;
  return v;
}

/*********************************************************************************************
* ���ƣ�BLOnOff
* ���ܣ�������ر�LCD����
* ������st��1���������⣻0���رձ���
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void BLOnOff(int st)
{
#ifdef ZXBEE_PLUSE                                              //zxbeepluse
  if (st) {
    GPIO_SetBits(GPIOD, GPIO_Pin_2);                            //��������
  } else {
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
  }
#else
  if (st) {
    GPIO_SetBits(GPIOB, GPIO_Pin_15);
  } else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
  }
#endif
}

/*********************************************************************************************
* ���ƣ�BLInit
* ���ܣ�����IO�ڳ�ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void BLInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
#ifdef ZXBEE_PLUSE                                              //zxbeepluse
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
#else
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
}

/*********************************************************************************************
* ���ƣ�ILI93xxInit
* ���ܣ�LCD��ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ILI93xxInit(void)
{  
  BLOnOff(1);        //9341Ҫ�ȿ���������ܶ�ȡ�Ĵ���ֵ
  LCD_ID = ILI93xx_ReadReg(0);  
  if (LCD_ID == 0) {
    LCD_WR_REG(0Xd3);	
    int a=LCD_RD_DATA();	//dummy read 	
    int b=LCD_RD_DATA();	//����0X00
    int c=LCD_RD_DATA();   	//��ȡ93								  
    int d=LCD_RD_DATA();  	//��ȡ41 	   			       
    LCD_ID = (c << 8) | d;
  } 
  
  if (LCD_ID == 0x72) {		
    CMD_WR_RAM = 0x2C;
    //****����2.6��IPS 240*400 BOE+HX8352-C**///
    //Power Voltage Setting
    ILI93xx_WriteReg(0x001A, 0x0002);                           //BT
    ILI93xx_WriteReg(0x001B, 0x0088);                           //VRH
    
    //****VCOM offset**///
    ILI93xx_WriteReg(0x0023, 0x0000);                           //SEL_VCM
    ILI93xx_WriteReg(0x0024, 0x005F);                           //VCM
    ILI93xx_WriteReg(0x0025, 0x0015);                           //VDV
    ILI93xx_WriteReg(0x002D, 0x0003);                           //NOW[2:0]=011
    
    //Power on Setting
    ILI93xx_WriteReg(0x0018, 0x0004);                           //Frame rate 72Hz
    ILI93xx_WriteReg(0x0019, 0x0001);                           //OSC_EN='1', start Osc
    ILI93xx_WriteReg(0x0001, 0x0000);                           //DP_STB='0', out deep sleep
    
    //262k/65k color selection
    ILI93xx_WriteReg(0x0017, 0x0005);                           //default 0x06 262k color // 0x05 65k color
    //SET PANEL
    ILI93xx_WriteReg(0x0036, 0x0013);                           //REV_P, SM_P, GS_P, BGR_P, SS_P
    
    //Gamma 2.2 Setting
    ILI93xx_WriteReg(0x0040,0x0000);
    ILI93xx_WriteReg(0x0041,0x0045);
    ILI93xx_WriteReg(0x0042,0x0045);
    ILI93xx_WriteReg(0x0043,0x0004);
    ILI93xx_WriteReg(0x0044,0x0000);
    ILI93xx_WriteReg(0x0045,0x0008);
    ILI93xx_WriteReg(0x0046,0x0023);
    ILI93xx_WriteReg(0x0047,0x0023);
    ILI93xx_WriteReg(0x0048,0x0077);
    ILI93xx_WriteReg(0x0049,0x0040);
    
    ILI93xx_WriteReg(0x004A,0x0004);
    ILI93xx_WriteReg(0x004B,0x0000);
    ILI93xx_WriteReg(0x004C,0x0088);
    ILI93xx_WriteReg(0x004D,0x0088);
    ILI93xx_WriteReg(0x004E,0x0088);
    
    ILI93xx_WriteReg(0x001F,0x00D0);                            //VCOMG=1
    
    //Display ON Setting
    ILI93xx_WriteReg(0x0028,0x003F);                            //GON=1, DTE=1, D=11
    //
    ILI93xx_WriteReg(0x016,0x00);  //
    //////////////////////////////////////////////////////
    //Set GRAM Area
    unsigned int xStar = 0, xEnd=240, yStar = 0, yEnd=400;
    ILI93xx_WriteReg(0x02, xStar>>8);
    ILI93xx_WriteReg(0x03, xStar);
    ILI93xx_WriteReg(0x04, xEnd>>8);
    ILI93xx_WriteReg(0x05, xEnd);
    
    ILI93xx_WriteReg(0x06, yStar>>8);
    ILI93xx_WriteReg(0x07, yStar);
    ILI93xx_WriteReg(0x08, yEnd>>8);
    ILI93xx_WriteReg(0x09, yEnd);
    
    ILI93xx_REG = 0x22;
    for (int i=0;i<240; i++) {
      for (int j=0; j<400; j++) {
        ILI93xx_DAT = 0xf800;
      }
    }
  } 
  else if ((LCD_ID == 0x9325) || (LCD_ID == 0x9328))            //ILI9328   OK  
  {
    CMD_WR_RAM = 0x22;
    char sm = 0;
    char ss = 1;
    char gs = 1;
#if SCREEN_ORIENTATION_LANDSCAPE
    gs = 0;
#endif    
    ILI93xx_WriteReg(0x00EC,0x108F);                            // internal timeing      
    ILI93xx_WriteReg(0x00EF,0x1234);                            // ADD        
    ILI93xx_WriteReg(0x0001,0x0000|(ss<<8) | (sm<<10));     
    ILI93xx_WriteReg(0x0002,0x0700);                            //��Դ����                    
    //DRIVE TABLE(�Ĵ��� 03H)
    //BIT3=AM BIT4:5=ID0:1
    //AM      ID0     ID1               FUNCATION
    // 0      0       0	                R->L    D->U
    // 1      0       0	                D->U	R->L
    // 0      1       0	                L->R    D->U
    // 1      1       0                 D->U	L->R
    // 0      0       1	                R->L    U->D
    // 1      0       1                 U->D	R->L
    // 0      1       1                 L->R    U->D �����������.
    // 1      1       1	                U->D	L->R
#if SCREEN_ORIENTATION_LANDSCAPE
    ILI93xx_WriteReg(0x0003,(1<<12)|(0<<5)|(1<<4)|(1<<3));      //65K  
#else
    ILI93xx_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3));      //65K    
#endif
    
    ILI93xx_WriteReg(0x0004,0x0000);                                   
    ILI93xx_WriteReg(0x0008,0x0202);	           
    ILI93xx_WriteReg(0x0009,0x0000);         
    ILI93xx_WriteReg(0x000a,0x0000);                            //display setting         
    ILI93xx_WriteReg(0x000c,0x0001);                            //display setting          
    ILI93xx_WriteReg(0x000d,0x0000);                            //0f3c          
    ILI93xx_WriteReg(0x000f,0x0000);
    //��Դ����
    ILI93xx_WriteReg(0x0010,0x0000);   
    ILI93xx_WriteReg(0x0011,0x0007);
    ILI93xx_WriteReg(0x0012,0x0000);                                                                 
    ILI93xx_WriteReg(0x0013,0x0000);                 
    ILI93xx_WriteReg(0x0007,0x0001);                 
    delay_ms(50); 
    ILI93xx_WriteReg(0x0010,0x1490);   
    ILI93xx_WriteReg(0x0011,0x0227);
    delay_ms(50); 
    ILI93xx_WriteReg(0x0012,0x008A);                  
    delay_ms(50); 
    ILI93xx_WriteReg(0x0013,0x1a00);   
    ILI93xx_WriteReg(0x0029,0x0006);
    ILI93xx_WriteReg(0x002b,0x000d);
    delay_ms(50); 
    ILI93xx_WriteReg(0x0020,0x0000);                                                            
    ILI93xx_WriteReg(0x0021,0x0000);           
    delay_ms(50); 
    //٤��У��
    ILI93xx_WriteReg(0x0030,0x0000); 
    ILI93xx_WriteReg(0x0031,0x0604);   
    ILI93xx_WriteReg(0x0032,0x0305);
    ILI93xx_WriteReg(0x0035,0x0000);
    ILI93xx_WriteReg(0x0036,0x0C09); 
    ILI93xx_WriteReg(0x0037,0x0204);
    ILI93xx_WriteReg(0x0038,0x0301);        
    ILI93xx_WriteReg(0x0039,0x0707);     
    ILI93xx_WriteReg(0x003c,0x0000);
    ILI93xx_WriteReg(0x003d,0x0a0a);
    delay_ms(50); 
    ILI93xx_WriteReg(0x0050,0x0000);                            //ˮƽGRAM��ʼλ�� 
    ILI93xx_WriteReg(0x0051,0x00ef);                            //ˮƽGRAM��ֹλ��     
    
    ILI93xx_WriteReg(0x0052,0x0000);                            //��ֱGRAM��ʼλ��                    
    ILI93xx_WriteReg(0x0053,0x013f);                            //��ֱGRAM��ֹλ��  
    
    ILI93xx_WriteReg(0x0060,(gs<<15)|0x2700);                   //320 line   
    
    ILI93xx_WriteReg(0x0061,0x0001); 
    ILI93xx_WriteReg(0x006a,0x0000);
    ILI93xx_WriteReg(0x0080,0x0000);
    ILI93xx_WriteReg(0x0081,0x0000);
    ILI93xx_WriteReg(0x0082,0x0000);
    ILI93xx_WriteReg(0x0083,0x0000);
    ILI93xx_WriteReg(0x0084,0x0000);
    ILI93xx_WriteReg(0x0085,0x0000);
    
    ILI93xx_WriteReg(0x0090,0x0010);     
    ILI93xx_WriteReg(0x0092,0x0600);  
    //������ʾ����    
    ILI93xx_WriteReg(0x0007,0x0133); 
    
#if SCREEN_ORIENTATION_LANDSCAPE  
    LCDSetWindow(0, 320, 0, 240);                               //���ô���Ϊ������Ļ 
#else
    LCDSetWindow(0, 240, 0, 320);
#endif
  }
  else if (LCD_ID == 0x9341) {
    CMD_WR_RAM = 0x2C;
    LCD_WR_REG(0xCF);  
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0xC1); 
    LCD_WR_DATA(0X30); 
    LCD_WR_REG(0xED);  
    LCD_WR_DATA(0x64); 
    LCD_WR_DATA(0x03); 
    LCD_WR_DATA(0X12); 
    LCD_WR_DATA(0X81); 
    LCD_WR_REG(0xE8);  
    LCD_WR_DATA(0x85); 
    LCD_WR_DATA(0x10); 
    LCD_WR_DATA(0x7A); 
    LCD_WR_REG(0xCB);  
    LCD_WR_DATA(0x39); 
    LCD_WR_DATA(0x2C); 
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x34); 
    LCD_WR_DATA(0x02); 
    LCD_WR_REG(0xF7);  
    LCD_WR_DATA(0x20); 
    LCD_WR_REG(0xEA);  
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x00); 
    LCD_WR_REG(0xC0);                                           //Power control 
    LCD_WR_DATA(0x1B);                                          //VRH[5:0] 
    LCD_WR_REG(0xC1);                                           //Power control 
    LCD_WR_DATA(0x01);                                          //SAP[2:0];BT[3:0] 
    LCD_WR_REG(0xC5);                                           //VCM control 
    LCD_WR_DATA(0x30); 	                                        //3F
    LCD_WR_DATA(0x30); 	                                        //3C
    LCD_WR_REG(0xC7);                                           //VCM control2 
    LCD_WR_DATA(0XB7); 
    
    LCD_WR_REG(0x36);                                           // Memory Access Control 

#if SCREEN_ORIENTATION_LANDSCAPE  
    LCD_WR_DATA((1<<7)|(0<<6)|(1<<5)|SWAP_RGB);                 //���µ���,������
#else
    LCD_WR_DATA((0<<7)|(0<<6)|(0<<5)|SWAP_RGB);                 //������,���ϵ���
#endif        
    
    LCD_WR_REG(0x3A);   
    LCD_WR_DATA(0x55); 
    LCD_WR_REG(0xB1);   
    LCD_WR_DATA(0x00);   
    LCD_WR_DATA(0x1A); 
    LCD_WR_REG(0xB6);                                           // Display Function Control 
    LCD_WR_DATA(0x0A); 
    LCD_WR_DATA(0xA2); 
    LCD_WR_REG(0xF2);                                           // 3Gamma Function Disable 
    LCD_WR_DATA(0x00); 
    LCD_WR_REG(0x26);                                           //Gamma curve selected 
    LCD_WR_DATA(0x01); 
    LCD_WR_REG(0xE0);                                           //Set Gamma 
    LCD_WR_DATA(0x0F); 
    LCD_WR_DATA(0x2A); 
    LCD_WR_DATA(0x28); 
    LCD_WR_DATA(0x08); 
    LCD_WR_DATA(0x0E); 
    LCD_WR_DATA(0x08); 
    LCD_WR_DATA(0x54); 
    LCD_WR_DATA(0XA9); 
    LCD_WR_DATA(0x43); 
    LCD_WR_DATA(0x0A); 
    LCD_WR_DATA(0x0F); 
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x00); 		 
    LCD_WR_REG(0XE1);                                           //Set Gamma 
    LCD_WR_DATA(0x00); 
    LCD_WR_DATA(0x15); 
    LCD_WR_DATA(0x17); 
    LCD_WR_DATA(0x07); 
    LCD_WR_DATA(0x11); 
    LCD_WR_DATA(0x06); 
    LCD_WR_DATA(0x2B); 
    LCD_WR_DATA(0x56); 
    LCD_WR_DATA(0x3C); 
    LCD_WR_DATA(0x05); 
    LCD_WR_DATA(0x10); 
    LCD_WR_DATA(0x0F); 
    LCD_WR_DATA(0x3F); 
    LCD_WR_DATA(0x3F); 
    LCD_WR_DATA(0x0F);
    
    LCD_WR_REG(0x2B); 
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);                                          //LCD_WR_DATA(0x01);
    LCD_WR_DATA(0xef);                                          //LCD_WR_DATA(0x3f);
    LCD_WR_REG(0x2A); 
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x01);                                          //LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x3f);                                          //LCD_WR_DATA(0xef);	 
    
    LCD_WR_REG(0x11);                                           //Exit Sleep
    delay_ms(120);
    LCD_WR_REG(0x29);                                           //display on  
#if SCREEN_ORIENTATION_LANDSCAPE  
    LCDSetWindow(0, 320, 0, 240);                               //���ô���Ϊ������Ļ 
#else
    LCDSetWindow(0, 240, 0, 320);
#endif
  }
}

/*********************************************************************************************
* ���ƣ�LCDSetWindow
* ���ܣ����ô���
* ������x:������ʼ�����꣬xe�������յ�����꣬y:������ʼ�����꣬ye�������յ�������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDSetWindow(int x, int xe, int y, int ye)
{
  if (LCD_ID == 0x9341) {
    LCD_WR_REG(0X2A); 
    LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF); 
    LCD_WR_DATA(xe>>8);LCD_WR_DATA(xe&0XFF);
    LCD_WR_REG(0X2B); 
    LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF);
    LCD_WR_DATA(ye>>8);LCD_WR_DATA(ye&0XFF);
  }
  else{
#if SCREEN_ORIENTATION_LANDSCAPE
    ILI93xx_WriteReg(0x52, x);	                                //xstart        
    ILI93xx_WriteReg(0x53, xe);                                 //xend
    
    ILI93xx_WriteReg(0x50, y);	                                //ystart
    ILI93xx_WriteReg(0x51, ye);	 
#else
    ILI93xx_WriteReg(0x52, y);	                                //xstart        
    ILI93xx_WriteReg(0x53, ye);                                 //xend
    
    ILI93xx_WriteReg(0x50, x);	                                //ystart
    ILI93xx_WriteReg(0x51, xe);     
#endif
  }
}

/*********************************************************************************************
* ���ƣ�LCDSetCursor
* ���ܣ���������
* ������x,y������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDSetCursor(int x, int y)
{
  if (LCD_ID == 0x9341) {
    LCD_WR_REG(0X2A); 
    LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF); 			 
    LCD_WR_REG(0X2B); 
    LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF); 	
  } 
  if ((LCD_ID == 0x9325) || (LCD_ID == 0x9328)){
#if SCREEN_ORIENTATION_LANDSCAPE
    ILI93xx_WriteReg(0x21, x);
    ILI93xx_WriteReg(0x20, y);
#else
    ILI93xx_WriteReg(0x21, y);
    ILI93xx_WriteReg(0x20, x);   
#endif 
  }
}

/*********************************************************************************************
* ���ƣ�LCDWriteData
* ���ܣ�д�̶���������
* ������dat:���ݣ�len�����ݳ���
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDWriteData(uint16_t *dat, int len)
{
  ILI93xx_REG = CMD_WR_RAM;
  for (int i=0; i<len; i++) {
    ILI93xx_DAT = dat[i];
  }
}

/*********************************************************************************************
* ���ƣ�LCDClear
* ���ܣ�LCD����
* ������color:������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDClear(uint16_t color)
{
  LCDSetCursor(0,0);
  ILI93xx_REG = CMD_WR_RAM;
  for (int i=0; i<320*240; i++) {
    ILI93xx_DAT = color;
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawPixel
* ���ܣ�����
* ������x,y�����꣬color�������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawPixel(int x, int y, uint16_t color)
{
  LCDSetCursor(x, y);
  ILI93xx_REG = CMD_WR_RAM;
  ILI93xx_DAT = color;
}

/*********************************************************************************************
* ���ƣ�LCDrawLineH
* ���ܣ�������
* ������x0��ֱ����ʼ�����꣬x1��ֱ���յ�����꣬y0��ֱ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDrawLineH(int x0, int x1, int y0, int color)
{
  LCDSetCursor(x0, y0);
  ILI93xx_REG = CMD_WR_RAM;
  for (int i=x0; i<x1; i++) {
    ILI93xx_DAT = color;
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawAsciiDot12x24_1
* ���ܣ���ʾһ��ASCII��12*24��
* ������x��y����ʾ���꣬ch����ʾ�ַ���color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawAsciiDot12x24_1(int x, int y, char ch, int color, int bc)
{
  char dot;
  if (ch<0x20 || ch > 0x7e) ch = 0x20;
  ch -= 0x20;
  for (int i=0; i<3; i++) {
    for (int j=0; j<12; j++) {
      dot = nAsciiDot12x24[ch*36+i*12+j];
      for (int k=0; k<8; k++) {
        if (dot&1)LCDDrawPixel(x+j, y+(i*8)+k, color);
        else if (bc > 0) LCDDrawPixel(x+j, y+(i*8)+k, bc&0xffff);
        dot >>= 1;
      }
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawAsciiDot12x24
* ���ܣ���ʾ���ASCII��12*24��
* ������x��y����ʾ���꣬str����ʾ�ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawAsciiDot12x24(int x, int y, char *str, int color, int bc)
{
  
  unsigned char ch = *str;
  
  while (ch != 0) {
    LCDDrawAsciiDot12x24_1(x, y, ch, color, bc);
    x += 12;
    ch = *++str;
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawAsciiDot8x16_1
* ���ܣ���ʾһ��ASCII��8*16��
* ������x��y����ʾ���꣬ch����ʾ�ַ���color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawAsciiDot8x16_1(int x, int y, char ch, int color, int bc)
{
  int i, j;
  char dot;
  if (ch<0x20 || ch > 0x7e) {
    ch = 0x20;
  }
  ch -= 0x20;
  for (i=0; i<16; i++) {
    dot = nAsciiDot8x16[ch*16+i];
    for (j=0; j<8; j++) {
      if (dot&0x80)LCDDrawPixel(x+j, y+i, color);
      else if (bc > 0)LCDDrawPixel(x+j, y+i, bc&0xffff);; 
      dot <<= 1;
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawAsciiDot8x16
* ���ܣ���ʾ���ASCII��8*16��
* ������x��y����ʾ���꣬str����ʾ�ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawAsciiDot8x16(int x, int y, char *str, int color, int bc)
{
  
  unsigned char ch = *str;
  
#define CWIDTH  8
  while (ch != 0) {
    LCDDrawAsciiDot8x16_1(x, y, ch, color, bc);
    x += CWIDTH;
    
    ch = *++str;	
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawGB_16_1
* ���ܣ���ʾһ�����֣�16*16��
* ������x��y����ʾ���꣬gb2�������ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawGB_16_1(int x, int y, char *gb2, int color, int bc) 
{
  char dot;
  
  for (int i=0; i<GB_16_SIZE; i++) {
    if (gb2[0] == GB_16[i].Index[0] && gb2[1] == GB_16[i].Index[1]) {
      for (int j=0; j<16; j++) {
        for (int k=0; k<2; k++) {
          dot = GB_16[i].Msk[j*2+k];
          for (int m=0; m<8; m++) {
            if (dot & 1<<(7-m)) {
              LCDDrawPixel(x+k*8+m, y+j, color);
            } else  if (bc > 0) {
              LCDDrawPixel(x+k*8+m, y+j, bc);
            }
          }
        }
      }
      break;
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawGB_24_1
* ���ܣ���ʾһ�����֣�24*24��
* ������x��y����ʾ���꣬gb2�������ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawGB_24_1(int x, int y, char *gb2, int color, int bc) 
{
  char dot;
  
  for (int i=0; i<GB_24_SIZE; i++) {
    if (gb2[0] == GB_24[i].Index[0] && gb2[1] == GB_24[i].Index[1]) {
      for (int j=0; j<24; j++) {
        for (int k=0; k<3; k++) {
          dot = GB_24[i].Msk[j*3+k];
          for (int m=0; m<8; m++) {
            if (dot & 1<<(7-m)) {
              LCDDrawPixel(x+k*8+m, y+j, color);
            } else  if (bc > 0){
              LCDDrawPixel(x+k*8+m, y+j, bc);
            }
          }
        }
      }
      break;
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawGB_16
* ���ܣ���ʾ������֣�16*16��
* ������x��y����ʾ���꣬str�������ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawFnt16(int x, int y, char *str, int color, int bc)
{
  while (*str != '\0') {
    if (*str & 0x80) {
      if (str[1] != '\0') {
        LCDDrawGB_16_1(x, y, str, color, bc);
        str += 2;
        x+= 16;
      } else break;
    } else {
      LCDDrawAsciiDot8x16_1(x, y, *str, color, bc);
      str ++;
      x += 8;
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCDDrawFnt24
* ���ܣ���ʾ������֣�24*24��
* ������x��y����ʾ���꣬str�������ַ�����color���ַ���ɫ��bc������ɫ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCDDrawFnt24(int x, int y, char *str, int color, int bc)
{
  while (*str != '\0') {
    if (*str & 0x80) {
      if (str[1] != '\0') {
        LCDDrawGB_24_1(x, y, str, color, bc);
        str += 2;
        x+= 24;
      } else break;
    } else {
      LCDDrawAsciiDot12x24_1(x, y, *str, color, bc);
      str ++;
      x += 12;
    }
  }
}

/*********************************************************************************************
* ���ƣ�LCD_Clear
* ���ܣ���ָ����С����Ļ
* ������x1,y1:��ʼ����ֵ��x2,y2:�յ�����ֵ�� color����Ļ��ɫ
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void LCD_Clear(int x1,int y1,int x2,int y2,uint16_t color)
{
  for(int i=x1;i<=x2;i++)
    for(int j=y1;j<y2;j++){
#if SCREEN_ORIENTATION_LANDSCAPE
      ILI93xx_WriteReg(0x20, j);                                //Y��
      ILI93xx_WriteReg(0x21, i);                                //X��       //������ꡱ�ƶ�����i,j��
#else
      ILI93xx_WriteReg(0x20, i);                                //Y��
      ILI93xx_WriteReg(0x21, j);                                //X�� 
#endif
      ILI93xx_REG = 0x22;                                       //��ʾ����
      ILI93xx_DAT = color;
    }
}