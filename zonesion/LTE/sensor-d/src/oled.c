/*********************************************************************************************
* �ļ���oled.c
* ���ߣ�zonesion
* ˵����
* �޸ģ�Chenkm 2017.01.05 ������ע��
* ע�ͣ�
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "iic.h"
#include "oled.h"
#include "oledfont.h"
#include <string.h>
 
#define ADDR_W  0X78                                            //����д��ַ
#define ADDR_R  0X79                                            //��������ַ
#define Max_Column	128
unsigned char OLED_GRAM[128][8];

/*********************************************************************************************
* ����: OLED_Init()
* ����: OLED��ʼ��
* ����: ��
* ����: ��
* ע�ͣ�
* �޸�:
*********************************************************************************************/
void  OLED_Init(void){
  iic_init();
  OLED_Write_command(0xAE);//--display off
	OLED_Write_command(0x00);//---set low column address
	OLED_Write_command(0x10);//---set high column address
	OLED_Write_command(0x40);//--set start line address  
	OLED_Write_command(0xB0);//--set page address
	OLED_Write_command(0x81); // contract control
	OLED_Write_command(0xFF);//--128   
	OLED_Write_command(0xA1);//set segment remap 
	OLED_Write_command(0xA6);//--normal / reverse
	OLED_Write_command(0xA8);//--set multiplex ratio(1 to 64)
	OLED_Write_command(0x3F);//--1/32 duty
	OLED_Write_command(0xC8);//Com scan direction
	OLED_Write_command(0xD3);//-set display offset
	OLED_Write_command(0x00);//
	
	OLED_Write_command(0xD5);//set osc division
	OLED_Write_command(0x80);//
	
	OLED_Write_command(0xD8);//set area color mode off
	OLED_Write_command(0x05);//
	
	OLED_Write_command(0xD9);//Set Pre-Charge Period
	OLED_Write_command(0xF1);//
	
	OLED_Write_command(0xDA);//set com pin configuartion
	OLED_Write_command(0x12);//
	
	OLED_Write_command(0xDB);//set Vcomh
	OLED_Write_command(0x30);//
	
	OLED_Write_command(0x8D);//set charge pump enable
	OLED_Write_command(0x14);//
	
	OLED_Write_command(0xAF);//--turn on oled panel
}

/*********************************************************************************************
* ����: OLED_Write_command()
* ����: IIC Write Command
* ����: ����
* ����: ��
* �޸�:
* ע��: 
*********************************************************************************************/
void OLED_Write_command(unsigned char IIC_Command)
{
  iic_start();						        //��������
  if(iic_write_byte(ADDR_W) == 0){		        //��ַ����
    if(iic_write_byte(0x00) == 0){			        //��������
      iic_write_byte(IIC_Command);	                                //�ȴ����ݴ������
    }
  }
  iic_stop();
}

/*********************************************************************************************
* ����: OLED_IIC_write()
* ����: IIC Write Data
* ����: ����
* ����: ��
* �޸�:
* ע��:
*********************************************************************************************/
void OLED_IIC_write(unsigned char IIC_Data)
{
  iic_start();						        //��������
  if(iic_write_byte(ADDR_W) == 0){		        //��ַ����
    if(iic_write_byte(0x40) == 0){			        //��������
      iic_write_byte(IIC_Data);	                                //�ȴ����ݴ������
    }
  }
  iic_stop();
}

/*********************************************************************************************
* ����: OLED_fillpicture()
* ����: OLED_fillpicture
* ����: ����
* ����: ��
* �޸�:
* ע��:
*********************************************************************************************/
void OLED_fillpicture(unsigned char fill_Data){
  unsigned char m,n;
  for(m=0;m<8;m++){
    OLED_Write_command(0xb0+m);		                        //page0-page1
    OLED_Write_command(0x00);		                        //low column start address
    OLED_Write_command(0x10);		                        //high column start address
    for(n=0;n<128;n++){
      OLED_IIC_write(fill_Data);
    }
  }
}

/*********************************************************************************************
* ����: OLED_Set_Pos()
* ����: ��������
* ����: ����
* ����: ��
* �޸�:
* ע��:
*********************************************************************************************/
void OLED_Set_Pos(unsigned char x, unsigned char y) { 	
  OLED_Write_command(0xb0+y);
  OLED_Write_command(((x&0xf0)>>4)|0x10);
  OLED_Write_command((x&0x0f)); 
} 

/*********************************************************************************************
* ����: OLED_Display_On()
* ����: ����OLED��ʾ
* ����: 
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/
void OLED_Display_On(void){
  OLED_Write_command(0X8D);                                     //SET DCDC����
  OLED_Write_command(0X14);                                     //DCDC ON
  OLED_Write_command(0XAF);                                     //DISPLAY ON
}

/*********************************************************************************************
* ����: OLED_Display_Off()
* ����: �ر�OLED��ʾ
* ����: 
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void OLED_Display_Off(void){
  OLED_Write_command(0X8D);                                     //SET DCDC����
  OLED_Write_command(0X10);                                     //DCDC OFF
  OLED_Write_command(0XAE);                                     //DISPLAY OFF
}

/*********************************************************************************************
* ����: OLED_Clear()
* ����: ��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
* ����: 
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void OLED_Clear(void)  {  
  unsigned char i,n;		    
  for(i=0;i<8;i++)  {  
    OLED_Write_command (0xb0+i);                                //����ҳ��ַ��0~7��
    OLED_Write_command (0x00);                                  //������ʾλ�á��е͵�ַ
    OLED_Write_command (0x10);                                  //������ʾλ�á��иߵ�ַ   
    for(n=0;n<128;n++)
      OLED_IIC_write(0); 
  } 
}

//�����Դ浽LCD		 
void OLED_Refresh_Gram(void)
{
  unsigned char i,n;		    
  for(i=0;i<8;i++)  
  {  
    OLED_Write_command (0xb0+i);                                //����ҳ��ַ��0~7��
    OLED_Write_command (0x00);                                  //������ʾλ�á��е͵�ַ
    OLED_Write_command (0x10);                                  //������ʾλ�á��иߵ�ַ   
    for(n=0;n<128;n++)OLED_IIC_write(OLED_GRAM[n][i]); 
  }   
}
//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���				   
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t)
{
  unsigned char pos,bx,temp=0;
  if(x>127||y>63)return;//������Χ��.
  pos=7-y/8;
  bx=y%8;
  temp=1<<(7-bx);
  if(t)OLED_GRAM[x][pos]|=temp;
  else OLED_GRAM[x][pos]&=~temp;	    
}
void OLED_DisFill(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char dot)  
{  
  unsigned char x,y;  
  for(x=x1;x<=x2;x++)
  {
    for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
  }													    
  //OLED_Refresh_Gram();//������ʾ
}
/*********************************************************************************************
* ����: OLED_DisClear()
* ����: �������
* ����: ����
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void OLED_DisClear(int hstart,int hend,int lstart,int lend){
  unsigned char i,n;
  for(i=hstart;i<=hend;i++)  {  
    OLED_Write_command (0xb0+i);                                //����ҳ��ַ��0~7��
    OLED_Write_command (0x00);                                  //������ʾλ�á��е͵�ַ
    OLED_Write_command (0x10);                                  //������ʾλ�á��иߵ�ַ   
    for(n=lstart;n<=lend;n++)
      OLED_IIC_write(0); 
  } 
}

/*********************************************************************************************
* ����: OLED_ShowChar()
* ����: ��ָ��λ����ʾһ���ַ�,���������ַ�
* ����: ���꣨x:0~127��y:0~63����chr�ַ���Char_Size�ַ�����
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char Char_Size){      	
  unsigned char c=0,i=0;	
  c=chr-' ';//�õ�ƫ�ƺ��ֵ			
  if(x>Max_Column-1){x=0;y=y+2;}
  if(Char_Size ==16){
    OLED_Set_Pos(x,y);	
    for(i=0;i<8;i++)
      OLED_IIC_write(F8X16[c*16+i]);
    OLED_Set_Pos(x,y+1);
    for(i=0;i<8;i++)
      OLED_IIC_write(F8X16[c*16+i+8]);
  }
  else if(Char_Size ==12){
    OLED_Set_Pos(x,y);	
    for(i=0;i<6;i++)
      OLED_IIC_write(F6X12[c*12+i]);
    OLED_Set_Pos(x,y+1);
    for(i=0;i<6;i++)
      OLED_IIC_write(F6X12[c*12+i+6]);
  }
  else {	
    OLED_Set_Pos(x,y);
    for(i=0;i<6;i++)
      OLED_IIC_write(F6x8[c][i]);
  }
}

/*********************************************************************************************
* ����: OLED_ShowString()
* ����: ��ʾһ���ַ���
* ����: ��ʼ���꣨x:0~127��y:0~63����chr�ַ���ָ�룻Char_Size�ַ�������
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr,unsigned char Char_Size){
  unsigned char j=0;
  while (chr[j]!='\0'){	
    OLED_ShowChar(x,y,chr[j],Char_Size);
    x+=8;
    if(x>120){
      x=0;
      y+=2;
    }
    j++;
  }
}

///*********************************************************************************************
//* ����: OLED_ShowCHinese()
//* ����: ��ʾһ������
//* ����: ��ʼ���꣨x:0~127��y:0~63����num�������Զ����ֿ��еı�ţ�oledfont.h�����
//* ����: 
//* �޸�:
//* ע��:
//*********************************************************************************************/ 
//void OLED_ShowCHinese(unsigned char x,unsigned char y,unsigned char num){      			    
//  unsigned char t,adder=0;
//  OLED_Set_Pos(x,y);	
//  for(t=0;t<16;t++){
//    OLED_IIC_write(Hzk[2*num][t]);
//    adder+=1;
//  }	
//  OLED_Set_Pos(x,y+1);	
//  for(t=0;t<16;t++){	
//    OLED_IIC_write(Hzk[2*num+1][t]);
//    adder+=1;
//  }					
//}

void OLED_Fill(void)  {  
  unsigned char i,n;		    
  for(i=0;i<8;i++)  {  
    OLED_Write_command (0xb0+i);                                //����ҳ��ַ��0~7��
    OLED_Write_command (0x00);                                  //������ʾλ�á��е͵�ַ
    OLED_Write_command (0x10);                                  //������ʾλ�á��иߵ�ַ   
    for(n=0;n<128;n++)
      OLED_IIC_write(0xff); 
  } 
}

/*********************************************************************************************
* ����: OLED_Display_On()
* ����: ����OLED��ʾ
* ����: 
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/
void oled_turn_on(void)
{
  char i=0;
  OLED_Clear();
  for(i = 0;i < 4;i++){
  OLED_DisFill(0,63-16*0-7,32*i+23,63-8*0,1);
  OLED_Refresh_Gram();
  }
  for(i = 0;i < 2;i++){
  OLED_DisFill(127-8*0-7,63-32*i-23,127-8*0,63,1);
  OLED_Refresh_Gram();
  }
  
  
  for(i = 0;i < 4;i++){
  OLED_DisFill(127-32*i-23,8*0,127,8*0+7,1);
  OLED_Refresh_Gram();
  }
  for(i = 0;i < 2;i++){
  OLED_DisFill(8*0,0,8*0+7,32*i+23,1);
  OLED_Refresh_Gram();
  }
  
//  for(i = 0;i < 4;i++){
//  OLED_DisFill(0,8*i,127,8*i+7,1);
//  OLED_DisFill(8*i,0,8*i+7,63,1);
//  OLED_DisFill(0,63-8*i-7,127,63-8*i,1);
//  OLED_DisFill(127-8*i-7,0,127-8*i,63,1);
//  OLED_Refresh_Gram();
//  }
}

/*********************************************************************************************
* ����: oled_turn_off()
* ����: �ر�OLED��ʾ
* ����: 
* ����: 
* �޸�:
* ע��:
*********************************************************************************************/  
void oled_turn_off(void)
{
  unsigned char i=0;
  OLED_Fill();
  memset(OLED_GRAM, 0xff, sizeof OLED_GRAM);
  for(i = 0;i < 4;i++){
  OLED_DisFill(0,8*i,127,8*i+7,0);
  OLED_DisFill(8*i,0,8*i+7,63,0);
  OLED_DisFill(0,63-8*i-7,127,63-8*i,0);
  OLED_DisFill(127-8*i-7,0,127-8*i,63,0);
  OLED_Refresh_Gram();
  }
  //OLED_Clear();
}
