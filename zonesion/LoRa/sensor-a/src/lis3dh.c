/*********************************************************************************************
* �ļ���lis3dh.c
* ���ߣ�zonesion
* ˵����lis3dh��������
* �޸ģ�Chenkm 2017.01.10 �޸Ĵ����ʽ�����Ӵ���ע�ͺ��ļ�˵��
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stm32f10x.h>
#include <math.h>
#include <stdio.h>
#include "lis3dh.h"
#include "iic.h"

/*********************************************************************************************
* ���ƣ�lis3dh_init()
* ���ܣ�lis3dh��ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char lis3dh_init(void)
{
  iic_init();		                                        //I2C��ʼ��
  delay(600);		                                        //����ʱ
  if(LIS3DH_ID != lis3dh_read_reg(LIS3DH_IDADDR))               //��ȡ�豸ID
    return 1;
  delay(600);                                                   //����ʱ
  if(lis3dh_write_reg(LIS3DH_CTRL_REG1,0x97))                   //1.25kHz��x,y,z���ʹ��
    return 1;
  delay(600);                                                   //����ʱ
  if(lis3dh_write_reg(LIS3DH_CTRL_REG4,0x10))                   //4G����
    return 1;
  return 0;
}

/*********************************************************************************************
* ���ƣ�lis3dh_read_reg()
* ���ܣ�htu21��ȡ�Ĵ���
* ������cmd -- �Ĵ�����ַ
* ���أ�data �Ĵ�������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char lis3dh_read_reg(unsigned char cmd)
{
  unsigned char data = 0; 				        //��������
  iic_start();						        //��������
  if(iic_write_byte(LIS3DHADDR & 0xfe) == 0){		        //��ַ����
    if(iic_write_byte(cmd) == 0){			        //��������
      do{
        delay(300);					        //��ʱ
        iic_start();					        //��������
      }
      while(iic_write_byte(LIS3DHADDR | 0x01) == 1);	        //�ȴ����ݴ������
      data = iic_read_byte(1);				        //��ȡ����
      iic_stop();					        //ֹͣ���ߴ���
    }
  }
  return data;						        //��������
}

/*********************************************************************************************
* ���ƣ�lis3dh_write_reg()
* ���ܣ�lis3dhд�Ĵ���
* ������cmd -- �Ĵ�����ַdata �Ĵ�������
* ���أ�0-�Ĵ���д��ɹ���1-дʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char lis3dh_write_reg(unsigned char cmd,unsigned char data)
{
  iic_start();						        //��������
  if(iic_write_byte(LIS3DHADDR & 0xfe) == 0){		        //��ַ����
    if(iic_write_byte(cmd) == 0){			        //��������
      if(iic_write_byte(data) == 0){			        //��������
        iic_stop();                                             //ֹͣ���ߴ���
        return 0;                                               //���ؽ��
      }
    }
  }
  iic_stop();
  return 1;						        //���ؽ��
}

/*********************************************************************************************
* ���ƣ�lis3dh_read_data()
* ���ܣ�lis3dh������
* ������accx��x����ٶ� accy��y����ٶ� accz��z����ٶ�
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void lis3dh_read_data(float *accx,float *accy,float *accz)
{
  char accxl,accxh,accyl,accyh,acczl,acczh;
  accxl = lis3dh_read_reg(LIS3DH_OUT_X_L);
  accxh = lis3dh_read_reg(LIS3DH_OUT_X_H);
  if(accxh & 0x80){
    *accx = (float)(((int)accxh << 4 | (int)accxl >> 4)-4096)/2048*9.8*4;
  }
  else{
    *accx = (float)((int)accxh << 4 | (int)accxl >> 4)/2048*9.8*4;
  }
  
  accyl = lis3dh_read_reg(LIS3DH_OUT_Y_L);
  accyh = lis3dh_read_reg(LIS3DH_OUT_Y_H);
  if(accyh & 0x80){
    *accy = (float)(((int)accyh << 4 | (int)accyl >> 4)-4096)/2048*9.8*4;
  }
  else{
    *accy = (float)((int)accyh << 4 | (int)accyl >> 4)/2048*9.8*4;
  }
   
  acczl = lis3dh_read_reg(LIS3DH_OUT_Z_L);
  acczh = lis3dh_read_reg(LIS3DH_OUT_Z_H);
  if(acczh & 0x80){
    *accz = (float)(((int)acczh << 4 | (int)acczl >> 4)-4096)/2048*9.8*4;
  }
  else{
    *accz = (float)((int)acczh << 4 | (int)acczl >> 4)/2048*9.8*4;
  }
}

