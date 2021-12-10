/*********************************************************************************************
* 文件：at.c
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
#include <ctype.h>

#include "contiki.h"
#include "usart.h"
#include "lib/ringbuf.h"
#include "sx1278-a.h"
#include "config.h"

#include "at-uart.h"
#include "at.h"

PROCESS(at, "at");
static process_event_t  event_at;

extern int LoraSendPackage(char *buf, int len);

#define AT_BUFF_SIZE 256
#define AT_BUFF_NUM	 4
static char at_echo = 0;
static  int at_datalen = 0; //指示用于接收发送的数据长度
static  int at_recvdata = 0;
static char atbuff[AT_BUFF_NUM][AT_BUFF_SIZE];
static char bufferbit = 0;

static int hex2bin(uint8_t *in, uint8_t *out, int len)
{
  int i, j=0;
  uint8_t h, l;
  
  if (len % 2 != 0) return -1;
  
  for (i=0; i<len; i+=2) {
    if (in[i] >= '0' && in[i]<='9') h = in[i] - '0';
    else if (in[i]>='a' && in[i] <= 'f') h = 10 + (in[i] - 'a');
    else if (in[i]>='A' && in[i] <= 'F') h = 10 + (in[i] - 'A');
    else return j;
    
    if (in[i+1] >= '0' && in[i+1]<='9') l = in[i+1] - '0';
    else if (in[i+1]>='a' && in[i+1] <= 'f') l = 10 + (in[i+1] - 'a');
    else if (in[i+1]>='A' && in[i+1] <= 'F') l = 10 + (in[i+1] - 'A');
    else return j;
    
    out[j++] = h<<4 | l;
  }
  return j;
}
/*********************************************************************************************
* 名称：at_quebuffer_get
* 功能：at缓存申请
* 参数：
* 返回：成功返回缓存地址，失败返回NULL
* 修改：
* 注释：
*********************************************************************************************/
char* at_quebuffer_get()
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (((bufferbit>>i) & 1) == 0) {
			bufferbit |= 1<<i;
			return atbuff[i];
		}
	}
	return NULL;
}
/*********************************************************************************************
* 名称：at_quebuffer_put
* 功能：at缓存释放
* 参数：buf：待释放缓存地址
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_quebuffer_put(char *buf)
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (atbuff[i] == buf){
			bufferbit &= ~(1<<i);
			return;
		}
	}
}

/*********************************************************************************************
* 名称：_at_get_ch
* 功能：at串口接收到1个字节处理函数
* 参数：ch：接收到的字节
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
static int _at_get_ch(char ch)
{
  static char idx = 0;
  static char *pbuf = NULL;
  if (pbuf == NULL) {
    pbuf = at_quebuffer_get();
    if (pbuf == NULL) { 
      //no buffer for command
      //at_uart_write("enb");
      return 0;
    }
  }
  if (at_datalen == 0) {
    if (at_echo) {
      at_uart_write(ch);
    }
    if (idx < AT_BUFF_SIZE-1) {
      pbuf[idx++] = ch;
      if (idx >= 2 && pbuf[idx-2]=='\r' && pbuf[idx-1]=='\n'){
        idx -= 2;
        pbuf[idx] = '\0';
        if (idx > 0) {
          /* 获取到at命令 */
          process_post(&at, event_at, (process_data_t)pbuf);
          pbuf = NULL;
          idx = 0;
        }
      }
    } else {
      /*丢弃*/
      idx = 0;
    }
  } else {
    pbuf[at_recvdata++] = ch;
    if (at_recvdata == at_datalen) {
      process_post(&at, event_at, (process_data_t)pbuf);
      //at_recvdata = 0;
      at_datalen = 0;
      pbuf = NULL;
    }
  }
  return 1;
}
/*********************************************************************************************
* 名称：at_response_buf
* 功能：at接口发送一段数据
* 参数：s：待发送数据地址
*      len：待发送数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response_buf(char *s, int len)
{
  for (int i=0; i<len; i++) {
    at_uart_write(s[i]);
  }
}
/*********************************************************************************************
* 名称：at_response
* 功能：at接口发送一段字符串
* 参数：s：待发送字符串
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response(char *s)
{
  at_response_buf(s, strlen(s));
}


/*********************************************************************************************
* 名称：at_notify_data
* 功能：从智云服务器接收到数据并输出到at串口，在zhiyun.c中被调用
* 参数：buf：接收到的zxbee数据
*       len： 数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_notify_data(char *buf, int len)
{
  char buff[32];
  sprintf(buff, "+RECV:%d\r\n", len);
  at_response(buff);
  at_response_buf(buf, len);
}
/*********************************************************************************************
* 名称：ATCommandInit
* 功能：at接口初始
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ATCommandInit(void)
{

 at_uart_init();
 at_uart_set_input_call(_at_get_ch);
 at_response("+HW:LoRaWAN\r\n");
 at_response("+RDY\r\n");

}
/*********************************************************************************************
* 名称：at
* 功能：at命令处理线程
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
PROCESS_THREAD(at, ev, data)
{

  char buf[64];
  
  PROCESS_BEGIN();

  event_at = process_alloc_event();
  
  
  ATCommandInit();
  
  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == event_at) {
      char *p_msg = (char *)data;
      if (at_recvdata != 0) { //got data
        void  ZXBeeInfSend(char *p, int len);
        ZXBeeInfSend(p_msg, at_recvdata);

        at_response(ATOK);
        at_recvdata = 0;
        
        int r = 0;
        if (r < 0) {
          at_response("+DATASEND:Error,Radio busy!\r\n");
        } else {
          sprintf(buf, "+DATASEND:%d\r\n", r);
          at_response(buf);
        }
      } else {
        int msg_size = strlen(p_msg);
        for (int i=0; i<msg_size; i++) {
          if (p_msg[i] == '?' || p_msg[i] == '=') break;
          p_msg[i] = toupper(p_msg[i]);
        }
        if (strcmp(p_msg, "AT") == 0) {
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE1") == 0) {
          at_echo = 1;
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE0") == 0) {
          at_echo = 0;
          at_response(ATOK);
        } 
        else if (strcmp(p_msg, "AT+HW?") == 0) {
          at_response("+HW:LoRaWAN\r\n");
          at_response(ATOK);
        }

        else if (memcmp(p_msg, "AT+SEND=", 8)==0){
          int sz = atoi(&p_msg[8]);
          if (sz > 0 && sz<256) {
            at_response(">");
            at_datalen = sz;
          } else {
            at_response(ATERROR);
          }
        }
        /*   lorawan env set*/
        else if (memcmp(p_msg, "AT+DEVEUI?", 10)==0){
          uint8_t *getDeviceEUI(void);
          uint8_t buf[32];
          uint8_t *eui = getDeviceEUI();
          sprintf((char*)buf, "+DEVEUI:%02X%02X%02X%02X%02X%02X%02X%02X\r\n", eui[0],eui[1],eui[2],
                  eui[3],eui[4],eui[5],eui[6],eui[7]);
          at_response((char*)buf);
          at_response(ATOK);
        }
        else if (memcmp(p_msg, "AT+JOINEUI", 10)==0){
          uint8_t buf[32];
          if (p_msg[10] == '='){
            uint8_t *p = (uint8_t*)&p_msg[11]; 
            if (strlen((char*)p) == 16 && hex2bin(p, buf, 16)==8) {
              memcpy(lorawan_cfg.JoinEUI, buf, 8);
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[10] == '?'){
            uint8_t *p = lorawan_cfg.JoinEUI;
            sprintf((char*)buf, "+JOINEUI:%02X%02X%02X%02X%02X%02X%02X%02X\r\n", 
                    p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+ZKEY", 7)==0){
          uint8_t buf[32];
          if (p_msg[7] == '=') {
            int a = atoi(&p_msg[8])!=0?1:0;
            lorawan_cfg.aKey = a;
            at_response(ATOK);
          } else if (p_msg[7] == '?') {
            sprintf((char*)buf, "+ZKEY:%d\r\n", lorawan_cfg.aKey);
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+KEY", 6)==0){
          uint8_t buf[64];
          if (p_msg[6] == '=') {
            uint8_t *p = (uint8_t*)&p_msg[7];
            if (strlen((char*)p) == 32 && hex2bin(p, buf, 32)==16) {
              memcpy(lorawan_cfg.Key, buf, 16);
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[6] == '?') {
            sprintf((char*)buf, "+KEY:");
            for (int i=0; i<16; i++) {
              sprintf((char*)buf+strlen((char*)buf), "%02X", lorawan_cfg.Key[i]);
            }
            sprintf((char*)buf+strlen((char*)buf), "\r\n");
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+CLASS", 8)==0){
          uint8_t buf[32];
          if (p_msg[8] == '=') {
            int a = atoi(&p_msg[9]);
            if (a == 0 || a == 2) {
              lorawan_cfg.ClassType = a;
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[8] == '?') {
            sprintf((char*)buf, "+CLASS:%d\r\n", lorawan_cfg.ClassType);
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+CHM", 6)==0){
          uint8_t buf[64];
          if (p_msg[6] == '=') {
            uint8_t *p = (uint8_t *)&p_msg[7];
            if (strlen((char*)p) == 24 && hex2bin(p, buf, 24)==12) {
              memcpy(lorawan_cfg.ChannelMask, buf, 12);
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[6] == '?') {
            sprintf((char*)buf, "+CHM:");
            for (int i=0; i<6; i++) {
              sprintf((char*)buf+strlen((char*)buf), "%02X%02X", lorawan_cfg.ChannelMask[i]&0xff, 
                      (lorawan_cfg.ChannelMask[i]>>8)&0xff);
            }
            sprintf((char*)buf+strlen((char*)buf), "\r\n");
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }   
        }
        else if (memcmp(p_msg, "AT+DDR", 6)==0){
          uint8_t buf[32];
          if (p_msg[6] == '=') {
            int a = atoi(&p_msg[7]);
            if ( a >= 0 && a <= 5 ) {
              lorawan_cfg.DefaultDataRate = a;
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[6] == '?') {
            sprintf((char*)buf, "+DDR:%d\r\n", lorawan_cfg.DefaultDataRate);
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }          
        }
        else if (memcmp(p_msg, "AT+ADR", 6)==0){
          uint8_t buf[32];
          if (p_msg[6] == '=') {
            int a = atoi(&p_msg[7]);
            if ( a >= 0 && a <= 1 ) {
              lorawan_cfg.ADREnable = a;
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[6] == '?') {
            sprintf((char*)buf, "+ADR:%d\r\n", lorawan_cfg.ADREnable);
            at_response((char*)buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }    
        }
        else if (memcmp(p_msg, "AT+ENVSAVE", 10)==0){
          config_save();
          at_response(ATOK);
        }

        else if (memcmp(p_msg, "AT+RESET", 8) == 0) {
          at_response(ATOK); 
          __set_FAULTMASK(1);
          NVIC_SystemReset();
        } 
        else {
           at_response(ATERROR);
        }
      }
      at_quebuffer_put(p_msg);
    }
  }
  PROCESS_END();
}
