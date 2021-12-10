/*********************************************************************************************
* 文件：sensor.c
* 作者：Xuzhy 2018.5.16
* 说明：xLab Sensor-EH传感器程序
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
#include "contiki.h"
#include "zxbee.h"
#include "zxbee-inf.h"
#include "relay.h"
#include "key.h"
#include "sensor.h"
#include "rfid900m.h"
#include "motor.h"


PROCESS(sensor, "sensor");

/*********************************************************************************************
* 宏定义
*********************************************************************************************/

/*********************************************************************************************
* 全局变量
*********************************************************************************************/
static unsigned char  D1 = 0;                                   // 继电器初始状态为全关
static char A0[32];                                              // A0存储卡号
static int A2 = 0;                                             //卡余额
static int V1;
static int V2;

static int motor = 0;
/*********************************************************************************************
* 函数原型
*********************************************************************************************/
void clock_delay_ms(unsigned int ms);
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
  motorInit();
  RFID900MInit();
}
/*********************************************************************************************
* 名称：sensorLinkOn()
* 功能：传感器节点入网成功调用函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
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
  static char last_epc[12];
  static int new_tag = 0;                       //检测到卡片并读取余额成功标记
  static char epc[12];                                    //记录当前检测到卡片epc
  static char write = 0;
  static char status = 1;                              //状态转换标识
  
  if (status == 1) {
    if (RFID900MCheckCardRsp(epc) > 0) {
      if (memcmp(last_epc, epc, 12) != 0)  {
        RFID900MReadReq(NULL, epc, BLK_USER, 0, 2); //发送读卡请求
        status = 2;       
      } else {
        
        new_tag = 8;
        if (V1 != 0) {
          long money = A2 + V1;
          RFID900MWriteReq(NULL, epc, BLK_USER, 0, 2, (char*)&money);
          write = 1;
          status = 3;
        } else if (V2 != 0) {
          long money = A2 - V2;
          RFID900MWriteReq(NULL, epc, BLK_USER, 0, 2, (char*)&money);
          write = 2;
          status = 3;
        } else {
          RFID900MCheckCardReq();
          status = 1;
        }
      }
    } else {
      //没有检测到卡片
      if (new_tag > 0) {
        new_tag -= 1;
        if (new_tag == 0) {
          memset(last_epc, 0, 12);
        }
      }
      RFID900MCheckCardReq();
      status = 1;
    }
  } else if (status == 2) {
    int money;
    if (RFID900MReadRsp((char*)&money) > 0) {  
      //读取到金额上报
      for (int j=0; j<12; j++) sprintf(&A0[j*2], "%02X", epc[j]);
      char buf[16];
      if (money < 0) money = 0;
      ZXBeeBegin();
      ZXBeeAdd("A0", A0);
      A2 = money;
      sprintf(buf, "%ld.%ld", A2/100, A2%100);
      ZXBeeAdd("A2", buf);
      char *p = ZXBeeEnd();
      if (p != NULL) {
        ZXBeeInfSend(p, strlen(p));
      }
      memcpy(last_epc, epc, 12);  //保存当前卡片id
      new_tag = 8;
      V1 = 0;  //初始化扣金额
      V2 = 0;
    }

    RFID900MCheckCardReq();
    status = 1;
    
  } else if (status == 3) {
    if (RFID900MWriteRsp() > 0) {
      char buf[16];
      ZXBeeBegin();
      if (write == 1) {
        A2 = A2 + V1;
        V1 = 0;
        ZXBeeAdd("V1", "1"); 
      } else if (write == 2) {
        A2 = A2 - V2;
        V2 = 0;
        ZXBeeAdd("V2", "1");
      }
      sprintf(buf, "%d.%d", A2/100, A2%100);
      ZXBeeAdd("A2", buf);
      char *p = ZXBeeEnd();
      if (p != NULL) {
        ZXBeeInfSend(p, strlen(p));
      }
    } 
    RFID900MCheckCardReq();
    status = 1;
  }
  
    /*增加马达状态检测*/
  static int tick = 0;
  if (tick == motor){
    motorSet(0);                //关闭马达
  }else if (tick > motor) {
    motorSet(2);                //开启闸机
    tick -= 1;
  } else if (tick < motor) {
    motorSet(1);                //关闭闸机
    tick += 1;
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
   if (cmd & 0x01) {
    motor = 3; //闸机开启时间300ms
  } else {
    motor = 0;
  }
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
  char buf[32];
  // 将字符串变量pval解析转换为整型变量赋值
  val = atoi(pval);	
  // 控制命令解析
  if (0 == strcmp("A2", ptag)) {
    if (pval[0] == '?'){
      sprintf(buf, "%d.%d", A2/100,A2%100);
      ZXBeeAdd("A2", buf);
    }
  }
  if (0 == strcmp("V1", ptag)) {
    float v = atof(pval);
     V1 = (long) (v *100);
  }
  if (0 == strcmp("V2", ptag)) {
     float v = atof(pval);
     V2 = (long) (v *100);
  }
  if (0 == strcmp("CD1", ptag)){                                // 对D1的位进行操作，CD1表示位清零操作
    D1 &= ~val;
    sensorControl(D1);
  }
  if (0 == strcmp("OD1", ptag)){                                // 对D1的位进行操作，OD1表示位置一操作
    D1 |= val;
    sensorControl(D1);
  }
  if (0 == strcmp("D1", ptag)){                                 
    if (0 == strcmp("?", pval)){                                
      sprintf(buf, "%u", D1);                         
      ZXBeeAdd("D1", buf);
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
  
  etimer_set(&et_update, CLOCK_SECOND*30); 
  etimer_set(&et_check, CLOCK_SECOND/10); //100 hz
  
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
    if (etimer_expired(&et_check)) {
      sensorCheck();
      etimer_set(&et_check, CLOCK_SECOND/10);
    }
    if (etimer_expired(&et_update)) {
      sensorUpdate();
      etimer_set(&et_update, CLOCK_SECOND*30); 
    } 
  }
  PROCESS_END();
}