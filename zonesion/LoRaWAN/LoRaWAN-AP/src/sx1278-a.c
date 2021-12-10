/*********************************************************************************************
* 文件：sx1278-a.c
* 作者：xuzhy
* 说明：
* 修改：
* 注释：
*
*********************************************************************************************/
/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include <contiki.h>
#include <dev/leds.h>
#include <stdio.h>
#include <string.h>
#include "sx1278-io.h"
#include "sx1278.h"
#include "sx1278-a.h"
#include "config.h"


#define  DEBUG  0

#if DEBUG
#define debug(x...)  printf(x)
#else
#define debug(x...) 
#endif

PROCESS(sx1278, "sx1278");
static process_event_t evt_sx1278;


static char recv_buf[256];
static int recv_length = 0;
static char send_buf[256];
static volatile  int send_length = 0;

void LoraOnMessage(char *buf, int len);

extern void clock_delay_ms(unsigned int ms);

static unsigned int rxT, txT;
/*********************************************************************************************
* 名称:PROCESS_THREAD()
* 功能:hello world 进程定义
* 参数:无
* 返回:无
* 修改:
* 注释:
*********************************************************************************************/
//  static unsigned t1, t2, t3;

void sx1278Callback(SX1278_EVT_T evt)
{
  if (evt == CADTimeout){
    //t2 = clock_time();
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else
  if (evt == CADDone){// begin rx
    //t2 = clock_time();
    leds_on(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else
  if (evt == RxTimeout) {
    leds_off(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else 
  if (evt == RxDone) {
    rxT = clock_time();
    leds_off(2);
    int rlen = sx1278RecvLength();
    if (recv_length == 0 && rlen > 0) {
      char *buf = sx1278RecvBuf();
      memcpy(recv_buf, buf, rlen);
      recv_length = rlen;
      process_post(&sx1278, evt_sx1278, (process_data_t)evt);
      
    } else {
      recv_length = 0;
    }
    
  } else 
  if (evt == TxDone) {
    leds_off(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  }
}

/*接收信道配置*/
unsigned int recv_channel = 0, recv_sf = 7;
unsigned int send_sf, send_channel, send_power;
int sx1278Rssi2();
static int sz1278_status_tx = 0;

  void sx1278LoraRxConfig(unsigned int fp, int pw, int sf, int cr, int bw);
  void sx1278LoraTxConfig(unsigned int fp, int pw, int sf, int cr, int bw);
  void sx1278rxContinus(void);
  int sx1278RxInProgress(void);
void sx1278Irq(void);
void EXTI9_5_CallSet_3(void (*func)(void));

int LoraSendPackage(char *buf, int len)
{
  /*if (send_length != 0) {
    debug("Error: radio busy, send fail!\r\n");
    return -1;
  }
  send_length = len;
  memcpy(send_buf, buf, len);
 */
  if (sz1278_status_tx != 0 || sx1278RxInProgress()) { 
    return -1;
  }
  leds_on(2);
  sz1278_status_tx = 1;
   txT = clock_time();
   sx1278LoraTxConfig(500300000+(send_channel%48)*200000, 14, send_sf, 1, 7);
   sx1278SendPacket((unsigned char*)buf, len);
   debug("Lora <<< [%u]", send_length);
   for (int i=0;  i<send_length; i++) {
    debug(" %02X", send_buf[i]);
   }
   debug("\r\n");
   printf("rxT:%u  txT:%u   dT:%u\r\n", rxT, txT, txT-rxT);         
  return len;
}


void sz1278LoraWANRxConfig(int ch, int sf)
{
  if (ch >= 0 && ch <= 95) {
    recv_sf = sf;
    recv_channel = ch;
    sx1278LoraRxConfig(470300000+ch*200000, 14/*pw*/, recv_sf, 1/*4/5*/, 7/*125k*/);
  }
}

PROCESS_THREAD(sx1278, ev, data)
{

  static struct etimer et;
  
  PROCESS_BEGIN();

  evt_sx1278 = process_alloc_event();
  
  sx1278_IO_Init();
  
  sx1278_reset();
  
  clock_delay_ms(100);
  
  sx1278Init();
  EXTI9_5_CallSet_3(sx1278Irq); 

  //SX1276LoRaSetRFPower(20);
  
  sx1278SetEvtCall(sx1278Callback);


  sx1278LoraRxConfig(470300000+recv_channel*200000, 14/*pw*/, recv_sf, 1/*4/5*/, 7/*125k*/);
  leds_on(1);
  static uint8_t cad = 0;
  if (cad != 0) {
    SX1276CheckCAD();
    etimer_set(&et, CLOCK_SECOND/5);
  } else {
    sx1278rxContinus();
  }
  while (1) {
     PROCESS_WAIT_EVENT();
     if (ev == evt_sx1278) {
       SX1278_EVT_T evt = (SX1278_EVT_T) data;
       if (evt == CADDone) {
          etimer_stop(&et);
          continue;
       }else
       if (evt == CADTimeout) {
          etimer_stop(&et); 
          if (send_length > 0) {
            leds_on(2);
            
            txT = clock_time();
            sx1278LoraTxConfig(500300000+(send_channel%48)*200000, send_power, send_sf, 1, 7);
            
            sx1278SendPacket((unsigned char*)send_buf, send_length);
            debug("Lora <<< [%u]", send_length);
            for (int i=0;  i<send_length; i++) {
              debug(" %02X", send_buf[i]);
            }
            debug("\r\n");
            printf("rxT:%u  txT:%u   dT:%u\r\n", rxT, txT, txT-rxT);
            send_length = 0;
            continue;
          }
        } else 
       if (evt == RxTimeout) {
          debug("LoRa: rx timeout!\r\n");
          if (cad == 0) {
            sx1278LoraRxConfig(470300000+recv_channel*200000, 14, recv_sf, 1, 7);
            sx1278rxContinus();
          }
       } else 
       if (evt == RxDone) {
          if (recv_length > 0) {
            leds_on(2);
            debug("Lora >>> [%u]", recv_length);
            for (int i=0; i<recv_length; i++) {
              debug(" %02X", recv_buf[i]);
            }
            debug("\r\n");
            void at_notify_data2(char *buf, int len, int rssi, int ch, int sf);
            at_notify_data2(recv_buf, recv_length, sx1278Rssi(), recv_channel, recv_sf);
            recv_length = 0;
            leds_off(2);
          }

        } else if (evt == TxDone) {
          sz1278_status_tx = 0;
          if (cad == 0) {
            sx1278LoraRxConfig(470300000+recv_channel*200000, 14, recv_sf, 1, 7);
            sx1278rxContinus();
          }
          leds_off(2);
        }  
        
      if (cad != 0) {
        SX1276CheckCAD();
        etimer_set(&et, CLOCK_SECOND/5);  
      }
     }
     if (ev == PROCESS_EVENT_TIMER) {
       if (et.p != NULL) {
         if (cad != 0) {
           SX1276CheckCAD();
           etimer_set(&et, CLOCK_SECOND/5);
         }
       }
     }
  }
    
  PROCESS_END();
}