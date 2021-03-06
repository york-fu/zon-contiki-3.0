/*********************************************************************************************
* 文件：
* 作者：
* 说明：
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 宏条件编译
*********************************************************************************************/
#ifndef __BEEP_H__
#define __BEEP_H__

/*********************************************************************************************
* 头文件
*********************************************************************************************/


/*********************************************************************************************
* 宏定义
*********************************************************************************************/
#define BEEP                      GPIO_Pin_2                    //宏定义风扇控制引脚PA3

#define BEEP_port                 GPIOA      

/*********************************************************************************************
* 函数声明
*********************************************************************************************/
extern void Beep_init(void);                                            //风扇控制引脚初始化函数
signed char Beep_on(unsigned char fan);                          //风扇打开控制函数
signed char Beep_off(unsigned char fan);                         //风扇关闭控制函数

#endif /*__FAN_H_*/

