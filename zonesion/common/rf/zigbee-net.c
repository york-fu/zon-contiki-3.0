#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"

#if (DEV_TYPE==3)
#include "wifi_cfg.h"
#endif

void uart2_putc(unsigned char x);
void uart2_init(unsigned int bpr);
void uart2_set_input(int(*fun)(unsigned char));
extern void _zxbee_onrecv_fun(char *pkg, int len);
PROCESS(zigbee_uart_process, "zigbee uart driver");
#if (DEV_TYPE==3)
PROCESS(wifi_config, "wifi_config");
#endif
static process_event_t uart_command_event;

char z_mac[24];
static unsigned int z_panid;
static unsigned int z_channel;
static void (*_onrecv_call)(char *, int) = NULL;
static void _zigbee_send(char *buf, int len);
static int z_type = -1;
#if (DEV_TYPE==3)
char wifi_status = 0;
#endif
char* zigbee_mac()
{
  return z_mac;
}
unsigned int zigbee_panid()
{
  return z_panid;
}
unsigned int zigbee_channel()
{
  return z_channel;
}
int zigbee_type()
{
  return z_type;
}

void zigbee_send(char *str)
{
#if (DEV_TYPE==1)
  char buf[32];
  int r;
  int len = strlen(str);
  r = sprintf(buf, "at+send=%u\n", len);
  _zigbee_send(buf, r);
  _zigbee_send(str, len);
#else
  int len = strlen(str);
  _zigbee_send(str, len);
#endif

}
void zigbee_set_onrecv(void (*f)(char *, int))
{
  _onrecv_call = f;
}
void _zigbee_send(char *buf, int len)
{
  printf(">>>%s\r\n",buf);
  for (int i=0; i<len; i++) {
    uart2_putc(*buf++);
  }

}


static int uart_rx_offset = 0;
static int _zigbee_input_byte(unsigned char c)
{
#define recv_timeout 10

#define buf_size 96
  static char uart_buf[2][buf_size];
#define buf1 uart_buf[0]
#define buf2 uart_buf[1]
  static char* pbuf = buf1;

  static unsigned int ltm = 0;
  unsigned int tm = clock_time()*(1000/CLOCK_SECOND);
  int dt = (int)tm  - (int)(ltm+recv_timeout);

  static int data = 0;
  ltm = tm;
  if (dt > 0) {
    uart_rx_offset = 0;
    data = 0;
  }
  if (data == 0) {
    if (c == '\n') {
      if (uart_rx_offset > 0) {
        pbuf[uart_rx_offset] = 0;
//#if (DEV_TYPE==1)
        if (uart_rx_offset>6 && memcmp(pbuf, "+recv=", 6) == 0) {
          data = atoi(&pbuf[6]);
          uart_rx_offset++;
        } else {
          process_post(&zigbee_uart_process, uart_command_event, pbuf);
          if (pbuf == buf1) pbuf = buf2;
          else pbuf = buf1;
          uart_rx_offset = 0;
        }
//#else
//        process_post(&zigbee_uart_process, uart_command_event, pbuf);
//#endif
      }
    } else {
      if (uart_rx_offset>=0 && uart_rx_offset<buf_size) {
        pbuf[uart_rx_offset++] = c;
      } else uart_rx_offset = -1;
    }
  } else if (data > 0) {
    if (uart_rx_offset>=0 && uart_rx_offset<buf_size) {
      pbuf[uart_rx_offset++] = c;
    } else uart_rx_offset = -1;
    data --;
    if (data == 0) {
      process_post(&zigbee_uart_process, uart_command_event, pbuf);
      if (pbuf == buf1) pbuf = buf2;
      else pbuf = buf1;
      uart_rx_offset = 0;
    }
  }

  return 0;
}


PROCESS_THREAD(zigbee_uart_process, ev, data)
{
#if (DEV_TYPE==3)
  char len = 0;
  char wifi_cfg_buf[64];
  PROCESS_BEGIN();
  uart2_init(38400);
  uart2_set_input(_zigbee_input_byte);
  uart_command_event = process_alloc_event();
  while (1) {
    PROCESS_WAIT_EVENT();                                       //等待事件发生
    if (ev == uart_command_event) {
      char* pdat = (char *)data;
      printf("<<<%s\r\n", pdat);
      _zxbee_onrecv_fun(&pdat[18],strlen(pdat)-19);
      if (memcmp(pdat, "ok", 2) == 0) {
        wifi_status |= 0x01;
      }
      if (memcmp(pdat, "+ssid=", 6) == 0) {
        if(memcmp(&pdat[7],Z_SSID_NAME,strlen(Z_SSID_NAME)) == 0)
          wifi_status |= 0x02;
        else{
          len = sprintf(wifi_cfg_buf,"at+ssid=\"%s\"\r\n",Z_SSID_NAME);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+stype=", 7) == 0) {
        int a = atoi(&pdat[7]);
        if(a == Z_SECURITY_TYPE)
          wifi_status |= 0x04;
        else{
          len = sprintf(wifi_cfg_buf,"at+stype=%u\r\n",Z_SECURITY_TYPE);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+skey=", 6) == 0) {
        if(memcmp(&pdat[7],Z_SECURITY_KEY,strlen(Z_SECURITY_KEY)) == 0)
          wifi_status |= 0x08;
        else{
          len = sprintf(wifi_cfg_buf,"at+skey=\"%s\"\r\n",Z_SECURITY_KEY);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+sip=", 5) == 0) {
        if(memcmp(&pdat[5],IP_ADDR,strlen(IP_ADDR)) == 0)
          wifi_status |= 0x10;
        else{
          len = sprintf(wifi_cfg_buf,"at+sip=%s\r\n",IP_ADDR);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+sport=", 7) == 0) {
        int a = atoi(&pdat[7]);
        if(a == GW_PORT)
          wifi_status |= 0x20;
        else{
          len = sprintf(wifi_cfg_buf,"at+sport=%u\r\n",GW_PORT);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+lport=", 7) == 0) {
        int a = atoi(&pdat[7]);
        if(a == LO_PORT)
          wifi_status |= 0x40;
        else{
          len = sprintf(wifi_cfg_buf,"at+lport=%u\r\n",LO_PORT);
          _zigbee_send(wifi_cfg_buf,len);
          _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
        }
      }
      if (memcmp(pdat, "+mac=", 5) == 0) {
        memcpy(z_mac, &pdat[5], 17);
        z_mac[17] = '\0';
        wifi_status |= 0x80;
      }
    }
  }
  PROCESS_END();
#else
  static struct etimer _timer;
  static int init_stetp = 0;
  static char* cmd[] = {"at\n", "at+mac=?\n", "at+panid=?\n", "at+type=?\n",  "at+channel=?\n"};

  PROCESS_BEGIN();

  uart2_init(115200);
  uart2_set_input(_zigbee_input_byte);
  uart_command_event = process_alloc_event();

  etimer_set(&_timer, 500*CLOCK_SECOND/1000);
  while (1) {
    PROCESS_YIELD();
    if ((init_stetp < sizeof cmd /sizeof cmd[0]) && etimer_expired(&_timer)) {
      _zigbee_send(cmd[init_stetp], strlen(cmd[init_stetp]));
      etimer_set(&_timer, 200*CLOCK_SECOND/1000);
      init_stetp ++;
    }
    if (ev == uart_command_event) {
      printf("<<< %s\r\n", (char*)data);
      char* pdat = (char *)data;
      if (memcmp(pdat, "ok", 2) == 0) {

      }
#if  (DEV_TYPE==1)
      if (memcmp(pdat, "+mac=", 5) == 0) {
        memcpy(z_mac, &pdat[5], 23);
        z_mac[23] = 0;
      }
#elif (DEV_TYPE==4)
      if (memcmp(pdat, "{MAC=BLE:", 9) == 0) {
        memcpy(z_mac, &pdat[9], 17);
        z_mac[17] = 0;
      }
      _zxbee_onrecv_fun(pdat,strlen(pdat));
#endif
      if (memcmp(pdat, "+panid=", 7) == 0) {
        z_panid = atoi(&pdat[7]);
      }
      if (memcmp(pdat, "+channel=", 9) == 0) {
        z_channel = atoi(&pdat[9]);
      }
      if (memcmp(pdat, "+type=", 6) == 0) {
        z_type = pdat[6] - '0';
      }
      if (memcmp(pdat, "+recv=", 6) == 0) {
        int dlen = atoi(&pdat[6]);
        pdat = &pdat[strlen(pdat)+1];
        if (dlen > 0 && _onrecv_call != NULL) {
          _onrecv_call(pdat, dlen);
        }
      }
    }
  }

  PROCESS_END();
#endif
}
#if (DEV_TYPE==3)
PROCESS_THREAD(wifi_config, ev, data)
{
  static struct etimer wifi_timer;
  PROCESS_BEGIN();
  etimer_set(&wifi_timer, 2000*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  _zigbee_send("+++",strlen("+++"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();

  if(!(wifi_status &= 0x01)){
    while(1){
      _zigbee_send("+++",strlen("+++"));
      etimer_set(&wifi_timer, 1000*CLOCK_SECOND/1000);
      PROCESS_YIELD();

      if(wifi_status &= 0x01)break;
    }
  }
  while(0x02 != (wifi_status & 0x02))
  {
  _zigbee_send("at+ssid=?\r\n",strlen("at+ssid=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  while(0x08 != (wifi_status & 0x08))
  {
  _zigbee_send("at+skey=?\r\n",strlen("at+skey=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  while(0x04 != (wifi_status & 0x04))
  {
  _zigbee_send("at+stype=?\r\n",strlen("at+stype=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  while(0x10 != (wifi_status & 0x10))
  {
  _zigbee_send("at+sip=?\r\n",strlen("at+sip=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  while(0x20 != (wifi_status & 0x20))
  {
  _zigbee_send("at+sport=?\r\n",strlen("at+sport=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  while(0x40 != (wifi_status & 0x40))
  {
  _zigbee_send("at+lport=?\r\n",strlen("at+lport=?\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();
  }
  _zigbee_send("at+save\r\n",strlen("at+save\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();

  while(0x80 != (wifi_status & 0x80))
  {
    printf("wifi_status:%x\r\n",wifi_status);
    _zigbee_send("at+mac=?\r\n",strlen("at+mac=?\r\n"));
    etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
    PROCESS_YIELD();
  }

  _zigbee_send("at+exit\r\n",strlen("at+exit\r\n"));
  etimer_set(&wifi_timer, 500*CLOCK_SECOND/1000);
  PROCESS_YIELD();

  PROCESS_END();
}
#endif