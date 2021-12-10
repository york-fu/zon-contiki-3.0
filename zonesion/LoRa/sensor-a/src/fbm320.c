#include "fbm320.h"
#include "iic.h"
#include "delay.h"

long UP_S=0, UT_S=0, RP_S=0, RT_S=0, OffP_S=0;
long UP_I=0, UT_I=0, RP_I=0, RT_I=0, OffP_I=0;
float H_S=0, H_I=0;
float Rpress;
unsigned int C0_S, C1_S, C2_S, C3_S, C6_S, C8_S, C9_S, C10_S, C11_S, C12_S; 
unsigned long C4_S, C5_S, C7_S;
unsigned int C0_I, C1_I, C2_I, C3_I, C6_I, C8_I, C9_I, C10_I, C11_I, C12_I; 
unsigned long C4_I, C5_I, C7_I;
unsigned char Formula_Select=1;

/*********************************************************************************************
* ���ƣ�fbm320_read_id()
* ���ܣ�FBM320_ID��ȡ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char fbm320_read_id(void)
{
  iic_start();                                                  //��������
  if(iic_write_byte(FBM320_ADDR) == 0){                         //������ߵ�ַ
    if(iic_write_byte(FBM320_ID_ADDR) == 0){                    //����ŵ�״̬
      do{
        delay(30);                                              //��ʱ30ָ��
        iic_start();	                                        //��������
      }
      while(iic_write_byte(FBM320_ADDR | 0x01) == 1);           //�ȴ�����ͨ�����
      unsigned char id = iic_read_byte(1);                      
      if(FBM320_ID == id){
        iic_stop();                                             //ֹͣ���ߴ���
        return 1;
      }
      
    }
  }
  iic_stop();                                                   //ֹͣ���ߴ���
  return 0;                                                     //��ַ���󷵻�0
}


/*********************************************************************************************
* ���ƣ�fbm320_read_reg()
* ���ܣ����ݶ�ȡ
* ������
* ���أ�data1 ����/0 ���󷵻�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char fbm320_read_reg(unsigned char reg)
{
  iic_start();                                                  //����IIC���ߴ���
  if(iic_write_byte(FBM320_ADDR) == 0){                         //������ߵ�ַ
    if(iic_write_byte(reg) == 0){                               //����ŵ�״̬
      do{
        delay(30);                                              //��ʱ30
        iic_start();                                            //����IIC���ߴ���	
      }
      while(iic_write_byte(FBM320_ADDR | 0x01) == 1);           //�ȴ�IIC���������ɹ�
      unsigned char data1 = iic_read_byte(1);                   //��ȡ����
      iic_stop();                                               //ֹͣIIC����
      return data1;                                             //��������
    }
  }
  iic_stop();                                                   //ֹͣIIC����
  return 0;                                                     //���ش���0
}

/*********************************************************************************************
* ���ƣ�fbm320_write_reg()
* ���ܣ�����ʶ����Ϣ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void fbm320_write_reg(unsigned char reg,unsigned char data)
{
  iic_start();                                                  //����IIC����
  if(iic_write_byte(FBM320_ADDR) == 0){                         //������ߵ�ַ
    if(iic_write_byte(reg) == 0){                               //����ŵ�״̬
      iic_write_byte(data);                                     //��������
    }
  }
  iic_stop();                                                   //ֹͣIIC����
}

/*********************************************************************************************
* ���ƣ�fbm320_read_data()
* ���ܣ����ݶ�ȡ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
long fbm320_read_data(void)
{
  unsigned char data[3];
  //char ret;
  iic_start();                                                  //��������
  iic_write_byte(FBM320_ADDR);                                  //���ߵ�ַ����
  
  iic_write_byte(FBM320_DATAM);                                 //��ȡ����ָ��
  //delay(30);
  iic_start();                                                  //��������
  iic_write_byte(FBM320_ADDR | 0x01);                           //��ȡ����
  data[2] = iic_read_byte(0);
  data[1] = iic_read_byte(0);
  data[0] = iic_read_byte(1);
  iic_stop();                                                   //ֹͣ���ߴ���
  /*
  data[2] = fbm320_read_reg(FBM320_DATAM);
  data[1] = fbm320_read_reg(FBM320_DATAC);
  
  data[0] = fbm320_read_reg(FBM320_DATAL);
  */
  return (((long)data[2] << 16) | ((long)data[1] << 8) | data[0]);
}


/*********************************************************************************************
* ���ƣ�Coefficient()
* ���ܣ�����ѹ��ϵ������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

void Coefficient(void)                                                                                                        //Receive Calibrate Coefficient
{
  unsigned char i;
  unsigned int R[10];
  unsigned int C0=0, C1=0, C2=0, C3=0, C6=0, C8=0, C9=0, C10=0, C11=0, C12=0; 
  unsigned long C4=0, C5=0, C7=0;
  
  for(i=0; i<9; i++)
    R[i]=(unsigned int)((unsigned int)fbm320_read_reg(0xAA + (i*2))<<8) | fbm320_read_reg(0xAB + (i*2));
  R[9]=(unsigned int)((unsigned int)fbm320_read_reg(0xA4)<<8) | fbm320_read_reg(0xF1);
  
  if(((Formula_Select & 0xF0) == 0x10) || ((Formula_Select & 0x0F) == 0x01))
  {
    C0 = R[0] >> 4;
    C1 = ((R[1] & 0xFF00) >> 5) | (R[2] & 7);
    C2 = ((R[1] & 0xFF) << 1) | (R[4] & 1);
    C3 = R[2] >> 3;
    C4 = ((unsigned long)R[3] << 2) | (R[0] & 3);
    C5 = R[4] >> 1;
    C6 = R[5] >> 3;
    C7 = ((unsigned long)R[6] << 3) | (R[5] & 7);
    C8 = R[7] >> 3;
    C9 = R[8] >> 2;
    C10 = ((R[9] & 0xFF00) >> 6) | (R[8] & 3);
    C11 = R[9] & 0xFF;
    C12 = ((R[0] & 0x0C) << 1) | (R[7] & 7);
  }
  else
  {
    C0 = R[0] >> 4;
    C1 = ((R[1] & 0xFF00) >> 5) | (R[2] & 7);
    C2 = ((R[1] & 0xFF) << 1) | (R[4] & 1);
    C3 = R[2] >> 3;
    C4 = ((unsigned long)R[3] << 1) | (R[5] & 1);
    C5 = R[4] >> 1;
    C6 = R[5] >> 3;
    C7 = ((unsigned long)R[6] << 2) | ((R[0] >> 2) & 3);
    C8 = R[7] >> 3;
    C9 = R[8] >> 2;
    C10 = ((R[9] & 0xFF00) >> 6) | (R[8] & 3);
    C11 = R[9] & 0xFF;
    C12 = ((R[5] & 6) << 2) | (R[7] & 7);
  }
  C0_I = C0;
  C1_I = C1;
  C2_I = C2;
  C3_I = C3;
  C4_I = C4;
  C5_I = C5;
  C6_I = C6;
  C7_I = C7;
  C8_I = C8;
  C9_I = C9;
  C10_I = C10;
  C11_I = C11;
  C12_I = C12;        
}
/*********************************************************************************************
* ���ƣ�Calculate()
* ���ܣ�����ѹ������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

void Calculate(long UP, long UT)                //Calculate Real Pressure & Temperautre
{
  signed char C12=0;
  int C0=0, C2=0, C3=0, C6=0, C8=0, C9=0, C10=0, C11=0; 
  //long C0=0, C2=0, C3=0, C6=0, C8=0, C9=0, C10=0, C11=0; 
  long C1=0, C4=0, C5=0, C7=0;
  long RP=0, RT=0;
  long DT, DT2, X01, X02, X03, X11, X12, X13, X21, X22, X23, X24, X25, X26, X31, X32, CF, PP1, PP2, PP3, PP4;
  C0 = C0_I;
  C1 = C1_I;
  C2 = C2_I;
  C3 = C3_I;
  C4 = C4_I;
  C5 = C5_I;
  C6 = C6_I;
  C7 = C7_I;
  C8 = C8_I;
  C9 = C9_I;
  C10 = C10_I;
  C11 = C11_I;
  C12 = C12_I;
  if(((Formula_Select & 0xF0) == 0x10) || ((Formula_Select & 0x0F) == 0x01))                        //For FBM320-02
  {
    DT        =        ((UT - 8388608) >> 4) + (C0 << 4);
    X01        =        (C1 + 4459) * DT >> 1;
    X02        =        ((((C2 - 256) * DT) >> 14) * DT) >> 4;
    X03        =        (((((C3 * DT) >> 18) * DT) >> 18) * DT);
    RT        =        (((long)2500 << 15) - X01 - X02 - X03) >> 15;
    
    DT2        =        (X01 + X02 + X03) >> 12;
    
    X11        =        ((C5 - 4443) * DT2);
    X12        =        (((C6 * DT2) >> 16) * DT2) >> 2;
    X13        =        ((X11 + X12) >> 10) + ((C4 + 120586) << 4);
    
    X21        =        ((C8 + 7180) * DT2) >> 10;
    X22        =        (((C9 * DT2) >> 17) * DT2) >> 12;
    if(X22 >= X21)
      X23        =        X22 - X21;
    else
      X23        =        X21 - X22;
    X24        =        (X23 >> 11) * (C7 + 166426);
    X25        =        ((X23 & 0x7FF) * (C7 + 166426)) >> 11;
    if((X22 - X21) < 0)
      X26        =        ((0 - X24 - X25) >> 11) + C7 + 166426;
    else        
      X26        =        ((X24 + X25) >> 11) + C7 + 166426;
    
    PP1        =        ((UP - 8388608) - X13) >> 3;
    PP2        =        (X26 >> 11) * PP1;
    PP3        =        ((X26 & 0x7FF) * PP1) >> 11;
    PP4        =        (PP2 + PP3) >> 10;
    
    CF        =        (2097152 + C12 * DT2) >> 3;
    X31        =        (((CF * C10) >> 17) * PP4) >> 2;
    X32        =        (((((CF * C11) >> 15) * PP4) >> 18) * PP4);
    RP        =        ((X31 + X32) >> 15) + PP4 + 99880;
  }
  else                                                                                                                                                                                                                                                                                                                //For FBM320
  {
    DT        =        ((UT - 8388608) >> 4) + (C0 << 4);
    X01        =        (C1 + 4418) * DT >> 1;
    X02        =        ((((C2 - 256) * DT) >> 14) * DT) >> 4;
    X03        =        (((((C3 * DT) >> 18) * DT) >> 18) * DT);
    RT = (((long)2500 << 15) - X01 - X02 - X03) >> 15;
    
    DT2        =        (X01 + X02 + X03) >>12;
    
    X11        =        (C5 * DT2);
    X12        =        (((C6 * DT2) >> 16) * DT2) >> 2;
    X13        =        ((X11 + X12) >> 10) + ((C4 + 211288) << 4);
    
    X21        =        ((C8 + 7209) * DT2) >> 10;
    X22        =        (((C9 * DT2) >> 17) * DT2) >> 12;
    if(X22 >= X21)
      X23        =        X22 - X21;
    else
      X23        =        X21 - X22;
    X24        =        (X23 >> 11) * (C7 + 285594);
    X25        =        ((X23 & 0x7FF) * (C7 + 285594)) >> 11;
    if((X22 - X21) < 0) 
      X26        =        ((0 - X24 - X25) >> 11) + C7 + 285594;
    else
      X26        =        ((X24 + X25) >> 11) + C7 + 285594;
    PP1        =        ((UP - 8388608) - X13) >> 3;
    PP2        =        (X26 >> 11) * PP1;
    PP3        =        ((X26 & 0x7FF) * PP1) >> 11;
    PP4        =        (PP2 + PP3) >> 10;
    
    CF        =        (2097152 + C12 * DT2) >> 3;
    X31        =        (((CF * C10) >> 17) * PP4) >> 2;
    X32        =        (((((CF * C11) >> 15) * PP4) >> 18) * PP4);
    RP = ((X31 + X32) >> 15) + PP4 + 99880;
  }
  
  RP_I = RP;
  RT_I = RT;
}
/*********************************************************************************************
* ���ƣ�Calculate()
* ���ܣ�����ѹ����������ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

unsigned char fbm320_init(void)
{
  iic_init();                                                   //IIC��ʼ��
  if(fbm320_read_id() == 0)                                     //�ж���ʼ���Ƿ�ɹ�
    return 0;
  return 1;
}
/*********************************************************************************************
* ���ƣ�fbm320_data_get()
* ���ܣ����������ݶ�ȡ����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void fbm320_data_get(float *temperature,long *pressure)
{
  Coefficient();                                                //ϵ������
  fbm320_write_reg(FBM320_CONFIG,TEMPERATURE);                  //����ʶ����Ϣ
   delay_ms(5);                                                 //��ʱ5ms
  UT_I = fbm320_read_data();                                    //��ȡ����������
  fbm320_write_reg(FBM320_CONFIG,OSR8192);                      //����ʶ����Ϣ
  delay_ms(10);                                                 //��ʱ10ms
  UP_I = fbm320_read_data();                                    //��ȡ����������
  Calculate( UP_I, UT_I);                                       //��������ֵ����
  *temperature = RT_I * 0.01f;                                  //�¶ȼ���
  *pressure = RP_I;                                             //ѹ������
}