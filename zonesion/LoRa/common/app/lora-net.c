/*********************************************************************************************
* 文件：lora-net.c
* 作者：xuzhy
* 说明：
* 修改：
* 注释：
*
*********************************************************************************************/
/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "stm32f10x.h"
#include "contiki-conf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sx1278-a.h"
#include "at.h"

//#define LoRa_NET_ID     0x32    
#define UUID            ((unsigned int *)0x1FFFF7E8)

#define APP_PROTOCOL_VER        0x81

static unsigned short int NAddr = 0;

extern int LoraSendPackage(char *buf, int len);
extern void onLoraNetMessage(unsigned short saddr, char *buf, int len);
extern void at_response_buf(char *s, int len);
extern void at_response(char *s);

static void (*_on_data_fun)(char *pkg, int len) = NULL;
/*********************************************************************************************
* 名称：LoraNetInit
* 功能：lora网络初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void LoraNetInit(void)
{
    /* Enable CRC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
  NAddr = CRC_CalcBlockCRC(UUID, 3);
#if defined(WITH_xLab_AP)
  NAddr = 0;
#endif
  srand(NAddr);
}
/*********************************************************************************************
* 名称：LoraNetId
* 功能：获取lora网络编号
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
unsigned short LoraNetId(void)
{
  return NAddr;
}
/*********************************************************************************************
* 名称：LoraNetSend
* 功能：lora接口发送函数
* 参数：da：目的地址
*       buf：待发送数据
*       len：发送数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int LoraNetSend(unsigned short da, char *buf, int len)
{
  int r;
  static char sbuf[256];
  if (len >(255-6)){
    return -1;
  }
  sbuf[0] = LoraGetID();
  sbuf[1] = da >> 8;
  sbuf[2] = da & 0xff;
  sbuf[3] = NAddr >> 8;
  sbuf[4] = NAddr & 0xff;
  sbuf[5] = APP_PROTOCOL_VER;
  memcpy(sbuf+6, buf, len);
  r = LoraSendPackage(sbuf, len+6); 
  if (r < 0) return -2;
  return r;
}
/*********************************************************************************************
* 名称：LoraOnMessage
* 功能：lora接口接收到数据处理函数
* 参数：buf：接收到的数据
*       len：接收到的数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void LoraOnMessage(char *buf, int len)
{
#if defined(WITH_xLab_AP)
  at_notify_data(buf, len);
#else
  unsigned short addr;
  
  if (((unsigned char)buf[0]) != LoraGetID()) {
    return;
  }
  addr = buf[1]<<8 | buf[2];
  if (addr != NAddr && addr != 0xffff ){
    return;
  }
  addr = buf[3]<<8 | buf[4];
  if (buf[5] == APP_PROTOCOL_VER) {
#if defined(LORA_Serial)
    at_notify_data(buf+6, len-6);//去掉数据头
#else
    at_notify_data(buf, len);
#endif //#if defined(LORA_Serial)
    
    if (_on_data_fun != NULL) {
      _on_data_fun(buf+6, len-6);
    }
  }
#endif //#if defined(WITH_xLab_AP)
}
/*********************************************************************************************
* 名称：LoraNetSetOnRecv
* 功能：设置接收到数据处理回掉函数
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void LoraNetSetOnRecv(void (*_fun)(char *pkg, int len))
{
  _on_data_fun = _fun;
}