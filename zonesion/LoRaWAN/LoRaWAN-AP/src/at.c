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
  sprintf(buff, "+RECV:%d,%d\r\n", len, sx1278Rssi());
  at_response(buff);
  at_response_buf(buf, len);
}
void at_notify_data2(char *buf, int len, int rssi, int ch, int sf)
{
  char buff[64];
  sprintf(buff, "+RECV:%d,%d,%d,%d\r\n", len, rssi, ch, sf);
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
 at_response("+HW:LoRaWANAP\r\n");
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
        int r = LoraSendPackage(p_msg, at_recvdata);
        if (r < 0) {
          at_response("+DATASEND:Error,Radio busy!\r\n");
        } else {
          sprintf(buf, "+DATASEND:%d\r\n", r);
          at_response(buf);
          at_response(ATOK);
        }
        at_recvdata = 0;
        
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
          at_response("+HW:LoRaWANAP\r\n");
          at_response(ATOK);
        }
      
        else if (memcmp(p_msg, "AT+RXCFG", 8) == 0) {
          void sz1278LoraWANRxConfig(int ch, int sf);
          void sx1278rxContinus(void);
          extern unsigned int recv_channel, recv_sf;
          if (p_msg[8] == '=') {
            int ch = atoi(&p_msg[9]);
            char *p = strchr(&p_msg[9], ',');
            if (p != NULL) {
              int sf = atoi(&p[1]);
              sz1278LoraWANRxConfig(ch, sf);
              sx1278rxContinus();
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else if (p_msg[8] == '?') {
            char buf[32];
            sprintf(buf, "+RXCFG:%u,%u\r\n", recv_channel, recv_sf);
            at_response(buf);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+SEND=", 8)==0){
          extern unsigned int send_sf, send_channel, send_power;
          
          char *p = &p_msg[8];
          char *np = strchr(p, ',');
          *np++ = '\0';
          int sz = atoi(p);
          p = np;
          np = strchr(p, ',');
          *np++ = '\0';
          char *np2 = strchr(np, ',');
          *np2++ = '\0';
          
          send_channel = atoi(p);
          send_sf = atoi(np);
          send_power = atoi(np2);
      
          if (sz > 0 && sz<256) {
            at_response(">");
            at_datalen = sz;
          } else {
            at_response(ATERROR);
          }
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
