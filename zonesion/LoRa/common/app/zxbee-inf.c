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
#include "zxbee.h"
#include "zxbee-inf.h"

extern char* _zxbee_onrecv_fun(char *pkg, int len);
extern int LoraSendPackage(char *buf, int len);
extern void LoraNetInit(void);
extern int LoraNetSend(unsigned short da, char *buf, int len);
extern void LoraNetSetOnRecv(void (*_fun)(char *pkg, int len));

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
  LoraNetInit();
  LoraNetSetOnRecv(ZXBeeInfRecv);
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
  LoraNetSend(0, p, len);
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
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}