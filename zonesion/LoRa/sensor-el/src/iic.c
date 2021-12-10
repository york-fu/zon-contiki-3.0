/*********************************************************************************************
* 文件：iic.c
* 作者：zonesion
* 说明：iic驱动程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "iic.h"
#include <math.h>
#include <stdio.h>

/*********************************************************************************************
* 宏定义
*********************************************************************************************/

/*********************************************************************************************
* 名称：iic_delay_us()
* 功能：延时函数
* 参数：i -- 延时设置
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void  iic_delay_us(unsigned int i)
{
  while(i--)
  {
    asm("nop");
  }
}

/*********************************************************************************************
* 名称：iic_init()
* 功能：I2C初始化函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //拉高数据线
//  iic_delay_us(2);                                             //延时10us
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //拉高时钟线
//  iic_delay_us(2);                                             //延时10us
}

void sda_in(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void sda_out(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*********************************************************************************************
* 名称：iic_start()
* 功能：I2C起始信号
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic_start(void)
{
  sda_out(); 
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //拉高数据线
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //拉高时钟线
  iic_delay_us(2);                                              //延时
  GPIO_ResetBits(GPIOB,  GPIO_Pin_0);                                                      //产生下降沿
  iic_delay_us(2);                                              //延时
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                      //拉低时钟线
}

/*********************************************************************************************
* 名称：iic_stop()
* 功能：I2C停止信号
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void iic_stop(void)
{
  sda_out(); 
  GPIO_ResetBits(GPIOB,  GPIO_Pin_0);                                                       //拉低数据线
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                       //拉高时钟线
  iic_delay_us(2);                                              //延时5us
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                        //产生上升沿
  iic_delay_us(2);                                              //延时5us
}


/*********************************************************************************************
* 名称：iic_write_byte()
* 功能：I2C写一个字节数据，返回ACK或者NACK，从高到低，依次发送
* 参数：data -- 要写的数据
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
unsigned char iic_write_byte(unsigned char data)
{
  unsigned char i;
  sda_out();                                                //设置P0_4/P0_5为输出模式
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                      //拉低时钟线
  iic_delay_us(2);                                              //延时2us
  for(i = 0;i < 8;i++){                                         
    if(data & 0x80){                                            //判断数据最高位是否为1
      GPIO_SetBits(GPIOB,  GPIO_Pin_0);
      //iic_delay_us(2);                                            //延时5us
    }
    else{
      GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
      //iic_delay_us(2);                                            //延时5us
    }
   // iic_delay_us(2);                                            //延时5us
    GPIO_SetBits(GPIOB,  GPIO_Pin_1);	                                                //输出SDA稳定后，拉高SCL给出上升沿，从机检测到后进行数据采样
    iic_delay_us(2);                                            //延时5us
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);                                                    //拉低时钟线
    //iic_delay_us(2);                                            //延时5us
    data <<= 1;                                                 //数组左移一位
  } 
  //iic_delay_us(2);                                              //延时2us
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);                                                      //拉高数据线
  //iic_delay_us(2);                                              //延时2us
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                                                      //拉高时钟线
  sda_in();
  //iic_delay_us(2);                                              //延时2us，等待从机应答
  if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 1){			                                //SDA为高，收到NACK
    return 1;	
  }else{ 				                        //SDA为低，收到ACK
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
    //iic_delay_us(2);
    return 0;
  }
}

/*********************************************************************************************
* 名称：iic_read_byte()
* 功能：I2C写一个字节数据，返回ACK或者NACK，从高到低，依次发送
* 参数：data -- 要写的数据
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
unsigned char iic_read_byte(unsigned char ack)
{
  unsigned char i,data = 0;
  sda_out();                                                //设置P0_4/P0_5为输出模式
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
  GPIO_SetBits(GPIOB,  GPIO_Pin_0);			                                //释放总线	
  sda_in();
  for(i = 0;i < 8;i++){
    GPIO_ResetBits(GPIOB,  GPIO_Pin_1);		                                        //给出上升沿
//    iic_delay_us(2);	                                        //延时等待信号稳定
    GPIO_SetBits(GPIOB,  GPIO_Pin_1);		                                        //给出上升沿
//    iic_delay_us(2);	                                        //延时等待信号稳定
    data <<= 1;	
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 1){ 		                                //采样获取数据
      data |= 0x01;
    }else{
      data &= 0xfe;
    }
    iic_delay_us(2);
  }   
  sda_out();                                                //设置P0_4/P0_5为输出模式
  if(ack == 0)
    GPIO_ResetBits(GPIOB,  GPIO_Pin_0);
  else
    GPIO_SetBits(GPIOB,  GPIO_Pin_0);
//  iic_delay_us(2);
  GPIO_SetBits(GPIOB,  GPIO_Pin_1);                         
//  iic_delay_us(2);          
  GPIO_ResetBits(GPIOB,  GPIO_Pin_1);
//  iic_delay_us(2);
  return data;
}

/*********************************************************************************************
* 名称：delay()
* 功能：延时
* 参数：t -- 设置时间
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void delay(unsigned int t)
{
  unsigned char i;
  while(t--){
    for(i = 0;i < 200;i++);
  }					   
}
