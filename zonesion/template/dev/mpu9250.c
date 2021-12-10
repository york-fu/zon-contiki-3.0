
#include "stm32f4xx.h"	
#include "iic3.h"
#include "stdio.h"

#define MPU9250_ADDR    0x68    //a0 -->0
#define AK8963_ADDR     0x0c
#define MPU6515_ADDR    0x68     //a0 -->0

#define ACCEL_ADDRESS   MPU6515_ADDR
#define MAG_ADDRESS     AK8963_ADDR


// 定义MPU9250内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)

#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40

#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42

#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48

		
#define MAG_XOUT_L		0x03
#define MAG_XOUT_H		0x04
#define MAG_YOUT_L		0x05
#define MAG_YOUT_H		0x06
#define MAG_ZOUT_L		0x07
#define MAG_ZOUT_H		0x08


#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I	        0x75	//IIC地址寄存器(默认数值0x68，只读)


int IIC3_Write(char addr, char r, char *buf, int len);


int Single_Write(char a, char r, char v)
{
  return IIC3_Write(a, r, &v, 1);
}
int Single_Read(char a, char r)
{
  char v;
  int ret;
  
  ret = IIC3_Read(a, r, &v, 1);
  if (ret == 1) {
    return v;
  }
  return -1;
}
/*
********************************************************************************
** 函数名称 ： mpu9250_init
** 函数功能 ： mpu9250 初始化
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
void mpu9250_init(void)
{
  char id;
  IIC3_Init();
  if (IIC3_Read(MPU9250_ADDR, 0x75, &id, 1) == 1) {
    printf("mpu9250 id %02x\r\n", id);
  } else {
    printf("mpu9250 error: can't find ic\r\n");
  }
  Single_Write(MPU9250_ADDR,PWR_MGMT_1, 0x00);	//解除休眠状态
  Single_Write(MPU9250_ADDR,SMPLRT_DIV, 0x07);
  Single_Write(MPU9250_ADDR,CONFIG, 0x05);  //10Hz
  Single_Write(MPU9250_ADDR,GYRO_CONFIG, 0x18);
  Single_Write(MPU9250_ADDR,ACCEL_CONFIG, 0x01); //0x01--2G //0x04 -- 4G
  
  //Single_Write(GYRO_ADDRESS,0x37,0x02);//turn on Bypass Mode 
}

int mpu9250_accel(float *x, float *y, float *z)
{ 
  char BUF[6];
  int16_t T_X, T_Y, T_Z;
    
   BUF[0]=Single_Read(MPU6515_ADDR,ACCEL_XOUT_L); 
   BUF[1]=Single_Read(MPU6515_ADDR,ACCEL_XOUT_H);
   T_X=	(BUF[1]<<8)|BUF[0];
  
						   //读取计算X轴数据
   *x = T_X/1640.0f;

   BUF[2]=Single_Read(MPU6515_ADDR,ACCEL_YOUT_L);
   BUF[3]=Single_Read(MPU6515_ADDR,ACCEL_YOUT_H);
   T_Y=	(BUF[3]<<8)|BUF[2];
   //T_Y/=164; 						   //读取计算Y轴数据
   *y = T_Y/1640.0f;
   
   BUF[4]=Single_Read(MPU6515_ADDR,ACCEL_ZOUT_L);
   BUF[5]=Single_Read(MPU6515_ADDR,ACCEL_ZOUT_H);
   T_Z=	(BUF[5]<<8)|BUF[4];
   //T_Z/=164; //23767/98/2				//读取计算Z轴数据
   *z = T_Z/1640.0f;
   
   //printf("acc %d, %d, %d \r\n", T_X, T_Y, T_Z);
   
   return 0;
}

int mpu9250_gyro(void)
{ 
    char BUF[6];
  u16 T_X, T_Y, T_Z;

   BUF[0]=Single_Read(MPU6515_ADDR,GYRO_XOUT_L); 
   BUF[1]=Single_Read(MPU6515_ADDR,GYRO_XOUT_H);
   T_X=	(BUF[1]<<8)|BUF[0];
   T_X/=16.4; 						   //读取计算X轴数据

   BUF[2]=Single_Read(MPU6515_ADDR,GYRO_YOUT_L);
   BUF[3]=Single_Read(MPU6515_ADDR,GYRO_YOUT_H);
   T_Y=	(BUF[3]<<8)|BUF[2];
   T_Y/=16.4; 						   //读取计算Y轴数据
   BUF[4]=Single_Read(MPU6515_ADDR,GYRO_ZOUT_L);
   BUF[5]=Single_Read(MPU6515_ADDR,GYRO_ZOUT_H);
   T_Z=	(BUF[5]<<8)|BUF[4];
   T_Z/=16.4; 					       //读取计算Z轴数据
   printf("gyro %u, %u, %u \r\n", T_X, T_Y, T_Z);
   return 0;
}

int mpu9250_mga(void)
{ 
      char BUF[6];
  u16 T_X, T_Y, T_Z;
   //Single_Write(GYRO_ADDRESS,0x37,0x02);//turn on Bypass Mode 
   //Delayms(10);	
   //Single_Write(AK8963_ADDR,0x0A,0x01);
   //Delayms(10);	
   BUF[0]=Single_Read (AK8963_ADDR,MAG_XOUT_L);
   BUF[1]=Single_Read (AK8963_ADDR,MAG_XOUT_H);
   T_X=(BUF[1]<<8)|BUF[0];

   BUF[2]=Single_Read(AK8963_ADDR,MAG_YOUT_L);
   BUF[3]=Single_Read(AK8963_ADDR,MAG_YOUT_H);
   T_Y=	(BUF[3]<<8)|BUF[2];
   						   //读取计算Y轴数据
	 
   BUF[4]=Single_Read(AK8963_ADDR,MAG_ZOUT_L);
   BUF[5]=Single_Read(AK8963_ADDR,MAG_ZOUT_H);
   T_Z=	(BUF[5]<<8)|BUF[4];
 					       //读取计算Z轴数据
    printf("mga %u, %u, %u \r\n", T_X, T_Y, T_Z);
    return 0;
}