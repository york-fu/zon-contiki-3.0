/*********************************************************************************************
* 文件：zxbee-inf.c
* 作者：xuzhy 
* 说明：
*       
*       
*      
* 修改：
* 注释：
*********************************************************************************************/
#include <string.h>
#include <stdint.h>
#include "zxbee.h"
#include "zxbee-inf.h"

void at_notify_data(char *buf, int len);
int loraWanRegisterOnPortCall(int port, void (*fun)(uint8_t*, int));
void loraWanSendData(int AppPort, uint8_t *AppData, int AppDataSize);

static void onAppData(uint8_t *dat, int len)
{
  ZXBeeInfRecv((char*)dat, len);
}
/*********************************************************************************************
* 名称：ZXBeeInfInit
* 功能：ZXBee接口底层初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeInfInit(void)
{
  loraWanRegisterOnPortCall(2, onAppData);      //zhiyun数据端口
  loraWanRegisterOnPortCall(200, onAppData);    //锐米云据端口
}
/*********************************************************************************************
* 名称：ZXBeeInfSend
* 功能：ZXBee底层发送接口
* 参数：p: ZXBee格式数据
       len：数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void  ZXBeeInfSend(char *p, int len)
{
  loraWanSendData(2, (uint8_t*)p, len); //智云和锐米服务接收用的是同一端口
}
/*********************************************************************************************
* 名称：ZXBeeInfRecv
* 功能：接收到ZXbee数据报处理函数
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeInfRecv(char *buf, int len)
{
  at_notify_data(buf, len);
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}