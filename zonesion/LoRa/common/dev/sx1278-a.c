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
#include "sx1278-a.h"


#define  DEBUG  0

#if DEBUG
#define debug(x...)  printf(x)
#else
#define debug(x...) 
#endif

PROCESS(sx1278, "sx1278");
process_event_t evt_sx1278;

sx1278_config_t  sx1278Config = {
  .pv = LoRa_PV,                                                // 发射功率
  .ps = LoRa_PS,                                                // 前导码长度
  .fp = LoRa_FP,                                                // 基频
  .sf = LoRa_SF,                                                // 扩频因子：6~12
  .cr = LoRa_CR,                                                // 编码率：1~4
  .bw = LoRa_BW,                                                // 带宽：0~9
  .hop = LoRa_HOP,                                              // 跳频开关：0，1
  .hop_tab = LoRa_HOPTAB,                                       // 跳频表
#ifdef WITH_xLab
  .id = LoRa_NET_ID,                                            // 应用组ID：0x01~0xFE
#endif
};

static char recv_buf[256];
static int recv_length = 0;
static char send_buf[256];
static volatile  int send_length = 0;

void LoraOnMessage(char *buf, int len);

extern void clock_delay_ms(unsigned int ms);
extern void LoraNetInit(void);

/*********************************************************************************************
* 名称:PROCESS_THREAD()
* 功能:hello world 进程定义
* 参数:无
* 返回:无
* 修改:
* 注释:
*********************************************************************************************/


void sx1278Callback(SX1278_EVT_T evt)
{
  if (evt == CADTimeout){
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else
  if (evt == CADDone){// begin rx
    leds_on(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else
  if (evt == RxTimeout) {
    leds_off(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else 
  if (evt == RxDone) {
    leds_off(2);
    int rlen = sx1278RecvLength();
    if (rlen > 0) {
      char *buf = sx1278RecvBuf();
      memcpy(recv_buf, buf, rlen);
      recv_length = rlen;
    } else {
      recv_length = 0;
    }
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  } else 
  if (evt == TxDone) {
    leds_off(2);
    process_post(&sx1278, evt_sx1278, (process_data_t)evt);
  }
}


int LoraSendPackage(char *buf, int len)
{
  if (send_length != 0) {
    debug("Error: radio busy, send fail!\r\n");
    return -1;
  }
  send_length = len;
  memcpy(send_buf, buf, len);
  return len;
}
static char update_flag = 0;

void LoraSetFP(int fp)
{
  sx1278Config.fp = fp;
  update_flag |= 1;
}
void LoraSetPS(int ps)
{
  sx1278Config.ps = ps;
  update_flag |= 2;
}
void LoraSetPV(int pv)
{
  sx1278Config.pv = pv;
  update_flag |= 4;
}
void LoraSetBW(int bw)
{
  sx1278Config.bw = bw;
  update_flag |= 8;
}
void LoraSetSF(int sf)
{
  sx1278Config.sf = sf;
  update_flag |= 0x10;
}
void LoraSetCR(int cr)
{
  sx1278Config.cr = cr;
  update_flag |= 0x20;
}
void LoraSetHOP(int hop)
{
  sx1278Config.hop = hop;
  update_flag |= 0x40;
}
void LoraSetHOPTAB(int *tab)
{
  memcpy((char *)sx1278Config.hop_tab, (char *)tab, HOP_TAB_SIZE*sizeof(int));
  update_flag |= 0x80;
}
#ifdef WITH_xLab              
void LoraSetID(int id)
{
  sx1278Config.id = id;
}
int LoraGetID(void)
{
  return sx1278Config.id;
}
#endif

PROCESS_THREAD(sx1278, ev, data)
{
  static struct etimer et;
  
  PROCESS_BEGIN();

  evt_sx1278 = process_alloc_event();
  
  sx1278_IO_Init();
  
  sx1278_reset();
  
  clock_delay_ms(100);
  
  sx1278Init();

  config_init();
 
  sx1278SetFP(sx1278Config.fp);
  sx1278SetPS(sx1278Config.ps);
  sx1278SetPV(sx1278Config.pv);
  sx1278SetBW(sx1278Config.bw);
  sx1278SetSF(sx1278Config.sf);
  sx1278SetCR(sx1278Config.cr);
  sx1278HopSet(sx1278Config.hop);
  sx1278HopTabSet(sx1278Config.hop_tab);
  
  sx1278SetEvtCall(sx1278Callback);
  
  SX1276CheckCAD();
  
  etimer_set(&et, CLOCK_SECOND/10);
  while (1) {
     PROCESS_WAIT_EVENT();
     if (ev == evt_sx1278) {
        SX1278_EVT_T evt = (SX1278_EVT_T) data;
       if (evt == CADDone) {
          //remove timer
          etimer_stop(&et);
          continue;
        }else
        if (evt == CADTimeout) {       
          if (send_length > 0) {
            //remove timer
            etimer_stop(&et);  
            leds_on(2);
            sx1278SendPacket((unsigned char*)send_buf, send_length);
            debug("Lora <<< [%u]", send_length);
            for (int i=0;  i<send_length; i++) {
              debug(" %02X", send_buf[i]);
            }
            debug("\r\n");
            send_length = 0;
            continue;
          }
        } else
        if (evt == RxTimeout) {
          debug("LoRa: rx timeout!\r\n");
        } else
        if (evt == RxDone) {
          if (recv_length > 0) {
           
            debug("Lora >>> [%u]", recv_length);
            for (int i=0; i<recv_length; i++) {
              debug(" %02X", recv_buf[i]);
            }
            debug("\r\n");
            //int rssi = sx1278Rssi();
            LoraOnMessage(recv_buf, recv_length);
          }
        } else
        if (evt == TxDone) {
          leds_off(2);
        }  
        
        if (update_flag != 0) {
          if (update_flag & 0x01) {
            sx1278SetFP(sx1278Config.fp);
            update_flag &= ~0x01;
          }
          if (update_flag & 0x02) {
            sx1278SetPS(sx1278Config.ps);
            update_flag &= ~0x02;
          }
          if (update_flag & 0x04) {
            sx1278SetPV(sx1278Config.pv);
            update_flag &= ~0x04;
          }
          if (update_flag & 0x08) {
            sx1278SetBW(sx1278Config.bw);
            update_flag &= ~0x08;
          }
          if (update_flag & 0x10) {
            sx1278SetSF(sx1278Config.sf);
            update_flag &= ~0x10;
          }
          if (update_flag & 0x20) {
            sx1278SetCR(sx1278Config.cr);
            update_flag &= ~0x20;
          }
          if (update_flag & 0x40) {
            sx1278HopSet(sx1278Config.hop);
            update_flag &= ~0x40;
          }
          if (update_flag & 0x80) {
            sx1278HopTabSet(sx1278Config.hop_tab);
            update_flag &= ~0x80;
          }
        }
        SX1276CheckCAD();
        etimer_set(&et, CLOCK_SECOND/10);
     }
     if (ev == PROCESS_EVENT_TIMER) {
       if (et.p != NULL) {
          SX1276CheckCAD();
          etimer_set(&et, CLOCK_SECOND/10);
       }
     }
  }
    
  PROCESS_END();
}