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
#include "zxbee-inf.h"

#include "at-uart.h"
#include "at.h"

PROCESS(at, "at");
static process_event_t  event_at;

extern int LoraSendPackage(char *buf, int len);
extern int LoraNetSend(unsigned short da, char *buf, int len);

#define AT_BUFF_SIZE 256
#define AT_BUFF_NUM	 4
static char at_echo = 1;
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
 at_response("+HW:LoRa\r\n");
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
  sx1278_config_t *pcfg = &sx1278Config;
  char buf[64];
  
  PROCESS_BEGIN();

  event_at = process_alloc_event();
  
  ATCommandInit();
  
#if defined(LORA_Serial)
    ZXBeeInfInit();
#endif
  
  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == event_at) {
      char *p_msg = (char *)data;
      if (at_recvdata != 0) { //got data
#if defined( LORA_Serial )
        int r = LoraNetSend(0, p_msg, at_recvdata);//发送地址+数据
#else
        int r = LoraSendPackage(p_msg, at_recvdata);//发送数据
#endif
        at_response(ATOK);
        at_recvdata = 0;
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
          at_response("+HW:LoRa\r\n");
          at_response(ATOK);
        }
        else if (memcmp(p_msg, "AT+FP", 5)==0) {
          if (p_msg[5] == '?'){
            sprintf(buf, "+FP:%u\r\n", pcfg->fp);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int fp = atoi(&p_msg[6]);
            if (fp>=410 && fp<=525) {
              if ( fp != pcfg->fp){
                LoraSetFP(fp);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+PV", 5)==0){
          if (p_msg[5] == '?'){
            sprintf(buf, "+PV:%u\r\n", pcfg->pv);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int pv = atoi(&p_msg[6]);
            if (pv>=0 && pv<=20) {
              if (pv != pcfg->pv) {
                LoraSetPV(pv);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
         
        } 
        else if (memcmp(p_msg, "AT+SF", 5)==0){
          if (p_msg[5] == '?'){
            sprintf(buf, "+SF:%u\r\n", pcfg->sf);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int sf = atoi(&p_msg[6]);
            if (sf>5 && sf<13) {
              if (pcfg->sf!=sf) {
               LoraSetSF(sf);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
        } 
        else if (memcmp(p_msg, "AT+CR", 5)==0){
          if (p_msg[5] == '?'){
            sprintf(buf, "+CR:%u\r\n", pcfg->cr);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int cr = atoi(&p_msg[6]);
            if (cr>0 && cr<5) {
              if (cr != pcfg->cr) {
                LoraSetCR(cr);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
        } 
        else if (memcmp(p_msg, "AT+BW", 5)==0){
          if (p_msg[5] == '?'){
            sprintf(buf, "+BW:%u\r\n", pcfg->bw);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int bw = atoi(&p_msg[6]);
            if (bw>=0 && bw<10) {
              if (bw != pcfg->bw) {
                LoraSetBW(bw);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
        } 
        else if (memcmp(p_msg, "AT+PS", 5)==0){
          if (p_msg[5] == '?'){
            sprintf(buf, "+PS:%u\r\n", pcfg->ps);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[5] == '='){
            int ps = atoi(&p_msg[6]);
            if (ps>3 && ps<513) {
              if (ps!=pcfg->ps) {
                LoraSetPS(ps);
              }
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }							
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+HOPTAB", 9) == 0) {
     
          if (p_msg[9] == '?'){
            int htab[HOP_TAB_SIZE];
            memcpy(htab, pcfg->hop_tab, HOP_TAB_SIZE*sizeof(int));
            int n = sprintf(buf, "+HOPTAB:%u",htab[0]);
            for (int i=1; i<HOP_TAB_SIZE; i++) {
              n += sprintf(&buf[n], ",%u", htab[i]);
            }
            sprintf(&buf[n], "\r\n");
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[9] == '='){
            char *p = &p_msg[10];
            int htab[HOP_TAB_SIZE];
            int i;
            for (i=0; i<HOP_TAB_SIZE; i++) {
              if (*p != '\0') {
                char *pn = strchr(p, ',');
                if (pn != NULL) {
                  *pn = '\0';
                  htab[i] = atoi(p);
                  p = pn+1;
                }else {
                  htab[i] = atoi(p);
                }
              }else {
                //at_response(ATERROR);
                break;
              }
            }
            if (i != HOP_TAB_SIZE) {
              at_response(ATERROR); 
            } else {
              LoraSetHOPTAB(htab);
              at_response(ATOK);
            }								
          } else {
            at_response(ATERROR);
          }	
        }				
        else if (memcmp(p_msg, "AT+HOP", 6) == 0) {
          if (p_msg[6] == '?'){
            sprintf(buf, "+HOP:%u\r\n", pcfg->hop);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[6] == '='){
            int h = atoi(&p_msg[7]);
            LoraSetHOP(h);
            at_response(ATOK);				
          } else {
            at_response(ATERROR);
          }	
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
        else if (memcmp(p_msg, "AT+ENVSAVE", 10)==0){
          config_save();
          at_response(ATOK);
        }
#ifdef WITH_xLab 
        else if (memcmp(p_msg, "AT+NETID", 8)==0){
          if (p_msg[8] == '?'){
            sprintf(buf, "+NETID:%u\r\n", pcfg->id);
            at_response(buf);
            at_response(ATOK);
          } else if (p_msg[8] == '=') {
            int nid = atoi(&p_msg[9]);
            if (nid>0 && nid<255) {
              pcfg->id = nid;
              at_response(ATOK);
            } else {
              at_response(ATERROR);
            }
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+NODEID?", 10)==0){
          unsigned short LoraNetId(void);
          sprintf(buf, "+NODEID:%u\r\n", LoraNetId());
          at_response(buf);
          at_response(ATOK);
        }
#endif
        else if (memcmp(p_msg, "AT+RESET", 8) == 0) {
          at_response(ATOK); 
          __set_FAULTMASK(1);
          NVIC_SystemReset();
        } 
        else {
#if !defined(WITH_xLab_AP)
          int8_t user_at_proc(char *msg);
          if (user_at_proc(p_msg) < 0) {
            at_response(ATERROR);
          }
#else
           at_response(ATERROR);
#endif
        }
      }
      at_quebuffer_put(p_msg);
    }
  }
  PROCESS_END();
}
