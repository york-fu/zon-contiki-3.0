/*********************************************************************************************
* 文件：sensor.c
* 作者：Xuzhy 2018.5.16
* 说明：xLab Sensor-B传感器程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "led.h"
#include "stepmotor.h"
#include "rgb.h"
#include "relay.h"
#include "FAN.h"
#include "beep.h"
#include "mode.h"

#include "zxbee.h"
#include "zxbee-inf.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* 全局变量
*********************************************************************************************/
static uint8_t  D0 = 0xFF;                                        // 默认打开主动上报功能
static uint8_t  D1 = 0;                                           // 继电器初始状态为全关
static uint16_t V0 = 30;                                          // V0设置为上报时间间隔，默认为30s
static uint8_t  mode = 0;                                         // 控制标识符
/*********************************************************************************************
* 名称：updateV0()
* 功能：更新V0的值
* 参数：*val -- 待更新的变量
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateV0(char *val)
{
  //将字符串变量val解析转换为整型变量赋值
  V0 = atoi(val);                                 // 获取数据上报时间更改值
}
/*********************************************************************************************
* 名称：sensorInit()
* 功能：传感器硬件初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorInit(void)
{
  if(mode_read() == 0){
    mode = 1;
    stepmotor_init(); 
    FAN_init();
  }else{
    mode = 2;
    rgb_init();
    Beep_init();
  }
  led_init();
  relay_init();
}
/*********************************************************************************************
* 名称：sensorUpdate()
* 功能：处理主动上报的数据
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  ZXBeeBegin();
  
  sprintf(p, "%u", D1);                                  // 上报控制编码 
  ZXBeeAdd("D1", p);
  
  p = ZXBeeEnd();                                               // 智云数据帧格式包尾
  if (p != NULL) {												
    ZXBeeInfSend(p, strlen(p));	                                // 将需要上传的数据进行打包操作，并通过zb_SendDataRequest()发送到协调器
  }
}
/*********************************************************************************************
* 名称：sensorCheck()
* 功能：传感器监测
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorCheck(void)
{

}
/*********************************************************************************************
* 名称：sensorControl()
* 功能：传感器控制
* 参数：cmd - 控制命令
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorControl(uint8_t cmd)
{
  static uint8_t stepmotor_flag = 0;
  if(mode == 1){                                                  // 传感器模式一跳线
    if ((cmd & 0x04) == 0x04) {                                   // 步进电机控制位：bit2
      if(stepmotor_flag != 1) {                                   // 步进电机正转
        stepmotor_flag = 1; 
        forward(5000);
      }
    } else {                                                      // 步进电机反转
      if(stepmotor_flag != 0) {
        stepmotor_flag = 0; 
        reversion(5000);
      }
    }
    if(cmd & 0x08){                                   // 根据cmd参数处理对应的控制程序
      FAN_on(0x01);                                             // 开启风扇
    }
    else{
      FAN_off(0x01);                                            // 关闭风扇       
    }
  }

  if(mode == 2){                                                  // 传感器模式二跳线
    if ((cmd & 0x01) == 0x01){                                    // RGB灯控制位：bit0~bit1
      if ((cmd & 0x02) == 0x02){                                  // cmd为3，亮蓝灯
        rgb_off(0x01);                                          
        rgb_off(0x02);                                         
        rgb_on(0x04);                                          
      }
      else{                                                       // cmd为1，亮红灯
        rgb_on(0x01);                                         
        rgb_off(0x02);                                          
        rgb_off(0x04);                                        
        
      }
    }
    else if ((cmd & 0x02) == 0x02){                               // cmd为2，亮绿灯
      rgb_off(0x01);                                           
      rgb_on(0x02);                                            
      rgb_off(0x04);                                           
    }
    else{                                                         // cmd为1，灯全灭
      rgb_off(0x01);                                           
      rgb_off(0x02);                                           
      rgb_off(0x04);                                           
    }
    if(cmd & 0x08){                                             // 蜂鸣器控制位：bit3
      Beep_on(0x01);                                            // 开启蜂鸣器
    }
    else{
      Beep_off(0x01);                                           // 关闭蜂鸣器       
    }
  }
  if(cmd & 0x20){                                                // LED2灯控制位：bit5
    led_on(0x02);                                                //开启LED2
  }
  else{
    led_off(0x02);                                               //关闭LED2        
  }
  if(cmd & 0x10){                                                // LED1灯控制位：bit4
    led_on(0x01);                                                //开启LED1
  }
  else{
    led_off(0x01);                                               //关闭LED1
  }
  
  relay_control(cmd>>6);
}
/*********************************************************************************************
* 名称：ZXBeeUserProcess()
* 功能：解析收到的控制命令
* 参数：*ptag -- 控制命令名称
*       *pval -- 控制命令参数
* 返回：ret -- p字符串长度
* 修改：
* 注释：
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  int ret = 0;	
  char pData[16];
  char *p = pData;
  
  // 将字符串变量pval解析转换为整型变量赋值
  val = atoi(pval);	
  // 控制命令解析
  if (0 == strcmp("CD0", ptag)){                                // 对D0的位进行操作，CD0表示位清零操作
    D0 &= ~val;
  }
  if (0 == strcmp("OD0", ptag)){                                // 对D0的位进行操作，OD0表示位置一操作
    D0 |= val;
  }
  if (0 == strcmp("D0", ptag)){                                 // 查询上报使能编码
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D0);
      ZXBeeAdd("D0", p);
    } 
  }
  if (0 == strcmp("CD1", ptag)){                                // 对D1的位进行操作，CD1表示位清零操作
    D1 &= ~val;
    sensorControl(D1);                                          // 处理执行命令
  }
  if (0 == strcmp("OD1", ptag)){                                // 对D1的位进行操作，OD1表示位置一操作
    D1 |= val;
    sensorControl(D1);                                          // 处理执行命令
  }
  if (0 == strcmp("D1", ptag)){                                 // 查询执行器命令编码
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D1);
      ZXBeeAdd("D1", p);
    } 
  }
  if (0 == strcmp("V0", ptag)){
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", V0);                         	// 上报时间间隔
      ZXBeeAdd("V0", p);
    }else{
      updateV0(pval);
    }
  } 
  return ret;
}
/*********************************************************************************************
* 名称：sensor()
* 功能：传感器采集线程
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
PROCESS_THREAD(sensor, ev, data)
{
  static struct etimer et_update;
  static struct etimer et_check;
  
  PROCESS_BEGIN();
  
  ZXBeeInfInit();
  
  sensorInit();
  
  etimer_set(&et_update, CLOCK_SECOND*V0); 
  etimer_set(&et_check, CLOCK_SECOND/10); //100 hz
  
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
    if (etimer_expired(&et_check)) {
      sensorCheck();
      etimer_set(&et_check, CLOCK_SECOND/10);
    }
    if (etimer_expired(&et_update)) {
      sensorUpdate();
      etimer_set(&et_update, CLOCK_SECOND*V0); 
    } 
  }
  PROCESS_END();
}