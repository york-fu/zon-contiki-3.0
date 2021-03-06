/*********************************************************************************************
* 文件：sensor.c
* 作者：xuzhy 
* 说明：
*       
*       
*      
* 修改：
* 注释：
*********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"
#include "zxbee.h"
#include "GPS.h"
#include "contiki.h"
#include "stepcounting.h"

#include "zxbee.h"
#include "zxbee-inf.h"
#include "mpu9250.h"

PROCESS(sensor, "sensor");

/*********************************************************************************************
* 全局变量
*********************************************************************************************/
unsigned int V0 = 30;
unsigned int A0 = 0;            //gps状态 0：不在线，1在线
static char A1[32];             //记录经纬度
unsigned int A2 = 0;            //记录总步数 

static char A3[32];				//加速度传感器x，y，z数据，格式：x&y&z
static char A4[32];				//陀螺仪传感器x，y，z数据，格式：x&y&z
static char A5[32];				//地磁仪传感器x，y，z数据，格式：x&y&z

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
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA0(void)
{
  char lat[16], lng[16];
  A0 = gps_get(lat, lng);       //获取GPS信息
}
/*********************************************************************************************
* 名称：updateA1()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA1(void)
{
  char lat[16], lng[16];
  unsigned char gps_status = 0;
  gps_status = gps_get(lat, lng);        //获取GPS信息
  if(gps_status == 1){
    sprintf(A1, "%s&%s", lat,lng);
  } else{
    sprintf(A1, "%s&%s", "0","0");
  }
}
/*********************************************************************************************
* 名称：updateA2()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA2(void)
{
	A2 = stepGet();
}
/*********************************************************************************************
* 名称：updateA2()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA3(void)
{
  float x, y, z;
  mpu9250_accel(&x, &y, &z);
  sprintf(A3, "%.1f&%.1f&%.1f", x, y, z);
}
/*********************************************************************************************
* 名称：updateA4()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA4(void)
{
   int16_t x, y, z;
  mpu9250_gyro(&x, &y, &z);
  sprintf(A4, "%d&%d&%d", x, y, z);
}
/*********************************************************************************************
* 名称：updateA5()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void updateA5(void)
{
   int16_t x, y, z;
  mpu9250_mga(&x, &y, &z);
  sprintf(A5, "%d&%d&%d", x, y, z);
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
  char buf[64];

  updateA0();
  updateA1();
  updateA2();
  
  updateA3();
  updateA4();
  updateA5();
  
  ZXBeeBegin();
  
  sprintf(buf, "%u", A0);
  
  ZXBeeAdd("A0", buf);
  ZXBeeAdd("A1", A1);
  sprintf(buf, "%u", A2);
  ZXBeeAdd("A2", buf);
  
  ZXBeeAdd("A3", A3);
  ZXBeeAdd("A4", A4);
  ZXBeeAdd("A5", A5);
  
  char *p = ZXBeeEnd();
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
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
}
/*********************************************************************************************
* 名称：ZXBeeUserProcess()
* 功能：解析收到的控制命令
* 参数：*ptag -- 控制命令名称
*       *pval -- 控制命令参数
* 返回：ret -- pout字符串长度
* 修改：
* 注释：
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{
  char buf[64];
  
  if (strcmp(ptag, "A0") == 0) {
    if (pval[0] == '?') {
      sprintf(buf, "%u", A0);
      ZXBeeAdd("A0", buf);
    }
  }
  if (strcmp(ptag, "A1") == 0) {
    if (pval[0] == '?') {
      ZXBeeAdd("A1", A1);
    }
  }
  if (strcmp(ptag, "A2") == 0) {
    if (pval[0] == '?') {
      sprintf(buf, "%u", A2);
      ZXBeeAdd("A2", buf);
    }
  }
  if (strcmp(ptag, "A3") == 0) {
    if (pval[0] == '?') {
      updateA3();
      ZXBeeAdd("A3", A3);
    }
  }
  if (strcmp(ptag, "A4") == 0) {
    if (pval[0] == '?') {
      updateA4();
      ZXBeeAdd("A4", A4);
    }
  }
  if (strcmp(ptag, "A5") == 0) {
    if (pval[0] == '?') {
      updateA5();
      ZXBeeAdd("A5", A5);
    }
  }
  
  if (strcmp(ptag, "V0") == 0) {
    if (pval[0] == '?') {
      sprintf(buf, "%u", V0);
      ZXBeeAdd("V0", buf);
    } else {
      updateV0(pval);
    }
  }
  return 0;
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
      etimer_set(&et_check, CLOCK_SECOND/30);
    }
    if (etimer_expired(&et_update)) {
      sensorUpdate();
      etimer_set(&et_update, CLOCK_SECOND*V0); 
    } 
  }
  PROCESS_END();
}
