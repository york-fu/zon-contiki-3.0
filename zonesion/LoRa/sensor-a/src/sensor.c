/*********************************************************************************************
* 文件：sensor.c
* 作者：Xuzhy 2018.5.16
* 说明：xLab Sensor-A传感器程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "stm32f10x.h"
#include "delay.h"
#include "htu21d.h"
#include "fbm320.h"
#include "BH1750.h"
#include "iic.h"
#include "lis3dh.h"
#include "MP-503.h"
#include "stadiometry.h"
#include "relay.h"
#include "ld3320.h"
#include "math.h"
#include "zxbee.h"
#include "zxbee-inf.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* 全局变量
*********************************************************************************************/
static uint8_t D0 = 0xFF;                                       // 默认打开主动上报功能
static uint8_t D1 = 0;                                          // 继电器初始状态为全关
static float A0 = 0.0;                                          // A0存储温度值
static float A1 = 0.0;                                          // A1存储湿度值
static float A2 = 0.0;                                          // A2存储光照值
static int A3 = 0;                                              // A3存储空气质量
static float A4 = 0.0;                                          // A4存储气压值
static int A5 = 0;                                              // A5存储跌倒信息
static float A6 = 0.0;                                          // A6存储距离值
static int A7 = 0;                                              // A7存储语音识别码
static uint16_t V0 = 30;                                        // V0设置为上报时间间隔，默认为30s
static uint8_t lis3dh_status = 0;                               // 三轴传感器状态
/*********************************************************************************************
* 名称：fallDect()
* 功能：跌倒检测处理算法
* 参数：x，y，z - 分别为三轴传感器的x，y，z值
* 返回：int - 跌倒状态
* 修改：
* 注释：
*********************************************************************************************/
int fallDect(float x, float y, float z)
{
  float x2 = x * x;
  float y2 = y * y;
  float z2 = z * z;
  float a;
  
  a = sqrt(x2 + y2 + z2);
  if (a<5) {
    return 1;
  }    
  return 0;
}
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
* 名称：updateA0()
* 功能：更新A0的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA0(void)
{
  // 读取温度值，并更新A0
  A0 = (htu21d_get_data(TEMP)/100.0f); 
}
/*********************************************************************************************
* 名称：updateA1()
* 功能：更新A1的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA1(void)
{
  // 读取湿度值，并更新A1
  A1 = (htu21d_get_data(HUMI)/100.0f);
}
/*********************************************************************************************
* 名称：updateA2()
* 功能：更新A2的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA2(void)
{
  // 读取光照值，并更新A2
  A2 = bh1750_get_data();
}
/*********************************************************************************************
* 名称：updateA3()
* 功能：更新A3的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA3(void)
{
  // 读取空气有害气体浓度值，并更新A3
  A3 = get_airgas_data(); 
}
/*********************************************************************************************
* 名称：updateA4()
* 功能：更新A4的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA4(void)
{
  // 读取大气压力值，并更新A4
  float temperature = 0;                                        // 存储温度数据变量
  long pressure = 0;                                            // 存储压力数据变量

  fbm320_data_get(&temperature,&pressure);                      // 获取温度、压力数据
  A4 = pressure/100.0f;                                         // 转换压力单位为百帕

}
/*********************************************************************************************
* 名称： updateA5()
* 功能：跌倒检测并上报状态
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA5(void)
{
  static uint32_t ct = 0;
  float accx,accy,accz;
  
  if (A5 == 0) {
    lis3dh_read_data(&accx,&accy,&accz);
    if(!(accx==0&&accy==0&&accz==0)) {
      A5 = fallDect(accx,accy,accz);
      ct = clock_time();
    } else {
      A5 = 0;
    }
  } else if (A5!=0 && clock_time()>(ct+2000)) {
    A5 = 0;
  }
}
/*********************************************************************************************
* 名称：updateA6()
* 功能：更新A6的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA6(void)
{
  // 读取红外测距值，并更新A6
  A6 = get_stadiometry_data();
}
/*********************************************************************************************
* 名称： updateA7()
* 功能：监测语音识别数据，更新A7的值
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA7(void)
{
  A7 = ld3320_read();                                       // 获取语音识别传感器返回的数据
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
  // 初始化传感器代码
  htu21d_init();                                                // 温湿度传感器初始化
  bh1750_init();                                                // 光强度传感器初始化
  airgas_init();                                                // 空气质量传感器初始化
  fbm320_init();                                                // 大气压力传感器初始化
  lis3dh_status = lis3dh_init();                                // 三轴传感器初始化
  stadiometry_init();                                           // 红外测距传感器初始化
  ld3320_init();                                                // 语音识别传感器初始化
  relay_init();                                                 // 继电器初始化
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
  
  ZXBeeBegin();                                                 // 智云数据帧格式包头
  
  // 根据D0的位状态判定需要主动上报的数值
  if ((D0 & 0x01) == 0x01){                                     // 若温度上报允许，则pData的数据包中添加温度数据
    updateA0();
    sprintf(p, "%.1f", A0); 
    ZXBeeAdd("A0", p);
  }
  if ((D0 & 0x02) == 0x02){                                     // 若湿度上报允许，则pData的数据包中添加湿度数据
    updateA1();
    sprintf(p, "%.1f", A1); 
    ZXBeeAdd("A1", p);
  }   
  if ((D0 & 0x04) == 0x04){                                     // 若光照上报允许，则pData的数据包中添加光照数据
    updateA2();
    sprintf(p, "%.1f", A2);  
    ZXBeeAdd("A2", p);
  }
  if ((D0 & 0x08) == 0x08){                                     // 若空气质量上报允许，则pData的数据包中添加空气质量数据
    updateA3();
    sprintf(p, "%u", A3);  
    ZXBeeAdd("A3", p);
  }
  if ((D0 & 0x10) == 0x10){                                     // 若大气压力上报允许，则pData的数据包中添加大气压力数据
    updateA4();
    sprintf(p, "%.1f", A4);
    ZXBeeAdd("A4", p);
  }
  if ((D0 & 0x10) == 0x10){                                     // 若跌倒状态上报允许，则pData的数据包中添加跌倒状态数据
    updateA5();
    sprintf(p, "%u", A5);
    ZXBeeAdd("A5", p);
  }
  if ((D0 & 0x40) == 0x40){                                     // 若红外测距上报允许，则pData的数据包中添加红外测距数据
    updateA6();
    sprintf(p, "%.1f", A6); 
    ZXBeeAdd("A6", p);
  }
  
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
  char pData[16];
  char *p = pData;
  static int lastA5 = 0;
 
  if(lis3dh_status == 0) updateA5();                            // 跌倒检测
  updateA7();                                                   // 语音识别检测
  
  ZXBeeBegin();
  
  if (A5 != lastA5) {                                           // A5具有保持2秒需要判断更新
    sprintf(p,"%u",A5);
    ZXBeeAdd("A5", p);
    lastA5 = A5;
  }

  if (A7 != 0){                                                 // 检测语音则上报A7
    sprintf(p,"%u",A7);
    ZXBeeAdd("A7", p);
  }

  p = ZXBeeEnd();
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
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
  // 根据cmd参数处理对应的控制程序
  relay_control(cmd>>6);
}
/*********************************************************************************************
* 名称：ZXBeeUserProcess()
* 功能：解析收到的控制命令
* 参数：*ptag -- 控制命令名称
*       *pval -- 控制命令参数
* 返回：<0:不支持指令，>=0 指令已处理
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
  if (0 == strcmp("A0", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA0();
      ret = sprintf(p, "%.1f", A0);     
      ZXBeeAdd("A0", p);
    } 
  }
  if (0 == strcmp("A1", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA1();
      ret = sprintf(p, "%.1f", A1);  
      ZXBeeAdd("A1", p);
    } 
  }  
  if (0 == strcmp("A2", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA2();
      ret = sprintf(p, "%.1f", A2);  
      ZXBeeAdd("A2", p);
    } 
  }
  if (0 == strcmp("A3", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA3();
      ret = sprintf(p, "%u", A3);    
      ZXBeeAdd("A3", p);
    } 
  }
  if (0 == strcmp("A4", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA4();
      ret = sprintf(p, "%.1f", A4); 
      ZXBeeAdd("A4", p);
    } 
  }  
  if (0 == strcmp("A6", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA6();
      ret = sprintf(p, "%.1f", A6);   
      ZXBeeAdd("A6", p);
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
